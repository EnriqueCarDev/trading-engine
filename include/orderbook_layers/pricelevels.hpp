#pragma once

#include <algorithm>
#include <concepts>
#include "order.hpp"
#include "orderlist.hpp"

struct PriceCompare {
   const bool ascending_;
   explicit PriceCompare(bool ascending) : ascending_(ascending) {}
   template <typename T, typename U>
   inline bool operator()(const T& t, const U& u) const {
      return (ascending_) ? t.getPrice() < u : t.getPrice() > u;
   }
};

template <typename Container>
concept ContainerOfStruct = requires(Container c) {
   typename Container::value_type;
   requires(std::same_as<typename Container::value_type, OrderList>);
};

template <typename ContainerOfStruct>
class PriceLevels {
   const PriceCompare compFn;
   ContainerOfStruct levels;

  public:
   PriceLevels(bool ascending) : compFn(ascending) {}
   void insertOrder(Order* order) {
      auto it = std::lower_bound(levels.begin(), levels.end(),
                                 order->getPrice(), compFn);
      if (it == levels.end() || it->getPrice() != order->getPrice()) {
         OrderList list(order->getPrice());
         list.pushBack(order);
         levels.insert(it, std::move(list));
      } else {
         it->pushBack(order);
      }
   };

   void removeOrder(Order* order) {
      auto it = std::lower_bound(levels.begin(), levels.end(),
                                 order->getPrice(), compFn);
      if (it == levels.end() || it->getPrice() != order->getPrice())
         throw std::runtime_error("Price level for order does not exist");

      it->remove(order);

      if (it->front() == nullptr) levels.erase(it);
   }

   Order* front() const {
      auto it = levels.begin();
      if (it == levels.end()) return nullptr;
      return it->front();
   }

   void forEach(std::function<void(const OrderList*)> fn) const {
      for (auto it = levels.begin(); it != levels.end(); ++it) {
         fn(&(*it));
      }
   }

   bool empty() const { return levels.empty(); }
   int size() const { return levels.size(); };
};
