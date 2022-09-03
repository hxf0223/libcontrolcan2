#pragma once

#include <chrono>
#include <cstdint>
#include <stdint.h>

#include "ticks/cycle.h"

namespace tick {

using namespace std::chrono;

struct tickExt {
private:
  uint64_t ticks_{0}, tm64_{0};
  double delta_ratio_{0.0};
  bool valid_tick_{false};

public:
  tickExt() { beginInitTick(); }
  int updateTick() { return endInitTick(); }
  uint64_t getTick() {
    if (!valid_tick_)
      return 0;

    uint64_t tick_diff = ::getticks() - ticks_;
    return (tm64_ + tick_diff * delta_ratio_);
  }

  void beginInitTick() {
    auto dur = system_clock::now().time_since_epoch();
    tm64_ = duration_cast<microseconds>(dur).count();
    ticks_ = ::getticks();
  }

  int endInitTick() {
    auto dur = system_clock::now().time_since_epoch();
    uint64_t tm64 = duration_cast<microseconds>(dur).count();
    uint64_t tick2 = ::getticks();

    if (tm64 < tm64_ || tick2 < ticks_) {
      valid_tick_ = false;
      return -1;
    }

    if ((tm64 - tm64_) < 10 || (tick2 - ticks_) < 1000) {
      valid_tick_ = false;
      return false;
    }

    double temp = tm64 - tm64_;
    delta_ratio_ = temp / (tick2 - ticks_);

    return 0;
  }
};

} // namespace tick