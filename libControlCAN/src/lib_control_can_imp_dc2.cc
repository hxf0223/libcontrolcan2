// #include <Windows.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>

#include "hex_dump.hpp"
#include "ini.h"
#include "lib_control_can_imp_dc2.hpp"
#include "misc.hpp"

#ifdef _MSC_VER
#pragma warning(disable : 4101)
#endif

typedef Ini<> ini_t;

CanImpDirectCan2::CanImpDirectCan2() {}

CanImpDirectCan2::~CanImpDirectCan2() {
#ifdef ENABLE_DC_DEBUG
  std::cout << "CanImpDirectCan2 dtor" << std::endl;
#endif
}

vciReturnType CanImpDirectCan2::VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) {
  const auto dir_path = getexepath().parent_path();
  boost::filesystem::path ini_path(dir_path / "kerneldlls" / "kerneldll.ini");

  _dev_type.release();
  _lib.release();

  if (boost::filesystem::exists(ini_path)) {
    ini_t ini(ini_path.string(), true);
    boost::filesystem::path sub_path(dir_path);
    sub_path /= "kerneldlls";

    auto sec = ini["KERNELDLL"];
    for (const auto &key : sec) {
      uint32_t type;
      if (!boost::conversion::try_lexical_convert<uint32_t>(key.first, type)) continue;
      if (type != DeviceType) continue;

      auto temp_path = sub_path / key.second;
      if (!boost::filesystem::exists(temp_path)) continue;

      auto up = load_library_s(temp_path.string());
      if (!up) continue;

      _dev_type.reset(new uint32_t(type));
      _lib = std::move(up);
      break;
    }
  } else {
    std::cout << "CanImpDirectCan2 ini file not exist" << std::endl;
  }

  if (_dev_type && _lib) {
    auto err = _lib->fOpenDevice(DeviceType, DeviceInd, Reserved);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto err = _lib->fCloseDevice(DeviceType, DeviceInd);
    _dev_type.release();
    _lib.release();

    return static_cast<vciReturnType>(err);
  }

  _dev_type.release();
  _lib.release();

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto err = _lib->fReadBoardInfo(DeviceType, DeviceInd, pInfo);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                                            PVCI_INIT_CONFIG pInitConfig) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
#ifdef ENABLE_DC_DEBUG
    std::cout << "AccCode: " << std::hex << (DWORD)(pInitConfig->AccCode) << ", ";
    std::cout << "AccMask: " << std::hex << (DWORD)(pInitConfig->AccMask) << ", ";
    std::cout << "Reserved: " << std::hex << (DWORD)(pInitConfig->Reserved) << ", ";
    std::cout << "Filter: " << std::hex << (DWORD)(pInitConfig->Filter) << ", ";
    std::cout << "Timing0: " << std::hex << (DWORD)(pInitConfig->Timing0) << ", ";
    std::cout << "Timing1: " << std::hex << (DWORD)(pInitConfig->Timing1) << ", ";
    std::cout << "Mode: " << std::hex << (DWORD)(pInitConfig->Mode) << std::endl;
#endif

    auto err = _lib->fInitCan(DeviceType, DeviceInd, CANInd, pInitConfig);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                                                PVCI_ERR_INFO pErrInfo) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto err = _lib->fReadErrInfo(DeviceType, DeviceInd, CANInd, pErrInfo);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
                                                  PVCI_CAN_STATUS pCANStatus) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto err = _lib->fReadCanStatus(DeviceType, DeviceInd, CANInd, pCANStatus);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_GetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType,
                                                 PVOID pData) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto err = _lib->fGetReference(DeviceType, DeviceInd, CANInd, RefType, pData);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_SetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType,
                                                 PVOID pData) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto err = _lib->fSetReference(DeviceType, DeviceInd, CANInd, RefType, pData);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

DWORD CanImpDirectCan2::VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto const err = _lib->fGetReceiveNum(DeviceType, DeviceInd, CANInd);
    return err;
  }

  return 0;
}

vciReturnType CanImpDirectCan2::VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto err = _lib->fClearBuffer(DeviceType, DeviceInd, CANInd);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto err = _lib->fStartCan(DeviceType, DeviceInd, CANInd);

#ifdef ENABLE_DC_DEBUG
    std::cout << "CanImpDirectCan::VCI_StartCAN(" << DeviceType << ", " << DeviceInd << ", " << CANInd << "): " << err
              << std::endl;
#endif
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto err = _lib->fResetCan(DeviceType, DeviceInd, CANInd);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend,
                                             ULONG Len) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto err = _lib->fTransmit(DeviceType, DeviceInd, CANInd, pSend, Len);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

ULONG CanImpDirectCan2::VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len,
                                    INT WaitTime) {
  if (_dev_type && _lib && *_dev_type == DeviceType) {
    auto const err = _lib->fReceive(DeviceType, DeviceInd, CANInd, pReceive, Len, WaitTime);
#ifdef ENABLE_DC_DEBUG
    std::cout << "CanImpDirectCan::VCI_Receive(" << DeviceType << ", " << DeviceInd << ", " << CANInd << ", ..., "
              << Len << ", " << WaitTime << "): " << err << std::endl;
#endif
    return err;
  }

  return 0;
}

dll_load_dll_type *CanImpDirectCan2::load_library(std::string path) {
  auto dll = new dll_load_dll_type(path);
  return dll;
}

std::unique_ptr<dll_load_dll_type> CanImpDirectCan2::load_library_s(std::string path) {
  return std::unique_ptr<dll_load_dll_type>(load_library(path));
}

#include "lib_control_can_imp.h"
#ifdef __cplusplus
extern "C" {
#endif

LIBCC_DLL CanImpInterface *createCanDC() { return new CanImpDirectCan2(); }

#ifdef __cplusplus
}
#endif
