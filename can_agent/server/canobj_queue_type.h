#pragma once

#include <cstdint>
#include <ostream>
#include <string>

#include <boost/lockfree/policies.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "eventpp/eventqueue.h"
#include "eventpp/utilities/orderedqueuelist.h"

constexpr size_t kCanobjCapacity = 256;
struct CanobjQueueNodeT {
  friend std::ostream& operator<<(std::ostream& os, const CanobjQueueNodeT& node) {
    os.write((const char*)(node.can_obj_), node.len_);
    return os;
  }
  uint8_t can_obj_[kCanobjCapacity];
  size_t len_{0};
};

template <typename CAPACITY>
using spsc_queue_t = boost::lockfree::spsc_queue<CanobjQueueNodeT, CAPACITY, boost::lockfree::fixed_sized<true>>;

constexpr int kPpqCanObjEvtId = 1;
using eventpp_queue_t = eventpp::EventQueue<int, void(const CanobjQueueNodeT&)>;
using eventpp_queue_handle_t = eventpp_queue_t::Handle;