#include <gtest/gtest.h>
#include "orderbook_layers/orderbook.hpp"

class TestListener : public DataConsummer {
  public:
   std::vector<Order> orders_;
   std::vector<Trade> trades_;

   void consumeOrder(const Order& order) override { orders_.push_back(order); }
   void consumeTrade(const Trade& trade) override { trades_.push_back(trade); }
};

TEST(OrderBookTest, SingleFullFill) {
   TestListener listener;

   OrderBookListener orderBookListener;

   OrderBook orderBook{orderBookListener};

   long exchangeId = 123;

   Order restingOrder(exchangeId, "1", 100, Order::SELL, Order::GoodTillCancel,
                      500);
   Order aggressorOrder(exchangeId, "2", 100, Order::BUY, Order::GoodTillCancel,
                        500);

   orderBook.insertOrder(&restingOrder);
   orderBook.insertOrder(&aggressorOrder);

   ASSERT_EQ(listener.trades_.size(), 1);
   ASSERT_EQ(listener.trades_[0].price_, 100);
   ASSERT_EQ(listener.trades_[0].quantity_, 500);

   Book snapshot = orderBook.book();
   ASSERT_TRUE(snapshot.bids_.empty());
   ASSERT_TRUE(snapshot.asks_.empty());
}

TEST(OrderBookTest, PartialFill) {
   TestListener listener;

   OrderBookListener orderBookListener;

   OrderBook orderBook{orderBookListener};

   long exchangeId = 123;

   Order restingOrder(exchangeId, "1", 100, Order::SELL, Order::GoodTillCancel,
                      500);
   Order aggressorOrder(exchangeId, "2", 100, Order::BUY, Order::GoodTillCancel,
                        300);

   orderBook.insertOrder(&restingOrder);
   orderBook.insertOrder(&aggressorOrder);

   ASSERT_EQ(listener.trades_.size(), 1);
   ASSERT_EQ(listener.trades_[0].quantity_, 300);

   Book snapshot = orderBook.book();
   ASSERT_FALSE(snapshot.asks_.empty());
   ASSERT_EQ(snapshot.asks_[0].quantity, 200);
}

TEST(OrderBookTest, CancelOrder) {
   TestListener listener;

   OrderBookListener orderBookListener;

   OrderBook orderBook{orderBookListener};

   long exchangeId = 123;

   Order order(exchangeId, "1", 100, Order::SELL, Order::GoodTillCancel, 500);

   orderBook.insertOrder(&order);
   Book snap1 = orderBook.book();
   ASSERT_EQ(snap1.asks_.size(), 1);
   orderBook.cancelOrder(&order);
   Book snap2 = orderBook.book();
   ASSERT_EQ(snap2.asks_.size(), 0);
}

TEST(OrderBookTest, BookLevels) {
   TestListener listener;

   OrderBookListener orderBookListener;

   OrderBook orderBook{orderBookListener};

   long exchangeId = 123;

   Order o1(exchangeId, "1", 100, Order::SELL, Order::GoodTillCancel, 500);
   Order o2(exchangeId, "1", 99, Order::SELL, Order::GoodTillCancel, 500);

   orderBook.insertOrder(&o1);
   orderBook.insertOrder(&o2);

   auto snapshot = orderBook.book();

   ASSERT_EQ(snapshot.asks_.size(), 2);
}