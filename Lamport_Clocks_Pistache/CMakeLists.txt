cmake_minimum_required(VERSION 3.10)
project(pistache_logical_clock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

# Use IMPORTED_TARGET so all cflags/ldflags/defines come through correctly
pkg_check_modules(Pistache REQUIRED IMPORTED_TARGET libpistache)
pkg_check_modules(Jsoncpp REQUIRED IMPORTED_TARGET jsoncpp)

add_executable(server main.cpp)

target_link_libraries(server
    PRIVATE
        PkgConfig::Pistache
        PkgConfig::Jsoncpp
)
