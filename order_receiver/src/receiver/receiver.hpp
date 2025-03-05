#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include <crow.h>
#include <cppkafka/producer.h>
#include "db.hpp"
#include "models/order.hpp"

class Receiver {
public:
    Receiver(short port, const std::string& db_conn_str, const std::string& kafka_brokers);
    void run();
private:
    Database db_;
    crow::SimpleApp app_;
    cppkafka::Producer kafka_producer_;
};

#endif