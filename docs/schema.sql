CREATE TABLE IF NOT EXISTS users (
    user_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    login    TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,
    role     TEXT NOT NULL CHECK (role IN ('admin','customer'))
);

INSERT OR IGNORE INTO users (login, password, role)
VALUES ('admin', 'admin123', 'admin');

CREATE TABLE IF NOT EXISTS customers (
    customer_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id           INTEGER UNIQUE,
    last_name         TEXT NOT NULL,
    first_name        TEXT NOT NULL,
    middle_name       TEXT,
    email             TEXT UNIQUE,
    phone             TEXT,
    delivery_address  TEXT,
    FOREIGN KEY (user_id) REFERENCES users(user_id)
);

CREATE TABLE IF NOT EXISTS products (
    product_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    name         TEXT NOT NULL,
    description  TEXT,
    price        REAL NOT NULL CHECK (price >= 0),
    quantity     INTEGER NOT NULL DEFAULT 0 CHECK (quantity >= 0),
    category     TEXT
);

CREATE TABLE IF NOT EXISTS orders (
    order_id     INTEGER PRIMARY KEY AUTOINCREMENT,
    order_date   DATETIME DEFAULT CURRENT_TIMESTAMP,
    status       TEXT NOT NULL DEFAULT 'new',
    customer_id  INTEGER NOT NULL,
    total_amount REAL NOT NULL DEFAULT 0,
    FOREIGN KEY (customer_id) REFERENCES customers(customer_id)
);

CREATE TABLE IF NOT EXISTS order_product (
    order_id    INTEGER NOT NULL,
    product_id  INTEGER NOT NULL,
    quantity    INTEGER NOT NULL CHECK (quantity > 0),
    PRIMARY KEY (order_id, product_id),
    FOREIGN KEY (order_id)   REFERENCES orders(order_id)   ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(product_id)
);

INSERT OR IGNORE INTO customers (customer_id, user_id, last_name, first_name, middle_name, email, phone, delivery_address)
VALUES (1, NULL, 'Smith', 'John', 'A', 'john.smith@mail.com', '+1234567890', '101 Main St, City A');

INSERT OR IGNORE INTO customers (customer_id, user_id, last_name, first_name, middle_name, email, phone, delivery_address)
VALUES (2, NULL, 'Doe', 'Jane', NULL, 'jane.doe@mail.com', '+0987654321', '202 Oak Ave, City B');

INSERT OR IGNORE INTO products (product_id, name, description, price, quantity, category)
VALUES (1, 'Laptop X1', 'Basic laptop', 900.0, 10, 'Electronics');

INSERT OR IGNORE INTO products (product_id, name, description, price, quantity, category)
VALUES (2, 'Mouse M2', 'Standard mouse', 30.0, 50, 'Peripherals');

INSERT OR IGNORE INTO orders (order_id, order_date, status, customer_id, total_amount)
VALUES (1, '2026-10-22 17:55:43', 'processing', 1, 900.0);

INSERT OR IGNORE INTO orders (order_id, order_date, status, customer_id, total_amount)
VALUES (2, '2026-10-22 17:55:43', 'new', 2, 30.0);

INSERT OR IGNORE INTO order_product (order_id, product_id, quantity)
VALUES (1, 1, 1);

INSERT OR IGNORE INTO order_product (order_id, product_id, quantity)
VALUES (2, 2, 1);