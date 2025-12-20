#pragma once
#include <functional>
#include <iostream>
#include <map>
#include <vector>
#include "pricelevels.hpp"

struct Trade {
   friend class OrderBook;

  private:
   Trade(std::uint32_t price, std::uint32_t quantity, const Order& aggressor,
         const Order& opposite)
       : price_(price),
         quantity_(quantity),
         aggressor_(aggressor),
         opposite_(opposite) {}

  public:
   const std::uint32_t price_;
   const std::uint32_t quantity_;
   const Order& aggressor_;
   const Order& opposite_;
};

class OrderBookListener {
  public:
   virtual void onOrder(const Order& order) = 0;
   virtual void onTrade(const Trade& trade) = 0;
   virtual ~OrderBookListener() = default;
};

struct BookLevel {
   std::uint32_t price;
   std::uint32_t quantity;
};

struct Book {
   std::vector<BookLevel> bids_;
   std::vector<long> bidOrderIds;
   std::vector<BookLevel> asks_;
   std::vector<long> askOrderIds;
};

struct QuoteOrders {
   Order* bid_;
   Order* ask_;
};

struct SessionQuoteId {
   const std::string sessionId_;
   const std::string quoteId_;

   SessionQuoteId(const std::string& sessionId, const std::string_view& quoteId)
       : sessionId_(sessionId), quoteId_(quoteId) {}

   bool operator==(const SessionQuoteId& other) const {
      return sessionId_ == other.sessionId_ && quoteId_ == other.quoteId_;
   }

   bool operator<(const SessionQuoteId& other) const {
      return sessionId_ < other.sessionId_ ||
             (sessionId_ == other.sessionId_ && quoteId_ < other.quoteId_);
   }
};

class OrderBook {
   PriceLevels<std::vector<OrderList>> bids_ =
       PriceLevels<std::vector<OrderList>>(false);
   PriceLevels<std::vector<OrderList>> asks_ =
       PriceLevels<std::vector<OrderList>>(true);
   OrderBookListener& listener_;
   std::map<SessionQuoteId, QuoteOrders> quotes_;

   void matchOrders(Order::Side side);

  public:
   OrderBook(OrderBookListener& listener) : listener_(listener) {}

   const Book book();

   void insertOrder(Order* order);
   void cancelOrder(Order* order);
   QuoteOrders getQuotes(const std::string& session, const std::string& quoteId,
                         std::function<QuoteOrders()> createOrders);
   void quote(const QuoteOrders& quotes, std::uint32_t bidPrice,
              std::uint32_t bidQuantity, std::uint32_t askPrice,
              std::uint32_t askQuantity);
   const Order getOrder(Order* order);
};