#pragma once

#include "usbcan.h"
#include <boost/dll.hpp>
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

#ifdef _WIN32
#define vci_call_conv __stdcall
#elif defined(__linux__)
#define vci_call_conv
#endif

using vci_open_device_t = std::function<vci_call_conv DWORD(DWORD, DWORD, DWORD)>;
using vci_close_device_t = std::function<vci_call_conv DWORD(DWORD, DWORD)>;
using vci_reset_can_t = std::function<vci_call_conv DWORD(DWORD, DWORD, DWORD)>;
using vci_init_can_t = std::function<vci_call_conv DWORD(DWORD, DWORD, DWORD, PVCI_INIT_CONFIG)>;
using vci_read_board_info_t = std::function<vci_call_conv DWORD(DWORD, DWORD, PVCI_BOARD_INFO)>;

using vci_read_err_info_t = std::function<vci_call_conv DWORD(DWORD, DWORD, DWORD, PVCI_ERR_INFO)>;
using vci_read_can_status_t = std::function<vci_call_conv DWORD(DWORD, DWORD, DWORD, PVCI_CAN_STATUS)>;

using vci_get_reference_t = std::function<vci_call_conv DWORD(DWORD, DWORD, DWORD, DWORD, PVOID)>;
using vci_set_reference_t = std::function<vci_call_conv DWORD(DWORD, DWORD, DWORD, DWORD, PVOID)>;

using vci_get_receive_num_t = std::function<vci_call_conv ULONG(DWORD, DWORD, DWORD)>;
using vci_clear_buffer_t = std::function<vci_call_conv DWORD(DWORD, DWORD, DWORD)>;

using vci_start_can_t = std::function<vci_call_conv DWORD(DWORD, DWORD, DWORD)>;
using vci_transmit_t = std::function<vci_call_conv ULONG(DWORD, DWORD, DWORD, PVCI_CAN_OBJ, ULONG)>;
using vci_receive_t = std::function<vci_call_conv ULONG(DWORD, DWORD, DWORD, PVCI_CAN_OBJ, ULONG, INT)>;

struct dll_load_dll_type {
  dll_load_dll_type(const std::string &dsoPath) : dll_(dsoPath), path(dsoPath) {
    fOpenDevice = dll_.get<vci_open_device_t>("VCI_OpenDevice");
    fCloseDevice = dll_.get<vci_close_device_t>("VCI_CloseDevice");
    fResetCan = dll_.get<vci_reset_can_t>("VCI_ResetCAN");
    fInitCan = dll_.get<vci_init_can_t>("VCI_InitCAN");
    fReadBoardInfo = dll_.get<vci_read_board_info_t>("VCI_ReadBoardInfo");
    fReadErrInfo = dll_.get<vci_read_err_info_t>("VCI_ReadErrInfo");
    fReadCanStatus = dll_.get<vci_read_can_status_t>("VCI_ReadCANStatus");
    fGetReference = dll_.get<vci_get_reference_t>("VCI_GetReference");
    fSetReference = dll_.get<vci_set_reference_t>("VCI_SetReference");
    fGetReceiveNum = dll_.get<vci_get_receive_num_t>("VCI_GetReceiveNum");
    fClearBuffer = dll_.get<vci_clear_buffer_t>("VCI_ClearBuffer");
    fStartCan = dll_.get<vci_start_can_t>("VCI_StartCAN");
    fTransmit = dll_.get<vci_transmit_t>("VCI_Transmit");
    fReceive = dll_.get<vci_receive_t>("VCI_Receive");
  }
  ~dll_load_dll_type() = default;

  boost::dll::shared_library dll_;
  std::string path;
  // HINSTANCE hDll;

  vci_open_device_t fOpenDevice;
  vci_close_device_t fCloseDevice;
  vci_reset_can_t fResetCan;
  vci_init_can_t fInitCan;
  vci_read_board_info_t fReadBoardInfo;

  vci_read_err_info_t fReadErrInfo;
  vci_read_can_status_t fReadCanStatus;

  vci_get_reference_t fGetReference;
  vci_set_reference_t fSetReference;

  vci_get_receive_num_t fGetReceiveNum;
  vci_clear_buffer_t fClearBuffer;

  vci_start_can_t fStartCan;
  vci_transmit_t fTransmit;
  vci_receive_t fReceive;
};
