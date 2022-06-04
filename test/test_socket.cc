#ifdef _WIN32
#include <sdkddkver.h> // avoid boost.asio warning: Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately
#endif

#include <boost/asio.hpp>
#include <chrono>
#include <thread>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "hex_dump.hpp"
#include "usbcan.h"

TEST(CAN, socket_perf) {
  using namespace boost::asio;
  using namespace boost::asio::ip;

  auto server_proc = []() {
    io_service io_service;
    tcp::acceptor acceptor_server(io_service, tcp::endpoint(tcp::v4(), 9999));
    tcp::socket server_socket(io_service); // Creating socket object
    acceptor_server.accept(server_socket); // waiting for connection

    constexpr char *cmd_recv = "VCI_Receive";
    uint64_t send_count = 0;
    char send_buff[256];
    while (true) {
      VCI_CAN_OBJ can_obj{};
      auto dur = std::chrono::system_clock::now().time_since_epoch();
      uint64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
      strcpy(send_buff, cmd_recv);

      auto size = can::utils::bin2hex::bin2hex_fast(send_buff, cmd_recv, &send_count, &now, &can_obj);
      boost::asio::write(server_socket, boost::asio::buffer(send_buff, size));
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      send_count++;
    }
  }; // server_proc

  auto client_proc = []() {};

  std::thread server_thread(server_proc);
  server_thread.join();
}