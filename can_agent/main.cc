#include <iostream>
#include <memory>
#include <vector>
#include <csignal>

#include "hex_dump.hpp"
#include "lib_control_can_imp.h"

#define BOOST_THREAD_USES_CHRONO
#include <boost/algorithm/string.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_unique.hpp>
#include <boost/thread.hpp>

constexpr DWORD devtype = 4;
constexpr DWORD devid = 0;
constexpr DWORD channel = 0;

namespace {
volatile std::sig_atomic_t gSignalStatus; 
}

//https://en.cppreference.com/w/cpp/utility/program/signal
void signal_handler(int signal) {
  gSignalStatus = signal;
}

int main(int argc, char **argv) {
  std::unique_ptr<CanImpInterface> can_dc(createCanDC());
  auto result = can_dc->VCI_OpenDevice(devtype, 0, 0);
  if (result != 1) {
    std::cout << "CAN DC VCI_OpenDevice fail." << std::endl;
    exit(1);
  }

  VCI_INIT_CONFIG cfg{};
  cfg.Timing0 = 0x00;
  cfg.Timing1 = 0x1C;
  cfg.Filter = 0;
  cfg.AccMask = 0xffffffff;
  cfg.AccCode = 0;
  cfg.Mode = 0;
  cfg.Reserved = 0;
  result = can_dc->VCI_InitCAN(devtype, devid, channel, &cfg);
  if (result != 1) {
    std::cout << "CAN DC VCI_InitCAN fail." << std::endl;
    exit(1);
  }

  std::signal(SIGINT, signal_handler);

  constexpr DWORD rec_buff_size = 100;
  VCI_CAN_OBJ can_recv_buff[rec_buff_size];
  result = can_dc->VCI_StartCAN(devtype, devid, channel);
  while (0 == gSignalStatus) {
    auto num = can_dc->VCI_GetReceiveNum(devtype, devid, channel);
    if (num > rec_buff_size) num = rec_buff_size;
    auto recv_frame_num = can_dc->VCI_Receive(devtype, devid, channel, can_recv_buff, num, 100);
    for (ULONG i = 0; i < recv_frame_num; i++) {
      std::string str = can::utils::bin2hex_dump(can_recv_buff[i].Data, 8);
      std::cout << "VCI_Receive: " << std::hex << can_recv_buff[i].ID << ", data: " << str << std::endl;
    }
  }

  result = can_dc->VCI_CloseDevice(devtype, 0);
  std::cout << "VCI_CloseDevice: " << (ULONG)result << std::endl;

  return 0;
}