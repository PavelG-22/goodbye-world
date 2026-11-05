#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "sqlite3.h"
#include "models.h"

/* Клиенты */
int repo_customer_add(sqlite3* db, const Customer* c);
int repo_customer_list(sqlite3* db);
int repo_customer_update(sqlite3* db, int customer_id, const Customer* c);
int repo_customer_delete(sqlite3* db, int customer_id);
int repo_customer_show(sqlite3* db, int customer_id);

/* Товары */
int repo_product_add(sqlite3* db, const Product* p);
int repo_product_list(sqlite3* db);
int repo_product_update(sqlite3* db, int product_id, const Product* p);
int repo_product_delete(sqlite3* db, int product_id);

/* Заказы */
int repo_order_create(sqlite3* db, int customer_id, int* out_order_id);
int repo_order_add_item(sqlite3* db, int order_id, int product_id, int quantity);
int repo_order_list(sqlite3* db);
int repo_order_show(sqlite3* db, int order_id);
int repo_order_delete(sqlite3* db, int order_id);
int repo_order_list_for_user(sqlite3* db, int user_id);
int repo_order_belongs_to_user(sqlite3* db, int order_id, int user_id);

int repo_customer_exists(sqlite3* db, int customer_id);
int repo_product_exists(sqlite3* db, int product_id);
int repo_customer_find_by_phone(sqlite3* db, const char* phone);
int repo_product_find_by_name(sqlite3* db, const char* name);

int repo_user_add(sqlite3* db, const User* u);
int repo_user_find_by_login(sqlite3* db, const char* login, User* out);
int repo_user_list(sqlite3* db);
int repo_user_count_admins(sqlite3* db, int* out_count);
int repo_user_delete(sqlite3* db, const char* login);

int repo_user_get_id(sqlite3* db, const char* login, int* out_id);
int repo_customer_find_by_user_id(sqlite3* db, int user_id);
int repo_customer_add_linked(sqlite3* db, const Customer* c, int user_id);

#endif