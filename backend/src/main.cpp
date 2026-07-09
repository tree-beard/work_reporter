#include <httplib/httplib.h>

#include "Config.h"
#include "db/Database.h"
#include "repositories/ReportRepository.h"
#include "repositories/UserRepository.h"
#include "routes/AuthRoutes.h"
#include "routes/ReportRoutes.h"
#include "services/AuthService.h"
#include "services/ReportService.h"

int main() {
    Config config = Config::fromEnv();

    Database database(config.dbPath);
    UserRepository userRepository(database);
    ReportRepository reportRepository(database);

    AuthService authService(userRepository, config.jwtSecret);
    ReportService reportService(reportRepository);

    httplib::Server server;
    registerAuthRoutes(server, authService);
    registerReportRoutes(server, reportService, config.jwtSecret);

    server.listen("0.0.0.0", config.port);
    return 0;
}
