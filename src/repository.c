#include <stdio.h>
#include <string.h>
#include "repository.h"

/* Печать одной строки результата запроса */
static int cb_print(void* arg, int ncols, char** vals, char** names) {
    (void)arg; (void)names;
    for (int i = 0; i < ncols; i++) {
        printf("%s | ", vals[i] ? vals[i] : "NULL");
    }
    printf("\n");
    return 0;
}

static void print_error(sqlite3* db, const char* what) {
    fprintf(stderr, "%s: %s\n", what, sqlite3_errmsg(db));
}

/* ---------- Клиенты ---------- */

int repo_customer_add(sqlite3* db, const Customer* c) {
    const char* sql =
        "INSERT INTO customers (last_name, first_name, phone) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        print_error(db, "customer_add");
        return -1;
    }
    sqlite3_bind_text(stmt, 1, c->last_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, c->first_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, c->phone, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "customer_add");
        return -1;
    }
    return 0;
}

int repo_customer_list(sqlite3* db) {
    char* err = NULL;
    printf("id | фамилия | имя | телефон\n");
    int rc = sqlite3_exec(db,
        "SELECT customer_id, last_name, first_name, phone FROM customers;",
        cb_print, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "customer_list: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    return 0;
}

/* ---------- Товары ---------- */

int repo_product_add(sqlite3* db, const Product* p) {
    const char* sql =
        "INSERT INTO products (name, price) VALUES (?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        print_error(db, "product_add");
        return -1;
    }
    sqlite3_bind_text(stmt, 1, p->name, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, p->price);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "product_add");
        return -1;
    }
    return 0;
}

int repo_product_list(sqlite3* db) {
    char* err = NULL;
    printf("id | название | цена\n");
    int rc = sqlite3_exec(db,
        "SELECT product_id, name, price FROM products;",
        cb_print, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "product_list: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    return 0;
}

/* ---------- Заказы ---------- */

int repo_order_create(sqlite3* db, int customer_id, int* out_order_id) {
    const char* sql =
        "INSERT INTO orders (customer_id, order_date) "
        "VALUES (?, datetime('now', 'localtime'));";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        print_error(db, "order_create");
        return -1;
    }
    sqlite3_bind_int(stmt, 1, customer_id);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        print_error(db, "order_create");
        return -1;
    }
    if (out_order_id) {
        *out_order_id = (int)sqlite3_last_insert_rowid(db);
    }
    sqlite3_finalize(stmt);
    return 0;
}

int repo_order_add_item(sqlite3* db, int order_id, int product_id, int quantity) {
    const char* sql =
        "INSERT INTO order_product (order_id, product_id, quantity) "
        "VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        print_error(db, "order_add_item");
        return -1;
    }
    sqlite3_bind_int(stmt, 1, order_id);
    sqlite3_bind_int(stmt, 2, product_id);
    sqlite3_bind_int(stmt, 3, quantity);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "order_add_item");
        return -1;
    }
    return 0;
}

int repo_order_list(sqlite3* db) {
    char* err = NULL;
    printf("id | дата | клиент\n");
    int rc = sqlite3_exec(db,
        "SELECT o.order_id, o.order_date, c.last_name "
        "FROM orders o JOIN customers c ON c.customer_id = o.customer_id;",
        cb_print, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "order_list: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    return 0;
}

int repo_order_show(sqlite3* db, int order_id) {
    /* --- шапка заказа: соединяем orders и customers --- */
    char sql[512];
    sqlite3_stmt* stmt = NULL;

    snprintf(sql, sizeof(sql),
        "SELECT o.order_date, c.last_name, c.first_name "
        "FROM orders o JOIN customers c ON c.customer_id = o.customer_id "
        "WHERE o.order_id = %d;", order_id);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "order_show: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        printf("Заказ с таким ID не найден.\n");
        return 1;
    }

    const unsigned char* date = sqlite3_column_text(stmt, 0);
    const unsigned char* ln   = sqlite3_column_text(stmt, 1);
    const unsigned char* fn   = sqlite3_column_text(stmt, 2);

    printf("\n========== ЗАКАЗ № %d ==========\n", order_id);
    printf("Дата:   %s\n", date ? (const char*)date : "-");
    printf("Клиент: %s %s\n", ln ? (const char*)ln : "-", fn ? (const char*)fn : "-");
    printf("--------------------------------\n");
    printf("%-24s %6s %10s %10s\n", "Товар", "Кол-во", "Цена", "Сумма");
    printf("--------------------------------\n");
    sqlite3_finalize(stmt);

    /* --- позиции заказа: название, количество, цена и сумма строки --- */
    snprintf(sql, sizeof(sql),
        "SELECT p.name, op.quantity, p.price, op.quantity * p.price "
        "FROM order_product op JOIN products p ON p.product_id = op.product_id "
        "WHERE op.order_id = %d;", order_id);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "order_show: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* name  = sqlite3_column_text(stmt, 0);
        int qty      = sqlite3_column_int(stmt, 1);
        double price = sqlite3_column_double(stmt, 2);
        double sum   = sqlite3_column_double(stmt, 3);
        printf("%-24.20s %6d %10.2f %10.2f\n",
               name ? (const char*)name : "-", qty, price, sum);
        count++;
    }
    sqlite3_finalize(stmt);

    /* --- итоговая сумма считается прямо в SQL --- */
    snprintf(sql, sizeof(sql),
        "SELECT COALESCE(SUM(op.quantity * p.price), 0) "
        "FROM order_product op JOIN products p ON p.product_id = op.product_id "
        "WHERE op.order_id = %d;", order_id);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "order_show: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    double total = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);

    printf("--------------------------------\n");
    if (count == 0) {
        printf("Позиции отсутствуют.\n");
    }
    printf("ИТОГО: %29.2f\n", total);
    return 0;
}
int repo_customer_update(sqlite3* db, int customer_id, const Customer* c) {
    const char* sql =
        "UPDATE customers SET last_name = ?, first_name = ?, phone = ? "
        "WHERE customer_id = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        print_error(db, "customer_update");
        return -1;
    }
    sqlite3_bind_text(stmt, 1, c->last_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, c->first_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, c->phone, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, customer_id);
    int rc = sqlite3_step(stmt);
    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "customer_update");
        return -1;
    }
    return (changed == 0) ? 1 : 0;
}

int repo_customer_delete(sqlite3* db, int customer_id) {
    const char* sql = "DELETE FROM customers WHERE customer_id = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        print_error(db, "customer_delete");
        return -1;
    }
    sqlite3_bind_int(stmt, 1, customer_id);
    int rc = sqlite3_step(stmt);
    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "customer_delete");
        return -1;
    }
    return (changed == 0) ? 1 : 0;
}

int repo_product_update(sqlite3* db, int product_id, const Product* p) {
    const char* sql =
        "UPDATE products SET name = ?, price = ? WHERE product_id = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        print_error(db, "product_update");
        return -1;
    }
    sqlite3_bind_text(stmt, 1, p->name, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, p->price);
    sqlite3_bind_int(stmt, 3, product_id);
    int rc = sqlite3_step(stmt);
    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "product_update");
        return -1;
    }
    return (changed == 0) ? 1 : 0;
}

int repo_product_delete(sqlite3* db, int product_id) {
    const char* sql = "DELETE FROM products WHERE product_id = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        print_error(db, "product_delete");
        return -1;
    }
    sqlite3_bind_int(stmt, 1, product_id);
    int rc = sqlite3_step(stmt);
    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "product_delete");
        return -1;
    }
    return (changed == 0) ? 1 : 0;
}

int repo_order_delete(sqlite3* db, int order_id) {
    char sql[256];
    snprintf(sql, sizeof(sql),
        "DELETE FROM order_product WHERE order_id = %d; "
        "DELETE FROM orders WHERE order_id = %d;", order_id, order_id);
    char* err = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "order_delete: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    return 0;
}
int repo_customer_exists(sqlite3* db, int customer_id) {
    const char* sql = "SELECT COUNT(*) FROM customers WHERE customer_id = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, customer_id);
    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        found = (sqlite3_column_int(stmt, 0) > 0);
    sqlite3_finalize(stmt);
    return found;
}

int repo_product_exists(sqlite3* db, int product_id) {
    const char* sql = "SELECT COUNT(*) FROM products WHERE product_id = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, product_id);
    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        found = (sqlite3_column_int(stmt, 0) > 0);
    sqlite3_finalize(stmt);
    return found;
}

int repo_customer_find_by_phone(sqlite3* db, const char* phone) {
    const char* sql =
        "SELECT COUNT(*) FROM customers WHERE phone = ? COLLATE NOCASE;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, phone, -1, SQLITE_STATIC);
    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        found = (sqlite3_column_int(stmt, 0) > 0);
    sqlite3_finalize(stmt);
    return found;
}

int repo_product_find_by_name(sqlite3* db, const char* name) {
    const char* sql =
        "SELECT COUNT(*) FROM products WHERE name = ? COLLATE NOCASE;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        found = (sqlite3_column_int(stmt, 0) > 0);
    sqlite3_finalize(stmt);
    return found;
}
/* ---------- Пользователи ---------- */

int repo_user_add(sqlite3* db, const User* u) {
    const char* sql =
        "INSERT INTO users (login, password, role) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, u->login, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, u->password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, u->role, -1, SQLITE_STATIC);
    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return rc;
}

int repo_user_find_by_login(sqlite3* db, const char* login, User* out) {
    const char* sql =
        "SELECT login, password, role FROM users WHERE login = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, login, -1, SQLITE_STATIC);
    int rc = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* pwd  = sqlite3_column_text(stmt, 1);
        const unsigned char* role = sqlite3_column_text(stmt, 2);
        snprintf(out->login,    sizeof(out->login),    "%s", login);
        snprintf(out->password, sizeof(out->password), "%s", pwd  ? (const char*)pwd  : "");
        snprintf(out->role,     sizeof(out->role),     "%s", role ? (const char*)role : "");
        rc = 0; /* нашли */
    } else {
        rc = 1; /* логина нет */
    }
    sqlite3_finalize(stmt);
    return rc;
}
int repo_user_list(sqlite3* db) {
    char* err = NULL;
    printf("логин | роль\n");
    int rc = sqlite3_exec(db,
        "SELECT login, role FROM users ORDER BY login;",
        cb_print, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "user_list: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    return 0;
}

int repo_user_count_admins(sqlite3* db, int* out_count) {
    const char* sql = "SELECT COUNT(*) FROM users WHERE role = 'admin';";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    if (out_count) *out_count = count;
    return 0;
}

int repo_user_delete(sqlite3* db, const char* login) {
    const char* sql = "DELETE FROM users WHERE login = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        print_error(db, "user_delete");
        return -1;
    }
    sqlite3_bind_text(stmt, 1, login, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "user_delete");
        return -1;
    }
    return (changed == 0) ? 1 : 0; /* 1 — логина нет, 0 — удалён */
}
/* ID учётки по логину; 0 — не найден, -1 — ошибка */
int repo_user_get_id(sqlite3* db, const char* login, int* out_id) {
    const char* sql = "SELECT user_id FROM users WHERE login = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, login, -1, SQLITE_STATIC);
    int id = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    if (out_id) *out_id = id;
    return (id == 0) ? 1 : 0;
}

/* ID клиента, привязанного к учётке; 0 — профиля нет, -1 — ошибка */
int repo_customer_find_by_user_id(sqlite3* db, int user_id) {
    const char* sql = "SELECT customer_id FROM customers WHERE user_id = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, user_id);
    int id = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return id;
}

/* Клиент, привязанный к учётке (создаётся при первом заказе покупателя) */
int repo_customer_add_linked(sqlite3* db, const Customer* c, int user_id) {
    const char* sql =
        "INSERT INTO customers (user_id, last_name, first_name, phone) "
        "VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        print_error(db, "customer_add_linked");
        return -1;
    }
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, c->last_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, c->first_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, c->phone, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        print_error(db, "customer_add_linked");
        return -1;
    }
    return 0;
}