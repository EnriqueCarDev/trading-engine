#pragma once
#include "orderbook.hpp"

class InventoryConsumer : DataConsummer {
   std::uint64_t position = 0;
   std::uint64_t openBuyQty = 0;
   std::uint64_t openSellQty = 0;

   double averagePrice = 0;
   double pnl = 0;

   void computeAveragePrice(std::uint32_t qty, std::uint32_t price) {}

   void updateInventory() {}

   void computePnl() {}

  public:
   void consumeTrade(const Trade& trade) override {}
   void consumeOrder(const Order& order) override {}
};
