#pragma once
#include <unordered_set>
#include "orderbook.hpp"

class MarketMakerOrderTracker {};

class InventoryConsumer : DataConsummer {
   std::unordered_set<std::string> myOrdersIds_;
   std::int64_t position = 0;
   std::int64_t openBuyQty = 0;
   std::int64_t openSellQty = 0;

   double averagePrice = 0;
   double pnl = 0;

   void computeAveragePrice(std::uint32_t qty, std::uint32_t price) {}

   void updateInventory(const Trade& trade) {
      if (myOrdersIds_.find(trade.aggressor_.getId()) != myOrdersIds_.end()) {
         if (trade.aggressor_.getSide() == Order::BUY) {
            position += trade.quantity_;
         } else {
            position -= trade.quantity_;
         }
      } else if (myOrdersIds_.find(trade.opposite_.getId()) !=
                 myOrdersIds_.end()) {
         if (trade.opposite_.getSide() == Order::BUY) {
            position += trade.quantity_;
         } else {
            position -= trade.quantity_;
         }
      }
   }

   void computePnl() {}

  public:
   void consumeTrade(const Trade& trade) override {}
   void consumeOrder(const Order& order) override {
      if (order.isMyOrder()) {
         myOrdersIds_.emplace(order.getId());
      }
      if (order.getSide() == Order::BUY) {
         openBuyQty += 1;
      } else {
         openSellQty += 1;
      }
   }
};
