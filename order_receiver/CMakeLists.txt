cmake_minimum_required(VERSION 3.10)
project(OrderReceiver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost COMPONENTS property_tree system REQUIRED)
find_package(Crow CONFIG REQUIRED)
find_package(cppkafka CONFIG REQUIRED)

add_subdirectory(${CMAKE_SOURCE_DIR}/../libdatabase ${CMAKE_BINARY_DIR}/libdatabase)

add_executable(order_receiver
    src/main.cpp
    src/receiver/receiver.cpp
    src/models/order.cpp
)

target_include_directories(order_receiver PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../libdatabase/src
)
target_link_libraries(order_receiver PRIVATE
    Boost::property_tree
    Boost::system
    Crow::Crow
    CppKafka::cppkafka
    libdatabase  
)