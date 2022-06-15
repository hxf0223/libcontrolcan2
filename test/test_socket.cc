#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
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
#include "test_helper.hpp"
#include "usbcan.h"

constexpr DWORD devtype = 4;
constexpr DWORD devid = 0;
constexpr DWORD channel = 0;

/**
 * @brief 作为测试服务器，不断发送CAN Object。
 */
TEST(Socket, perfServer) {
  using namespace boost::asio;
  using namespace boost::asio::ip;

  auto server_proc = []() {
    io_service io_service;
    tcp::acceptor acceptor_server(io_service, tcp::endpoint(tcp::v4(), 9999));
    tcp::socket server_socket(io_service); // Creating socket object
    acceptor_server.accept(server_socket); // waiting for connection

    boost::system::error_code ec{};
    const char *cmd_recv = "VCI_Receive";
    uint64_t send_count = 0;
    char send_buff[256];

    while (!ec) {
      VCI_CAN_OBJ can_obj;
      *(uint64_t *)(can_obj.Data) = send_count;
      auto dur = std::chrono::system_clock::now().time_since_epoch();
      uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();

      auto size = can::utils::bin2hex::bin2hex_fast(send_buff, cmd_recv, &send_count, &now, &can_obj, "\n");
      LOG(INFO) << "size: " << size << ", data: " << send_buff;
      // getchar();

      boost::asio::write(server_socket, boost::asio::buffer(send_buff, size), ec);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      send_count++;
    }

    LOG(INFO) << "exit with boost asio error_code: " << ec.value() << ": " << ec.message();
  }; // server_proc

  std::thread server_thread(server_proc);
  server_thread.join();
}

/**
 * @brief 与 Socket.perfServer 配合，接收其发送的CAN Object帧，并解析出CAN Object。
 */
TEST(Socket, perfClient) {
  auto client_proc = [](std::shared_ptr<CanImpInterface> canDc) {
    constexpr DWORD rec_buff_size = 100;
    VCI_CAN_OBJ can_recv_buff[rec_buff_size];
    const size_t recv_cnt_max = 10000 * 30;
    ULONG recv_frame_cnt = 0;

    auto tm0 = std::chrono::steady_clock::now();
    while (recv_frame_cnt < recv_cnt_max) {
      auto recv_frame_num = canDc->VCI_Receive(devtype, devid, channel, can_recv_buff, rec_buff_size, 10);
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
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_OpenDevice fail: " << result;

  auto cfg = test::helper::create_vci_init_cfg(0x00, 0x1C, 0xffffffff);
  result = can_dc->VCI_InitCAN(devtype, devid, channel, &cfg);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_InitCAN fail: " << result;

  result = can_dc->VCI_StartCAN(devtype, devid, channel);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_StartCAN fail: " << result;

  std::thread client_thread(client_proc, can_dc);
  client_thread.join();
}

namespace {
inline std::string make_string(const boost::asio::streambuf &streambuf) {
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
    boost::asio::streambuf read_buffer;

    while (!ec) {
      VCI_CAN_OBJ can_obj;
      auto read_num = boost::asio::read_until(server_socket, read_buffer, "\n", ec);
      auto data = make_string(read_buffer);
      read_buffer.consume(read_num); // remove data that was read

      auto str = can::utils::bin2hex::bin2hex_fast(data.data(), data.size());
      LOG(INFO) << str;
    }

    LOG(INFO) << "exit with boost asio error_code: " << ec.value() << ": " << ec.message();
  }; // server_proc

  std::thread server_thread(server_proc);
  server_thread.join();
}

/**
 * @brief 与 Socket.perfServer2 匹配，调用CAN TCP不断发送
 * CAN Object帧消息。
 */
TEST(Socket, perfClient2) {
  auto client_proc = [](std::shared_ptr<CanImpInterface> canDc) {
    constexpr DWORD can_tx_buff_size = 1;
    VCI_CAN_OBJ can_tx_buff[can_tx_buff_size];
    const size_t recv_cnt_max = 10000 * 30;
    ULONG send_count = 0;

    auto tm0 = std::chrono::steady_clock::now();
    while (send_count < recv_cnt_max) {
      *(uint64_t *)(can_tx_buff[0].Data) = send_count;
      auto dur = std::chrono::system_clock::now().time_since_epoch();
      uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();

      auto e = canDc->VCI_Transmit(devtype, devid, channel, can_tx_buff, can_tx_buff_size);
      CHECK(e == vciReturnType::STATUS_OK) << "VCI_Transmit return " << e;
      send_count++;
    }

    auto tm2 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> dur1 = tm2 - tm0;
    LOG(INFO) << "transmit frame num: " << send_count << ", time in ms: " << dur1.count();
  }; // client_proc

  std::shared_ptr<CanImpInterface> can_dc(createCanNet());
  auto result = can_dc->VCI_OpenDevice(devtype, 0, 0);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_OpenDevice fail: " << result;

  auto cfg = test::helper::create_vci_init_cfg(0x00, 0x1C, 0xffffffff);
  result = can_dc->VCI_InitCAN(devtype, devid, channel, &cfg);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_InitCAN fail: " << result;

  result = can_dc->VCI_StartCAN(devtype, devid, channel);
  CHECK(result == vciReturnType::STATUS_OK) << "CAN NET VCI_StartCAN fail: " << result;

  std::thread client_thread(client_proc, can_dc);
  client_thread.join();
}