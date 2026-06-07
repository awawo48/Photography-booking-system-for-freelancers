#pragma once

#include <mysql.h>
#include "UserSession.hpp"

void adminDashboard(MYSQL* conn, const UserSession& session);
