#pragma once

#include "order.hpp"

class OrderList {
   friend class OrderBook;

  private:
   Node* head = nullptr;
   Node* tail = nullptr;
   std::uint32_t price_;

  public:
   OrderList(std::uint32_t price) : price_{price} {}
   const std::uint32_t getPrice() const { return price_; }
   struct Iterator {
      friend class OrderList;
      using iterator_category = std::forward_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = Order*;
      using reference = value_type&;
      value_type operator*() const { return current->order; }
      Iterator& operator++() {
         current = current->next;
         return *this;
      }
      bool operator==(const Iterator& other) const {
         return current == other.current;
      }
      explicit operator bool() const { return current != nullptr; }

     private:
      Iterator(Node* node) : current(node) {}
      Node* current;
   };

   void pushBack(Order* const order) {
      auto node = &order->node;
      node->order = order;
      if (head == nullptr) {
         head = node;
         tail = node;
      } else {
         node->prev = tail;
         tail->next = node;
         tail = node;
      }
   }

   void remove(Order* const order) {
      if (order->node.order == nullptr)
         throw std::runtime_error("node does not contain an order on removal");
      auto node = &order->node;
      order->node.order = nullptr;
      if (head == node) {
         head = node->next;
      }
      if (tail == node) {
         tail = node->prev;
      }

      if (node->prev) {
         node->prev->next = node->next;
      }

      if (node->next) {
         node->next->prev = node->prev;
      }
   }

   Order* front() const { return head == nullptr ? nullptr : head->order; }
   Iterator begin() const { return Iterator(head); }
   Iterator end() const { return Iterator(nullptr); }
};