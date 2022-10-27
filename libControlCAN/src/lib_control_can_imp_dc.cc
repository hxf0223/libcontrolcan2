// #include <Windows.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>

#include "hex_dump.hpp"
#include "ini.h"
#include "lib_control_can_imp_dc.hpp"
#include "misc.hpp"

#ifdef _MSC_VER
#  pragma warning(disable : 4101)
#endif

using ini_t = Ini<>;

CanImpDirectCan::CanImpDirectCan() {
  const auto dir_path = getexepath().parent_path();
  const boost::filesystem::path ini_path(dir_path / "kerneldlls" / "kerneldll.ini");

#ifdef ENABLE_DC_DEBUG
  std::cout << "ctor CanImpDirectCan: " << ini_path.string() << std::endl;
#endif

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

      auto temp_path = sub_path / key.second;
      if (!boost::filesystem::exists(temp_path)) {
        continue;
      }

      auto up = load_library_s(temp_path.string());
      if (!up) {
        continue;
      }

      _lib_map.insert(std::pair<uint32_t, std::unique_ptr<dll_load_dll_type>>(type, std::move(up)));
    }

#ifdef ENABLE_DC_DEBUG
    for (auto const& entry : _lib_map) {
      std::cout << entry.first << " = " << entry.second->path << std::endl;
    }
#endif
  } else {
    std::cout << "CanImpDirectCan ini file not exist" << std::endl;
  }
}

CanImpDirectCan::~CanImpDirectCan() {
#ifdef ENABLE_DC_DEBUG
  std::cout << "CanImpDirectCan dtor" << std::endl;
#endif
}

vciReturnType CanImpDirectCan::VCI_OpenDevice(DWORD deviceType, DWORD deviceInd, DWORD reserved) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fOpenDevice(deviceType, deviceInd, reserved);
#ifdef ENABLE_DC_DEBUG
    std::cout << "CanImpDirectCan::VCI_OpenDevice(" << DeviceType << ", " << DeviceInd << ", " << Reserved << "): " << err
              << std::endl;
#endif
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_CloseDevice(DWORD deviceType, DWORD deviceInd) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fCloseDevice(deviceType, deviceInd);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_ReadBoardInfo(DWORD deviceType, DWORD deviceInd, PVCI_BOARD_INFO pInfo) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fReadBoardInfo(deviceType, deviceInd, pInfo);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_InitCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_INIT_CONFIG pInitConfig) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
#ifdef ENABLE_DC_DEBUG
    std::cout << "AccCode: " << std::hex << (DWORD)(pInitConfig->AccCode) << ", ";
    std::cout << "AccMask: " << std::hex << (DWORD)(pInitConfig->AccMask) << ", ";
    std::cout << "Reserved: " << std::hex << (DWORD)(pInitConfig->Reserved) << ", ";
    std::cout << "Filter: " << std::hex << (DWORD)(pInitConfig->Filter) << ", ";
    std::cout << "Timing0: " << std::hex << (DWORD)(pInitConfig->Timing0) << ", ";
    std::cout << "Timing1: " << std::hex << (DWORD)(pInitConfig->Timing1) << ", ";
    std::cout << "Mode: " << std::hex << (DWORD)(pInitConfig->Mode) << std::endl;
#endif

    auto err = it->second->fInitCan(deviceType, deviceInd, canInd, pInitConfig);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_ReadErrInfo(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_ERR_INFO pErrInfo) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fReadErrInfo(deviceType, deviceInd, canInd, pErrInfo);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_ReadCANStatus(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_CAN_STATUS pCANStatus) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fReadCanStatus(deviceType, deviceInd, canInd, pCANStatus);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_GetReference(DWORD deviceType, DWORD deviceInd, DWORD canInd, DWORD refType, PVOID pData) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fGetReference(deviceType, deviceInd, canInd, refType, pData);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_SetReference(DWORD deviceType, DWORD deviceInd, DWORD canInd, DWORD refType, PVOID pData) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fSetReference(deviceType, deviceInd, canInd, refType, pData);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

DWORD CanImpDirectCan::VCI_GetReceiveNum(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto const err = it->second->fGetReceiveNum(deviceType, deviceInd, canInd);
    return err;
  }

  return 0;
}

vciReturnType CanImpDirectCan::VCI_ClearBuffer(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fClearBuffer(deviceType, deviceInd, canInd);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_StartCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  auto const it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fStartCan(deviceType, deviceInd, canInd);

#ifdef ENABLE_DC_DEBUG
    std::cout << "CanImpDirectCan::VCI_StartCAN(" << DeviceType << ", " << DeviceInd << ", " << CANInd << "): " << err << std::endl;
#endif
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_ResetCAN(DWORD deviceType, DWORD deviceInd, DWORD canInd) {
  auto it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fResetCan(deviceType, deviceInd, canInd);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_Transmit(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_CAN_OBJ pSend, ULONG len) {
  auto it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    // debug output
    /*std::cout << "CanImpDirectCan::VCI_Transmit, devType: " << DeviceType <<
    ", devId: " << DeviceInd << ", canId: "
    << CANInd << std::endl; for ( ULONG i = 0; i < Len && pSend; i++ ) {
    std::cout.setf(std::ios::hex, std::ios::basefield); std::cout <<
    "CanImpDirectCan::VCI_Transmit, ID: " << pSend[i].ID << ", TimeStamp:" <<
    pSend[i].TimeStamp; std::cout << ", TimeFlag: " << (UINT)(pSend[i].TimeFlag)
    << ", SendType: " << (UINT)(pSend[i].SendType) << ", RemoteFlag: " <<
    (UINT)(pSend[i].RemoteFlag); std::cout << ", ExternFlag: " <<
    (UINT)(pSend[i].ExternFlag) << ", DataLen:" << (UINT)(pSend[i].DataLen);

      std::cout.unsetf(std::ios::hex);
      std::cout << ", Data: " << can::utils::bin2hex_fast(pSend[i].Data, 8) <<
    std::endl;
    }*/
    auto err = it->second->fTransmit(deviceType, deviceInd, canInd, pSend, len);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

ULONG CanImpDirectCan::VCI_Receive(DWORD deviceType, DWORD deviceInd, DWORD canInd, PVCI_CAN_OBJ pReceive, ULONG len, INT waitTime) {
  const auto it = _lib_map.find(deviceType);
  if (it != _lib_map.end()) {
    auto const err = it->second->fReceive(deviceType, deviceInd, canInd, pReceive, len, waitTime);
#ifdef ENABLE_DC_DEBUG
    std::cout << "CanImpDirectCan::VCI_Receive(" << DeviceType << ", " << DeviceInd << ", " << CANInd << ", ..., " << Len << ", "
              << WaitTime << "): " << err << std::endl;
#endif
    return err;
  }

  return 0;
}

dll_load_dll_type* CanImpDirectCan::load_library(const std::string& path) {
  auto* dll = new dll_load_dll_type(path);
  return dll;
}

std::unique_ptr<dll_load_dll_type> CanImpDirectCan::load_library_s(const std::string& path) {
  return std::unique_ptr<dll_load_dll_type>(load_library(path));
}
