#pragma once

#ifdef _WIN32
#include <sdkddkver.h> // avoid boost.asio warning: Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately
#endif

#include "Easysocket.h"
#include "lib_control_can_imp.h"
#include "usbcan.h"

#include <mutex> // std::mutex, std::unique_lock
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/regex.hpp>
#include <boost/xpressive/xpressive.hpp>

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
  int connect(const std::string &host, const std::string &service, int timeoutMs);
  void io_context_run(const std::chrono::steady_clock::duration &timeout);

  std::string read_line(const std::chrono::steady_clock::duration &timeout, boost::system::error_code &ec);
  inline int write_line(const char *p, size_t len, boost::system::error_code &ec);

private:
  boost::atomic_bool _connected;
  std::string _str_sock_addr;
  std::string _str_sock_port;

  boost::asio::io_context io_context_;
  boost::asio::ip::tcp::socket client_socket_{io_context_};
  std::string input_buffer_;

private:
  static boost::xpressive::sregex _hex_str_pattern;
  static boost::xpressive::sregex _receive_pattern;
  static boost::regex _receive_line_feed_pattern;
};