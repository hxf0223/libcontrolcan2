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
#include <cstddef>
#include <cstdint>

#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "hex_dump.hpp"
#include "ini.h"
#include "lib_control_can_imp_net.hpp"
#include "misc.hpp"
#include "spdlog/spdlog.h"
#include "usbcan.h"

#ifdef _WIN32
#  pragma warning(disable : 4101)
#  pragma warning(disable : 4819)

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#  pragma comment(lib, "Ws2_32.lib")
#  pragma comment(lib, "Mswsock.lib")
#  pragma comment(lib, "AdvApi32.lib")
#endif

// NOLINTBEGIN(misc-no-recursion)
// #define TCP_WRITE_TIMEOUT 200
using ini_t = Ini<>;
using std::chrono::microseconds;
using namespace can::utils;

namespace {

inline std::string makeString(const boost::asio::streambuf& streambuf) {
  return {boost::asio::buffers_begin(streambuf.data()), boost::asio::buffers_end(streambuf.data())};
}

} // namespace

// _receive_pattern match frame head. _hex_str_pattern match VCI_CAN_OBJ,
// boost::read_until will remove last \n
std::regex CanImpCanNet::_receive_pattern2("^(VCI_Receive,|VCI_Transmit,)[0-9a-fA-F]{32}");
std::regex CanImpCanNet::_hex_str_pattern2("^(?:[0-9a-fA-F][0-9a-fA-F])+$");

CanImpCanNet::CanImpCanNet()
    : _connected(false) {
  const auto dir_path = getexepath().parent_path();
  const boost::filesystem::path ini_path(dir_path / "control_can.ini");
  rx_buff_.prepare(static_cast<std::size_t>(1024 * 8));

  if (boost::filesystem::exists(ini_path)) {
    ini_t ini(ini_path.string(), true);
    _str_sock_addr = ini["Server"]["IpAddr"];
    _str_sock_port = ini["Server"]["IpPort"];
  } else {
    SPDLOG_WARN("File not exist: {}.", ini_path.string());
  }
}

CanImpCanNet::~CanImpCanNet() {
  if (_connected.load()) {
    client_socket_.close();
  }
}

vciReturnType CanImpCanNet::VCI_OpenDevice(DWORD deviceType, DWORD deviceInd, DWORD reserved) {
  if (_str_sock_addr.length() <= 0 || _str_sock_port.length() <= 0) {
    return vciReturnType::STATUS_NET_CONN_FAIL;
  }

  if (!_connected.load()) {
    auto ec = connect(_str_sock_addr, _str_sock_port, 400);
    if (0 != ec) {
      return vciReturnType::STATUS_NET_CONN_FAIL;
    }
    _connected.store(true);
  }

  char buff[128];
  const char* head = "VCI_OpenDevice,";
  const char* lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &deviceType, &deviceInd, &reserved, lr);

  error_code_t ec;
  auto ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror) {
    return vciReturnType::STATUS_ERR;
  }
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_CloseDevice(DWORD deviceType, DWORD deviceInd) {
  if (!_connected.load()) {
    return vciReturnType::STATUS_ERR;
  }

  char buff[128];
  const char* head = "VCI_CloseDevice,";
  const char* lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &deviceType, &deviceInd, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror) {
    return vciReturnType::STATUS_ERR;
  }

  client_socket_.close();
  _connected.store(false);
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadBoardInfo(DWORD /*DeviceType*/, DWORD /*DeviceInd*/, PVCI_BOARD_INFO /*pInfo*/) {
  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpCanNet::VCI_InitCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_INIT_CONFIG pInitConfig) {
  if (!_connected.load()) {
    return vciReturnType::STATUS_ERR;
  }

  char buff[256];
  const char* head = "VCI_InitCAN,";
  const char* lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &deviceType, &deviceInd, &canInd, pInitConfig, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror) {
    return vciReturnType::STATUS_ERR;
  }
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadErrInfo(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_ERR_INFO pErrInfo) {
  if (!_connected.load() || (pErrInfo == nullptr)) {
    return vciReturnType::STATUS_ERR;
  }

  char buff[256];
  const char* head = "VCI_ReadErrInfo,";
  const char* lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &deviceType, &deviceInd, &canInd, pErrInfo, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror) {
    return vciReturnType::STATUS_ERR;
  }
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadCANStatus(DWORD /*DeviceType*/, DWORD /*DeviceInd*/, DWORD /*CANInd*/,
                                              PVCI_CAN_STATUS /*pCANStatus*/) {
  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpCanNet::VCI_GetReference(DWORD /*DeviceType*/, DWORD /*DeviceInd*/, DWORD /*CANInd*/, DWORD /*RefType*/,
                                             PVOID /*pData*/) {
  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpCanNet::VCI_SetReference(DWORD /*DeviceType*/, DWORD /*DeviceInd*/, DWORD /*CANInd*/, DWORD /*RefType*/,
                                             PVOID /*pData*/) {
  return vciReturnType::STATUS_ERR;
}

DWORD CanImpCanNet::VCI_GetReceiveNum(DWORD /*DeviceType*/, DWORD /*DeviceInd*/, DWORD /*CANInd*/) {
  return 1;
}

vciReturnType CanImpCanNet::VCI_ClearBuffer(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  if (!_connected.load()) {
    return vciReturnType::STATUS_ERR;
  }

  char buff[256];
  const char* head = "VCI_ClearBuffer,";
  const char* lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &deviceType, &deviceInd, &canInd, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror) {
    return vciReturnType::STATUS_ERR;
  }
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_StartCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  if (!_connected.load()) {
    return vciReturnType::STATUS_ERR;
  }

  char buff[256];
  const char* head = "VCI_StartCAN,";
  const char* lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &deviceType, &deviceInd, &canInd, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror) {
    return vciReturnType::STATUS_ERR;
  }
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ResetCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  if (!_connected.load()) {
    return vciReturnType::STATUS_ERR;
  }

  char buff[256];
  const char* head = "VCI_ResetCAN,";
  const char* lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &deviceType, &deviceInd, &canInd, lr);

  error_code_t ec;
  auto const ierror = write_line(buff, size, ec);
  if (ec) {
    _connected.store(false);
    client_socket_.close();
  }

  if (size != ierror) {
    return vciReturnType::STATUS_ERR;
  }
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_Transmit(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_CAN_OBJ pSend, ULONG len) {
  if (!_connected.load()) {
    return vciReturnType::STATUS_ERR;
  }
  error_code_t ec;
  const char* head = "VCI_Transmit,";
  const char* lr = "\n";
  char line_buff[256];

  for (ULONG i = 0; i < len; i++) {
    PVCI_CAN_OBJ p = pSend + i;
    auto write_len = can::utils::bin2hex::bin2hex_fast(line_buff, head, &deviceType, &deviceInd, &canInd, p, lr);
    const auto ret = write_line(line_buff, write_len, ec);
    if (ec && ret < 0) {
      SPDLOG_WARN("write_line error: {0}, {1}.", ec.value(), ec.message());
      _connected.store(false);
      client_socket_.close();
    }

    if (ret != write_len) {
      return vciReturnType::STATUS_ERR;
    }
  }

  return vciReturnType::STATUS_OK;
}

int CanImpCanNet::line_process(const std::string_view& str, PVCI_CAN_OBJ dst) {
  using sv_regex_iter_t = std::regex_iterator<std::string_view::const_iterator>;
  const auto end = sv_regex_iter_t();
  const sv_regex_iter_t it(str.begin(), str.end(), _receive_pattern2);
  if (it == end || (*it)[0].second == str.end()) {
    return -1;
  }

  const auto* ptr = str.data() + ((*it)[0].second - str.begin());
  const auto left_len = str.end() - (*it)[0].second;
  auto pl = std::string_view(ptr, left_len);
  const sv_regex_iter_t it2(pl.begin(), pl.end(), _hex_str_pattern2);
  if (it2 == end || pl.size() < (sizeof(VCI_CAN_OBJ) * 2)) {
    return -1;
  }

  const auto* p2 = (const char*)(pl.data());
  can::utils::hex_string_to_bin_fastest(p2, pl.size(), (uint8_t*)dst);

  return 0;
}

void CanImpCanNet::async_read(std::atomic<asyncReadParam>& param, error_code_t& ec) {
  const auto temp_param = param.load(std::memory_order_acquire);
  if (temp_param.read_cnt_ >= temp_param.len_) {
    timer_.cancel();
    return;
  }

  using namespace boost::asio::error;
  using namespace boost::system;

  boost::asio::async_read_until(client_socket_, rx_buff_, '\n', [&](const error_code_t& resultError, std::size_t /*result_n*/) {
    const auto avail_num = rx_buff_.size();
    const auto* begin = boost::asio::buffer_cast<const char*>(rx_buff_.data());
    const auto* pos = std::find(begin, begin + avail_num, '\n');
    ec = resultError; // save error code

    if (!resultError && pos != (begin + avail_num)) {
      auto my_param = param.load(std::memory_order_acquire);
      const std::string_view line(begin, pos - begin); // remove \n
      if (0 != line_process(line, my_param.can_objs_)) {
        SPDLOG_WARN("line post process error."); // NOLINT
      }
      rx_buff_.consume(pos - begin + 1);
      if (my_param.read_cnt_ < my_param.len_) {
        my_param.read_cnt_++, my_param.can_objs_++;
        param.store(my_param, std::memory_order_release);
        async_read(param, ec);
      } else { // if read finished, stop the deadline timer
        timer_.cancel();
      }
    } else if (!resultError) {
      const std::string temp_str = makeString(rx_buff_);
      SPDLOG_WARN("rx buffer error: {}", temp_str); // NOLINT
    } else {
      // SPDLOG_WARN(result_error.message());
    }
  });
}

// NOLINTNEXTLINE
ULONG CanImpCanNet::VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG len, INT waitTime) {
  if (!_connected.load() || (pReceive == nullptr) || (len == 0U)) {
    return 0;
  }

  using namespace boost::asio::error;
  using namespace boost::system;

  error_code_t ec;
  std::atomic<asyncReadParam> param{asyncReadParam{pReceive, len, 0}};

  io_context_.restart();
  async_read(param, ec);
  timer_.expires_from_now(boost::posix_time::milliseconds(waitTime));
  timer_.async_wait([&](const error_code& ec) {
    if (!ec) { // no error mean timeout
      client_socket_.cancel();
    }
  });
  io_context_.run();

  const auto ev = ec.value();
  if (ec && ev != errc::operation_canceled && ev != operation_aborted) {
    SPDLOG_WARN("error: {0}, {1}.", ev, ec.message());
    _connected.store(false);
    client_socket_.close();
  }

  // post process: try read available data in rx buffer
  uint32_t read_num = param.load().read_cnt_; // frame count
  while (_connected.load() && rx_buff_.size() > 0 && read_num < len) {
    const auto avail_num = rx_buff_.size();
    const auto* begin = boost::asio::buffer_cast<const char*>(rx_buff_.data());
    const auto* pos = std::find(begin, begin + avail_num, '\n');
    if (pos == (begin + avail_num)) {
      break;
    }

    const std::string_view line(begin, pos - begin); // remove \n
    line_process(line, pReceive + read_num);
    rx_buff_.consume(pos - begin + 1);
    read_num++;
  }

  return read_num;
}

int CanImpCanNet::connect(const std::string& host, const std::string& service, int timeoutMs) {
  using boost::asio::ip::tcp;
  using namespace boost::system;
  // Resolve the host name and service to a list of endpoints.
  auto endpoints = tcp::resolver(io_context_).resolve(host, service);

  error_code_t error;
  io_context_.restart();
  boost::asio::async_connect(client_socket_, endpoints,
                             [&](const error_code_t& resultError, const tcp::endpoint& /*result_endpoint*/) {
                               const auto v = resultError.value();
                               error = resultError;
                               if (errc::operation_canceled != v) {
                                 timer_.cancel();
                               }
                             });

  timer_.expires_from_now(boost::posix_time::milliseconds(timeoutMs));
  timer_.async_wait([&](const error_code_t& resultError) {
    if (!resultError) { // no error mean timeout, need cancel socket ops.
      error = errc::make_error_code(errc::timed_out);
      client_socket_.cancel();
    }
  });

  io_context_.run();

  // Determine whether a connection was successfully established.
  if (error) {
    SPDLOG_WARN("Connect fail: {0}, {1}.", error.value(), error.message());
    return error.value();
  }

  return 0;
}

int CanImpCanNet::write_line(const char* p, size_t len, error_code_t& ec) {
  using namespace boost::asio::error;
  using namespace boost::system;

  io_context_.restart();
  boost::asio::async_write(client_socket_, boost::asio::buffer(p, len),
                           [&](const error_code_t& resultError, std::size_t /*result_n*/) {
                             if (!resultError) {
                               timer_.cancel();
                             }
                             ec = resultError;
                           });

  timer_.expires_from_now(boost::posix_time::milliseconds(100));
  timer_.async_wait([&](const error_code_t& resultEc) {
    if (!resultEc) { // no error mean timeout, cancel socket ops.
      ec = errc::make_error_code(errc::operation_canceled);
      client_socket_.cancel();
    }
  });

  io_context_.run();

  if (!ec) {
    return static_cast<int>(len);
  }

  auto ec_code = ec.value();
  if (ec_code != errc::operation_canceled) {
    SPDLOG_WARN("write_line error: {0}, {1}.", ec.value(), ec.message());
    return -std::abs(ec.value());
  }

  return 0;
}

// NOLINTEND(misc-no-recursion)

// export function
#include "lib_control_can_imp.h"
#ifdef __cplusplus
extern "C" {
#endif

LIBCC_DLL CanImpInterface* createCanNet() {
  return new CanImpCanNet();
}

#ifdef __cplusplus
}
#endif
