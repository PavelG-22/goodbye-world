#include <stdio.h>
#include <string.h>
#include <direct.h>
#include "sqlite3.h"
#include "export.h"

static const char *DB_FILE = "shop.db";

typedef struct {
    char name[256];
    int  qty;
    double price;
} Item;

static void quoted_str(FILE *f, const char *s)
{
    fputc('"', f);
    for (; *s; s++) {
        if (*s == '"' || *s == '\\') { fputc('\\', f); fputc(*s, f); }
        else if (*s == '\n') fputs("\\n", f);
        else if (*s == '\r') { }
        else fputc(*s, f);
    }
    fputc('"', f);
}

static void xml_str(FILE *f, const char *s)
{
    for (; *s; s++) {
        if (*s == '<') fputs("&lt;", f);
        else if (*s == '>') fputs("&gt;", f);
        else if (*s == '&') fputs("&amp;", f);
        else if (*s == '"') fputs("&quot;", f);
        else fputc(*s, f);
    }
}

static void csv_field(FILE *f, const char *s)
{
    int need = s && strpbrk(s, ";\r\n\"");
    if (need) fputc('"', f);
    for (; s && *s; s++) {
        if (*s == '"') fputs("\"\"", f);
        else fputc(*s, f);
    }
    if (need) fputc('"', f);
}


static int load_items(sqlite3 *db, int order_id, Item *items, int max, int *count)
{
    char sql[512];
    sqlite3_stmt *st;
    int n = 0;

     sprintf_s(sql, sizeof(sql),
        "SELECT IFNULL(p.name, ''), op.quantity, p.price "
        "FROM order_product op LEFT JOIN products p ON op.product_id = p.product_id "
        "WHERE op.order_id = %d", order_id);

    *count = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK) return -1;
    while (sqlite3_step(st) == SQLITE_ROW && n < max) {
        strncpy_s(items[n].name, sizeof(items[n].name),
                  (const char *)sqlite3_column_text(st, 0), _TRUNCATE);
        items[n].qty   = sqlite3_column_int(st, 1);
        items[n].price = sqlite3_column_double(st, 2);
        n++;
    }
    sqlite3_finalize(st);
    *count = n;
    return 0;
}


int export_data(void)
{
    sqlite3 *db;
    sqlite3_stmt *st;
    FILE *fj, *fc, *fx, *fy;
    int first_order = 1;

    if (_mkdir("out") != 0) { }

    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        printf("Ошибка: не удалось открыть %s\n", DB_FILE);
        return 1;
    }

    fj = fc = fx = fy = NULL;
    if (fopen_s(&fj, "out/data.json", "w")  || !(fj)) goto fail;
    if (fopen_s(&fc, "out/data.csv",  "wb") || !(fc)) goto fail;
    if (fopen_s(&fx, "out/data.xml",  "w")  || !(fx)) goto fail;
    if (fopen_s(&fy, "out/data.yaml", "w")  || !(fy)) goto fail;

    fprintf(fc, "\xEF\xBB\xBF");

    fprintf(fj, "{\n  \"orders\": [\n");
    fprintf(fc, "order_id;date;client_name;email;item_name;qty;price\r\n");
    fprintf(fx, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<orders>\n");

     const char *q =
        "SELECT o.order_id, o.order_date, "
        "IFNULL(c.last_name, '') || ' ' || IFNULL(c.first_name, ''), "
        "IFNULL(c.email, '') "
        "FROM orders o LEFT JOIN customers c ON o.customer_id = c.customer_id";

    if (sqlite3_prepare_v2(db, q, -1, &st, NULL) != SQLITE_OK) {
        printf("Ошибка запроса: %s\n", sqlite3_errmsg(db));
        goto fail;
    }

    while (sqlite3_step(st) == SQLITE_ROW) {
        int id = sqlite3_column_int(st, 0);
        const char *date   = (const char *)sqlite3_column_text(st, 1);
        const char *cname  = (const char *)sqlite3_column_text(st, 2);
        const char *cemail = (const char *)sqlite3_column_text(st, 3);
        Item items[64];
        int n = 0, i;
        load_items(db, id, items, 64, &n);

        if (!first_order) fprintf(fj, ",\n");
        first_order = 0;
        fprintf(fj, "    {\"id\": %d, \"date\": ", id);
        quoted_str(fj, date);
        fprintf(fj, ", \"client\": {\"name\": ");
        quoted_str(fj, cname);
        fprintf(fj, ", \"email\": ");
        quoted_str(fj, cemail);
        fprintf(fj, "}, \"items\": [\n");
        for (i = 0; i < n; i++) {
            fprintf(fj, "        {\"name\": ");
            quoted_str(fj, items[i].name);
            fprintf(fj, ", \"qty\": %d, \"price\": %.2f}%s\n",
                    items[i].qty, items[i].price, (i + 1 < n) ? "," : "");
        }
        fprintf(fj, "      ]\n    }");

        if (n == 0) {
            fprintf(fc, "%d;", id);
            csv_field(fc, date);   fputc(';', fc);
            csv_field(fc, cname);  fputc(';', fc);
            csv_field(fc, cemail); fputc(';', fc);
            fputs(";;;\r\n", fc);
        }
        for (i = 0; i < n; i++) {
            fprintf(fc, "%d;", id);
            csv_field(fc, date);          fputc(';', fc);
            csv_field(fc, cname);         fputc(';', fc);
            csv_field(fc, cemail);        fputc(';', fc);
            csv_field(fc, items[i].name); fputc(';', fc);
            fprintf(fc, "%d;%.2f\r\n", items[i].qty, items[i].price);
        }

        fprintf(fx, "  <order id=\"%d\">\n    <date>", id);
        xml_str(fx, date);
        fprintf(fx, "</date>\n    <client>\n      <name>");
        xml_str(fx, cname);
        fprintf(fx, "</name>\n      <email>");
        xml_str(fx, cemail);
        fprintf(fx, "</email>\n    </client>\n");
        if (n == 0) fputs("    <items />\n", fx);
        else {
            fputs("    <items>\n", fx);
            for (i = 0; i < n; i++) {
                fprintf(fx, "      <item>\n        <name>");
                xml_str(fx, items[i].name);
                fprintf(fx, "</name>\n        <qty>%d</qty>\n        <price>%.2f</price>\n      </item>\n",
                        items[i].qty, items[i].price);
            }
            fputs("    </items>\n", fx);
        }
        fprintf(fx, "  </order>\n");

        fprintf(fy, "- id: %d\n  date: ", id);
        quoted_str(fy, date);
        fprintf(fy, "\n  client:\n    name: ");
        quoted_str(fy, cname);
        fprintf(fy, "\n    email: ");
        quoted_str(fy, cemail);
        fprintf(fy, "\n  items:\n");
        if (n == 0) fputs("    - {}\n", fy);
        for (i = 0; i < n; i++) {
            fprintf(fy, "    - name: ");
            quoted_str(fy, items[i].name);
            fprintf(fy, "\n      qty: %d\n      price: %.2f\n",
                    items[i].qty, items[i].price);
        }
    }
    sqlite3_finalize(st);

    fprintf(fj, "\n  ]\n}\n");
    fprintf(fx, "</orders>\n");

    fclose(fj); fclose(fc); fclose(fx); fclose(fy);
    sqlite3_close(db);
    printf("Экспорт завершён: out/data.json, out/data.csv, out/data.xml, out/data.yaml\n");
    return 0;

fail:
    printf("Ошибка: не удалось создать файл в out/\n");
    if (fj) fclose(fj);
    if (fc) fclose(fc);
    if (fx) fclose(fx);
    if (fy) fclose(fy);
    sqlite3_close(db);
    return 1;
}
