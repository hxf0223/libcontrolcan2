#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <optional>
#include <signal.h>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "glog/logging.h"
#include "hex_dump.hpp"
#include "lib_control_can.h"

#include "boost/asio.hpp"
#include "server/canobj_queue_type.h"
#include "server/server.h"
#include "ticks/tick_ext.h"

// #include "zmq.hpp"
// #include "zmq_addon.hpp"

constexpr DWORD devtype = 4;
constexpr DWORD devid = 0;
constexpr DWORD channel = 0;
constexpr short lisenPort = 9999;

namespace {

inline VCI_INIT_CONFIG create_vci_init_cfg(UCHAR timing0, UCHAR timing1,
                                           DWORD mask) {
  VCI_INIT_CONFIG cfg{};
  cfg.Timing0 = timing0;
  cfg.Timing1 = timing1;
  cfg.AccMask = mask;
  return cfg;
}

std::atomic_bool pub_thd_init_done{false};

} // namespace

static void can_rx_func(std::atomic_bool *runFlag, eventpp_queue_t &ppq) {
  using namespace std::chrono;
  // auto dur = system_clock::now().time_since_epoch();
  // uint64_t now = duration_cast<microseconds>(dur).count();

  auto e = VCI_OpenDevice(devtype, 0, 0);
  auto cfg = create_vci_init_cfg(0x00, 0x1C, 0xffffffff);
  e = VCI_InitCAN(devtype, devid, channel, &cfg);
  e = VCI_StartCAN(devtype, devid, channel);

  tick::tickExt tick_ext;
  tick_ext.beginInitTick();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  tick_ext.endInitTick();

  pub_thd_init_done.store(true);

  constexpr DWORD rx_buff_size = 100;
  VCI_CAN_OBJ can_rx_buff[rx_buff_size];
  const char *cmd_recv = "VCI_Receive,";
  canobj_queue_node_t send_buff;
  uint64_t send_count = 0;

  while (runFlag->load()) {
    auto frame_num =
        VCI_Receive(devtype, devid, channel, can_rx_buff, rx_buff_size, 10);
    for (ULONG i = 0; i < frame_num; i++) {
      uint64_t now = tick_ext.getTick();
      tick_ext.updateTick();

      auto &can_obj = can_rx_buff[i];
      auto ptr_dst = (char *)send_buff.can_obj_;
      send_buff.len_ = can::utils::bin2hex::bin2hex_fast(
          ptr_dst, cmd_recv, &send_count, &now, &can_obj, "\n");
      ppq.enqueue(ppq_can_obj_evt_id, send_buff);
      send_count++;
    }

    ppq.processIf([&frame_num](const canobj_queue_node_t /*event*/) {
      return (frame_num > 0);
    });
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  LOG(INFO) << "can_rx_func exit.";
}

static void pub_simu_func(std::atomic_bool *runFlag, eventpp_queue_t &ppq) {
  using namespace std::chrono;

  tick::tickExt tick_ext;
  tick_ext.beginInitTick();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  tick_ext.endInitTick();

  pub_thd_init_done.store(true);

  // auto dur = system_clock::now().time_since_epoch();
  // uint64_t now = duration_cast<microseconds>(dur).count();
  const char *cmd_recv = "VCI_Receive,";
  canobj_queue_node_t send_buff;
  uint64_t send_count = 0;

  while (runFlag->load()) {
    for (size_t i = 0; i < 10; i++) {
      uint64_t now = tick_ext.getTick();
      tick_ext.updateTick();

      VCI_CAN_OBJ can_obj{};
      *(uint64_t *)(can_obj.Data) = send_count;

      auto ptr_dst = (char *)send_buff.can_obj_;
      send_buff.len_ = can::utils::bin2hex::bin2hex_fast(
          ptr_dst, cmd_recv, &send_count, &now, &can_obj, "\n");
      ppq.enqueue(ppq_can_obj_evt_id, send_buff);
      send_count++;
    }

    auto err = ppq.process();
    std::this_thread::sleep_for(milliseconds(10));
  }

  LOG(INFO) << "pub_simu_func exit.";
}

int main(int argc, char **argv) {
  FLAGS_alsologtostderr = 1;
  FLAGS_colorlogtostderr = true;
  google::InitGoogleLogging(argv[0]);

  eventpp_queue_t ppq;
  std::atomic_bool run_flag{true};
  std::thread pub_thd(can_rx_func, &run_flag, std::ref(ppq));
  while (!pub_thd_init_done.load()) { // wait vci open device finish.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  try {
    boost::asio::io_context io_context;
    boost::asio::signal_set sigset(io_context, SIGINT, SIGTERM);
    sigset.async_wait([&run_flag, &io_context](
                          const boost::system::error_code &err, int signal) {
      run_flag.store(false);
      io_context.stop();
    });

    Server s(lisenPort, io_context, ppq);
    s.startAccepting();
    io_context.run();
  } catch (boost::system::system_error &ec) {
    LOG(WARNING) << ec.what();
    run_flag.store(false);
  }

  pub_thd.join();
  return 0;
}