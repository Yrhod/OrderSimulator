#ifndef DB_HPP
#define DB_HPP

#include <pqxx/pqxx>
#include <string>

class Database {
public:
    Database(const std::string& conn_str);
    int add_order(const std::string& item_name, int quantity);
    void update_order_status(int order_id, const std::string& status);

private:
    pqxx::connection conn_;
};

#endif