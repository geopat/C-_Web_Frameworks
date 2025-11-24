cmake_minimum_required(VERSION 3.5)
project(drogon_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)

add_executable(server main.cpp)
target_link_libraries(server PRIVATE Drogon::Drogon)