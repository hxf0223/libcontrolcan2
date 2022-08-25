#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <signal.h>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "glog/logging.h"
#include "hex_dump.hpp"
#include "lib_control_can.h"

#include "boost/asio.hpp"
#include "server/server.h"

//#include "zmq.hpp"
//#include "zmq_addon.hpp"

constexpr DWORD devtype = 4;
constexpr DWORD devid = 0;
constexpr DWORD channel = 0;
constexpr short lisenPort = 9999;

namespace {

template <typename T>
std::ostream &operator<<(std::ostream &os, std::optional<T> const &opt) {
  return opt ? os << opt.value() : os;
}

inline VCI_INIT_CONFIG create_vci_init_cfg(UCHAR timing0, UCHAR timing1, DWORD mask) {
  VCI_INIT_CONFIG cfg{};
  cfg.Timing0 = timing0;
  cfg.Timing1 = timing1;
  cfg.AccMask = mask;
  return cfg;
}

} // namespace

static void can_rx_func(std::atomic_bool *runFlag) {
  auto e = VCI_OpenDevice(devtype, 0, 0);
  auto cfg = create_vci_init_cfg(0x00, 0x1C, 0xffffffff);
  e = VCI_InitCAN(devtype, devid, channel, &cfg);
  e = VCI_StartCAN(devtype, devid, channel);

  // Give the subscribers a chance to connect, so they don't lose any messages
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  constexpr DWORD rec_buff_size = 100;
  VCI_CAN_OBJ can_recv_buff[rec_buff_size];
  const char *cmd_recv = "VCI_Receive,";
  uint64_t send_count = 0;
  char send_buff[256];

  while (runFlag->load()) {
    auto recv_frame_num = VCI_Receive(devtype, devid, channel, can_recv_buff, rec_buff_size, 10);
    for (ULONG i = 0; i < rec_buff_size; i++) {
      auto dur = std::chrono::system_clock::now().time_since_epoch();
      uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();

      auto &can_obj = can_recv_buff[i];
      auto size = can::utils::bin2hex::bin2hex_fast(send_buff, cmd_recv, &send_count, &now, &can_obj, "\n");
      // zmq::send_result_t rc = publisher.send(zmq::buffer(std::string_view(send_buff, size)));
      // std::cout << "publib send result: " << rc << std::endl;

      send_count++;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  std::cout << "can_rx_func exit." << std::endl;
}

static void pub_simu_func(std::atomic_bool *runFlag) {
  const char *cmd_recv = "VCI_Receive,";
  uint64_t send_count = 0;
  char send_buff[256];

  // Give the subscribers a chance to connect, so they don't lose any messages
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  while (runFlag->load()) {
    auto dur = std::chrono::system_clock::now().time_since_epoch();
    uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();

    VCI_CAN_OBJ can_obj{};
    *(uint64_t *)(can_obj.Data) = send_count;

    auto size = can::utils::bin2hex::bin2hex_fast(send_buff, cmd_recv, &send_count, &now, &can_obj, "\n");
    // zmq::send_result_t rc = publisher.send(zmq::buffer(std::string_view(send_buff, size)));
    // std::cout << "publib send result: " << rc << std::endl;
    send_count++;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  std::cout << "pub_simu_func exit." << std::endl;
}

std::atomic_bool run_flag{true};
static void signal_hander(int sig) { run_flag.store(false); }

int main(int argc, char **argv) {
  signal(SIGINT, signal_hander);
  std::thread pub_thd(pub_simu_func, &run_flag);

  try {
    boost::asio::io_context io_context;
    Server s = Server(io_context, lisenPort);
    s.startAccepting();
    io_context.run();
  } catch (boost::system::system_error &ec) {
    std::cout << ec.what() << std::endl;
  }

  pub_thd.join();
  return 0;
}