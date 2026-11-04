#ifndef MODELS_H
#define MODELS_H

/* Статусы заказа — строковые значения, как в БД (orders.status) */
#define STATUS_NEW        "new"
#define STATUS_PROCESSING "processing"
#define STATUS_SHIPPED    "shipped"
#define STATUS_DELIVERED  "delivered"
#define STATUS_CANCELLED  "cancelled"

/* Покупатель: соответствует таблице customers */
typedef struct {
    int  customer_id;
    char last_name[64];
    char first_name[64];
    char middle_name[64];      /* "" = отчество нет (аналог NULL) */
    char email[128];
    char phone[32];
    char delivery_address[256];
} Customer;

/* Товар: соответствует таблице products */
typedef struct {
    int  product_id;
    char name[128];
    char description[256];
    double price;
    int  quantity;
    char category[64];
} Product;

/* Заказ: соответствует таблице orders */
typedef struct {
    int  order_id;
    char order_date[32];       /* "2026-10-22 17:55:43" */
    char status[32];           /* "new", "processing", ... */
    int  customer_id;
    double total_amount;

    /* --- служебные поля: в БД их нет, заполняются при чтении --- */
    char customer_name[128];   /* ФИО клиента вместо ID */
} Order;

/* Позиция заказа: соответствует таблице order_product */
typedef struct {
    int  order_id;
    int  product_id;
    int  quantity;

    /* --- служебные поля: в БД их нет --- */
    char product_name[128];    /* название товара вместо ID */
    double unit_price;         /* цена товара на момент чтения */
    double subtotal;           /* unit_price * quantity */
} OrderItem;

typedef struct {
    char login[64];
    char password[64];
    char role[16]; /* "admin" или "customer" */
} User;

#endif