cmake_minimum_required(VERSION 3.10)
project(OrderProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost COMPONENTS asio property_tree system beast REQUIRED)
find_package(cppkafka CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)


add_subdirectory(${CMAKE_SOURCE_DIR}/../libdatabase ${CMAKE_BINARY_DIR}/libdatabase)

add_executable(order_processor
    src/main.cpp
    src/processor/processor.cpp
    src/models/order.cpp
)

target_include_directories(order_processor PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../libdatabase/src
)
target_link_libraries(order_processor PRIVATE
    Boost::asio
    Boost::property_tree
    Boost::system
    Boost::beast
    CppKafka::cppkafka
    libdatabase
    nlohmann_json::nlohmann_json
)