#pragma once
#include "orderbook.hpp"

class CustomListener : OrderBookListener {
   std::vector<Trade> trades_;
   std::uint32_t midPrice_;
   std::uint32_t inventory;

   void updateInventory(const Trade& trade) {
      if (trade.aggressor_.getSide() == Order::BUY) {
         inventory += trade.quantity_;
      } else {
         inventory -= trade.quantity_;
      }
   }

  public:
   void onTrade(const Trade& trade) override {
      trades_.push_back(trade);
      updateInventory(trade);
   }
};