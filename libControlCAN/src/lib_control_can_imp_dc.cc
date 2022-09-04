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
#pragma warning(disable : 4101)
#endif

typedef Ini<> ini_t;

CanImpDirectCan::CanImpDirectCan() {
  const auto dir_path = getexepath().parent_path();
  boost::filesystem::path ini_path(dir_path / "kerneldlls" / "kerneldll.ini");

#ifdef ENABLE_DC_DEBUG
  std::cout << "ctor CanImpDirectCan: " << ini_path.string() << std::endl;
#endif

  if (boost::filesystem::exists(ini_path)) {
    ini_t ini(ini_path.string(), true);
    boost::filesystem::path sub_path(dir_path);
    sub_path /= "kerneldlls";

    auto sec = ini["KERNELDLL"];
    for (const auto &key : sec) {
      uint32_t type;
      if (!boost::conversion::try_lexical_convert<uint32_t>(key.first, type))
        continue;

      auto temp_path = sub_path / key.second;
      if (!boost::filesystem::exists(temp_path))
        continue;

      auto up = load_library_s(temp_path.string());
      if (!up)
        continue;

      _lib_map.insert(std::pair<uint32_t, std::unique_ptr<dll_load_dll_type>>(
          type, std::move(up)));
    }

#ifdef ENABLE_DC_DEBUG
    for (auto const &entry : _lib_map) {
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

vciReturnType CanImpDirectCan::VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd,
                                              DWORD Reserved) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fOpenDevice(DeviceType, DeviceInd, Reserved);
#ifdef ENABLE_DC_DEBUG
    std::cout << "CanImpDirectCan::VCI_OpenDevice(" << DeviceType << ", "
              << DeviceInd << ", " << Reserved << "): " << err << std::endl;
#endif
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_CloseDevice(DWORD DeviceType,
                                               DWORD DeviceInd) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fCloseDevice(DeviceType, DeviceInd);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_ReadBoardInfo(DWORD DeviceType,
                                                 DWORD DeviceInd,
                                                 PVCI_BOARD_INFO pInfo) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fReadBoardInfo(DeviceType, DeviceInd, pInfo);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd,
                                           DWORD CANInd,
                                           PVCI_INIT_CONFIG pInitConfig) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
#ifdef ENABLE_DC_DEBUG
    std::cout << "AccCode: " << std::hex << (DWORD)(pInitConfig->AccCode)
              << ", ";
    std::cout << "AccMask: " << std::hex << (DWORD)(pInitConfig->AccMask)
              << ", ";
    std::cout << "Reserved: " << std::hex << (DWORD)(pInitConfig->Reserved)
              << ", ";
    std::cout << "Filter: " << std::hex << (DWORD)(pInitConfig->Filter) << ", ";
    std::cout << "Timing0: " << std::hex << (DWORD)(pInitConfig->Timing0)
              << ", ";
    std::cout << "Timing1: " << std::hex << (DWORD)(pInitConfig->Timing1)
              << ", ";
    std::cout << "Mode: " << std::hex << (DWORD)(pInitConfig->Mode)
              << std::endl;
#endif

    auto err = it->second->fInitCan(DeviceType, DeviceInd, CANInd, pInitConfig);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_ReadErrInfo(DWORD DeviceType,
                                               DWORD DeviceInd, DWORD CANInd,
                                               PVCI_ERR_INFO pErrInfo) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto err =
        it->second->fReadErrInfo(DeviceType, DeviceInd, CANInd, pErrInfo);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_ReadCANStatus(DWORD DeviceType,
                                                 DWORD DeviceInd, DWORD CANInd,
                                                 PVCI_CAN_STATUS pCANStatus) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto err =
        it->second->fReadCanStatus(DeviceType, DeviceInd, CANInd, pCANStatus);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_GetReference(DWORD DeviceType,
                                                DWORD DeviceInd, DWORD CANInd,
                                                DWORD RefType, PVOID pData) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fGetReference(DeviceType, DeviceInd, CANInd, RefType,
                                         pData);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_SetReference(DWORD DeviceType,
                                                DWORD DeviceInd, DWORD CANInd,
                                                DWORD RefType, PVOID pData) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fSetReference(DeviceType, DeviceInd, CANInd, RefType,
                                         pData);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

DWORD CanImpDirectCan::VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd,
                                         DWORD CANInd) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto const err = it->second->fGetReceiveNum(DeviceType, DeviceInd, CANInd);
    return err;
  }

  return 0;
}

vciReturnType CanImpDirectCan::VCI_ClearBuffer(DWORD DeviceType,
                                               DWORD DeviceInd, DWORD CANInd) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fClearBuffer(DeviceType, DeviceInd, CANInd);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd,
                                            DWORD CANInd) {
  auto const it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fStartCan(DeviceType, DeviceInd, CANInd);

#ifdef ENABLE_DC_DEBUG
    std::cout << "CanImpDirectCan::VCI_StartCAN(" << DeviceType << ", "
              << DeviceInd << ", " << CANInd << "): " << err << std::endl;
#endif
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd,
                                            DWORD CANInd) {
  auto it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto err = it->second->fResetCan(DeviceType, DeviceInd, CANInd);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

vciReturnType CanImpDirectCan::VCI_Transmit(DWORD DeviceType, DWORD DeviceInd,
                                            DWORD CANInd, PVCI_CAN_OBJ pSend,
                                            ULONG Len) {
  auto it = _lib_map.find(DeviceType);
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
    auto err = it->second->fTransmit(DeviceType, DeviceInd, CANInd, pSend, Len);
    return static_cast<vciReturnType>(err);
  }

  return vciReturnType::STATUS_ERR;
}

ULONG CanImpDirectCan::VCI_Receive(DWORD DeviceType, DWORD DeviceInd,
                                   DWORD CANInd, PVCI_CAN_OBJ pReceive,
                                   ULONG Len, INT WaitTime) {
  const auto it = _lib_map.find(DeviceType);
  if (it != _lib_map.end()) {
    auto const err = it->second->fReceive(DeviceType, DeviceInd, CANInd,
                                          pReceive, Len, WaitTime);
#ifdef ENABLE_DC_DEBUG
    std::cout << "CanImpDirectCan::VCI_Receive(" << DeviceType << ", "
              << DeviceInd << ", " << CANInd << ", ..., " << Len << ", "
              << WaitTime << "): " << err << std::endl;
#endif
    return err;
  }

  return 0;
}

dll_load_dll_type *CanImpDirectCan::load_library(const std::string &path) {
  auto dll = new dll_load_dll_type(path);
  return dll;
}

std::unique_ptr<dll_load_dll_type>
CanImpDirectCan::load_library_s(const std::string &path) {
  return std::unique_ptr<dll_load_dll_type>(load_library(path));
}
