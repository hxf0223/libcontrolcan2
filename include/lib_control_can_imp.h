#pragma once

#include "lib_control_can_exp.h"
#include "usbcan.h"

class CanImpInterface {
public:
  CanImpInterface() = default;
  CanImpInterface(const CanImpInterface &) = delete;
  CanImpInterface(CanImpInterface &&) = delete;
  CanImpInterface &operator=(const CanImpInterface &) = delete;
  CanImpInterface &operator=(CanImpInterface &&) = delete;
  virtual ~CanImpInterface() = default;

  virtual vciReturnType VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) = 0;
  virtual vciReturnType VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) = 0;
  virtual vciReturnType VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo) = 0;

  virtual vciReturnType VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig) = 0;
  virtual vciReturnType VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo) = 0;
  virtual vciReturnType VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                                          PVCI_CAN_STATUS pCANStatus) = 0;

  virtual vciReturnType VCI_GetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType,
                                         PVOID pData) = 0;
  virtual vciReturnType VCI_SetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType,
                                         PVOID pData) = 0;

  virtual DWORD VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) = 0;
  virtual vciReturnType VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) = 0;

  virtual vciReturnType VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) = 0;
  virtual vciReturnType VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) = 0;

  virtual vciReturnType VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend,
                                     ULONG Len) = 0;
  virtual ULONG VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len,
                            INT WaitTime) = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

LIBCC_DLL CanImpInterface *createCanDC();
LIBCC_DLL CanImpInterface *createCanNet();

#ifdef __cplusplus
}
#endif
