#ifndef SERVICE_H
#define SERVICE_H

#include "sqlite3.h"
#include "models.h"

/* Клиенты */
int svc_customer_list(sqlite3* db);
int svc_customer_add(sqlite3* db, const Customer* c, char* err, int err_size);
int svc_customer_update(sqlite3* db, int customer_id, const Customer* c,
                        char* err, int err_size);
int svc_customer_delete(sqlite3* db, int customer_id);
int svc_customer_show(sqlite3* db, int customer_id);

/* Товары */
int svc_product_list(sqlite3* db);
int svc_product_add(sqlite3* db, const Product* p, char* err, int err_size);
int svc_product_update(sqlite3* db, int product_id, const Product* p,
                       char* err, int err_size);
int svc_product_delete(sqlite3* db, int product_id);

/* Заказы */
int svc_order_list(sqlite3* db);
int svc_order_create(sqlite3* db, int customer_id, int* out_order_id,
                     char* err, int err_size);
int svc_order_add_item(sqlite3* db, int order_id, int product_id, int quantity,
                       char* err, int err_size);
int svc_order_show(sqlite3* db, int order_id);
int svc_order_delete(sqlite3* db, int order_id);
int svc_order_list_for_user(sqlite3* db, int user_id);
int svc_order_belongs_to_user(sqlite3* db, int order_id, int user_id);

/* Пользователи и вход */
int svc_user_register(sqlite3* db, const User* u, const char* caller_role, char* err, int err_size);
int svc_user_login(sqlite3* db, const char* login, const char* password,
                   char* role_out, int role_size, char* err, int err_size);
int svc_user_list(sqlite3* db);
int svc_user_delete(sqlite3* db, const char* login, const char* caller_login,
                    char* err, int err_size);

/* пользователь-клиент */
int svc_customer_id_for_user(sqlite3* db, int user_id);
int svc_order_create_for_user(sqlite3* db, int user_id, const Customer* profile,
                              int* out_customer_id, int* out_order_id,
                              char* err, int err_size);

#endif