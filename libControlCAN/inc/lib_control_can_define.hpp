#pragma once

#include "usbcan.h"
#include <boost/dll.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <memory>

#ifdef _WIN32
typedef DWORD(__stdcall* fVCI_OpenDevice)(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved);
typedef DWORD(__stdcall* fVCI_CloseDevice)(DWORD DeviceType, DWORD DeviceInd);
typedef DWORD(__stdcall* fVCI_ResetCAN)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
typedef DWORD(__stdcall* fVCI_InitCAN)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig);

typedef DWORD(__stdcall* fVCI_ReadBoardInfo)(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo);
typedef DWORD(__stdcall* fVCI_ReadErrInfo)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo);
typedef DWORD(__stdcall* fVCI_ReadCANStatus)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_STATUS pCANStatus);

typedef DWORD(__stdcall* fVCI_GetReference)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData);
typedef DWORD(__stdcall* fVCI_SetReference)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData);

typedef ULONG(__stdcall* fVCI_GetReceiveNum)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
typedef DWORD(__stdcall* fVCI_ClearBuffer)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);

typedef DWORD(__stdcall* fVCI_StartCAN)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
typedef ULONG(__stdcall* fVCI_Transmit)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, ULONG Len);
typedef ULONG(__stdcall* fVCI_Receive)(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len,
                                       INT WaitTime);

struct dll_load_dll_type_win {
  dll_load_dll_type_win(const std::string& dsoPath)
      : path(dsoPath) {
    hDll = LoadLibrary(path.c_str());
    if (nullptr == hDll) {
      std::cout << "load " << path << " fail." << std::endl;
      return;
    }

    fOpenDevice = reinterpret_cast<fVCI_OpenDevice>(GetProcAddress(hDll, "VCI_OpenDevice"));
    fCloseDevice = reinterpret_cast<fVCI_CloseDevice>(GetProcAddress(hDll, "VCI_CloseDevice"));
    fResetCan = reinterpret_cast<fVCI_ResetCAN>(GetProcAddress(hDll, "VCI_ResetCAN"));
    fInitCan = reinterpret_cast<fVCI_InitCAN>(GetProcAddress(hDll, "VCI_InitCAN"));
    fReadBoardInfo = reinterpret_cast<fVCI_ReadBoardInfo>(GetProcAddress(hDll, "VCI_ReadBoardInfo"));
    fReadErrInfo = reinterpret_cast<fVCI_ReadErrInfo>(GetProcAddress(hDll, "VCI_ReadErrInfo"));
    fReadCanStatus = reinterpret_cast<fVCI_ReadCANStatus>(GetProcAddress(hDll, "VCI_ReadCANStatus"));
    fGetReference = reinterpret_cast<fVCI_GetReference>(GetProcAddress(hDll, "VCI_GetReference"));
    fSetReference = reinterpret_cast<fVCI_SetReference>(GetProcAddress(hDll, "VCI_SetReference"));
    fGetReceiveNum = reinterpret_cast<fVCI_GetReceiveNum>(GetProcAddress(hDll, "VCI_GetReceiveNum"));
    fClearBuffer = reinterpret_cast<fVCI_ClearBuffer>(GetProcAddress(hDll, "VCI_ClearBuffer"));
    fStartCan = reinterpret_cast<fVCI_StartCAN>(GetProcAddress(hDll, "VCI_StartCAN"));
    fTransmit = reinterpret_cast<fVCI_Transmit>(GetProcAddress(hDll, "VCI_Transmit"));
    fReceive = reinterpret_cast<fVCI_Receive>(GetProcAddress(hDll, "VCI_Receive"));
  }

  ~dll_load_dll_type_win() {
    try {
      if (nullptr != hDll)
        FreeLibrary(hDll);
    } catch (std::exception& e) {
      std::cout << "lib_control_can_imp_dc dll_load_dll_type dtor: " << e.what() << std::endl;
    }
  }

  std::string path;
  HINSTANCE hDll{nullptr};

  fVCI_OpenDevice fOpenDevice{nullptr};
  fVCI_CloseDevice fCloseDevice{nullptr};
  fVCI_ResetCAN fResetCan{nullptr};
  fVCI_InitCAN fInitCan{nullptr};
  fVCI_ReadBoardInfo fReadBoardInfo{nullptr};

  fVCI_ReadErrInfo fReadErrInfo{nullptr};
  fVCI_ReadCANStatus fReadCanStatus{nullptr};

  fVCI_GetReference fGetReference{nullptr};
  fVCI_SetReference fSetReference{nullptr};

  fVCI_GetReceiveNum fGetReceiveNum{nullptr};
  fVCI_ClearBuffer fClearBuffer{nullptr};

  fVCI_StartCAN fStartCan{nullptr};
  fVCI_Transmit fTransmit{nullptr};
  fVCI_Receive fReceive{nullptr};
};
#endif

#ifdef _WIN32
#  define vci_call_conv __stdcall
#elif defined(__linux__)
#  define vci_call_conv
#endif

using vci_open_device_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD, DWORD)>;
using vci_close_device_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD)>;
using vci_reset_can_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD, DWORD)>;
using vci_init_can_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD, DWORD, PVCI_INIT_CONFIG)>;
using vci_read_board_info_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD, PVCI_BOARD_INFO)>;

using vci_read_err_info_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD, DWORD, PVCI_ERR_INFO)>;
using vci_read_can_status_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD, DWORD, PVCI_CAN_STATUS)>;

using vci_get_reference_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD, DWORD, DWORD, PVOID)>;
using vci_set_reference_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD, DWORD, DWORD, PVOID)>;

using vci_get_receive_num_t = boost::function<ULONG /*vci_call_conv*/ (DWORD, DWORD, DWORD)>;
using vci_clear_buffer_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD, DWORD)>;

using vci_start_can_t = boost::function<DWORD /*vci_call_conv*/ (DWORD, DWORD, DWORD)>;
using vci_transmit_t = boost::function<ULONG /*vci_call_conv*/ (DWORD, DWORD, DWORD, PVCI_CAN_OBJ, ULONG)>;
using vci_receive_t = boost::function<ULONG /*vci_call_conv*/ (DWORD, DWORD, DWORD, PVCI_CAN_OBJ, ULONG, INT)>;

struct dll_load_dll_type {
  dll_load_dll_type(const std::string& dsoPath)
      : dll_(dsoPath)
      , path(dsoPath) {
    using namespace boost::dll;
    // auto &sym_open_dev = dll_.get<DWORD __stdcall(DWORD, DWORD,
    // DWORD)>("VCI_OpenDevice"); auto &sym_close_dev = dll_.get<DWORD
    // __stdcall(DWORD, DWORD)>("VCI_CloseDevice");

    fOpenDevice = import_symbol<DWORD vci_call_conv(DWORD, DWORD, DWORD)>(dll_, "VCI_OpenDevice");
    fCloseDevice = import_symbol<DWORD vci_call_conv(DWORD, DWORD)>(dll_, "VCI_CloseDevice");
    fResetCan = import_symbol<DWORD vci_call_conv(DWORD, DWORD, DWORD)>(dll_, "VCI_ResetCAN");
    fInitCan = import_symbol<DWORD vci_call_conv(DWORD, DWORD, DWORD, PVCI_INIT_CONFIG)>(dll_, "VCI_InitCAN");
    fReadBoardInfo = import_symbol<DWORD vci_call_conv(DWORD, DWORD, PVCI_BOARD_INFO)>(dll_, "VCI_ReadBoardInfo");
    fReadErrInfo = import_symbol<DWORD vci_call_conv(DWORD, DWORD, DWORD, PVCI_ERR_INFO)>(dll_, "VCI_ReadErrInfo");
    fReadCanStatus = import_symbol<DWORD vci_call_conv(DWORD, DWORD, DWORD, PVCI_CAN_STATUS)>(dll_, "VCI_ReadCANStatus");
    fGetReference = import_symbol<DWORD vci_call_conv(DWORD, DWORD, DWORD, DWORD, PVOID)>(dll_, "VCI_GetReference");
    fSetReference = import_symbol<DWORD vci_call_conv(DWORD, DWORD, DWORD, DWORD, PVOID)>(dll_, "VCI_SetReference");
    fGetReceiveNum = import_symbol<ULONG vci_call_conv(DWORD, DWORD, DWORD)>(dll_, "VCI_GetReceiveNum");
    fClearBuffer = import_symbol<DWORD vci_call_conv(DWORD, DWORD, DWORD)>(dll_, "VCI_ClearBuffer");
    fStartCan = import_symbol<DWORD vci_call_conv(DWORD, DWORD, DWORD)>(dll_, "VCI_StartCAN");
    fTransmit = import_symbol<ULONG vci_call_conv(DWORD, DWORD, DWORD, PVCI_CAN_OBJ, ULONG)>(dll_, "VCI_Transmit");
    fReceive = import_symbol<ULONG vci_call_conv(DWORD, DWORD, DWORD, PVCI_CAN_OBJ, ULONG, INT)>(dll_, "VCI_Receive");
  }
  ~dll_load_dll_type() = default;

  boost::dll::shared_library dll_;
  std::string path;

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
