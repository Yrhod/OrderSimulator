#include <iostream>
#include <csignal>
#include <string>
#include <cstdlib> 
#include "processor/processor.hpp"

Processor* global_processor = nullptr;

void signal_handler(int signal) {
    if (signal == SIGINT && global_processor) {
        std::cout << "\nShutting down processor...\n";
        global_processor->stop();
    }
}

int main(int argc, char* argv[]) {
    try {
        std::string conn_str = "dbname=order_simulator user=postgres password=admin host=localhost port=5432";
        std::string kafka_brokers = "localhost:9092"; 


        if (argc > 1) {
            int port = std::atoi(argv[1]); 
            if (port > 0 && port < 65536) { 
                kafka_brokers = "localhost:" + std::to_string(port);
            } else {
                std::cerr << "Invalid port number. Using default (9092).\n";
            }
        }

        Processor processor(conn_str, kafka_brokers, 8);
        global_processor = &processor;

        std::signal(SIGINT, signal_handler);
        processor.run();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}