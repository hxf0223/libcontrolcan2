#ifdef _MSC_VER
//#include "stdafx.h"
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4101)
#pragma warning(disable : 4819)
#endif

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <vector>

#include "INI.h"
#include "hex_dump.hpp"
#include "lib_control_can_imp_net.hpp"
#include "misc.hpp"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

using namespace boost::xpressive;
using namespace boost::placeholders; // avoid message BOOST_BIND_GLOBAL_PLACEHOLDERS
using boost::lambda::_1;
using boost::lambda::bind;
using boost::lambda::var;

typedef Ini<> ini_t;
#define TCP_WRITE_TIMEOUT 200

#include <chrono>
#include <iostream>

using std::chrono::duration_cast;
using std::chrono::microseconds;


boost::xpressive::sregex CanImpCanNet::_hex_str_pattern(sregex::compile("^(?:[0-9a-fA-F][0-9a-fA-F])+$"));
// boost::xpressive::sregex CanImpCanNet::_receive_pattern(sregex::compile("^VCI_Receive[0-9a-fA-F]{32},"));
boost::xpressive::sregex CanImpCanNet::_receive_pattern(sregex::compile("^(VCI_Receive|VCI_Transmit)[0-9a-fA-F]{32},"));
boost::regex CanImpCanNet::_receive_line_feed_pattern("^VCI_Receive[0-9a-fA-F]{32},");
std::string CanImpCanNet::_empty_string;
int CanImpCanNet::_conn_timeout_ms = 300;

#ifdef BUFFER_LOCK_CS
#define lock_enter() EnterCriticalSection(&_buffer_cs)
#define lock_leave() LeaveCriticalSection(&_buffer_cs)
#else
#define lock_enter() std::unique_lock<std::mutex> lck(_buffer_mutex)
#define lock_leave()
#endif

CanImpCanNet::CanImpCanNet() : _connected(false), _str_sock_addr(""), _str_sock_port(""), _socket(INVALID_SOCKET) {
  const auto dir_path = getexepath().parent_path();
  boost::filesystem::path ini_path(dir_path / "control_can.ini");

  if (boost::filesystem::exists(ini_path)) {
    ini_t ini(ini_path.string(), true);
    _str_sock_addr = ini["Server"]["IpAddr"];
    _str_sock_port = ini["Server"]["IpPort"];
  } else {
    // std::cout << "CanImpCanNet ctor: " << ini_file_name << " not exist." << std::endl;
  }

#ifdef BUFFER_LOCK_CS
  InitializeCriticalSectionAndSpinCount(&_buffer_cs, 4000);
#endif

  _join_buffer.reserve(1024);
  buffer_list_init(); // init at end of ctor
}

CanImpCanNet::~CanImpCanNet() {
  if (_connected.load()) {
    disconnect();
  }
}

vciReturnType CanImpCanNet::VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) {
  std::string data("VCI_OpenDevice,");

  buffer_list_init();

  if (_str_sock_addr.length() <= 0 || _str_sock_port.length() <= 0) {
    return vciReturnType::STATUS_NET_CONN_FAIL;
  }

  auto ierror = connect(_str_sock_addr, _str_sock_port, _conn_timeout_ms);
  if (!_connected.load()) return vciReturnType::STATUS_NET_CONN_FAIL;

  data += can::utils::bin2hex_fast(&DeviceType, sizeof(DeviceType));
  data += can::utils::bin2hex_fast(&DeviceInd, sizeof(DeviceInd));
  data += can::utils::bin2hex_fast(&Reserved, sizeof(Reserved));
  data += '\n';

  // std::cout << "VCI_OpenDevice: " << data << std::endl;
  ierror = write_line(data);
  if (data.size() != ierror) {
    // std::cout << "VCI_OpenDevice: write fail" << std::endl;
    return vciReturnType::STATUS_ERR;
  }

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  std::string data("VCI_CloseDevice,");
  data += can::utils::bin2hex_fast(&DeviceType, sizeof(DeviceType));
  data += can::utils::bin2hex_fast(&DeviceInd, sizeof(DeviceInd));
  data += '\n';

  auto const ierror = write_line(data);
  if (data.size() != ierror) {
    return vciReturnType::STATUS_ERR;
  }

  disconnect();
  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo) {
  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpCanNet::VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  std::string data("VCI_InitCAN,");
  data += can::utils::bin2hex_fast(&DeviceType, sizeof(DeviceType));
  data += can::utils::bin2hex_fast(&DeviceInd, sizeof(DeviceInd));
  data += can::utils::bin2hex_fast(&CANInd, sizeof(CANInd));
  data += can::utils::bin2hex_fast(pInitConfig, sizeof(VCI_INIT_CONFIG));
  data += '\n';

  // std::cout << "VCI_InitCAN: " << data;
  auto const ierror = write_line(data);
  if (data.size() != ierror) {
    return vciReturnType::STATUS_ERR;
  }

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo) {
  if (!_connected.load() || !pErrInfo) return vciReturnType::STATUS_ERR;

  std::vector<uint8_t> v;
  v.reserve(64);

  auto p = reinterpret_cast<uint8_t *>(&DeviceType);
  std::copy(p, p + sizeof(DeviceType), std::back_inserter(v));
  p = reinterpret_cast<uint8_t *>(&DeviceInd);
  std::copy(p, p + sizeof(DeviceInd), std::back_inserter(v));
  p = reinterpret_cast<uint8_t *>(&CANInd);
  std::copy(p, p + sizeof(CANInd), std::back_inserter(v));
  p = reinterpret_cast<uint8_t *>(pErrInfo);
  std::copy(p, p + sizeof(VCI_ERR_INFO), std::back_inserter(v));

  std::string data;
  data.reserve(256); // need more buffer

  data.append("VCI_ReadErrInfo,");
  auto &&data2 = can::utils::bin2hex_fast(v.data(), v.size());
  data.append(data2);
  data.append("\n");

  auto const ierror = write_line(data);
  if (data.size() != ierror) {
    return vciReturnType::STATUS_ERR;
  }

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

DWORD CanImpCanNet::VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  return 1;
}

vciReturnType CanImpCanNet::VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  std::string data("VCI_ClearBuffer,");
  data += can::utils::bin2hex_fast(&DeviceType, sizeof(DeviceType));
  data += can::utils::bin2hex_fast(&DeviceInd, sizeof(DeviceInd));
  data += can::utils::bin2hex_fast(&CANInd, sizeof(CANInd));
  data += '\n';

  auto const ierror = write_line(data);
  if (data.size() != ierror) {
    return vciReturnType::STATUS_ERR;
  }

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  std::string data("VCI_StartCAN,");
  data += can::utils::bin2hex_fast(&DeviceType, sizeof(DeviceType));
  data += can::utils::bin2hex_fast(&DeviceInd, sizeof(DeviceInd));
  data += can::utils::bin2hex_fast(&CANInd, sizeof(CANInd));
  data += '\n';

  auto const ierror = write_line(data);
  if (data.size() != ierror) {
    return vciReturnType::STATUS_ERR;
  }

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  std::string data("VCI_ResetCAN,");
  data += can::utils::bin2hex_fast(&DeviceType, sizeof(DeviceType));
  data += can::utils::bin2hex_fast(&DeviceInd, sizeof(DeviceInd));
  data += can::utils::bin2hex_fast(&CANInd, sizeof(CANInd));
  data += '\n';

  auto const ierror = write_line(data);
  if (data.size() != ierror) {
    return vciReturnType::STATUS_ERR;
  }

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend,
                                         ULONG Len) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  std::vector<uint8_t> v;
  v.reserve(64);

  auto p = reinterpret_cast<uint8_t *>(&DeviceType);
  std::copy(p, p + sizeof(DeviceType), std::back_inserter(v));
  p = reinterpret_cast<uint8_t *>(&DeviceInd);
  std::copy(p, p + sizeof(DeviceInd), std::back_inserter(v));
  p = reinterpret_cast<uint8_t *>(&CANInd);
  std::copy(p, p + sizeof(CANInd), std::back_inserter(v));
  p = reinterpret_cast<uint8_t *>(pSend);
  std::copy(p, p + sizeof(VCI_CAN_OBJ) * Len, std::back_inserter(v));
  // p = reinterpret_cast<uint8_t*>(&Len); std::copy(p, p + sizeof(Len), std::back_inserter(v));	// ignore Len

  std::string data;
  data.reserve(256); // need more buffer

  data.append("VCI_Transmit,");
  auto &&data2 = can::utils::bin2hex_fast(v.data(), v.size());
  data.append(data2);
  data.append("\n");

  auto const ierror = write_line(data);
  if (data.size() != ierror) {
    return vciReturnType::STATUS_ERR;
  }

  return vciReturnType::STATUS_OK;
}

ULONG CanImpCanNet::VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len,
                                INT WaitTime) {
  const auto start = boost::chrono::high_resolution_clock::now();
  boost::chrono::milliseconds ms;
  ULONG receive_num;

  do {
    receive_num = vci_receive_tool(DeviceType, DeviceInd, CANInd, pReceive, Len, WaitTime);
    ms = boost::chrono::duration_cast<boost::chrono::milliseconds>(boost::chrono::high_resolution_clock::now() - start);
    // std::cout << "VCI_Receive read time " << ms.count() << ", receive num " << receive_num << std::endl;
    // OutputDebugStringA("CanImpCanNet::VCI_Receive\n");
  } while (receive_num == 0 && ms.count() < WaitTime);

  return receive_num;
}

ULONG CanImpCanNet::vci_receive_tool(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len,
                                     INT WaitTime) {
  static std::string prefix_str("VCI_Receive,");
  if (!_connected.load() || nullptr == pReceive) return 0;

  auto rec_str = read_line(WaitTime);
  if (rec_str.empty()) return 0;
  // std::cout << "rec_str len: " << rec_str.length() << rec_str << std::endl;
  // OutputDebugStringA("CanImpCanNet::vci_receive_tool\n");

  const sregex_iterator end;
  const sregex_iterator it(rec_str.begin(), rec_str.end(), _receive_pattern);
  if (it == end || (*it)[0].second == rec_str.end()) return 0;

  auto data_str = rec_str.substr((*it)[0].second - rec_str.begin());
  // std::cout << "data_str len:" << data_str.length() << "data: " << data_str << std::endl;

  const sregex_iterator it2(data_str.begin(), data_str.end(), _hex_str_pattern);
  if (it2 == end) return 0;

  auto v = can::utils::hex_string_to_bin_fastest(data_str);
  if (Len > (v.size() / sizeof(VCI_CAN_OBJ))) Len = v.size() / sizeof(VCI_CAN_OBJ);

  memcpy(pReceive, v.data(), sizeof(VCI_CAN_OBJ) * Len);
  return Len;
}

int CanImpCanNet::connect(const std::string &host, const std::string &service, int timeout) {
  struct addrinfo *result = nullptr;
  WSADATA wsa_data;

  // Initialize Winsock
  auto ierror = WSAStartup(MAKEWORD(2, 2), &wsa_data);
  if (ierror != 0) {
    std::cout << "WSAStartup failed with error: " << ierror << std::endl;
    _connected.store(false);
    return -1;
  }

  struct addrinfo hints;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // Resolve the server address and port
  ierror = getaddrinfo(host.c_str(), service.c_str(), &hints, &result);
  if (ierror != 0) {
    // std::cout << "getaddrinfo failed with error: " << ierror << std::endl;
    WSACleanup();
    _connected.store(false);
    return -1;
  }

  int async_error = -1;
  int async_error_len = sizeof(int);
  fd_set set;

  // Attempt to connect to an address until one succeeds
  for (auto ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
    // Create a SOCKET for connecting to server
    _socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (_socket == INVALID_SOCKET) {
      // std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
      WSACleanup();
      _connected.store(false);
      return -1;
    }

    // set the socket in non-blocking
    unsigned long async_mode = 1;
    ioctlsocket(_socket, FIONBIO, &async_mode);

    // Connect to server.
    ierror = ::connect(_socket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
    if (ierror == SOCKET_ERROR) {
      if (WSAGetLastError() != WSAEWOULDBLOCK) {
        closesocket(_socket);
        _socket = INVALID_SOCKET;
        continue;
      }

      timeval tm{};
      tm.tv_sec = 0;
      tm.tv_usec = 1000 * 120;

      FD_ZERO(&set);
      FD_SET(_socket, &set);

      if (0 == select(_socket + 1, nullptr, &set, nullptr, &tm)) {
        closesocket(_socket);
        _socket = INVALID_SOCKET;
        continue;
      }

      if (FD_ISSET(_socket, &set)) {
        if (getsockopt(_socket, SOL_SOCKET, SO_ERROR, (char *)&async_error, &async_error_len) < 0) {
          closesocket(_socket);
          _socket = INVALID_SOCKET;
        } else {
          async_mode = 0;
          ioctlsocket(_socket, FIONBIO, &async_mode); //设置为阻塞模式
          break;
        }
      } else {
        closesocket(_socket);
        _socket = INVALID_SOCKET;
      }
    }
  }

  if (_socket == INVALID_SOCKET) {
    // std::cout << "Unable to connect to server!" << std::endl;
    WSACleanup();
    _connected.store(false);
    return -1;
  }

  DWORD dw_timeout = timeout;
  if (setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&dw_timeout), sizeof(dw_timeout))) {
    std::cout << "setsockopt SO_SNDTIMEO fail: " << WSAGetLastError() << std::endl;
  }

  DWORD n_zero = 0; // set send without buffer
  setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&n_zero), sizeof(n_zero));

  freeaddrinfo(result);
  _connected.store(true);
  return 0;
}

int CanImpCanNet::connect2(const std::string &host, const std::string &service) {
  uint32_t port;
  disconnect();

  if (!boost::conversion::try_lexical_convert<uint32_t>(service, port)) {
    _connected.store(false);
    return -1;
  }

  _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (_socket == INVALID_SOCKET) {
    auto wsa_err = WSAGetLastError();
    _connected.store(false);
    return -1;
  }

  struct sockaddr_in serv_addr {};

  //以服务器地址填充结构serv_addr
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(host.c_str());
  serv_addr.sin_port = htons(port);

  timeval tm;
  int async_error = -1;
  int async_error_len = sizeof(int);
  fd_set set;

  unsigned long async_mode = 1;
  ioctlsocket(_socket, FIONBIO, &async_mode); //设置为非阻塞模式

  auto ret = true;
  if (::connect(_socket, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) == -1) {
    ret = false;
    tm.tv_sec = 0;
    tm.tv_usec = 1000 * 120;

    FD_ZERO(&set);
    FD_SET(_socket, &set);

    if (select(_socket + 1, nullptr, &set, nullptr, &tm) > 0) {
      getsockopt(_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&async_error), &async_error_len);
      ret = async_error == 0;
    }
  }

  async_mode = 0;
  ioctlsocket(_socket, FIONBIO, &async_mode); //设置为阻塞模式
  if (!ret) {
    closesocket(_socket);
    _connected.store(false);
    _socket = INVALID_SOCKET;
    return -1;
  }

  _connected.store(true);
  return 0;
}

void CanImpCanNet::disconnect() {
  if (_socket != INVALID_SOCKET) {
    closesocket(_socket);
    WSACleanup();

    _socket = INVALID_SOCKET;
  }

  _connected.store(false);
}

int CanImpCanNet::write_line(const std::string &line) const {
  auto const ierror = send(_socket, line.c_str(), (int)(line.length()), 0);
  if (ierror == SOCKET_ERROR) {
    std::cout << "write_line failed with error: " << WSAGetLastError() << std::endl;
    return -1;
  }

  return ierror;
}

/* socket peek:
 * https://stackoverflow.com/questions/12984816/get-the-number-of-bytes-available-in-socket-by-recv-with-msg-peek-in-c
 */
std::string CanImpCanNet::read_line(int timeout) {
  char buff[256] = {0};
  std::string line;
  DWORD dw_timeout = timeout;

  if (!_buffer_list_empty.load()) {
    if (buffer_list_pop(line)) // lock
      return line;
  }

#if 0
	if ( setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&dw_timeout), sizeof(dw_timeout)) ) {
		std::cout << "setsockopt SO_RCVTIMEO fail: " << WSAGetLastError() << std::endl;
		return _empty_string;
	}
#else
  FD_SET readfds;
  FD_ZERO(&readfds);
  FD_SET(_socket, &readfds);

  timeval tv_timeout{};
  tv_timeout.tv_sec = timeout / 1000;
  tv_timeout.tv_usec = (timeout - tv_timeout.tv_sec * 1000) * 1000;

  const auto select_ret = select(_socket + 1, &readfds, nullptr, nullptr, &tv_timeout);
  if (select_ret == 0) return _empty_string;
  if (select_ret == SOCKET_ERROR) {
    const auto err_code = WSAGetLastError();
    std::cout << "CanImpCanNet::read_line select error: " << err_code << std::endl;
    return _empty_string;
  }
#endif

  auto const bytes_read = recv(_socket, buff, sizeof(buff), 0);
  if (bytes_read > 0 && buff[bytes_read - 1] == '\n') {
    return read_line_post_process(buff, bytes_read); // lock
  }

  if (bytes_read > 0) {
    lock_enter();
    std::copy(buff, buff + bytes_read, std::back_inserter(_join_buffer));
    std::string join_str(_join_buffer.begin(), _join_buffer.end());

    if (!_join_buffer.empty()) {
      _join_buffer.erase(_join_buffer.begin(), _join_buffer.end());
    }
    lock_leave();

    return read_line_post_process(join_str.c_str(), join_str.length()); // lock
  }

  return _empty_string;
}

std::string CanImpCanNet::read_line_post_process(const char *buffer, size_t len) {
  if (len <= 0) return _empty_string;

  std::vector<std::string> vec_line;
  auto const buff_end_with_feed = (buffer[len - 1] == '\n');
  boost::split(vec_line, buffer, boost::is_any_of("\n"));

  if (!vec_line.empty()) {
    auto &first_str = vec_line.at(0);
    for (auto it = vec_line.begin() + 1; it != vec_line.end(); ++it) {
      if (!(*it).empty()) buffer_list_push(*it);
    }

    if (!buff_end_with_feed) {
      lock_enter();
      auto &left_str = vec_line.at(vec_line.size() - 1);
      std::copy(left_str.begin(), left_str.end(), std::back_inserter(_join_buffer));
      lock_leave();
    }

    return first_str;
  }

  return _empty_string;
}

void CanImpCanNet::buffer_list_init() {
  lock_enter();

  _buffer_list.clear();
  if (!_join_buffer.empty()) _join_buffer.erase(_join_buffer.begin(), _join_buffer.end());

  _buffer_list_empty.store(true);
  lock_leave();
}

void CanImpCanNet::buffer_list_push(const std::string &line) {
  lock_enter();
  _buffer_list.push_back(line);
  _buffer_list_empty.store(false);
  lock_leave();
}

bool CanImpCanNet::buffer_list_pop(std::string &line) {
  lock_enter();

  if (_buffer_list.empty()) {
    _buffer_list_empty.store(true);
    lock_leave();
    return false;
  }

  line = _buffer_list.front();
  _buffer_list.pop_front();

  if (_buffer_list.empty()) _buffer_list_empty.store(true);

  lock_leave();
  return true;
}
