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
  CanImpDirectCan(const CanImpDirectCan &) = delete;
  CanImpDirectCan(CanImpDirectCan &&) = delete;
  CanImpDirectCan &operator=(const CanImpDirectCan &) = delete;
  CanImpDirectCan &operator=(CanImpDirectCan &&) = delete;
  virtual ~CanImpDirectCan();

  vciReturnType VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd,
                               DWORD Reserved) override;
  vciReturnType VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) override;
  vciReturnType VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd,
                                  PVCI_BOARD_INFO pInfo) override;

  vciReturnType VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                            PVCI_INIT_CONFIG pInitConfig) override;
  vciReturnType VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                                PVCI_ERR_INFO pErrInfo) override;
  vciReturnType VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd,
                                  DWORD CANInd,
                                  PVCI_CAN_STATUS pCANStatus) override;

  vciReturnType VCI_GetReference(DWORD DeviceType, DWORD DeviceInd,
                                 DWORD CANInd, DWORD RefType,
                                 PVOID pData) override;
  vciReturnType VCI_SetReference(DWORD DeviceType, DWORD DeviceInd,
                                 DWORD CANInd, DWORD RefType,
                                 PVOID pData) override;

  DWORD VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd,
                          DWORD CANInd) override;
  vciReturnType VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd,
                                DWORD CANInd) override;

  vciReturnType VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd,
                             DWORD CANInd) override;
  vciReturnType VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd,
                             DWORD CANInd) override;

  vciReturnType VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                             PVCI_CAN_OBJ pSend, ULONG Len) override;
  ULONG VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                    PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime) override;

private:
  static dll_load_dll_type *load_library(const std::string &path);
  static std::unique_ptr<dll_load_dll_type>
  load_library_s(const std::string &path);

private:
  std::map<uint32_t, std::unique_ptr<dll_load_dll_type>> _lib_map;
};
