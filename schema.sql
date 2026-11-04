-- ============================================================
-- Интернет-магазин: структура БД + тестовые данные
-- Порядок: сначала независимые таблицы, затем зависимые.
-- Удаление старой структуры — в обратном порядке.
-- ============================================================

-- ===== 1. Удаление старой структуры (сначала зависимые) =====
DROP TABLE IF EXISTS order_product;
DROP TABLE IF EXISTS orders;
DROP TABLE IF EXISTS products;
DROP TABLE IF EXISTS customers;

-- ===== 2. Покупатели (независимая таблица) =====
CREATE TABLE customers (
    customer_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    last_name         TEXT NOT NULL,
    first_name        TEXT NOT NULL,
    middle_name       TEXT,
    email             TEXT UNIQUE,
    phone             TEXT,
    delivery_address  TEXT
);

-- ===== 3. Товары (независимая таблица) =====
CREATE TABLE products (
    product_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    name         TEXT NOT NULL,
    description  TEXT,
    price        REAL NOT NULL CHECK (price >= 0),
    quantity     INTEGER NOT NULL DEFAULT 0 CHECK (quantity >= 0),
    category     TEXT
);

-- ===== 4. Заказы (зависимая: ссылается на customers) =====
CREATE TABLE orders (
    order_id     INTEGER PRIMARY KEY AUTOINCREMENT,
    order_date   DATETIME DEFAULT CURRENT_TIMESTAMP,
    status       TEXT NOT NULL DEFAULT 'new',
    customer_id  INTEGER NOT NULL,
    total_amount REAL NOT NULL DEFAULT 0,
    FOREIGN KEY (customer_id) REFERENCES customers(customer_id)
);

-- ===== 5. Связка заказ–товар (доработка: развязка M:N) =====
-- quantity — сколько единиц товара в заказе;
-- составной PRIMARY KEY — один товар не попадает в заказ дважды
CREATE TABLE order_product (
    order_id    INTEGER NOT NULL,
    product_id  INTEGER NOT NULL,
    quantity    INTEGER NOT NULL CHECK (quantity > 0),
    PRIMARY KEY (order_id, product_id),
    FOREIGN KEY (order_id)   REFERENCES orders(order_id)   ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(product_id)
);

-- ===== 6. Начальные данные =====
INSERT INTO customers VALUES (1,'Smith','John','A','john.smith@mail.com','+1234567890','101 Main St, City A');
INSERT INTO customers VALUES (2,'Doe','Jane',NULL,'jane.doe@mail.com','+0987654321','202 Oak Ave, City B');

INSERT INTO products VALUES (1,'Laptop X1','Basic laptop',900.0,10,'Electronics');
INSERT INTO products VALUES (2,'Mouse M2','Standard mouse',30.0,50,'Peripherals');

INSERT INTO orders VALUES (1,'2026-10-22 17:55:43','processing',1,900.0);
INSERT INTO orders VALUES (2,'2026-10-22 17:55:43','new',2,30.0);

-- Состав заказов: заказ 1 = ноутбук, заказ 2 = мышь
INSERT INTO order_product VALUES (1,1,1);
INSERT INTO order_product VALUES (2,2,1);
-- ===== 7. Пользователи системы (авторизация) =====
-- Таблица НЕ удаляется при перезапуске: учётные записи живут между запусками,
-- в отличие от тестовых данных клиентов/товаров/заказов.
CREATE TABLE IF NOT EXISTS users (
    user_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    login    TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,
    role     TEXT NOT NULL CHECK (role IN ('admin','customer'))
);

INSERT OR IGNORE INTO users (login, password, role)
VALUES ('admin', 'admin123', 'admin');