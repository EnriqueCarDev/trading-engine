#include "orderbook_layers/orderbook.hpp"

void OrderBook::matchOrders(Order::Side aggressorSide) {
   while (!bids_.empty() && !asks_.empty()) {
      Order* bestBid = bids_.front();
      Order* bestAsk = asks_.front();

      if (bestBid->price_ >= bestAsk->price_) {
         std::uint32_t quantity = std::min(bestBid->getRemainingQuantity(),
                                           bestAsk->getRemainingQuantity());
         std::uint32_t price = std::min(bestBid->price_, bestAsk->price_);

         Order* aggressor = aggressorSide == Order::BUY ? bestBid : bestAsk;
         Order* opposite = aggressorSide == Order::BUY ? bestAsk : bestBid;

         aggressor->fillOrder(quantity);
         opposite->fillOrder(quantity);

         const Trade trade(price, quantity, *aggressor, *opposite);

         if (bestBid->remaining_quantity_ == 0) bids_.removeOrder(bestBid);
         if (bestAsk->remaining_quantity_ == 0) asks_.removeOrder(bestAsk);

         listener_.onOrder(*bestBid);
         listener_.onOrder(*bestAsk);
         listener_.onTrade(trade);
      } else {
         break;
      }
   }

   auto orders = aggressorSide == Order::BUY ? bids_ : asks_;

   if (!orders.empty()) {
      Order* order = orders.front();
      if (order->isMarket()) {
         order->cancel();
         orders.removeOrder(order);
         listener_.onOrder(*order);
      }
   }
}

void OrderBook::insertOrder(Order* order) {
   if (order->getSide() == Order::BUY) {
      bids_.insertOrder(order);
   } else {
      asks_.insertOrder(order);
   }
   listener_.onOrder(*order);

   matchOrders(order->getSide());
}

void OrderBook::cancelOrder(Order* order) {
   if (order->getSide() == Order::BUY) {
      bids_.removeOrder(order);
   } else {
      asks_.removeOrder(order);
   }
}

void OrderBook::quote(const QuoteOrders& quotes, std::uint32_t bidPrice,
                      std::uint32_t bidQuantity, std::uint32_t askPrice,
                      std::uint32_t askQuantity) {
   Order* bid = quotes.bid_;
   Order* ask = quotes.ask_;

   if (bid->isOnList()) bids_.removeOrder(bid);
   if (ask->isOnList()) asks_.removeOrder(ask);

   if (bid->remaining_quantity_ != 0) {
      bid->price_ = bidPrice;
      bid->initial_quantity_ = bidQuantity;
      bid->remaining_quantity_ = bidQuantity;
      bid->filled_ = 0;
      bids_.insertOrder(bid);
      matchOrders(Order::BUY);
   }
   if (ask->remaining_quantity_ != 0) {
      ask->price_ = askPrice;
      ask->initial_quantity_ = askQuantity;
      ask->remaining_quantity_ = askQuantity;
      ask->filled_ = 0;
      asks_.insertOrder(ask);
      matchOrders(Order::SELL);
   }
}

QuoteOrders OrderBook::getQuotes(const std::string& sessionId,
                                 const std::string& quoteId,
                                 std::function<QuoteOrders()> createOrders) {
   SessionQuoteId key = SessionQuoteId(sessionId, quoteId);
   auto it = quotes_.find(key);
   if (it == quotes_.end()) {
      return quotes_[key] = createOrders();
   } else {
      return it->second;
   }
}

const Book OrderBook::book() {
   Book book;

   auto fillBook = [](const PriceLevels<std::vector<OrderList>>& priceLevels,
                      std::vector<BookLevel>& bookLevels,
                      std::vector<long>& orderIds) {
      auto cumSum = [&](const OrderList* orders) {
         std::uint32_t quantity(0);
         for (auto it = orders->begin(); it != orders->end(); ++it) {
            quantity = quantity + (*it)->remaining_quantity_;
            orderIds.push_back((*it)->exchangeId_);
         }
         bookLevels.emplace_back(BookLevel{orders->getPrice(), quantity});
      };
      priceLevels.forEach(cumSum);
   };
   book.bids_.reserve(bids_.size());
   book.asks_.reserve(asks_.size());
   fillBook(bids_, book.bids_, book.bidOrderIds);
   fillBook(asks_, book.asks_, book.askOrderIds);
   return book;
}
