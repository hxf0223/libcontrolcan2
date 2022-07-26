#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "hex_dump.hpp"
#include "lib_control_can.h"

#include "boost/asio.hpp"
#include "server/server.h"
#include "zmq.hpp"

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

// https://en.cppreference.com/w/cpp/string/basic_string_view/basic_string_view
static void canReceiveThread(std::atomic_bool &runFlag, zmq::context_t *ctx) {
  auto e = VCI_OpenDevice(devtype, 0, 0);
  auto cfg = create_vci_init_cfg(0x00, 0x1C, 0xffffffff);
  e = VCI_InitCAN(devtype, devid, channel, &cfg);
  e = VCI_StartCAN(devtype, devid, channel);

  zmq::socket_t publisher(*ctx, zmq::socket_type::pub);
  publisher.bind("inproc://#1");

  constexpr DWORD rec_buff_size = 100;
  VCI_CAN_OBJ can_recv_buff[rec_buff_size];
  const char *cmd_recv = "VCI_Receive,";
  uint64_t send_count = 0;
  char send_buff[256];

  while (runFlag.load()) {
    auto recv_frame_num = VCI_Receive(devtype, devid, channel, can_recv_buff, rec_buff_size, 10);
    for (ULONG i = 0; i < rec_buff_size; i++) {
      auto dur = std::chrono::system_clock::now().time_since_epoch();
      uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();

      auto &can_obj = can_recv_buff[i];
      auto size = can::utils::bin2hex::bin2hex_fast(send_buff, cmd_recv, &send_count, &now, &can_obj, "\n");
      std::string_view sv{send_buff};
      auto sb = zmq::str_buffer(sv);

      send_count++;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
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