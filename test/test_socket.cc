#include <cstdint>
#ifdef _WIN32
#include <sdkddkver.h> // avoid boost.asio warning: Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately
#endif

#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <thread>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "hex_dump.hpp"
#include "lib_control_can_imp.h"
#include "usbcan.h"

constexpr DWORD devtype = 4;
constexpr DWORD devid = 0;
constexpr DWORD channel = 0;

TEST(Socket, perfServer) {
  using namespace boost::asio;
  using namespace boost::asio::ip;

  auto server_proc = []() {
    io_service io_service;
    tcp::acceptor acceptor_server(io_service, tcp::endpoint(tcp::v4(), 9999));
    tcp::socket server_socket(io_service); // Creating socket object
    acceptor_server.accept(server_socket); // waiting for connection

    const char *cmd_recv = "VCI_Receive";
    uint64_t send_count = 0;
    char send_buff[256];
    while (true) {
      VCI_CAN_OBJ can_obj{};
      *(uint64_t *)(can_obj.Data) = send_count;
      auto dur = std::chrono::system_clock::now().time_since_epoch();
      uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();

      auto size = can::utils::bin2hex::bin2hex_fast(send_buff, cmd_recv, &send_count, &now, &can_obj, "\n");
      LOG(INFO) << "size: " << size << ", data: " << send_buff;
      // getchar();

      boost::asio::write(server_socket, boost::asio::buffer(send_buff, size));
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      send_count++;
    }
  }; // server_proc

  std::thread server_thread(server_proc);
  server_thread.join();
}

TEST(Socket, perfClient) {
  auto client_proc = [](std::shared_ptr<CanImpInterface> canDc) {
    constexpr DWORD rec_buff_size = 100;
    VCI_CAN_OBJ can_recv_buff[rec_buff_size];
    const size_t recv_cnt_max = 10000 * 100;
    ULONG recv_frame_cnt = 0;

    auto tm0 = std::chrono::steady_clock::now();
    while (recv_frame_cnt < recv_cnt_max) {
      auto recv_frame_num = canDc->VCI_Receive(devtype, devid, channel, can_recv_buff, rec_buff_size, 100);
      for (ULONG i = 0; i < recv_frame_num; i++) {
        std::string str = can::utils::bin2hex_dump(can_recv_buff[i].Data, 8);
        LOG(INFO) << recv_frame_cnt << ": " << str;
        recv_frame_cnt++;
      }
    }

    auto tm2 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> dur1 = tm2 - tm0;
    LOG(INFO) << "receive frame num: " << recv_frame_cnt << ", time in ms: " << dur1.count();
  }; // client_proc

  std::shared_ptr<CanImpInterface> can_dc(createCanNet());
  auto result = can_dc->VCI_OpenDevice(devtype, 0, 0);
  CHECK(result != 0) << "CAN NET VCI_OpenDevice fail.";

  VCI_INIT_CONFIG cfg{};
  cfg.Timing0 = 0x00;
  cfg.Timing1 = 0x1C;
  cfg.Filter = 0;
  cfg.AccMask = 0xffffffff;
  cfg.AccCode = 0;
  cfg.Mode = 0;
  cfg.Reserved = 0;
  result = can_dc->VCI_InitCAN(devtype, devid, channel, &cfg);
  CHECK(result != 0) << "CAN NET VCI_InitCAN fail.";

  result = can_dc->VCI_StartCAN(devtype, devid, channel);
  CHECK(result != 0) << "CAN NET VCI_StartCAN fail.";

  std::thread client_thread(client_proc, can_dc);
  client_thread.join();
}