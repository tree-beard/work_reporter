cmake_minimum_required(VERSION 3.16)
project(work_reporter_api CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(httplib REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(jwt-cpp REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(work_reporter_api
    src/main.cpp
    src/db/Database.cpp
    src/repositories/UserRepository.cpp
    src/repositories/ReportRepository.cpp
    src/services/AuthService.cpp
    src/services/ReportService.cpp
    src/middleware/AuthGuard.cpp
    src/routes/AuthRoutes.cpp
    src/routes/ReportRoutes.cpp
)

target_link_libraries(work_reporter_api PRIVATE
    httplib::httplib
    SQLiteCpp
    nlohmann_json::nlohmann_json
    jwt-cpp::jwt-cpp
    OpenSSL::Crypto
)

if(APPLE)
    # cpp-httplib uses CFHost APIs for DNS resolution with timeout on Apple
    # platforms; its Conan recipe only links these frameworks when TLS is
    # enabled, even though the code path is unconditional. Not needed on
    # Linux, where this project actually deploys.
    target_link_libraries(work_reporter_api PRIVATE "-framework CoreFoundation" "-framework CFNetwork")
endif()
