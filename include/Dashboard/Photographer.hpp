#pragma once

#include <mysql.h>
#include "UserSession.hpp"

void photographerDashboard(MYSQL* conn, const UserSession& session);
