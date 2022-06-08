#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

#include "hex_dump.hpp"
#include "ini.h"
#include "lib_control_can_imp_net.hpp"
#include "misc.hpp"
#include "usbcan.h"

#ifdef _WIN32
#pragma warning(disable : 4101)
#pragma warning(disable : 4819)

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#endif

using namespace boost::xpressive;
using namespace boost::placeholders; // avoid message BOOST_BIND_GLOBAL_PLACEHOLDERS

typedef Ini<> ini_t;
#define TCP_WRITE_TIMEOUT 200

#include <chrono>
#include <iostream>

using std::chrono::microseconds;

boost::xpressive::sregex CanImpCanNet::_hex_str_pattern(sregex::compile("^(?:[0-9a-fA-F][0-9a-fA-F])+$"));
// boost::xpressive::sregex CanImpCanNet::_receive_pattern(sregex::compile("^VCI_Receive[0-9a-fA-F]{32},"));
boost::xpressive::sregex CanImpCanNet::_receive_pattern(sregex::compile("^(VCI_Receive|VCI_Transmit)[0-9a-fA-F]{32},"));
boost::regex CanImpCanNet::_receive_line_feed_pattern("^VCI_Receive[0-9a-fA-F]{32},");

CanImpCanNet::CanImpCanNet() : _connected(false), _str_sock_addr(""), _str_sock_port("") {
  const auto dir_path = getexepath().parent_path();
  boost::filesystem::path ini_path(dir_path / "control_can.ini");

  if (boost::filesystem::exists(ini_path)) {
    ini_t ini(ini_path.string(), true);
    _str_sock_addr = ini["Server"]["IpAddr"];
    _str_sock_port = ini["Server"]["IpPort"];
  } else {
    std::cout << "CanImpCanNet ctor: " << ini_path << " not exist." << std::endl;
  }
}

CanImpCanNet::~CanImpCanNet() {
  if (_connected.load()) {
    client_socket_.close();
  }
}

vciReturnType CanImpCanNet::VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) {
  if (_str_sock_addr.length() <= 0 || _str_sock_port.length() <= 0) {
    return vciReturnType::STATUS_NET_CONN_FAIL;
  }

  if (!_connected.load()) {
    auto ec = this->connect(_str_sock_addr, _str_sock_port, 400);
    if (0 != ec) return vciReturnType::STATUS_NET_CONN_FAIL;
    _connected.store(true);
  }

  char buff[128];
  const char *head = "VCI_OpenDevice,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType, &DeviceInd, &Reserved, lr);

  // std::cout << "VCI_OpenDevice: " << data << std::endl;
  auto ierror = write_line(buff, size);
  if (size != ierror) return vciReturnType::STATUS_ERR;

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  char buff[128];
  const char *head = "VCI_CloseDevice,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType, &DeviceInd, lr);

  auto const ierror = write_line(buff, size);
  if (size != ierror) return vciReturnType::STATUS_ERR;

  client_socket_.close();
  _connected.store(false);
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo) {
  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpCanNet::VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  char buff[256];
  const char *head = "VCI_InitCAN,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType, &DeviceInd, &CANInd, pInitConfig, lr);

  // std::cout << "VCI_InitCAN: " << data;
  auto const ierror = write_line(buff, size);
  if (size != ierror) return vciReturnType::STATUS_ERR;

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo) {
  if (!_connected.load() || !pErrInfo) return vciReturnType::STATUS_ERR;

  char buff[256];
  const char *head = "VCI_ReadErrInfo,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType, &DeviceInd, &CANInd, pErrInfo, lr);

  auto const ierror = write_line(buff, size);
  if (size != ierror) return vciReturnType::STATUS_ERR;

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                                              PVCI_CAN_STATUS pCANStatus) {
  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpCanNet::VCI_GetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType,
                                             PVOID pData) {
  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpCanNet::VCI_SetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType,
                                             PVOID pData) {
  return vciReturnType::STATUS_ERR;
}

DWORD CanImpCanNet::VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) { return 1; }

vciReturnType CanImpCanNet::VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  char buff[256];
  const char *head = "VCI_ClearBuffer,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType, &DeviceInd, &CANInd, lr);

  auto const ierror = write_line(buff, size);
  if (size != ierror) return vciReturnType::STATUS_ERR;

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  char buff[256];
  const char *head = "VCI_StartCAN,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType, &DeviceInd, &CANInd, lr);

  auto const ierror = write_line(buff, size);
  if (size != ierror) return vciReturnType::STATUS_ERR;

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  char buff[256];
  const char *head = "VCI_ResetCAN,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType, &DeviceInd, &CANInd, lr);

  auto const ierror = write_line(buff, size);
  if (size != ierror) return vciReturnType::STATUS_ERR;

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend,
                                         ULONG Len) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  const char *head = "VCI_Transmit,", *lr = "\n";

  if (1 == Len) {
    char line_buff[256];
    auto write_len = can::utils::bin2hex::bin2hex_fast(line_buff, head, &DeviceType, &DeviceInd, &CANInd, pSend, lr);
    const auto e = write_line(line_buff, write_len);
    if (e != write_len) return vciReturnType::STATUS_ERR;
  } else if (Len > 1) {
    char *line_buff = new char[128 + 2 * sizeof(VCI_CAN_OBJ)];
    auto write_len = can::utils::bin2hex::bin2hex_fast(line_buff, head, &DeviceType, &DeviceInd, &CANInd);
    for (ULONG i = 0; i < Len; i++) {
      VCI_CAN_OBJ *ptr_can_obj = pSend + i;
      write_len += can::utils::bin2hex::bin2hex_fast(line_buff + write_len, ptr_can_obj);
    }
    line_buff[write_len++] = '\n';
    line_buff[write_len] = '\0';

    const auto e = write_line(line_buff, write_len);
    if (e != write_len) return vciReturnType::STATUS_ERR;
  }

  return vciReturnType::STATUS_OK;
}

ULONG CanImpCanNet::VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len,
                                INT WaitTime) {
  if (!_connected.load() || nullptr == pReceive) return 0;

  ULONG recv_line_cnt = 0;
  const auto t0 = std::chrono::steady_clock::now();
  while (recv_line_cnt < Len) {
    auto rec_str = read_line(std::chrono::milliseconds(WaitTime));
    if (rec_str.empty()) break;

    auto v = can::utils::hex_string_to_bin_fastest(rec_str);
    memcpy(pReceive + recv_line_cnt, v.data(), sizeof(VCI_CAN_OBJ));
    recv_line_cnt++;

    const auto t1 = std::chrono::steady_clock::now();
    if (std::chrono::duration<double, std::milli>(t1 - t0).count() > WaitTime) break;
  }

  return recv_line_cnt;
}

int CanImpCanNet::connect(const std::string &host, const std::string &service, int timeoutMs) {
  using boost::asio::ip::tcp;
  // Resolve the host name and service to a list of endpoints.
  auto endpoints = tcp::resolver(io_context_).resolve(host, service);

  // Start the asynchronous operation itself. The lambda that is used as a
  // callback will update the error variable when the operation completes.
  // The blocking_udp_client.cpp example shows how you can use std::bind
  // rather than a lambda.
  boost::system::error_code error;
  boost::asio::async_connect(client_socket_, endpoints,
                             [&](const boost::system::error_code &result_error,
                                 const tcp::endpoint & /*result_endpoint*/) { error = result_error; });

  // Run the operation until it completes, or until the timeout.
  io_context_run(std::chrono::milliseconds(timeoutMs));

  // Determine whether a connection was successfully established.
  if (error) { // throw std::system_error(error);
    return error.value();
  }

  return 0;
}

void CanImpCanNet::io_context_run(const std::chrono::steady_clock::duration &timeout) {
  // Restart the io_context, as it may have been left in the "stopped" state
  // by a previous operation.
  io_context_.restart();

  // Block until the asynchronous operation has completed, or timed out. If
  // the pending asynchronous operation is a composed operation, the deadline
  // applies to the entire operation, rather than individual operations on
  // the socket.
  io_context_.run_for(timeout);

  // If the asynchronous operation completed successfully then the io_context
  // would have been stopped due to running out of work. If it was not
  // stopped, then the io_context::run_for call must have timed out.
  if (!io_context_.stopped()) {
    // Close the socket to cancel the outstanding asynchronous operation.
    client_socket_.close();

    // Run the io_context again until the operation completes.
    io_context_.run();
  }
}

std::string CanImpCanNet::read_line(const std::chrono::steady_clock::duration &timeout) {
  // Start the asynchronous operation. The lambda that is used as a callback
  // will update the error and n variables when the operation completes. The
  // blocking_udp_client.cpp example shows how you can use std::bind rather
  // than a lambda.
  boost::system::error_code error;
  std::size_t n = 0;
  boost::asio::async_read_until(client_socket_, boost::asio::dynamic_buffer(input_buffer_), '\n',
                                [&](const boost::system::error_code &result_error, std::size_t result_n) {
                                  error = result_error;
                                  n = result_n;
                                });

  // Run the operation until it completes, or until the timeout.
  io_context_run(timeout);

  // Determine whether the read completed successfully.
  if (error) { // throw std::system_error(error);
    return std::string();
  }

  std::string line(input_buffer_.substr(0, n - 1));
  input_buffer_.erase(0, n);
  return line;
}

int CanImpCanNet::write_line(const char *p, size_t len) {
  // Start the asynchronous operation itself. The lambda that is used as a
  // callback will update the error variable when the operation completes.
  // The blocking_udp_client.cpp example shows how you can use std::bind
  // rather than a lambda.
  boost::system::error_code error;
  boost::asio::async_write(
    client_socket_, boost::asio::buffer(p, len),
    [&](const boost::system::error_code &result_error, std::size_t /*result_n*/) { error = result_error; });

  // Run the operation until it completes, or until the timeout.
  std::chrono::steady_clock::duration timeout{std::chrono::milliseconds(30)};
  io_context_run(timeout);

  // Determine whether the read completed successfully.
  if (error) { // throw std::system_error(error);
    return error.value();
  }

  return len;
}

#include "lib_control_can_imp.h"
#ifdef __cplusplus
extern "C" {
#endif

LIBCC_DLL CanImpInterface *createCanNet() { return new CanImpCanNet(); }

#ifdef __cplusplus
}
#endif
