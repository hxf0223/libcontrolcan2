#pragma once

#include "usbcan.h"
#include <Windows.h>
#include <iostream>
#include <memory>

typedef DWORD(__stdcall *fVCI_OpenDevice)(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved);
typedef DWORD(__stdcall *fVCI_CloseDevice)(DWORD DeviceType, DWORD DeviceInd);
typedef DWORD(__stdcall *fVCI_ResetCAN)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
typedef DWORD(__stdcall *fVCI_InitCAN)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig);
typedef DWORD(__stdcall *fVCI_ReadBoardInfo)(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo);

typedef DWORD(__stdcall *fVCI_ReadErrInfo)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo);
typedef DWORD(__stdcall *fVCI_ReadCANStatus)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                                             PVCI_CAN_STATUS pCANStatus);

typedef DWORD(__stdcall *fVCI_GetReference)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType,
                                            PVOID pData);
typedef DWORD(__stdcall *fVCI_SetReference)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType,
                                            PVOID pData);

typedef ULONG(__stdcall *fVCI_GetReceiveNum)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
typedef DWORD(__stdcall *fVCI_ClearBuffer)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);

typedef DWORD(__stdcall *fVCI_StartCAN)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
typedef ULONG(__stdcall *fVCI_Transmit)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, ULONG Len);
typedef ULONG(__stdcall *fVCI_Receive)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive,
                                       ULONG Len, INT WaitTime);

struct dll_load_dll_type {
  dll_load_dll_type() = default;
  ~dll_load_dll_type() {
    // std::cout << "dll_load_dll_type dtor "
    //		<< path << " " << hDll << std::endl;
    try {
      if (nullptr != hDll) FreeLibrary(hDll);
    } catch (std::exception &e) {
      std::cout << "lib_control_can_imp_dc dll_load_dll_type dtor: " << e.what() << std::endl;
    }
  }

  std::string path;
  HINSTANCE hDll;

  fVCI_OpenDevice fOpenDevice;
  fVCI_CloseDevice fCloseDevice;
  fVCI_ResetCAN fResetCan;
  fVCI_InitCAN fInitCan;
  fVCI_ReadBoardInfo fReadBoardInfo;

  fVCI_ReadErrInfo fReadErrInfo;
  fVCI_ReadCANStatus fReadCanStatus;

  fVCI_GetReference fGetReference;
  fVCI_SetReference fSetReference;

  fVCI_GetReceiveNum fGetReceiveNum;
  fVCI_ClearBuffer fClearBuffer;

  fVCI_StartCAN fStartCan;
  fVCI_Transmit fTransmit;
  fVCI_Receive fReceive;
};
