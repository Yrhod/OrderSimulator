#ifndef ORDER_HPP
#define ORDER_HPP

#include<string>

struct Order {
    int id;
    std::string item_name;
    int quantity;
    std::string status;

    Order(const std::string& item, int qty) : id(-1), item_name(item), quantity(qty), status("pending") {}
    Order() : id(-1) {}
};


#endif 