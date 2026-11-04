#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"

/* Открывает соединение с файлом БД и включает проверку внешних ключей.
   Возвращает открытое соединение или NULL при ошибке. */
sqlite3 *get_connection(void);

/* Выполняет schema.sql: удаляет старые таблицы, создаёт новые,
   заполняет начальными данными. */
void init_db(void);

#endif