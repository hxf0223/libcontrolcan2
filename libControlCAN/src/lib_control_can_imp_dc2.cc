// #include <Windows.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "hex_dump.hpp"
#include "ini.h"
#include "lib_control_can_imp_dc2.hpp"
#include "misc.hpp"

#ifdef _MSC_VER
#  pragma warning(disable : 4101)
#endif

using ini_t = Ini<>;

CanImpDirectCan2::CanImpDirectCan2() {}

CanImpDirectCan2::~CanImpDirectCan2() {
#ifdef ENABLE_DC_DEBUG
  std::cout << "CanImpDirectCan2 dtor" << std::endl;
#endif
}

vciReturnType CanImpDirectCan2::VCI_OpenDevice(DWORD deviceType, DWORD deviceInd, DWORD reserved) {
  const auto dir_path = getexepath().parent_path();
  boost::filesystem::path const ini_path(dir_path / "kerneldlls" / "kerneldll.ini");

  _dev_type.reset();
  _lib.reset();

  if (boost::filesystem::exists(ini_path)) {
    ini_t ini(ini_path.string(), true);
    boost::filesystem::path sub_path(dir_path);
    sub_path /= "kerneldlls";

    auto sec = ini["KERNELDLL"];
    for (const auto& key : sec) {
      uint32_t type;
      if (!boost::conversion::try_lexical_convert<uint32_t>(key.first, type)) {
        continue;
      }
      if (type != deviceType) {
        continue;
      }

      auto temp_path = sub_path / key.second;
      if (!boost::filesystem::exists(temp_path)) {
        continue;
      }

      auto up = load_library_s(temp_path.string());
      if (!up) {
        continue;
      }

      _dev_type = std::make_unique<uint32_t>(type);
      _lib = std::move(up);
      break;
    }
  } else {
    std::cout << "CanImpDirectCan2 ini file not exist" << std::endl;
  }

  if (_dev_type && _lib) {
    auto err = _lib->fOpenDevice(deviceType, deviceInd, reserved);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_CloseDevice(DWORD deviceType, DWORD deviceInd) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto err = _lib->fCloseDevice(deviceType, deviceInd);
    _dev_type.reset();
    _lib.reset();

    return static_cast<vciReturnType>(err);
  }

  _dev_type.reset();
  _lib.reset();

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_ReadBoardInfo(DWORD deviceType, DWORD deviceInd, PVCI_BOARD_INFO pInfo) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto err = _lib->fReadBoardInfo(deviceType, deviceInd, pInfo);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_InitCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_INIT_CONFIG pInitConfig) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
#ifdef ENABLE_DC_DEBUG
    std::cout << "AccCode: " << std::hex << (DWORD)(pInitConfig->AccCode) << ", ";
    std::cout << "AccMask: " << std::hex << (DWORD)(pInitConfig->AccMask) << ", ";
    std::cout << "Reserved: " << std::hex << (DWORD)(pInitConfig->Reserved) << ", ";
    std::cout << "Filter: " << std::hex << (DWORD)(pInitConfig->Filter) << ", ";
    std::cout << "Timing0: " << std::hex << (DWORD)(pInitConfig->Timing0) << ", ";
    std::cout << "Timing1: " << std::hex << (DWORD)(pInitConfig->Timing1) << ", ";
    std::cout << "Mode: " << std::hex << (DWORD)(pInitConfig->Mode) << std::endl;
#endif

    auto err = _lib->fInitCan(deviceType, deviceInd, canInd, pInitConfig);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_ReadErrInfo(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_ERR_INFO pErrInfo) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto err = _lib->fReadErrInfo(deviceType, deviceInd, canInd, pErrInfo);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_ReadCANStatus(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_CAN_STATUS pCANStatus) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto err = _lib->fReadCanStatus(deviceType, deviceInd, canInd, pCANStatus);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_GetReference(DWORD deviceType, DWORD deviceInd, DWORD canInd, DWORD refType, PVOID pData) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto err = _lib->fGetReference(deviceType, deviceInd, canInd, refType, pData);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_SetReference(DWORD deviceType, DWORD deviceInd, DWORD canInd, DWORD refType, PVOID pData) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto err = _lib->fSetReference(deviceType, deviceInd, canInd, refType, pData);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

DWORD CanImpDirectCan2::VCI_GetReceiveNum(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto const err = _lib->fGetReceiveNum(deviceType, deviceInd, canInd);
    return err;
  }

  return 0;
}

vciReturnType CanImpDirectCan2::VCI_ClearBuffer(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto err = _lib->fClearBuffer(deviceType, deviceInd, canInd);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_StartCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto err = _lib->fStartCan(deviceType, deviceInd, canInd);

#ifdef ENABLE_DC_DEBUG
    std::cout << "CanImpDirectCan::VCI_StartCAN(" << DeviceType << ", " << DeviceInd << ", " << CANInd << "): " << err << std::endl;
#endif
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_ResetCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto err = _lib->fResetCan(deviceType, deviceInd, canInd);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan2::VCI_Transmit(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_CAN_OBJ pSend, ULONG len) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto err = _lib->fTransmit(deviceType, deviceInd, canInd, pSend, len);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

ULONG CanImpDirectCan2::VCI_Receive(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_CAN_OBJ pReceive, ULONG len, INT waitTime) {
  if (_dev_type && _lib && *_dev_type == deviceType) {
    auto const err = _lib->fReceive(deviceType, deviceInd, canInd, pReceive, len, waitTime);
#ifdef ENABLE_DC_DEBUG
    std::cout << "CanImpDirectCan::VCI_Receive(" << DeviceType << ", " << DeviceInd << ", " << CANInd << ", ..., " << Len << ", "
              << WaitTime << "): " << err << std::endl;
#endif
    return err;
  }

  return 0;
}

dll_load_dll_type* CanImpDirectCan2::load_library(const std::string& path) {
  // auto dll_win = dll_load_dll_type_win(path);
  auto* dll = new dll_load_dll_type(path);
  return dll;
}

std::unique_ptr<dll_load_dll_type> CanImpDirectCan2::load_library_s(std::string path) { // NOLINT
  return std::unique_ptr<dll_load_dll_type>(load_library(path));
}

#include "lib_control_can_imp.h"
#ifdef __cplusplus
extern "C" {
#endif

LIBCC_DLL CanImpInterface* createCanDC() {
  return new CanImpDirectCan2();
}

#ifdef __cplusplus
}
#endif
