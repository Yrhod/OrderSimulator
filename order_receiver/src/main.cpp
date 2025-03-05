#include <iostream>
#include <string>
#include <cstdlib> 
#include "receiver/receiver.hpp"

int main(int argc, char* argv[]) {
    try {
        std::string conn_str = "dbname=order_simulator user=postgres password=admin host=localhost port=5432";
        std::string kafka_brokers = "localhost:9092";


        if (argc > 1) {
            int port = std::atoi(argv[1]); 
            if (port > 0 && port < 65536) { 
                kafka_brokers = "localhost:" + std::to_string(port);
            } else {
                std::cerr << "Invalid Kafka port number. Using default (9092).\n";
            }
        }

        Receiver receiver(8081, conn_str, kafka_brokers); 
        receiver.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}