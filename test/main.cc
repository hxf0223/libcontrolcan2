#include <Windows.h>

#include <iostream>
#include <memory>
#include <vector>

#include "hex_dump.hpp"
#include "lib_control_can.h"

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

#define BOOST_DATE_TIME_SOURCE
#define BOOST_THREAD_NO_LIB

constexpr DWORD devtype = 4;
constexpr DWORD devid = 0;
constexpr DWORD channel = 0;

TEST(CAN, F001_P0) {
  auto result = VCI_OpenDevice(devtype, 0, 0);
  CHECK(0 == result) << "VCI_OpenDevice fail: " << result;

  VCI_INIT_CONFIG cfg{};
  cfg.Timing0 = 0x00;
  cfg.Timing1 = 0x1C;
  cfg.Filter = 0;
  cfg.AccMask = 0xffffffff;
  cfg.AccCode = 0;
  cfg.Mode = 0;
  cfg.Reserved = 0;

  result = VCI_InitCAN(devtype, devid, channel, &cfg);
  result = VCI_StartCAN(devtype, devid, channel);

  VCI_CAN_OBJ can_obj;
  uint8_t send_data[] = {0x03, 0x22, 0x28, 0x00, 0xAA, 0xAA, 0xAA, 0xAA};
  memset(&can_obj, 0, sizeof(VCI_CAN_OBJ));
  memcpy(can_obj.Data, send_data, 8);

  can_obj.ID = (0x102) & (~0x80000000);
  can_obj.DataLen = 8;
  can_obj.RemoteFlag = 0;
  can_obj.ExternFlag = (0x102 & 0x80000000) ? 1 : 0;
  can_obj.TimeFlag = 0;
  can_obj.SendType = 0;
  can_obj.TimeStamp = 0;

  VCI_CAN_OBJ can_recv_buff[10];
  const auto start = boost::chrono::high_resolution_clock::now();
  boost::chrono::microseconds us;

  for (int i = 0; i < 4000; i++, can_obj.Data[2]++) {
    result = VCI_Transmit(devtype, devid, channel, &can_obj, 1);
    auto len = VCI_Receive(devtype, devid, channel, can_recv_buff, 1, 100);
    if (len > 0) {
      std::string str = can::utils::bin2hex_dump(can_recv_buff[0].Data, 8);
      LOG(INFO) << "VCI_Receive: " << std::setfill('0') << std::hex << std::setw(8) << can_recv_buff[0].ID
                << ", data: " << str;
    } else {
      LOG(WARNING) << "VCI_Receive fail.";
    }
  }

  result = VCI_CloseDevice(devtype, 0);
  CHECK(0 == result) << "VCI_CloseDevice fail: " << result;
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}