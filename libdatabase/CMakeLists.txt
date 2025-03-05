cmake_minimum_required(VERSION 3.10)
project(LibDatabase)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(libpqxx CONFIG REQUIRED)

add_library(libdatabase STATIC
    src/db.cpp
)

target_include_directories(libdatabase PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(libdatabase PUBLIC libpqxx::pqxx)