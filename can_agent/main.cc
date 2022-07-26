#include <atomic>
#include <csignal>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <zmp.h>

#include "hex_dump.hpp"
#include "lib_control_can.h"

#include "boost/asio.hpp"
#include "server/server.h"

constexpr DWORD devtype = 4;
constexpr DWORD devid = 0;
constexpr DWORD channel = 0;
constexpr short lisenPort = 9999;

namespace {

inline VCI_INIT_CONFIG create_vci_init_cfg(UCHAR timing0, UCHAR timing1, DWORD mask) {
  VCI_INIT_CONFIG cfg{};
  cfg.Timing0 = timing0;
  cfg.Timing1 = timing1;
  cfg.AccMask = mask;
  return cfg;
}

} // namespace

static void canReceiveThread(std::atomic_bool &runFlag) {
  auto e = VCI_OpenDevice(devtype, 0, 0);
  auto cfg = create_vci_init_cfg(0x00, 0x1C, 0xffffffff);
  e = VCI_InitCAN(devtype, devid, channel, &cfg);
  e = VCI_StartCAN(devtype, devid, channel);

  constexpr DWORD rec_buff_size = 100;
  VCI_CAN_OBJ can_recv_buff[rec_buff_size];
  while (runFlag.load()) {
    auto recv_frame_num = VCI_Receive(devtype, devid, channel, can_recv_buff, rec_buff_size, 10);
  }
}

int main(int argc, char **argv) {
  try {
    boost::asio::io_context io_context;
    Server s = Server(io_context, lisenPort);
    s.startAccepting();
    io_context.run();
  } catch (boost::system::system_error &ec) {
    std::cout << ec.what() << std::endl;
  }

  return 0;
}