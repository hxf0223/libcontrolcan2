#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ratio>
#include <string>

#include <boost/asio.hpp>
#include <chrono>
#include <filesystem>
#include <memory>
#include <thread>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "hex_dump.hpp"
#include "lib_control_can_imp.h"
#include "test_helper.hpp"

namespace {
constexpr DWORD kDevtype = 4;
constexpr DWORD kDevid = 0;
constexpr DWORD kChannel = 0;
} // namespace

/**
 * @brief 作为测试服务器，不断发送CAN Object。
 */
TEST(Socket, perfServer) {
  using namespace boost::asio;
  using namespace boost::asio::ip;
  using namespace std::literals::chrono_literals;
  using microsec = std::chrono::microseconds;
  using namespace can::utils;

  auto server_proc = []() {
    io_service io_service;
    tcp::acceptor acceptor_server(io_service, tcp::endpoint(tcp::v4(), 9999));
    tcp::socket server_socket(io_service); // Creating socket object
    acceptor_server.accept(server_socket); // waiting for connection

    boost::system::error_code ec{};
    const char* cmd_recv = "VCI_Receive,";
    uint64_t txcount = 0;
    char txbuff[256];

    while (!ec) {
      VCI_CAN_OBJ can_obj{};
      *(uint64_t*)(can_obj.Data) = txcount;
      auto dur = std::chrono::system_clock::now().time_since_epoch();
      uint64_t now = std::chrono::duration_cast<microsec>(dur).count();

      auto size = bin2hex::bin2hex_fast(txbuff, cmd_recv, &txcount, &now, &can_obj, "\n");
      LOG(INFO) << "size: " << size << ", data: " << txbuff;

      boost::asio::write(server_socket, boost::asio::buffer(txbuff, size), ec);
      std::this_thread::sleep_for(10ms);
      txcount++;
    }

    LOG(INFO) << "exit: " << ec.value() << " : " << ec.message();
  }; // server_proc

  std::thread server_thread(server_proc);
  server_thread.join();
}

/**
 * @brief 与 Socket.perfServer 配合，
 * 接收其发送的CAN Object帧，并解析出CANObject。
 */
TEST(Socket, perfClient) {
  auto client_proc = [](const std::shared_ptr<CanImpInterface>& canDc) {
    constexpr DWORD kRxBuffSize = 100;
    VCI_CAN_OBJ can_rx_buff[kRxBuffSize];
    const size_t recv_cnt_max = static_cast<long>(10000 * 300);
    ULONG recv_frame_cnt = 0;

    // auto tm0 = std::chrono::high_resolution_clock::now();
    while (recv_frame_cnt < recv_cnt_max) {
      auto recv_frame_num = canDc->VCI_Receive(kDevtype, kDevid, kChannel, can_rx_buff, kRxBuffSize, 10);
      for (ULONG i = 0; i < recv_frame_num; i++) {
        std::string const str = can::utils::bin2hex_dump(can_rx_buff[i].Data, 8);
        LOG(INFO) << recv_frame_cnt << ": " << str;
        recv_frame_cnt++;
      }
    }

    // auto tm2 = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> dur1 = tm2 - tm0;
    // LOG(INFO) << "receive frame num: " << recv_frame_cnt << ", time in ms: "
    // << dur1.count();
  }; // client_proc

  std::shared_ptr<CanImpInterface> can_dc(createCanNet());
  auto result = can_dc->VCI_OpenDevice(kDevtype, 0, 0);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_OpenDevice fail: " << result;

  auto cfg = test::helper::createVciInitCfg(0x00, 0x1C, 0xffffffff);
  result = can_dc->VCI_InitCAN(kDevtype, kDevid, kChannel, &cfg);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_InitCAN fail: " << result;

  result = can_dc->VCI_StartCAN(kDevtype, kDevid, kChannel);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_StartCAN fail: " << result;

  std::thread client_thread(client_proc, can_dc);
  client_thread.join();
}

namespace {
inline std::string make_string(const boost::asio::streambuf& streambuf) { // NOLINT
  return {boost::asio::buffers_begin(streambuf.data()), boost::asio::buffers_end(streambuf.data())};
}
} // namespace

/**
 * @brief 作为测试服务器，接收来自客户端的CAN Object帧消息，并解析成CAN Object。
 */
TEST(Socket, perfServer2) {
  using namespace boost::asio;
  using namespace boost::asio::ip;

  auto server_proc = []() {
    io_service io_service;
    tcp::acceptor acceptor_server(io_service, tcp::endpoint(tcp::v4(), 9999));
    tcp::socket server_socket(io_service); // Creating socket object
    acceptor_server.accept(server_socket); // waiting for connection

    boost::system::error_code ec{};
    std::string input_buffer;

    while (!ec) {
      // VCI_CAN_OBJ can_obj;
      auto read_num = boost::asio::read_until(server_socket, boost::asio::dynamic_buffer(input_buffer), "\n", ec);
      if (ec) {
        continue;
      }

      const size_t pos = input_buffer.find('\n');
      if (pos == std::string::npos) {
        continue;
      }

      std::string const data(input_buffer.begin(), input_buffer.begin() + pos);
      input_buffer.erase(0, pos + 1);

      LOG(INFO) << "len: " << data.length() << ": " << data;
    }

    LOG(INFO) << "exit: " << ec.value() << ": " << ec.message();
  }; // server_proc

  std::thread server_thread(server_proc);
  server_thread.join();
}

/**
 * @brief 与 Socket.perfServer2 匹配，调用CAN TCP不断发送
 * CAN Object帧消息。
 */
TEST(Socket, perfClient2) {
  using namespace std::chrono_literals;
  auto client_proc = [](const std::shared_ptr<CanImpInterface>& canDc) {
    constexpr DWORD kCanTxBuffSize = 1;
    VCI_CAN_OBJ can_tx_buff[kCanTxBuffSize];
    const size_t recv_cnt_max = 10000 * 30; // NOLINT
    ULONG send_count = 0;

    auto tm0 = std::chrono::steady_clock::now();
    while (send_count < recv_cnt_max) {
      *(uint64_t*)(can_tx_buff[0].Data) = send_count;
      auto e = canDc->VCI_Transmit(kDevtype, kDevid, kChannel, can_tx_buff, kCanTxBuffSize);
      CHECK(e == vciReturnType::STATUS_OK) << "VCI_Transmit return " << e;
      std::this_thread::sleep_for(300ms);
      send_count++;
    }

    auto tm2 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> const dur1 = tm2 - tm0;
    LOG(INFO) << "transmit frame num: " << send_count << ", time in ms: " << dur1.count();
  }; // client_proc

  std::shared_ptr<CanImpInterface> can_dc(createCanNet());
  auto result = can_dc->VCI_OpenDevice(kDevtype, 0, 0);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_OpenDevice fail: " << result;

  auto cfg = test::helper::createVciInitCfg(0x00, 0x1C, 0xffffffff);
  result = can_dc->VCI_InitCAN(kDevtype, kDevid, kChannel, &cfg);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_InitCAN fail: " << result;

  result = can_dc->VCI_StartCAN(kDevtype, kDevid, kChannel);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_StartCAN fail: " << result;

  std::thread client_thread(client_proc, can_dc);
  client_thread.join();
}