#pragma once

#include <cstdint>
#include <ostream>
#include <stdint.h>
#include <string>

#include <boost/lockfree/policies.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "eventpp/eventqueue.h"
#include "eventpp/utilities/orderedqueuelist.h"

constexpr size_t canobj_capacity = 256;
struct canobj_queue_node_t {
  friend std::ostream& operator<<(std::ostream& os, const canobj_queue_node_t& node) {
    os.write((const char*)(node.can_obj_), node.len_);
    return os;
  }
  uint8_t can_obj_[canobj_capacity];
  size_t len_{0};
};

template <typename CAPACITY>
using spsc_queue_t = boost::lockfree::spsc_queue<canobj_queue_node_t, CAPACITY, boost::lockfree::fixed_sized<true>>;

constexpr int ppq_can_obj_evt_id = 1;
using eventpp_queue_t = eventpp::EventQueue<int, void(const canobj_queue_node_t&)>;
using eventpp_queue_handle_t = eventpp_queue_t::Handle;