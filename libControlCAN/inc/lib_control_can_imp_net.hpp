#pragma once

#include "usbcan.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#include <mutex> // std::mutex, std::unique_lock
#include <string>
#include <vector>

#include "lib_control_can_imp.h"
#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/regex.hpp>
#include <boost/xpressive/xpressive.hpp>

#define BUFFER_LOCK_CS

class CanImpCanNet : public CanImpInterface {
public:
  CanImpCanNet();
  virtual ~CanImpCanNet();
  CanImpCanNet(const CanImpCanNet &) = delete;
  CanImpCanNet(CanImpCanNet &&) = delete;
  CanImpCanNet &operator=(const CanImpCanNet &) = delete;
  CanImpCanNet &operator=(CanImpCanNet &&) = delete;

  vciReturnType VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) override;
  vciReturnType VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) override;
  vciReturnType VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo) override;

  vciReturnType VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig) override;
  vciReturnType VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo) override;
  vciReturnType VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_STATUS pCANStatus) override;

  vciReturnType VCI_GetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData) override;
  vciReturnType VCI_SetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData) override;

  DWORD VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) override;
  vciReturnType VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) override;

  vciReturnType VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) override;
  vciReturnType VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) override;

  vciReturnType VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, ULONG Len) override;
  ULONG VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len,
                    INT WaitTime) override;

private:
  ULONG vci_receive_tool(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len,
                         INT WaitTime);

private:
  int connect(const std::string &host, const std::string &service, int timeout);
  int connect2(const std::string &host, const std::string &service);
  void disconnect();

  int write_line(const std::string &line) const;
  std::string read_line(int timeout);
  std::string read_line_post_process(const char *buffer, size_t len);

  void buffer_list_init();
  void buffer_list_push(const std::string &line);
  bool buffer_list_pop(std::string &line);

private:
  boost::atomic_bool _connected;
  std::string _str_sock_addr;
  std::string _str_sock_port;
  SOCKET _socket;

#ifdef BUFFER_LOCK_CS
  // http://memleap.com/windows/multithreading/performance-comparison-stdmutex-c11-critical_section-win32-mutex-win32/
  // https://stoyannk.wordpress.com/2016/04/30/msvc-mutex-is-slower-than-you-might-expect/
  CRITICAL_SECTION _buffer_cs;
#else
  std::mutex _buffer_mutex;
#endif

  std::list<std::string> _buffer_list;
  std::vector<char> _join_buffer;
  boost::atomic_bool _buffer_list_empty;

private:
  static boost::xpressive::sregex _hex_str_pattern;
  static boost::xpressive::sregex _receive_pattern;
  static boost::regex _receive_line_feed_pattern;
  static std::string _empty_string;
  static int _conn_timeout_ms;
};