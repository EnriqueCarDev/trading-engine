#include <gtest/gtest.h>
#include "orderbook_layers/order.hpp"

TEST(OrderTest, OrderCreation) {
   long exchangeId = 123;
   Order order(exchangeId, "1", 100, Order::BUY, Order::FillAndKill, 10);

   ASSERT_EQ(order.getId(), "1");
   ASSERT_EQ(order.getPrice(), 100);
   ASSERT_EQ(order.getInitialQuantity(), 10);
   ASSERT_EQ(order.getSide(), Order::BUY);
   ASSERT_EQ(order.getType(), Order::FillAndKill);
}