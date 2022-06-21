#include "hex_dump.hpp"
#include "lib_control_can_imp.h"
#include "test_helper.hpp"

#include "glog/logging.h"
#include "gtest/gtest.h"

constexpr DWORD devtype = 4;
constexpr DWORD devid = 0;
constexpr DWORD channel = 0;

TEST(CANDC, load) {
  std::shared_ptr<CanImpInterface> can_dc(createCanDC());
  auto result = can_dc->VCI_OpenDevice(devtype, 0, 0);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_OpenDevice fail: " << result;

  auto cfg = test::helper::create_vci_init_cfg(0x00, 0x1C, 0xffffffff);
  result = can_dc->VCI_InitCAN(devtype, devid, channel, &cfg);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_InitCAN fail: " << result;
}
