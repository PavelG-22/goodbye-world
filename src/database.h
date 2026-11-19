#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"

sqlite3 *get_connection(void);

void init_db(void);

#endif