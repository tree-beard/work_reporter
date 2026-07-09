#pragma once

#include <httplib/httplib.h>

#include "../services/AuthService.h"

void registerAuthRoutes(httplib::Server& server, AuthService& authService);
