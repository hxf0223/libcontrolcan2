#include <iostream>
#include <memory>
#include <vector>

#include "hex_dump.hpp"
#include "lib_control_can.h"
#include "test_helper.hpp"

#define BOOST_THREAD_USES_CHRONO
#include <boost/algorithm/string.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_unique.hpp>
#include <boost/thread.hpp>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace {
constexpr DWORD kDevtype = 4;
constexpr DWORD kDevid = 0;
constexpr DWORD kChannel = 0;
} // namespace

TEST(CAN, F001_P0) {
  auto result = VCI_OpenDevice(kDevtype, 0, 0);
  CHECK(1 == result) << "VCI_OpenDevice fail: " << result;

  auto cfg = test::helper::createVciInitCfg(0x00, 0x1C, 0xffffffff);
  result = VCI_InitCAN(kDevtype, kDevid, kChannel, &cfg);
  result = VCI_StartCAN(kDevtype, kDevid, kChannel);

  VCI_CAN_OBJ can_obj;
  uint8_t send_data[] = {0x03, 0x22, 0x28, 0x00, 0xAA, 0xAA, 0xAA, 0xAA};
  memset(&can_obj, 0, sizeof(VCI_CAN_OBJ));
  memcpy(can_obj.Data, send_data, 8);

  can_obj.ID = (0x102) & (~0x80000000);
  can_obj.DataLen = 8;
  can_obj.RemoteFlag = 0;
  can_obj.ExternFlag = (0x102 & 0x80000000) != 0U ? 1 : 0;
  can_obj.TimeFlag = 0;
  can_obj.SendType = 0;
  can_obj.TimeStamp = 0;

  VCI_CAN_OBJ can_recv_buff[10];

  for (int i = 0; i < 4000; i++, can_obj.Data[2]++) {
    result = VCI_Transmit(kDevtype, kDevid, kChannel, &can_obj, 1);
    auto len = VCI_Receive(kDevtype, kDevid, kChannel, can_recv_buff, 1, 100);
    if (len > 0) {
      std::string const str =
          can::utils::bin2hex_dump(can_recv_buff[0].Data, 8);
      LOG(INFO) << "VCI_Receive: " << std::setfill('0') << std::hex
                << std::setw(8) << can_recv_buff[0].ID << ", data: " << str;
    } else {
      LOG(WARNING) << "VCI_Receive fail.";
    }
  }

  result = VCI_CloseDevice(kDevtype, 0);
  CHECK(1 == result) << "VCI_CloseDevice fail: " << result;
}

TEST(CAN, F002_P0) {
  auto result = VCI_OpenDevice(kDevtype, 0, 0);
  CHECK(1 == result) << "VCI_OpenDevice fail: " << result;

  auto cfg = test::helper::createVciInitCfg(0x00, 0x1C, 0xffffffff);
  result = VCI_InitCAN(kDevtype, kDevid, kChannel, &cfg);

  VCI_CAN_OBJ can_recv_buff[100];
  result = VCI_StartCAN(kDevtype, kDevid, kChannel);

  LOG(INFO) << std::setfill('0') << std::setw(8);
  for (int i = 0; i < 4000; i++) {
    auto len = VCI_Receive(kDevtype, kDevid, kChannel, can_recv_buff, 1, 100);
    if (len <= 0) {
      continue;
    }

    std::string const str = can::utils::bin2hex_dump(can_recv_buff[0].Data, 8);
    LOG(INFO) << "VCI_Receive: " << std::hex << can_recv_buff[0].ID
              << ", data: " << str;
  }

  result = VCI_CloseDevice(kDevtype, 0);
  CHECK(1 == result) << "VCI_CloseDevice fail: " << result;
}

int main(int argc, char **argv) {
  FLAGS_alsologtostderr = true;
  FLAGS_colorlogtostderr = true;
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);
  int const ret = RUN_ALL_TESTS();
  return ret;
}