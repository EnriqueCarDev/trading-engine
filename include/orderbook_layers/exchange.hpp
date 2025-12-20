#pragma once

#include "order.hpp"
#include "orderbook.hpp"

struct ExchangeListener {
   virtual void onTrade(const Trade& order) {};
   virtual void onOrder(const Order& order) {};
};

class Exchange : OrderBookListener {};