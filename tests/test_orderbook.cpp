#include <gtest/gtest.h>
#include <random>
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

   orderBookListener.attach(&listener);

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

   orderBookListener.attach(&listener);

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

TEST(OrderBookTest, HugeFillPartial) {
   TestListener listener;

   OrderBookListener orderBookListener;
   orderBookListener.attach(&listener);
   OrderBook orderBook{orderBookListener};

   long exchangeId = 123;

   Order restingOrder1(exchangeId, "1", 101, Order::SELL, Order::GoodTillCancel,
                       20);
   Order restingOrder2(exchangeId, "2", 102, Order::SELL, Order::GoodTillCancel,
                       90);
   Order aggressorOrder(exchangeId, "3", 103, Order::BUY, Order::GoodTillCancel,
                        100);

   orderBook.insertOrder(&restingOrder1);
   orderBook.insertOrder(&restingOrder2);
   orderBook.insertOrder(&aggressorOrder);

   Book snapshot = orderBook.book();
   ASSERT_FALSE(snapshot.asks_.empty());
   ASSERT_EQ(snapshot.asks_.size(), 1);
   ASSERT_EQ(snapshot.asks_[0].quantity, 10);
}

TEST(OrderBookTest, CancelOrder) {
   TestListener listener;

   OrderBookListener orderBookListener;
   orderBookListener.attach(&listener);
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
   orderBookListener.attach(&listener);
   OrderBook orderBook{orderBookListener};

   long exchangeId = 123;

   Order o1(exchangeId, "1", 100, Order::SELL, Order::GoodTillCancel, 500);
   Order o2(exchangeId, "1", 99, Order::SELL, Order::GoodTillCancel, 500);

   orderBook.insertOrder(&o1);
   orderBook.insertOrder(&o2);

   auto snapshot = orderBook.book();

   ASSERT_EQ(snapshot.asks_.size(), 2);
}

TEST(OrderBookTest, FIFOAtSamePrice) {
   TestListener listener;
   OrderBookListener obl;
   obl.attach(&listener);

   OrderBook book{obl};

   Order s1(1, "s1", 100, Order::SELL, Order::GoodTillCancel, 100);
   Order s2(1, "s2", 100, Order::SELL, Order::GoodTillCancel, 100);
   Order b1(1, "b1", 100, Order::BUY, Order::GoodTillCancel, 150);

   book.insertOrder(&s1);
   book.insertOrder(&s2);
   book.insertOrder(&b1);

   ASSERT_EQ(listener.trades_.size(), 2);
   ASSERT_EQ(listener.trades_[0].quantity_, 100);
   ASSERT_EQ(listener.trades_[1].quantity_, 50);
}

TEST(OrderBookTest, NoCrossNoTrade) {
   TestListener listener;
   OrderBookListener obl;
   obl.attach(&listener);

   OrderBook book{obl};

   Order sell(1, "s", 105, Order::SELL, Order::GoodTillCancel, 100);
   Order buy(1, "b", 100, Order::BUY, Order::GoodTillCancel, 100);

   book.insertOrder(&sell);
   book.insertOrder(&buy);

   ASSERT_TRUE(listener.trades_.empty());
}

TEST(OrderBookTest, SweepMultipleLevels) {
   TestListener listener;
   OrderBookListener obl;
   obl.attach(&listener);

   OrderBook book{obl};

   Order s1(1, "s1", 100, Order::SELL, Order::GoodTillCancel, 50);
   Order s2(1, "s2", 101, Order::SELL, Order::GoodTillCancel, 50);
   Order s3(1, "s3", 102, Order::SELL, Order::GoodTillCancel, 50);
   Order b(1, "b", 103, Order::BUY, Order::GoodTillCancel, 120);

   book.insertOrder(&s1);
   book.insertOrder(&s2);
   book.insertOrder(&s3);
   book.insertOrder(&b);

   ASSERT_EQ(listener.trades_.size(), 3);
   ASSERT_EQ(book.book().asks_[0].price, 102);
   ASSERT_EQ(book.book().asks_[0].quantity, 30);
}

TEST(OrderBookTest, CancelAfterPartialFill) {
   TestListener listener;
   OrderBookListener obl;
   obl.attach(&listener);

   OrderBook book{obl};

   Order s(1, "s", 100, Order::SELL, Order::GoodTillCancel, 100);
   Order b(1, "b", 100, Order::BUY, Order::GoodTillCancel, 40);

   book.insertOrder(&s);
   book.insertOrder(&b);
   book.cancelOrder(&s);

   ASSERT_TRUE(book.book().asks_.empty());
}

TEST(OrderBookTest, CancelNonExistingOrder) {
   TestListener listener;
   OrderBookListener obl;
   obl.attach(&listener);

   OrderBook book{obl};

   Order s(1, "s", 100, Order::SELL, Order::GoodTillCancel, 100);
   Order b(1, "b", 100, Order::BUY, Order::GoodTillCancel, 100);

   book.insertOrder(&s);
   book.insertOrder(&b);

   EXPECT_THROW(book.cancelOrder(&b), std::runtime_error);
}

TEST(OrderBookTest, BookIsSorted) {
   TestListener listener;
   OrderBookListener obl;
   obl.attach(&listener);

   OrderBook book{obl};
   auto snap = book.book();

   for (size_t i = 1; i < snap.asks_.size(); ++i)
      ASSERT_LT(snap.asks_[i - 1].price, snap.asks_[i].price);

   for (size_t i = 1; i < snap.bids_.size(); ++i)
      ASSERT_GT(snap.bids_[i - 1].price, snap.bids_[i].price);
}

TEST(OrderBookTest, ZeroQuantityIgnored) {
   TestListener listener;
   OrderBookListener obl;
   obl.attach(&listener);

   OrderBook book{obl};
   Order o(1, "bad", 100, Order::BUY, Order::GoodTillCancel, 0);
   book.insertOrder(&o);
   ASSERT_TRUE(book.book().bids_.empty());
}

TEST(OrderBookTest, Insert100kRandomOrders) {
   TestListener listener;
   OrderBookListener obl;
   obl.attach(&listener);
   OrderBook book{obl};

   std::mt19937 rng(42);
   std::uniform_int_distribution<int> sideDist(0, 1);
   std::uniform_int_distribution<int> priceDist(90, 110);
   std::uniform_int_distribution<int> qtyDist(1, 100);

   constexpr int N = 100000;

   std::vector<Order> orders;
   orders.reserve(N);

   for (size_t i{}; i < N; ++i) {
      orders.emplace_back(1, std::to_string(i), priceDist(rng),
                          sideDist(rng) ? Order::SELL : Order::BUY,
                          Order::GoodTillCancel, qtyDist(rng));
   }

   for (Order& o : orders) {
      book.insertOrder(&o);
   }

   auto snap = book.book();

   ASSERT_TRUE(snap.bids_.empty() || snap.asks_.empty() ||
               snap.bids_[0].price < snap.asks_[0].price);
}