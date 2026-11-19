#ifndef MODELS_H
#define MODELS_H

#define STATUS_NEW        "new"
#define STATUS_PROCESSING "processing"
#define STATUS_SHIPPED    "shipped"
#define STATUS_DELIVERED  "delivered"
#define STATUS_CANCELLED  "cancelled"

/* customers */
typedef struct {
    int  customer_id;
    char last_name[64];
    char first_name[64];
    char middle_name[64];      /* "" = отчество нет (аналог NULL) */
    char email[128];
    char phone[32];
    char delivery_address[256];
} Customer;

/* products */
typedef struct {
    int  product_id;
    char name[128];
    char description[256];
    double price;
    int  quantity;
    char category[64];
} Product;

/* orders */
typedef struct {
    int  order_id;
    char order_date[32];   
    char status[32];  
    int  customer_id;
    double total_amount;

    char customer_name[128];
} Order;

/* order_product */
typedef struct {
    int  order_id;
    int  product_id;
    int  quantity;

    char product_name[128];
    double unit_price;
    double subtotal;
} OrderItem;

typedef struct {
    char login[64];
    char password[64];
    char role[16];
} User;

#endif