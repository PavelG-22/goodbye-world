#include "database.h"

#include <stdio.h>
#include <stdlib.h>

/* Имя файла базы данных и скрипта схемы.
   Файлы ищутся в текущей рабочей директории запуска программы. */
static const char *DB_FILE     = "shop.db";
static const char *SCHEMA_FILE = "schema.sql";

sqlite3 *get_connection(void) {
    sqlite3 *db = NULL;

    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        fprintf(stderr, "Не удалось открыть БД: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    /* ВАЖНО: в SQLite проверка внешних ключей выключена по умолчанию.
       Без этой строки FOREIGN KEY в схеме игнорировались бы. */
    if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL) != SQLITE_OK) {
        fprintf(stderr, "Не удалось включить внешние ключи: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    return db;
}

void init_db(void) {
    /* --- чтение файла скрипта в память --- */
    FILE *f = fopen(SCHEMA_FILE, "rb");
    if (!f) {
        fprintf(stderr, "Не найден файл %s\n", SCHEMA_FILE);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size < 0) {
        fprintf(stderr, "Не удалось определить размер %s\n", SCHEMA_FILE);
        fclose(f);
        exit(1);
    }

    char *sql = malloc((size_t)size + 1);
    if (!sql) {
        fprintf(stderr, "Не хватило памяти под скрипт\n");
        fclose(f);
        exit(1);
    }

    if (fread(sql, 1, (size_t)size, f) != (size_t)size) {
        fprintf(stderr, "Ошибка чтения %s\n", SCHEMA_FILE);
        fclose(f);
        free(sql);
        exit(1);
    }
    sql[size] = '\0';
    fclose(f);

    /* --- выполнение скрипта --- */
    sqlite3 *db = get_connection();
    if (!db) {
        free(sql);
        exit(1);
    }

    char *err = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "Ошибка выполнения схемы БД: %s\n", err);
        sqlite3_free(err);
        sqlite3_close(db);
        free(sql);
        exit(1);
    }

    sqlite3_close(db);
    free(sql);
}