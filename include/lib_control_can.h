//
// Created by Administrator on 2017/8/30 0030.
//

#ifndef LIBCONTROLCAN_LIB_CONTROL_CAN_H
#define LIBCONTROLCAN_LIB_CONTROL_CAN_H

#include "usbcan.h"

#ifdef _MSC_VER

#ifdef BUILDING_LIBCC_DLL
#define LIBCC_DLL __declspec(dllexport)
#else
#define LIBCC_DLL __declspec(dllimport)
#endif

#define LIBCC_EXPORT_CALL LIBCC_DLL __stdcall

#else

#ifdef BUILDING_LIBCC_DLL
#define LIBCC_DLL //__declspec(dllexport)   //
                  //添加dllexport会使得导出函数添加前缀_imp_，导致调用方链接时找不到
#else
#define LIBCC_DLL //__declspec(dllimport)
#endif

#define LIBCC_EXPORT_CALL __stdcall LIBCC_DLL

#endif

#define LIBCC_CALLCONV __stdcall

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

#endif // LIBCONTROLCAN_LIB_CONTROL_CAN_H
