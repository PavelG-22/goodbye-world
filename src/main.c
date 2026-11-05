#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "repository.h"
#include "service.h"

static void read_line(char* buf, int size) {
    if (fgets(buf, size, stdin) != NULL) {
        buf[strcspn(buf, "\n")] = '\0';
    } else {
        buf[0] = '\0';
    }
}

static int read_int(void) {
    char line[64];
    read_line(line, sizeof(line));
    return atoi(line);
}

static double read_double(void) {
    char line[64];
    read_line(line, sizeof(line));
    return atof(line); /* цену вводить через точку: 59.90 */
}

static void print_result(int r, const char* ok, const char* not_found) {
    if (r == 0) printf("%s", ok);
    else if (r == 1) printf("%s", not_found);
    else printf("Ошибка (см. сообщение выше).\n");
}

int main(void) {
    init_db();

    sqlite3* db = get_connection();
    if (db == NULL) {
        return 1;
    }

    char line[256];
    char role[16] = "";
    char current_login[64] = "";
    int current_user_id = 0;
    int is_admin = 0;

    /* --- Цикл входа: пока не войдём, меню магазина не показываем --- */
    for (;;) {
        printf("\n=== ВХОД ===\n");
        printf("1 - Войти\n");
        printf("2 - Регистрация\n");
        printf("0 - Выход\n");
        printf("> ");
        read_line(line, sizeof(line));

        if (strcmp(line, "1") == 0) {
            char login[64], password[64], err[256];
            printf("Логин: ");  read_line(login, sizeof(login));
            printf("Пароль: "); read_line(password, sizeof(password));
            if (svc_user_login(db, login, password, role, sizeof(role),
                               err, sizeof(err)) == 0) {
                is_admin = (strcmp(role, "admin") == 0);
                snprintf(current_login, sizeof(current_login), "%s", login);
                repo_user_get_id(db, current_login, &current_user_id);
                printf("Добро пожаловать, %s (%s).\n", login, role);
                break; /* вход успешен — переходим в магазин */
            } else {
                printf("%s\n", err);
            }
        } else if (strcmp(line, "2") == 0) {
            User u;
            char err[256];
            printf("Логин: ");  read_line(u.login, sizeof(u.login));
            printf("Пароль: "); read_line(u.password, sizeof(u.password));
            snprintf(u.role, sizeof(u.role), "customer"); /* саморегистрация — всегда покупатель */
            if (svc_user_register(db, &u, "guest", err, sizeof(err)) == 0)
                printf("Пользователь зарегистрирован. Теперь войдите.\n");
            else
                printf("%s\n", err);
        } else if (strcmp(line, "0") == 0) {
            sqlite3_close(db);
            return 0;
        } else {
            printf("Неизвестная команда.\n");
        }
    }

    /* --- Главное меню магазина --- */
    for (;;) {
        printf("\n=== МАГАЗИН ===\n");
        printf("1 - Показать клиентов\n");
        if (is_admin) printf("2 - Добавить клиента\n");
        if (is_admin) printf("3 - Изменить клиента\n");
        if (is_admin) printf("4 - Удалить клиента\n");
        printf("5 - Показать товары\n");
        if (is_admin) printf("6 - Добавить товар\n");
        if (is_admin) printf("7 - Изменить товар\n");
        if (is_admin) printf("8 - Удалить товар\n");
        printf("9 - Показать заказы\n");
        printf("10 - Создать заказ\n");
        printf("11 - Содержимое заказа\n");
        if (is_admin) printf("12 - Удалить заказ\n");
        if (is_admin) printf("13 - Регистрация пользователя\n");
        if (is_admin) printf("14 - Список пользователей\n");
        if (is_admin) printf("15 - Удалить пользователя\n");
        printf("0 - Выход\n");
        printf("> ");

        read_line(line, sizeof(line));

        if (strcmp(line, "1") == 0) {
            svc_customer_list(db);
        } else if (strcmp(line, "2") == 0) {
            if (!is_admin) {
                printf("Недоступно: нужна роль администратора.\n");
            } else {
                Customer c;
                printf("Фамилия: "); read_line(c.last_name, sizeof(c.last_name));
                printf("Имя: ");     read_line(c.first_name, sizeof(c.first_name));
                printf("Телефон: "); read_line(c.phone, sizeof(c.phone));
                char err[256];
                int r = svc_customer_add(db, &c, err, sizeof(err));
                if (r == 0) printf("Клиент добавлен.\n");
                else if (r == 1) printf("%s\n", err);
                else printf("Ошибка базы данных.\n");
            }
        } else if (strcmp(line, "3") == 0) {
            if (!is_admin) {
                printf("Недоступно: нужна роль администратора.\n");
            } else {
                printf("ID клиента для изменения: ");
                int cid = read_int();
                Customer c;
                printf("Новая фамилия: "); read_line(c.last_name, sizeof(c.last_name));
                printf("Новое имя: ");     read_line(c.first_name, sizeof(c.first_name));
                printf("Новый телефон: "); read_line(c.phone, sizeof(c.phone));
                char err[256];
                int r = svc_customer_update(db, cid, &c, err, sizeof(err));
                if (r == 0) printf("Клиент изменён.\n");
                else if (r == 1) printf("%s\n", err);
                else printf("Ошибка базы данных.\n");
            }
        } else if (strcmp(line, "4") == 0) {
            if (!is_admin) {
                printf("Недоступно: нужна роль администратора.\n");
            } else {
                printf("ID клиента для удаления: ");
                int cid = read_int();
                print_result(svc_customer_delete(db, cid),
                    "Клиент удалён.\n", "Клиент с таким ID не найден.\n");
            }
        } else if (strcmp(line, "5") == 0) {
            svc_product_list(db);
        } else if (strcmp(line, "6") == 0) {
            if (!is_admin) {
                printf("Недоступно: нужна роль администратора.\n");
            } else {
                Product p;
                printf("Название: "); read_line(p.name, sizeof(p.name));
                printf("Цена: ");     p.price = read_double();
                char err[256];
                int r = svc_product_add(db, &p, err, sizeof(err));
                if (r == 0) printf("Товар добавлен.\n");
                else if (r == 1) printf("%s\n", err);
                else printf("Ошибка базы данных.\n");
            }
        } else if (strcmp(line, "7") == 0) {
            if (!is_admin) {
                printf("Недоступно: нужна роль администратора.\n");
            } else {
                printf("ID товара для изменения: ");
                int pid = read_int();
                Product p;
                printf("Новое название: "); read_line(p.name, sizeof(p.name));
                printf("Новая цена: ");     p.price = read_double();
                char err[256];
                int r = svc_product_update(db, pid, &p, err, sizeof(err));
                if (r == 0) printf("Товар изменён.\n");
                else if (r == 1) printf("%s\n", err);
                else printf("Ошибка базы данных.\n");
            }
        } else if (strcmp(line, "8") == 0) {
            if (!is_admin) {
                printf("Недоступно: нужна роль администратора.\n");
            } else {
                printf("ID товара для удаления: ");
                int pid = read_int();
                print_result(svc_product_delete(db, pid),
                    "Товар удалён.\n", "Товар с таким ID не найден.\n");
            }
        } else if (strcmp(line, "9") == 0) {
            svc_order_list(db);
        } else if (strcmp(line, "10") == 0) {
    int oid = 0;
    char err[256];

    if (is_admin) {
        /* админ создаёт заказ для любого клиента по ID */
        printf("ID клиента: ");
        int cid = read_int();
        if (svc_order_create(db, cid, &oid, err, sizeof(err)) != 0) {
            printf("%s\n", err);
            oid = 0;
        }
    } else {
        /* покупатель заказывает для себя */
        int cid = svc_customer_id_for_user(db, current_user_id);
        if (cid < 0) {
            printf("Ошибка базы данных.\n");
            oid = 0;
        } else {
            Customer c;
            if (cid == 0) {
                printf("Первый заказ — заполним ваш профиль.\n");
                printf("Фамилия: "); read_line(c.last_name, sizeof(c.last_name));
                printf("Имя: ");     read_line(c.first_name, sizeof(c.first_name));
                printf("Телефон: "); read_line(c.phone, sizeof(c.phone));
            }
            if (svc_order_create_for_user(db, current_user_id, &c,
                                          &cid, &oid, err, sizeof(err)) != 0) {
                printf("%s\n", err);
                oid = 0;
            }
        }
    }

    if (oid != 0) {
        printf("Создан заказ № %d. Добавляйте товары.\n", oid);
        for (;;) {
            printf("ID товара (0 — закончить): ");
            int pid = read_int();
            if (pid == 0) break;
            printf("Количество: ");
            int qty = read_int();
            if (svc_order_add_item(db, oid, pid, qty, err, sizeof(err)) != 0)
                printf("%s\n", err);
            else
                printf("Добавлено.\n");
        }
        svc_order_show(db, oid);
    }
        } else if (strcmp(line, "11") == 0) {
            printf("ID заказа: ");
            int oid = read_int();
            svc_order_show(db, oid);
        } else if (strcmp(line, "12") == 0) {
            if (!is_admin) {
                printf("Недоступно: нужна роль администратора.\n");
            } else {
                printf("ID заказа для удаления: ");
                int oid = read_int();
                print_result(svc_order_delete(db, oid),
                    "Заказ удалён.\n", "");
            }
        } else if (strcmp(line, "13") == 0) {
            if (!is_admin) {
                printf("Недоступно: нужна роль администратора.\n");
            } else {
                User u;
                char err[256];
                printf("Логин: ");    read_line(u.login, sizeof(u.login));
                printf("Пароль: ");   read_line(u.password, sizeof(u.password));
                printf("Роль (admin/customer): ");
                read_line(u.role, sizeof(u.role));
                if (svc_user_register(db, &u, role, err, sizeof(err)) == 0)
                    printf("Пользователь зарегистрирован.\n");
                else
                    printf("%s\n", err);
            }
        } else if (strcmp(line, "14") == 0) {
            if (!is_admin) {
                printf("Недоступно: нужна роль администратора.\n");
            } else {
                svc_user_list(db);
            }
        } else if (strcmp(line, "15") == 0) {
            if (!is_admin) {
                printf("Недоступно: нужна роль администратора.\n");
            } else {
                printf("Логин для удаления: ");
                char target[64];
                read_line(target, sizeof(target));
                char err[256];
                int r = svc_user_delete(db, target, current_login,
                                        err, sizeof(err));
                if (r == 0) printf("Пользователь удалён.\n");
                else if (r == 1) printf("%s\n", err);
                else printf("Ошибка базы данных.\n");
            }
        } else if (strcmp(line, "0") == 0) {
            break;
        } else {
            printf("Неизвестная команда.\n");
        }
    }

    sqlite3_close(db);
    return 0;
}