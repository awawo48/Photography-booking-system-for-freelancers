#pragma once

#include <mysql.h>
#include "UserSession.hpp"

void customerDashboard(MYSQL* conn, const UserSession& session);
