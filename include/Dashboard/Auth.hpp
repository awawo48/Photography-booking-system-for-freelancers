#pragma once

#include <mysql.h>

void registerUser(MYSQL* conn);
int loginUser(MYSQL* conn);
