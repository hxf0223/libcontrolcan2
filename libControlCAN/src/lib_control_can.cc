#ifdef _MSC_VER
// #include "stdafx.h"
#endif

#ifdef _MSC_VER
#  pragma warning(disable : 4101)
#  pragma warning(disable : 4819)
#endif

#include "lib_control_can.h"
#include "lib_control_can_imp.h"
#include "lib_control_can_imp_dc.hpp"
#include "lib_control_can_imp_dc2.hpp"
#include "lib_control_can_imp_net.hpp"
#include <memory>

// there maybe some inheritance chain
// in abstract implementation if needed
class CanAbstract {
public:
  CanAbstract() = default;
  CanAbstract(const CanAbstract&) = delete;
  CanAbstract(CanAbstract&&) = delete;
  CanAbstract& operator=(const CanAbstract&) = delete;
  CanAbstract& operator=(CanAbstract&&) = delete;
  virtual ~CanAbstract() = default;

  vciReturnType VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) { // NOLINT
    can_imp_ = std::make_unique<CanImpCanNet>();
    auto err = can_imp_->VCI_OpenDevice(DeviceType, DeviceInd, Reserved);
    if (vciReturnType::STATUS_NET_CONN_FAIL == err) {
      can_imp_.reset();
      return VCI_DeirectOpenCan(DeviceType, DeviceInd, Reserved);
    }

    return err;
  }

  vciReturnType VCI_DeirectOpenCan(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) { // NOLINT
    if (!can_imp_) {
      can_imp_ = std::make_unique<CanImpDirectCan2>();
    }
    return can_imp_->VCI_OpenDevice(DeviceType, DeviceInd, Reserved);
  }

  vciReturnType VCI_OpenNetCan(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) { // NOLINT
    if (!can_imp_) {
      can_imp_ = std::make_unique<CanImpCanNet>();
    }
    return can_imp_->VCI_OpenDevice(DeviceType, DeviceInd, Reserved);
  }

  vciReturnType VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) { // NOLINT
    vciReturnType err = vciReturnType::STATUS_ERR;
    if (can_imp_) {
      err = can_imp_->VCI_CloseDevice(DeviceType, DeviceInd);
      can_imp_.reset();
    }

    return err;
  }

  vciReturnType VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_ReadBoardInfo(DeviceType, DeviceInd, pInfo);
    }

    return vciReturnType::STATUS_ERR;
  }

  vciReturnType VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_InitCAN(DeviceType, DeviceInd, CANInd, pInitConfig);
    }

    return vciReturnType::STATUS_ERR;
  }

  vciReturnType VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_ReadErrInfo(DeviceType, DeviceInd, CANInd, pErrInfo);
    }

    return vciReturnType::STATUS_ERR;
  }

  vciReturnType VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_STATUS pCANStatus) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_ReadCANStatus(DeviceType, DeviceInd, CANInd, pCANStatus);
    }

    return vciReturnType::STATUS_ERR;
  }

  vciReturnType VCI_GetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_GetReference(DeviceType, DeviceInd, CANInd, RefType, pData);
    }

    return vciReturnType::STATUS_ERR;
  }

  vciReturnType VCI_SetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_SetReference(DeviceType, DeviceInd, CANInd, RefType, pData);
    }

    return vciReturnType::STATUS_ERR;
  }

  ULONG VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_GetReceiveNum(DeviceType, DeviceInd, CANInd);
    }

    return 0;
  }

  vciReturnType VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_ClearBuffer(DeviceType, DeviceInd, CANInd);
    }

    return vciReturnType::STATUS_ERR;
  }

  vciReturnType VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_StartCAN(DeviceType, DeviceInd, CANInd);
    }

    return vciReturnType::STATUS_ERR;
  }

  vciReturnType VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_ResetCAN(DeviceType, DeviceInd, CANInd);
    }

    return vciReturnType::STATUS_ERR;
  }

  vciReturnType VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, ULONG Len) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_Transmit(DeviceType, DeviceInd, CANInd, pSend, Len);
    }

    return vciReturnType::STATUS_ERR;
  }

  ULONG VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime) const { // NOLINT
    if (can_imp_) {
      return can_imp_->VCI_Receive(DeviceType, DeviceInd, CANInd, pReceive, Len, WaitTime);
    }

    return 0;
  }

private:
  std::unique_ptr<CanImpInterface> can_imp_;
};

// static std::unique_ptr<CanAbstract> gLibCC;
static CanAbstract g_lib_cc;

DWORD __stdcall VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) { // NOLINT
  // if ( !gLibCC ) gLibCC = make_unique<CanImpDirectCan>();
  return static_cast<DWORD>(g_lib_cc.VCI_OpenDevice(DeviceType, DeviceInd, Reserved));
}

DWORD __stdcall VCI_DeirectOpenCan(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_DeirectOpenCan(DeviceType, DeviceInd, Reserved));
}

DWORD __stdcall VCI_OpenNetCan(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_OpenNetCan(DeviceType, DeviceInd, Reserved));
}

DWORD __stdcall VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_CloseDevice(DeviceType, DeviceInd));
}

DWORD __stdcall VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_ReadBoardInfo(DeviceType, DeviceInd, pInfo));
}

DWORD __stdcall VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_InitCAN(DeviceType, DeviceInd, CANInd, pInitConfig));
}

DWORD __stdcall VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_ReadErrInfo(DeviceType, DeviceInd, CANInd, pErrInfo));
}

DWORD __stdcall VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_STATUS pCANStatus) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_ReadCANStatus(DeviceType, DeviceInd, CANInd, pCANStatus));
}

DWORD __stdcall VCI_GetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_GetReference(DeviceType, DeviceInd, CANInd, RefType, pData));
}

DWORD __stdcall VCI_SetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_SetReference(DeviceType, DeviceInd, CANInd, RefType, pData));
}

ULONG __stdcall VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) { // NOLINT
  return static_cast<ULONG>(g_lib_cc.VCI_GetReceiveNum(DeviceType, DeviceInd, CANInd));
}

DWORD __stdcall VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_ClearBuffer(DeviceType, DeviceInd, CANInd));
}

DWORD __stdcall VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_StartCAN(DeviceType, DeviceInd, CANInd));
}

DWORD __stdcall VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) { // NOLINT
  return static_cast<DWORD>(g_lib_cc.VCI_ResetCAN(DeviceType, DeviceInd, CANInd));
}

ULONG __stdcall VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, ULONG Len) { // NOLINT
  return static_cast<ULONG>(g_lib_cc.VCI_Transmit(DeviceType, DeviceInd, CANInd, pSend, Len));
}

// NOLINTNEXTLINE
ULONG __stdcall VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime) {
  return g_lib_cc.VCI_Receive(DeviceType, DeviceInd, CANInd, pReceive, Len, WaitTime);
}