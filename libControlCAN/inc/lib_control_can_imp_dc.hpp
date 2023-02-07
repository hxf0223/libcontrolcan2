#pragma once

#include "usbcan.h"
#include <map>
#include <memory>
#include <string>

#include "lib_control_can_define.hpp"
#include "lib_control_can_imp.h"

class CanImpDirectCan : public CanImpInterface {
public:
  CanImpDirectCan();
  CanImpDirectCan(const CanImpDirectCan&) = delete;
  CanImpDirectCan(CanImpDirectCan&&) = delete;
  CanImpDirectCan& operator=(const CanImpDirectCan&) = delete;
  CanImpDirectCan& operator=(CanImpDirectCan&&) = delete;
  ~CanImpDirectCan() override;

  vciReturnType VCI_OpenDevice(DWORD deviceType, DWORD deviceInd, DWORD reserved) override;
  vciReturnType VCI_CloseDevice(DWORD deviceType, DWORD deviceInd) override;
  vciReturnType VCI_ReadBoardInfo(DWORD deviceType, DWORD deviceInd, PVCI_BOARD_INFO pInfo) override;

  vciReturnType VCI_InitCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_INIT_CONFIG pInitConfig) override;
  vciReturnType VCI_ReadErrInfo(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_ERR_INFO pErrInfo) override;
  vciReturnType VCI_ReadCANStatus(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_CAN_STATUS pCANStatus) override;

  vciReturnType VCI_GetReference(DWORD deviceType, DWORD deviceInd, DWORD canInd, DWORD refType, PVOID pData) override;
  vciReturnType VCI_SetReference(DWORD deviceType, DWORD deviceInd, DWORD canInd, DWORD refType, PVOID pData) override;

  DWORD VCI_GetReceiveNum(DWORD deviceType, DWORD deviceInd, DWORD canInd) override;
  vciReturnType VCI_ClearBuffer(DWORD deviceType, DWORD deviceInd, DWORD canInd) override;

  vciReturnType VCI_StartCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd) override;
  vciReturnType VCI_ResetCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd) override;

  vciReturnType VCI_Transmit(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_CAN_OBJ pSend, ULONG len) override;
  ULONG VCI_Receive(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_CAN_OBJ pReceive, ULONG len, INT waitTime) override;

private:
  static dll_load_dll_type* load_library(const std::string& path);
  static std::unique_ptr<dll_load_dll_type> load_library_s(const std::string& path);

private:
  std::map<uint32_t, std::unique_ptr<dll_load_dll_type>> lib_map_;
};
