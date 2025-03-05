#include "db.hpp"
#include <stdexcept>

Database::Database(const std::string& conn_str) : conn_(conn_str) {
    if (!conn_.is_open()) {
        throw std::runtime_error("Failed to connect to database");
    }
}

int Database::add_order(const std::string& item_name, int quantity) {
    pqxx::work txn(conn_);
    pqxx::params params;
    params.append(item_name);
    params.append(quantity);
    params.append("pending");

    pqxx::result res = txn.exec("INSERT INTO orders (item_name, quantity, status) VALUES ($1, $2, $3) RETURNING id", params);
    txn.commit();

    return res[0][0].as<int>();
}

void Database::update_order_status(int order_id, const std::string& status) {
    pqxx::work txn(conn_);
    pqxx::params params;
    params.append(status);
    params.append(order_id);
    txn.exec("UPDATE orders SET status = $1 WHERE id = $2", params);
    txn.commit();
}