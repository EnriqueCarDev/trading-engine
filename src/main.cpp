#include <iostream>
#include "orderbook_layers/order.hpp"
#include "orderbook_layers/orderbook.hpp"

class Test {
   const long exchange_id{123};

  public:
   std::vector<Order*> generateOrders() {
      std::vector<Order*> orders;
      for (size_t i{}; i <= 1000; i++) {
         std::uint32_t randNum = rand() % (97 - 103 + 1) + 97;
         if (i % 3 == 0) {
            Order* order = new Order(exchange_id, std::to_string(i), randNum,
                                     Order::BUY, Order::FillAndKill, 10);
            orders.push_back(order);
         } else {
            Order* order = new Order(exchange_id, std::to_string(i), randNum,
                                     Order::SELL, Order::FillAndKill, 10);
            orders.push_back(order);
         }
      }

      return orders;
   }

   void insertOrders(std::vector<Order*>& orders, OrderBook* orderBook) {
      for (auto order : orders) {
         orderBook->insertOrder(order);
      }
   }

   void run() {
      OrderBookListener listener;
      OrderBook* orderBook = new OrderBook(listener);
      std::vector<Order*> orders{generateOrders()};
      insertOrders(orders, orderBook);
   }
};

int main() {
   Test test;

   test.run();

   return 0;
}