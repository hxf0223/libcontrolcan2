#pragma once

#include "usbcan.h"

namespace test::helper {

static inline VCI_INIT_CONFIG createVciInitCfg(UCHAR timing0, UCHAR timing1, DWORD mask) {
  VCI_INIT_CONFIG cfg{};
  cfg.Timing0 = timing0;
  cfg.Timing1 = timing1;
  cfg.AccMask = mask;
  return cfg;
}

} // namespace test::helper