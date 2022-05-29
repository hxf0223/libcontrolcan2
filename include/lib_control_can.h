//
// Created by Administrator on 2017/8/30 0030.
//

#pragma once

#include "usbcan.h"
#include "lib_control_can_exp.h"

#ifdef __cplusplus
extern "C" {
#endif

DWORD LIBCC_EXPORT_CALL VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved);
DWORD LIBCC_EXPORT_CALL VCI_DeirectOpenCan(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved);
DWORD LIBCC_EXPORT_CALL VCI_OpenNetCan(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved);
DWORD LIBCC_EXPORT_CALL VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd);
DWORD LIBCC_EXPORT_CALL VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo);
DWORD LIBCC_EXPORT_CALL VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig);
DWORD LIBCC_EXPORT_CALL VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo);
DWORD LIBCC_EXPORT_CALL VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_STATUS pCANStatus);
DWORD LIBCC_EXPORT_CALL VCI_GetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData);
DWORD LIBCC_EXPORT_CALL VCI_SetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData);
ULONG LIBCC_EXPORT_CALL VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
DWORD LIBCC_EXPORT_CALL VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
DWORD LIBCC_EXPORT_CALL VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
DWORD LIBCC_EXPORT_CALL VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
ULONG LIBCC_EXPORT_CALL VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, ULONG Len);
ULONG LIBCC_EXPORT_CALL VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len,
                                    INT WaitTime);

#ifdef __cplusplus
}
#endif


