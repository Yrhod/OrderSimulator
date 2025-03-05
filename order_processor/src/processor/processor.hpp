#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <cppkafka/consumer.h>
#include <boost/asio/ip/tcp.hpp>
#include "db.hpp"
#include "models/order.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <crow.h>
#include <memory>

struct CompleteTask {
    int order_id;
    std::string status;
};

class Processor {
public:
    Processor(const std::string& db_conn_str, const std::string& kafka_brokers, size_t num_threads = 2);
    void run();
    void stop();
    

private:
    void process_queues();
    void make_restaraunt_request(const Order& order);
    crow::SimpleApp app_;
    Database db_;
    cppkafka::Consumer kafka_consumer_;
    std::queue<Order> order_queue_;
    std::queue<CompleteTask> complete_queue_;
    std::mutex order_mutex_;
    std::mutex complete_mutex_;
    std::condition_variable queue_cv_;
    std::vector<std::thread> workers_;
    bool running_;
    std::shared_ptr<boost::asio::io_context> ioc_; 
};

#endif 