#pragma once

#include "lib_control_can_imp.h"
#include "usbcan.h"

#include <atomic>
#include <boost/asio/streambuf.hpp>
#include <boost/system.hpp>
#include <functional>
#include <iterator>
#include <mutex> // std::mutex, std::unique_lock
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include <boost/asio.hpp>
#include <boost/atomic.hpp>

class CanImpCanNet : public CanImpInterface {
  using read_line_cb_t = std::function<int(const std::string_view&, VCI_CAN_OBJ*)>;
  using dur_t = std::chrono::steady_clock::duration;
  using error_code_t = boost::system::error_code;

public:
  CanImpCanNet();
  virtual ~CanImpCanNet();
  CanImpCanNet(const CanImpCanNet&) = delete;
  CanImpCanNet(CanImpCanNet&&) = delete;
  CanImpCanNet& operator=(const CanImpCanNet&) = delete;
  CanImpCanNet& operator=(CanImpCanNet&&) = delete;

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

  ULONG VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime) override;

private:
  struct asyncReadParam {
    PVCI_CAN_OBJ can_objs_{nullptr};
    uint32_t len_{0}, read_cnt_{0};
  };

  int connect(const std::string& host, const std::string& service, int timeoutMs);

  void async_read(std::atomic<asyncReadParam>& param, error_code_t& ec);
  inline int write_line(const char* p, size_t len, error_code_t& ec);
  static int line_process(const std::string_view& str, PVCI_CAN_OBJ dst);

private:
  boost::atomic_bool _connected;
  std::string _str_sock_addr;
  std::string _str_sock_port;

  boost::asio::io_context io_context_;
  boost::asio::streambuf rx_buff_;
  boost::asio::ip::tcp::socket client_socket_{io_context_};
  boost::asio::deadline_timer timer_{io_context_};

private:
  static std::regex _hex_str_pattern2;
  static std::regex _receive_pattern2;
};