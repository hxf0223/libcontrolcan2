#include <memory>

#include "hex_dump.hpp"
#include "lib_control_can_imp.h"
#include "test_helper.hpp"

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace {
constexpr DWORD kDevtype = 4;
constexpr DWORD kDevid = 0;
constexpr DWORD kChannel = 0;
} // namespace

TEST(CANDC, load) {
  std::shared_ptr<CanImpInterface> const can_dc(createCanDC());
  auto result = can_dc->VCI_OpenDevice(kDevtype, 0, 0);
  LOG(INFO) << "CAN DC VCI_OpenDevice return: " << result;
  /* CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_OpenDevice fail:
  " << result;

  auto cfg = test::helper::create_vci_init_cfg(0x00, 0x1C, 0xffffffff);
  result = can_dc->VCI_InitCAN(devtype, devid, channel, &cfg);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_InitCAN fail: " <<
  result;*/
}

TEST(CANDC, receive) {
  std::shared_ptr<CanImpInterface> const can_dc(createCanDC());
  auto result = can_dc->VCI_OpenDevice(kDevtype, 0, 0);
  CHECK(result == vciReturnType::STATUS_OK)
      << "CAN NET VCI_OpenDevice fail: " << result;

  auto cfg = test::helper::createVciInitCfg(0x00, 0x1C, 0xffffffff);
  result = can_dc->VCI_InitCAN(kDevtype, kDevid, kChannel, &cfg);
  CHECK(result == vciReturnType::STATUS_OK)
      << "CAN NET VCI_InitCAN fail: " << result;

  result = can_dc->VCI_StartCAN(kDevtype, kDevid, kChannel);
  CHECK(result == vciReturnType::STATUS_OK)
      << "CAN NET VCI_StartCAN fail: " << result;

  for (size_t i = 0; i < 1000; i++) {
    VCI_CAN_OBJ obj{};
    auto len = can_dc->VCI_Receive(kDevtype, kDevid, kChannel, &obj, 1, 100);
    if (len == 0) {
      continue;
    }

    std::string const recv_str = can::utils::bin2hex_dump(obj.Data, 8);
    LOG(INFO) << "received: " << recv_str;
  }

  result = can_dc->VCI_CloseDevice(kDevtype, kDevid);
  CHECK(result == vciReturnType::STATUS_OK)
      << "CAN DC VCI_CloseDevice fail: " << result;
}