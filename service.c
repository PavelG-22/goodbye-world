#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "service.h"
#include "repository.h"

/* Строка из одних пробелов считается пустой — как strip() в методичке */
static int is_blank(const char* s) {
    for (; *s; s++)
        if (!isspace((unsigned char)*s)) return 0;
    return 1;
}

/* ---------- Клиенты ---------- */

int svc_customer_list(sqlite3* db) {
    return repo_customer_list(db);
}

int svc_customer_add(sqlite3* db, const Customer* c, char* err, int err_size) {
    if (is_blank(c->last_name)) {
        snprintf(err, err_size, "Фамилия не может быть пустой");
        return 1;
    }
    if (is_blank(c->first_name)) {
        snprintf(err, err_size, "Имя не может быть пустым");
        return 1;
    }
    if (repo_customer_find_by_phone(db, c->phone) == 1) {
        snprintf(err, err_size, "Клиент с таким телефоном уже существует");
        return 1;
    }
    return repo_customer_add(db, c);
}

int svc_customer_update(sqlite3* db, int customer_id, const Customer* c,
                        char* err, int err_size) {
    if (is_blank(c->last_name)) {
        snprintf(err, err_size, "Фамилия не может быть пустой");
        return 1;
    }
    if (is_blank(c->first_name)) {
        snprintf(err, err_size, "Имя не может быть пустым");
        return 1;
    }
    if (repo_customer_exists(db, customer_id) != 1) {
        snprintf(err, err_size, "Клиент с таким ID не найден");
        return 1;
    }
    return repo_customer_update(db, customer_id, c);
}

int svc_customer_delete(sqlite3* db, int customer_id) {
    return repo_customer_delete(db, customer_id);
}

/* ---------- Товары ---------- */

int svc_product_list(sqlite3* db) {
    return repo_product_list(db);
}

int svc_product_add(sqlite3* db, const Product* p, char* err, int err_size) {
    if (is_blank(p->name)) {
        snprintf(err, err_size, "Название не может быть пустым");
        return 1;
    }
    if (p->price <= 0) {
        snprintf(err, err_size, "Цена должна быть положительным числом");
        return 1;
    }
    if (repo_product_find_by_name(db, p->name) == 1) {
        snprintf(err, err_size, "Товар с таким названием уже существует");
        return 1;
    }
    return repo_product_add(db, p);
}

int svc_product_update(sqlite3* db, int product_id, const Product* p,
                       char* err, int err_size) {
    if (is_blank(p->name)) {
        snprintf(err, err_size, "Название не может быть пустым");
        return 1;
    }
    if (p->price <= 0) {
        snprintf(err, err_size, "Цена должна быть положительным числом");
        return 1;
    }
    if (repo_product_exists(db, product_id) != 1) {
        snprintf(err, err_size, "Товар с таким ID не найден");
        return 1;
    }
    return repo_product_update(db, product_id, p);
}

int svc_product_delete(sqlite3* db, int product_id) {
    return repo_product_delete(db, product_id);
}

/* ---------- Заказы ---------- */

int svc_order_list(sqlite3* db) {
    return repo_order_list(db);
}

int svc_order_create(sqlite3* db, int customer_id, int* out_order_id,
                     char* err, int err_size) {
    if (repo_customer_exists(db, customer_id) != 1) {
        snprintf(err, err_size, "Клиент с таким ID не найден");
        return 1;
    }
    return repo_order_create(db, customer_id, out_order_id);
}

int svc_order_add_item(sqlite3* db, int order_id, int product_id, int quantity,
                       char* err, int err_size) {
    if (quantity <= 0) {
        snprintf(err, err_size, "Количество должно быть больше нуля");
        return 1;
    }
    if (repo_product_exists(db, product_id) != 1) {
        snprintf(err, err_size, "Товар с таким ID не найден");
        return 1;
    }
    return repo_order_add_item(db, order_id, product_id, quantity);
}

int svc_order_show(sqlite3* db, int order_id) {
    return repo_order_show(db, order_id);
}

int svc_order_delete(sqlite3* db, int order_id) {
    return repo_order_delete(db, order_id);
}
/* ---------- Пользователи и вход ---------- */

int svc_user_register(sqlite3* db, const User* u, const char* caller_role,
                      char* err, int err_size) {
    if (is_blank(u->login)) {
        snprintf(err, err_size, "Логин не может быть пустым");
        return 1;
    }
    if (is_blank(u->password)) {
        snprintf(err, err_size, "Пароль не может быть пустым");
        return 1;
    }
    if (strcmp(u->role, "admin") != 0 && strcmp(u->role, "customer") != 0) {
        snprintf(err, err_size, "Недопустимая роль");
        return 1;
    }
    /* Создавать администратора может только администратор.
       caller_role == "guest" — саморегистрация до входа, там админа не будет. */
    if (strcmp(u->role, "admin") == 0 &&
        (caller_role == NULL || strcmp(caller_role, "admin") != 0)) {
        snprintf(err, err_size,
                 "Создавать администраторов может только администратор");
        return 1;
    }
    User tmp;
    if (repo_user_find_by_login(db, u->login, &tmp) == 0) {
        snprintf(err, err_size, "Такой логин уже занят");
        return 1;
    }
    return repo_user_add(db, u);
}

int svc_user_login(sqlite3* db, const char* login, const char* password,
                   char* role_out, int role_size, char* err, int err_size) {
    if (is_blank(login) || is_blank(password)) {
        snprintf(err, err_size, "Логин и пароль не могут быть пустыми");
        return 1;
    }
    User u;
    int rc = repo_user_find_by_login(db, login, &u);
    if (rc == 1) {
        snprintf(err, err_size, "Пользователь не найден");
        return 1;
    }
    if (rc != 0) return -1;
    if (strcmp(u.password, password) != 0) {
        snprintf(err, err_size, "Неверный пароль");
        return 1;
    }
    snprintf(role_out, role_size, "%s", u.role);
    return 0;
}
int svc_user_list(sqlite3* db) {
    return repo_user_list(db);
}

int svc_user_delete(sqlite3* db, const char* login, const char* caller_login,
                    char* err, int err_size) {
    if (is_blank(login)) {
        snprintf(err, err_size, "Логин не может быть пустым");
        return 1;
    }
    /* Нельзя удалить самого себя */
    if (caller_login != NULL && strcmp(login, caller_login) == 0) {
        snprintf(err, err_size, "Нельзя удалить свою собственную учётную запись");
        return 1;
    }
    User u;
    int rc = repo_user_find_by_login(db, login, &u);
    if (rc == 1) {
        snprintf(err, err_size, "Пользователь не найден");
        return 1;
    }
    if (rc != 0) return -1;
    /* Нельзя удалить последнего администратора */
    if (strcmp(u.role, "admin") == 0) {
        int admins = 0;
        if (repo_user_count_admins(db, &admins) != 0) return -1;
        if (admins <= 1) {
            snprintf(err, err_size, "Нельзя удалить последнего администратора");
            return 1;
        }
    }
    return repo_user_delete(db, login);
}