#include "processor/processor.hpp"
#include <nlohmann/json.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <memory>
#include <cppkafka/consumer.h>

using json = nlohmann::json;

Processor::Processor(const std::string& db_conn_str, const std::string& kafka_brokers, size_t num_threads)
    : db_(db_conn_str),
      kafka_consumer_(cppkafka::Configuration{
          {"metadata.broker.list", kafka_brokers},
          {"group.id", "order_processor_group"},
          {"auto.offset.reset", "earliest"},
          {"fetch.wait.max.ms", "50"},
          {"queued.max.messages.kbytes", "20000"}
      }),
      running_(true),
      ioc_(std::make_shared<boost::asio::io_context>()) {
    kafka_consumer_.subscribe({"orders"});
    std::cout << "Subscribing to Kafka topic 'orders'...\n";
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this]() { process_queues(); });
    }

    CROW_ROUTE(app_, "/complete").methods("POST"_method)([this](const crow::request& req) {
        auto json_body = crow::json::load(req.body);
        if (!json_body) return crow::response(400, "Invalid JSON");

        CompleteTask task;
        task.order_id = json_body["order_id"].i();
        task.status = json_body["status"].s();

        {
            std::lock_guard<std::mutex> lock(complete_mutex_);
            complete_queue_.push(task);
        }
        queue_cv_.notify_one();
        std::cout << "Received completion for order " << task.order_id << " [Thread ID: " << std::this_thread::get_id() << "]\n";
        return crow::response(200);
    });

    app_.port(8082);
    app_.concurrency(256);
}

void Processor::run() {
    std::cout << "Processor running...\n";
    std::thread crow_thread([this]() { app_.run(); });
    try {
        while (running_) {
            auto msg = kafka_consumer_.poll(std::chrono::milliseconds(50));
            if (msg) {
                std::cout << "Received message from Kafka\n";
                std::string payload(msg.get_payload().begin(), msg.get_payload().end());
                std::cout << "Payload: " << payload << "\n";
                std::stringstream json_stream(payload);
                boost::property_tree::ptree pt;
                boost::property_tree::read_json(json_stream, pt);

                Order order(pt.get<std::string>("item_name"), pt.get<int>("quantity"));
                order.id = pt.get<int>("id");

                {
                    std::lock_guard<std::mutex> lock(order_mutex_);
                    order_queue_.push(order);
                    std::cout << "Order " << order.id << " added to queue\n";
                }
                queue_cv_.notify_one();
                kafka_consumer_.commit(msg);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in main loop: " << e.what() << "\n";
    }
    stop();
    crow_thread.join();
}

void Processor::stop() {
    running_ = false;
    queue_cv_.notify_all();
    app_.stop();
    ioc_->stop();

    while (ioc_->run_one()) {}

    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}

void Processor::process_queues() {
    while (running_) {
        Order order;
        CompleteTask complete_task;
        bool has_order = false;
        bool has_complete = false;

        {
            std::unique_lock<std::mutex> order_lock(order_mutex_, std::defer_lock);
            std::unique_lock<std::mutex> complete_lock(complete_mutex_, std::defer_lock);
            order_lock.lock();
            if (!order_queue_.empty()) {
                order = order_queue_.front();
                order_queue_.pop();
                has_order = true;
            }
            order_lock.unlock();

            complete_lock.lock();
            if (!complete_queue_.empty()) {
                complete_task = complete_queue_.front();
                complete_queue_.pop();
                has_complete = true;
            }
            complete_lock.unlock();

            if (!has_order && !has_complete) {
                std::unique_lock<std::mutex> lock(order_mutex_);
                queue_cv_.wait(lock, [this]() { 
                    return !order_queue_.empty() || !complete_queue_.empty() || !running_; 
                });
                if (!running_ && order_queue_.empty() && complete_queue_.empty()) {
                    std::cout << "Thread exiting: no tasks and not running\n";
                    return;
                }
                continue;
            }
        }

        if (has_order) {
            try {
                std::cout << "Processing order " << order.id << " in thread\n";
                db_.update_order_status(order.id, "processing");
                make_restaraunt_request(order);
            } catch (const std::exception& e) {
                std::cerr << "Error processing order " << order.id << ": " << e.what() << "\n";
            }
        } else if (has_complete) {
            std::cout << "Completing order " << complete_task.order_id << " in thread\n";
            if (complete_task.status == "completed") {
                db_.update_order_status(complete_task.order_id, "completed");
                std::cout << "Order " << complete_task.order_id << " completed by Restaurant API\n";
            }
        }
    }
}

void Processor::make_restaraunt_request(const Order& order) {
    boost::asio::ip::tcp::socket socket(*ioc_);
    boost::asio::ip::tcp::resolver resolver(*ioc_);

    std::cout << "Attempting to resolve localhost:8083 for order " << order.id << "\n";
    auto const results = resolver.resolve("localhost", "8083");
    std::cout << "Resolved localhost:8083\n";
    boost::asio::connect(socket, results);
    std::cout << "Connected to Restaurant API\n";

    json j;
    j["item"] = order.item_name;
    j["quantity"] = order.quantity;
    j["id"] = order.id;
    std::string body_content = j.dump();

    std::ostringstream request;
    request << "POST /process HTTP/1.1\r\n";
    request << "Host: localhost\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Connection: keep-alive\r\n";
    request << "Content-Length: " << body_content.length() << "\r\n";
    request << "\r\n";
    request << body_content;
    std::string request_str = request.str();

    std::cout << "Sending request to Restaurant API for order " << order.id << ": " << body_content << "\n";
    boost::asio::write(socket, boost::asio::buffer(request_str));
    std::cout << "Request successfully sent for order " << order.id << "\n";

    std::string response;
    boost::system::error_code ec;
    std::array<char, 1024> buffer;
    while (!ec) {
        size_t bytes_read = socket.read_some(boost::asio::buffer(buffer), ec);
        if (bytes_read > 0) {
            response.append(buffer.data(), bytes_read);
        }
    }
    if (ec != boost::asio::error::eof) {
        throw boost::system::system_error(ec);
    }

    std::istringstream response_stream(response);
    std::string http_version, status_code, status_message;
    response_stream >> http_version >> status_code;
    std::getline(response_stream, status_message);
    std::cout << "HTTP Response: " << http_version << " " << status_code << " " << status_message << "\n";

    std::string header;
    while (std::getline(response_stream, header) && header != "\r") {
        std::cout << "Header: " << header << "\n";
    }

    std::string body = response.substr(response.find("\r\n\r\n") + 4);
    std::cout << "Response body: " << body << "\n";

    if (status_code == "200") {
        std::stringstream json_stream(body);
        boost::property_tree::ptree response_pt;
        boost::property_tree::read_json(json_stream, response_pt);
        std::string status = response_pt.get<std::string>("status");

        if (status == "processing") {
            std::cout << "Order " << order.id << " sent to Restaurant API\n";
        }
    } else {
        std::cerr << "Error: Received non-200 status code " << status_code << " for order " << order.id << "\n";
    }
}