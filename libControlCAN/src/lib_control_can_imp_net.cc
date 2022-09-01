#include <algorithm>
#include <atomic>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/errc.hpp>
#include <boost/thread.hpp>
#include <chrono>
#include <cmath>
#include <cstdint>

#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <unistd.h>
#include <vector>

#include "hex_dump.hpp"
#include "ini.h"
#include "lib_control_can_imp_net.hpp"
#include "misc.hpp"
#include "spdlog/spdlog.h"
#include "usbcan.h"

#ifdef _WIN32
#pragma warning(disable : 4101)
#pragma warning(disable : 4819)

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#endif

typedef Ini<> ini_t;
#define TCP_WRITE_TIMEOUT 200

#include <chrono>
#include <iostream>

using std::chrono::microseconds;

// _receive_pattern match frame head. _hex_str_pattern match VCI_CAN_OBJ,
// boost::read_until will remove last \n
std::regex CanImpCanNet::_receive_pattern2(
    "^(VCI_Receive,|VCI_Transmit,)[0-9a-fA-F]{32}");
std::regex CanImpCanNet::_hex_str_pattern2("^(?:[0-9a-fA-F][0-9a-fA-F])+$");

CanImpCanNet::CanImpCanNet()
    : _connected(false), _str_sock_addr(""), _str_sock_port("") {
  const auto dir_path = getexepath().parent_path();
  boost::filesystem::path ini_path(dir_path / "control_can.ini");
  rx_buff_.prepare(1024 * 8);

  if (boost::filesystem::exists(ini_path)) {
    ini_t ini(ini_path.string(), true);
    _str_sock_addr = ini["Server"]["IpAddr"];
    _str_sock_port = ini["Server"]["IpPort"];
  } else {
    spdlog::warn("File not exist: {}.", ini_path.string());
  }
}

CanImpCanNet::~CanImpCanNet() {
  if (_connected.load()) {
    client_socket_.close();
  }
}

vciReturnType CanImpCanNet::VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd,
                                           DWORD Reserved) {
  if (_str_sock_addr.length() <= 0 || _str_sock_port.length() <= 0) {
    return vciReturnType::STATUS_NET_CONN_FAIL;
  }

  if (!_connected.load()) {
    auto ec = connect(_str_sock_addr, _str_sock_port, 400);
    if (0 != ec)
      return vciReturnType::STATUS_NET_CONN_FAIL;
    _connected.store(true);
  }

  char buff[128];
  const char *head = "VCI_OpenDevice,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType,
                                                &DeviceInd, &Reserved, lr);

  error_code_t ec;
  auto ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror)
    return vciReturnType::STATUS_ERR;
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) {
  if (!_connected.load())
    return vciReturnType::STATUS_ERR;

  char buff[128];
  const char *head = "VCI_CloseDevice,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType,
                                                &DeviceInd, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror)
    return vciReturnType::STATUS_ERR;

  client_socket_.close();
  _connected.store(false);
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd,
                                              PVCI_BOARD_INFO pInfo) {
  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpCanNet::VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd,
                                        DWORD CANInd,
                                        PVCI_INIT_CONFIG pInitConfig) {
  if (!_connected.load())
    return vciReturnType::STATUS_ERR;

  char buff[256];
  const char *head = "VCI_InitCAN,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(
      buff, head, &DeviceType, &DeviceInd, &CANInd, pInitConfig, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror)
    return vciReturnType::STATUS_ERR;
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd,
                                            DWORD CANInd,
                                            PVCI_ERR_INFO pErrInfo) {
  if (!_connected.load() || !pErrInfo)
    return vciReturnType::STATUS_ERR;

  char buff[256];
  const char *head = "VCI_ReadErrInfo,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(
      buff, head, &DeviceType, &DeviceInd, &CANInd, pErrInfo, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror)
    return vciReturnType::STATUS_ERR;
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd,
                                              DWORD CANInd,
                                              PVCI_CAN_STATUS pCANStatus) {
  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpCanNet::VCI_GetReference(DWORD DeviceType, DWORD DeviceInd,
                                             DWORD CANInd, DWORD RefType,
                                             PVOID pData) {
  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpCanNet::VCI_SetReference(DWORD DeviceType, DWORD DeviceInd,
                                             DWORD CANInd, DWORD RefType,
                                             PVOID pData) {
  return vciReturnType::STATUS_ERR;
}

DWORD CanImpCanNet::VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd,
                                      DWORD CANInd) {
  return 1;
}

vciReturnType CanImpCanNet::VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd,
                                            DWORD CANInd) {
  if (!_connected.load())
    return vciReturnType::STATUS_ERR;

  char buff[256];
  const char *head = "VCI_ClearBuffer,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType,
                                                &DeviceInd, &CANInd, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror)
    return vciReturnType::STATUS_ERR;
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd,
                                         DWORD CANInd) {
  if (!_connected.load())
    return vciReturnType::STATUS_ERR;

  char buff[256];
  const char *head = "VCI_StartCAN,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType,
                                                &DeviceInd, &CANInd, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror)
    return vciReturnType::STATUS_ERR;
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd,
                                         DWORD CANInd) {
  if (!_connected.load())
    return vciReturnType::STATUS_ERR;

  char buff[256];
  const char *head = "VCI_ResetCAN,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType,
                                                &DeviceInd, &CANInd, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror)
    return vciReturnType::STATUS_ERR;
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_Transmit(DWORD DeviceType, DWORD DeviceInd,
                                         DWORD CANInd, PVCI_CAN_OBJ pSend,
                                         ULONG Len) {
  if (!_connected.load())
    return vciReturnType::STATUS_ERR;
  error_code_t ec;
  const char *head = "VCI_Transmit,", *lr = "\n";
  char line_buff[256];

  for (ULONG i = 0; i < Len; i++) {
    PVCI_CAN_OBJ p = pSend + i;
    auto write_len = can::utils::bin2hex::bin2hex_fast(
        line_buff, head, &DeviceType, &DeviceInd, &CANInd, p, lr);
    const auto ret = write_line(line_buff, write_len, ec);
    if (ec && ret < 0) {
      spdlog::warn("write_line error: {0}, {1}.", ec.value(), ec.message());
      _connected.store(false);
      client_socket_.close();
    }

    if (ret != write_len) {
      return vciReturnType::STATUS_ERR;
    }
  }

  return vciReturnType::STATUS_OK;
}

void CanImpCanNet::async_read(std::atomic<PVCI_CAN_OBJ> &ptrCanObj, ULONG Len,
                              error_code_t &ec, std::atomic_uint32_t &readNum) {
  if (readNum.load(std::memory_order_acquire) >= Len) {
    timer_.cancel();
    return;
  }

  using sv_regex_iter_t = std::regex_iterator<std::string_view::const_iterator>;
  using namespace boost::asio::error;
  using namespace boost::system;

  auto line_func = [](const std::string_view &str,
                      std::atomic<PVCI_CAN_OBJ> &dst) -> int {
    const auto end = sv_regex_iter_t();
    const sv_regex_iter_t it(str.begin(), str.end(), _receive_pattern2);
    if (it == end || (*it)[0].second == str.end())
      return -1;

    auto ptr = str.data() + ((*it)[0].second - str.begin());
    const auto left_len = str.end() - (*it)[0].second;
    auto pl = std::string_view(ptr, left_len);
    const sv_regex_iter_t it2(pl.begin(), pl.end(), _hex_str_pattern2);
    if (it2 == end || pl.size() < (sizeof(VCI_CAN_OBJ) * 2))
      return -1;

    can::utils::hex_string_to_bin_fastest(
        (const char *)(&pl[0]), pl.size(),
        (uint8_t *)(dst.load(std::memory_order_relaxed)));
    return 0;
  };

  boost::asio::async_read_until(
      client_socket_, rx_buff_, '\n',
      [&](const error_code_t &result_error, std::size_t result_n) {
        const auto avail_num = rx_buff_.size();
        auto begin = boost::asio::buffer_cast<const char *>(rx_buff_.data());
        auto pos = std::find(begin, begin + avail_num, '\n');
        ec = result_error; // save error code

        if (!result_error && pos != (begin + avail_num)) {
          std::string_view line(begin, pos - begin); // remove \n
          line_func(line, ptrCanObj);
          rx_buff_.consume(pos - begin + 1);
          if (readNum.load() < Len) {
            readNum.fetch_add(1, std::memory_order_release); // readNum += 1;
            ptrCanObj.fetch_add(1, std::memory_order_release);
            async_read(ptrCanObj, Len, ec, readNum);
          } else { // if read finished, stop the deadline timer
            timer_.cancel();
          }
        }
      });
}

ULONG CanImpCanNet::VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                                PVCI_CAN_OBJ pReceive, ULONG Len,
                                INT WaitTime) {
  if (!_connected.load() || !pReceive || !Len)
    return 0;

  using namespace boost::asio::error;
  using namespace boost::system;

  error_code_t ec;
  ULONG require_rx_num = Len;
  std::atomic_uint32_t rx_num{0};

  io_context_.reset();
  std::atomic<PVCI_CAN_OBJ> atomic_p(pReceive);
  async_read(atomic_p, require_rx_num, ec, rx_num);
  timer_.expires_from_now(boost::posix_time::milliseconds(WaitTime));
  timer_.async_wait([&](const error_code &ec) {
    if (!ec) { // no error mean timeout
      client_socket_.cancel();
    }
  });
  io_context_.run();

  const auto ev = ec.value();
  if (ec && ev != errc::operation_canceled && ev != operation_aborted) {
    spdlog::warn("error: {0}, {1}.", ev, ec.message());
    _connected.store(false);
    client_socket_.close();
  }

  auto temp = rx_num.load();
  spdlog::info("parsed obj num {}", temp);
  return temp;
}

int CanImpCanNet::connect(const std::string &host, const std::string &service,
                          int timeoutMs) {
  using boost::asio::ip::tcp;
  using namespace boost::system;
  // Resolve the host name and service to a list of endpoints.
  auto endpoints = tcp::resolver(io_context_).resolve(host, service);

  error_code_t error;
  io_context_.reset();
  boost::asio::async_connect(client_socket_, endpoints,
                             [&](const error_code_t &result_error,
                                 const tcp::endpoint & /*result_endpoint*/) {
                               const auto v = result_error.value();
                               error = result_error;
                               if (errc::operation_canceled != v) {
                                 timer_.cancel();
                               }
                             });

  timer_.expires_from_now(boost::posix_time::milliseconds(timeoutMs));
  timer_.async_wait([&](const error_code_t &result_error) {
    if (!result_error) { // no error mean timeout, need cancel socket ops.
      error = errc::make_error_code(errc::timed_out);
      client_socket_.cancel();
    }
  });

  io_context_.run();

  // Determine whether a connection was successfully established.
  if (error) {
    spdlog::warn("Connect fail: {0}, {1}.", error.value(), error.message());
    return error.value();
  }

  return 0;
}

int CanImpCanNet::write_line(const char *p, size_t len, error_code_t &ec) {
  using namespace boost::asio::error;
  using namespace boost::system;

  io_context_.reset();

  boost::asio::async_write(
      client_socket_, boost::asio::buffer(p, len),
      [&](const error_code_t &result_error, std::size_t /*result_n*/) {
        if (!result_error) {
          timer_.cancel();
        }
        ec = result_error;
      });

  timer_.expires_from_now(boost::posix_time::milliseconds(100));
  timer_.async_wait([&](const error_code_t &result_ec) {
    if (!result_ec) { // no error mean timeout, cancel socket ops.
      ec = errc::make_error_code(errc::operation_canceled);
      client_socket_.cancel();
    }
  });

  io_context_.run();

  if (!ec)
    return len;

  auto ec_code = ec.value();
  if (ec_code != errc::operation_canceled) {
    spdlog::warn("write_line error: {0}, {1}.", ec.value(), ec.message());
    return -std::abs(ec.value());
  }

  return 0;
}

// export function
#include "lib_control_can_imp.h"
#ifdef __cplusplus
extern "C" {
#endif

LIBCC_DLL CanImpInterface *createCanNet() { return new CanImpCanNet(); }

#ifdef __cplusplus
}
#endif
