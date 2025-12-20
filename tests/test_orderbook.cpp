#include <gtest/gtest.h>
#include <iostream>
#include "orderbook_layers/orderbook.hpp"

class TestListener : public OrderBookListener {
  public:
   std::vector<Order> orders_;
   std::vector<Trade> trades_;

   void onOrder(const Order& order) override { orders_.push_back(order); }
   void onTrade(const Trade& trade) override { trades_.push_back(trade); }
};

TEST(OrderBookTest, SingleFullFill) {
   TestListener listener;
   OrderBook orderBook{listener};

   long exchangeId = 123;

   Order* restingOrder =
       new Order(exchangeId, "1", 100, Order::SELL, Order::GoodTillCancel, 500);
   Order* aggressorOrder =
       new Order(exchangeId, "2", 100, Order::BUY, Order::GoodTillCancel, 500);

   orderBook.insertOrder(restingOrder);
   orderBook.insertOrder(aggressorOrder);

   ASSERT_EQ(listener.trades_.size(), 1);
   ASSERT_EQ(listener.trades_[0].price_, 100);
   ASSERT_EQ(listener.trades_[0].quantity_, 500);

   Book snapshot = orderBook.book();
   ASSERT_TRUE(snapshot.bids_.empty());
   ASSERT_TRUE(snapshot.asks_.empty());
}

TEST(OrderBookTest, PartialFill) {
   TestListener listener;
   OrderBook orderBook{listener};

   long exchangeId = 123;

   Order* restingOrder =
       new Order(exchangeId, "1", 100, Order::SELL, Order::GoodTillCancel, 500);
   Order* aggressorOrder =
       new Order(exchangeId, "2", 100, Order::BUY, Order::GoodTillCancel, 300);

   orderBook.insertOrder(restingOrder);
   orderBook.insertOrder(aggressorOrder);

   ASSERT_EQ(listener.trades_.size(), 1);
   ASSERT_EQ(listener.trades_[0].quantity_, 300);

   Book snapshot = orderBook.book();
   ASSERT_FALSE(snapshot.asks_.empty());
   ASSERT_EQ(snapshot.asks_[0].quantity, 200);
}
