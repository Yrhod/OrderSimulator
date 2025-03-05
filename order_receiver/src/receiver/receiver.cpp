#include "receiver/receiver.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>

Receiver::Receiver(short port, const std::string& db_conn_str, const std::string& kafka_brokers) 
    : db_(db_conn_str), 
      kafka_producer_(cppkafka::Configuration{
        {"metadata.broker.list", kafka_brokers},
        {"acks", "all"}
      }) {

    CROW_ROUTE(app_, "/order").methods("POST"_method)([this](const crow::request& req) {
        auto json_body = crow::json::load(req.body);
        if(!json_body) return crow::response(400, "invalid JSON");

        std::string item_name = json_body["item"].s();
        int quantity = json_body["quantity"].i();

        int order_id = db_.add_order(item_name, quantity);

        boost::property_tree::ptree kafka_pt;
        kafka_pt.put("id", order_id);
        kafka_pt.put("item_name", item_name);
        kafka_pt.put("quantity", quantity);
        std::stringstream kafka_json;
        boost::property_tree::write_json(kafka_json, kafka_pt);

        std::string json_str = kafka_json.str();
        cppkafka::Buffer kafka_buffer(json_str.data(), json_str.size());

        kafka_producer_.produce(cppkafka::MessageBuilder("orders")
            .partition(0)
            .payload(kafka_buffer));
        kafka_producer_.flush();

        std::cout << "Order " << order_id << " sent to kafka\n";
        return crow::response("Order recieved");
    });
    
    app_.port(port);
    app_.concurrency(32);
}

void Receiver::run() {
    std::cout << "Server running on port " << app_.port() << "...\n";
    app_.run();
}