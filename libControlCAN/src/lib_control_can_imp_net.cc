#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
// #include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread.hpp>
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
/*using boost::lambda::_1;
using boost::lambda::bind;
using boost::lambda::var;*/

typedef Ini<> ini_t;
#define TCP_WRITE_TIMEOUT 200

#include <chrono>
#include <iostream>

using std::chrono::microseconds;

boost::xpressive::sregex CanImpCanNet::_hex_str_pattern(sregex::compile("^(?:[0-9a-fA-F][0-9a-fA-F])+$"));
// boost::xpressive::sregex CanImpCanNet::_receive_pattern(sregex::compile("^VCI_Receive[0-9a-fA-F]{32},"));
boost::xpressive::sregex CanImpCanNet::_receive_pattern(sregex::compile("^(VCI_Receive|VCI_Transmit)[0-9a-fA-F]{32},"));
boost::regex CanImpCanNet::_receive_line_feed_pattern("^VCI_Receive[0-9a-fA-F]{32},");
std::string CanImpCanNet::_empty_string;
int CanImpCanNet::_conn_timeout_ms = 300;

CanImpCanNet::CanImpCanNet() : _connected(false), _str_sock_addr(""), _str_sock_port(""), _socket(-1) {
  const auto dir_path = getexepath().parent_path();
  boost::filesystem::path ini_path(dir_path / "control_can.ini");

  if (boost::filesystem::exists(ini_path)) {
    ini_t ini(ini_path.string(), true);
    _str_sock_addr = ini["Server"]["IpAddr"];
    _str_sock_port = ini["Server"]["IpPort"];
  } else {
    // std::cout << "CanImpCanNet ctor: " << ini_file_name << " not exist." << std::endl;
  }

  _join_buffer.reserve(1024);
  buffer_list_init(); // init at end of ctor
}

CanImpCanNet::~CanImpCanNet() {
  if (_connected.load()) {
    _socket.Close();
  }
}

vciReturnType CanImpCanNet::VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) {
  buffer_list_init();

  if (_str_sock_addr.length() <= 0 || _str_sock_port.length() <= 0) {
    return vciReturnType::STATUS_NET_CONN_FAIL;
  }

  if (!_connected.load()) {
    auto ip_address = boost::lexical_cast<uint32_t>(_str_sock_addr);
    auto serv_port = boost::lexical_cast<uint16_t>(_str_sock_port);
    _socket.Connect(ip_address, serv_port);
    auto is_connected = _socket.IsConnected();
    _connected.store(is_connected);
  }
  if (!_connected.load()) {
    return vciReturnType::STATUS_NET_CONN_FAIL;
  }

  char buff[128];
  const char *head = "VCI_OpenDevice,", *lr = "\n";
  can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType);
  can::utils::bin2hex::bin2hex_fast(buff, &DeviceType, lr);
  can::utils::bin2hex::bin2hex_fast(buff, &DeviceInd);
  can::utils::bin2hex::bin2hex_fast(buff, &Reserved);
  can::utils::bin2hex::bin2hex_fast(buff, lr);
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType, &DeviceInd, &Reserved, lr);

  // std::cout << "VCI_OpenDevice: " << data << std::endl;
  auto ierror = write_line(buff, size);
  if (size != ierror) {
    // std::cout << "VCI_OpenDevice: write fail" << std::endl;
    return vciReturnType::STATUS_ERR;
  }

  return vciReturnType::STATUS_OK;
}

vciReturnType CanImpCanNet::VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) {
  if (!_connected.load()) return vciReturnType::STATUS_ERR;

  char buff[128];
  const char *head = "VCI_CloseDevice,", *lr = "\n";
  auto size = can::utils::bin2hex::bin2hex_fast(buff, head, &DeviceType, &DeviceInd, lr);

  auto const ierror = write_line(buff, size);
  if (size != ierror) {
    return vciReturnType::STATUS_ERR;
  }

  _socket.Close();
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

#if 0
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
    if (_socket == INV_SOCKET) {
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
        _socket = INV_SOCKET;
        continue;
      }

      timeval tm{};
      tm.tv_sec = 0;
      tm.tv_usec = 1000 * 120;

      FD_ZERO(&set);
      FD_SET(_socket, &set);

      if (0 == select(_socket + 1, nullptr, &set, nullptr, &tm)) {
        closesocket(_socket);
        _socket = INV_SOCKET;
        continue;
      }

      if (FD_ISSET(_socket, &set)) {
        if (getsockopt(_socket, SOL_SOCKET, SO_ERROR, (char *)&async_error, &async_error_len) < 0) {
          closesocket(_socket);
          _socket = INV_SOCKET;
        } else {
          async_mode = 0;
          ioctlsocket(_socket, FIONBIO, &async_mode); // ����Ϊ����ģʽ
          break;
        }
      } else {
        closesocket(_socket);
        _socket = INV_SOCKET;
      }
    }
  }

  if (_socket == INV_SOCKET) {
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
#endif

#if 0
int CanImpCanNet::connect2(const std::string &host, const std::string &service) {
  uint32_t port;
  disconnect();

  if (!boost::conversion::try_lexical_convert<uint32_t>(service, port)) {
    _connected.store(false);
    return -1;
  }

  _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (_socket == INV_SOCKET) {
    auto wsa_err = WSAGetLastError();
    _connected.store(false);
    return -1;
  }

  struct sockaddr_in serv_addr {};

  // �Է�������ַ���ṹserv_addr
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(host.c_str());
  serv_addr.sin_port = htons(port);

  timeval tm;
  int async_error = -1;
  int async_error_len = sizeof(int);
  fd_set set;

  unsigned long async_mode = 1;
  ioctlsocket(_socket, FIONBIO, &async_mode); // ����Ϊ������ģʽ

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
  ioctlsocket(_socket, FIONBIO, &async_mode); // ����Ϊ����ģʽ
  if (!ret) {
    closesocket(_socket);
    _connected.store(false);
    _socket = INV_SOCKET;
    return -1;
  }

  _connected.store(true);
  return 0;
}
#endif

#if 0
void CanImpCanNet::disconnect() {
  if (_socket != INV_SOCKET) {
    closesocket(_socket);
    WSACleanup();

    _socket = INV_SOCKET;
  }

  _connected.store(false);
}
#endif

int CanImpCanNet::write_line(const char *p, size_t len) const {
  auto const ierror = _socket.Send(p, (int)len);
  if (ierror != 0) {
    std::cout << "write_line error: " << ierror << std::endl;
  }

  return ierror;
}

int CanImpCanNet::write_line(const std::string &line) const { return write_line(line.data(), line.size()); }

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

  auto const bytes_read = _socket.Receive(buff, sizeof(buff));
  if (bytes_read > 0 && buff[bytes_read - 1] == '\n') {
    return read_line_post_process(buff, bytes_read); // lock
  }

  if (bytes_read > 0) {
    std::string join_str;
    {
      std::lock_guard lk(_buffer_mutex);
      std::copy(buff, buff + bytes_read, std::back_inserter(_join_buffer));
      join_str = std::string(_join_buffer.begin(), _join_buffer.end());

      if (!_join_buffer.empty()) {
        _join_buffer.erase(_join_buffer.begin(), _join_buffer.end());
      }
    }

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
      std::lock_guard lk(_buffer_mutex);
      auto &left_str = vec_line.at(vec_line.size() - 1);
      std::copy(left_str.begin(), left_str.end(), std::back_inserter(_join_buffer));
    }

    return first_str;
  }

  return _empty_string;
}

void CanImpCanNet::buffer_list_init() {
  std::lock_guard lk(_buffer_mutex);
  _buffer_list.clear();
  if (!_join_buffer.empty()) _join_buffer.erase(_join_buffer.begin(), _join_buffer.end());

  _buffer_list_empty.store(true);
}

void CanImpCanNet::buffer_list_push(const std::string &line) {
  std::lock_guard lk(_buffer_mutex);
  _buffer_list.push_back(line);
  _buffer_list_empty.store(false);
}

bool CanImpCanNet::buffer_list_pop(std::string &line) {
  std::lock_guard lk(_buffer_mutex);
  if (_buffer_list.empty()) {
    _buffer_list_empty.store(true);
    return false;
  }

  line = _buffer_list.front();
  _buffer_list.pop_front();

  if (_buffer_list.empty()) {
    _buffer_list_empty.store(true);
  }

  return true;
}

#include "lib_control_can_imp.h"
#ifdef __cplusplus
extern "C" {
#endif

LIBCC_DLL CanImpInterface *createCanNet() { return new CanImpCanNet(); }

#ifdef __cplusplus
}
#endif
