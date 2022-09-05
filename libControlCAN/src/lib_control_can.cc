#ifdef _MSC_VER
// #include "stdafx.h"
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4101)
#pragma warning(disable : 4819)
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

    vciReturnType VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd,
        DWORD Reserved) {
        _can_imp.reset(new CanImpCanNet());
        auto err = _can_imp->VCI_OpenDevice(DeviceType, DeviceInd, Reserved);
        if (vciReturnType::STATUS_NET_CONN_FAIL == err) {
            _can_imp.reset();
            std::cout << "open can dc." << std::endl;
            return VCI_DeirectOpenCan(DeviceType, DeviceInd, Reserved);
        } else {
            std::cout << "open can net." << std::endl;
        }

        return err;
    }

    vciReturnType VCI_DeirectOpenCan(DWORD DeviceType, DWORD DeviceInd,
        DWORD Reserved) {
        if (!_can_imp)
            _can_imp.reset(new CanImpDirectCan2());
        return _can_imp->VCI_OpenDevice(DeviceType, DeviceInd, Reserved);
    }

    vciReturnType VCI_OpenNetCan(DWORD DeviceType, DWORD DeviceInd,
        DWORD Reserved) {
        if (!_can_imp)
            _can_imp.reset(new CanImpCanNet());
        return _can_imp->VCI_OpenDevice(DeviceType, DeviceInd, Reserved);
    }

    vciReturnType VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) {
        vciReturnType err = vciReturnType::STATUS_ERR;
        if (_can_imp) {
            err = _can_imp->VCI_CloseDevice(DeviceType, DeviceInd);
            _can_imp.reset();
        }

        return err;
    }

    vciReturnType VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd,
        PVCI_BOARD_INFO pInfo) const {
        if (_can_imp) {
            return _can_imp->VCI_ReadBoardInfo(DeviceType, DeviceInd, pInfo);
        }

        return vciReturnType::STATUS_ERR;
    }

    vciReturnType VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
        PVCI_INIT_CONFIG pInitConfig) const {
        if (_can_imp) {
            return _can_imp->VCI_InitCAN(DeviceType, DeviceInd, CANInd, pInitConfig);
        }

        return vciReturnType::STATUS_ERR;
    }

    vciReturnType VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
        PVCI_ERR_INFO pErrInfo) const {
        if (_can_imp) {
            return _can_imp->VCI_ReadErrInfo(DeviceType, DeviceInd, CANInd, pErrInfo);
        }

        return vciReturnType::STATUS_ERR;
    }

    vciReturnType VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd,
        DWORD CANInd,
        PVCI_CAN_STATUS pCANStatus) const {
        if (_can_imp) {
            return _can_imp->VCI_ReadCANStatus(DeviceType, DeviceInd, CANInd,
                pCANStatus);
        }

        return vciReturnType::STATUS_ERR;
    }

    vciReturnType VCI_GetReference(DWORD DeviceType, DWORD DeviceInd,
        DWORD CANInd, DWORD RefType,
        PVOID pData) const {
        if (_can_imp) {
            return _can_imp->VCI_GetReference(DeviceType, DeviceInd, CANInd, RefType,
                pData);
        }

        return vciReturnType::STATUS_ERR;
    }

    vciReturnType VCI_SetReference(DWORD DeviceType, DWORD DeviceInd,
        DWORD CANInd, DWORD RefType,
        PVOID pData) const {
        if (_can_imp) {
            return _can_imp->VCI_SetReference(DeviceType, DeviceInd, CANInd, RefType,
                pData);
        }

        return vciReturnType::STATUS_ERR;
    }

    ULONG VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd,
        DWORD CANInd) const {
        if (_can_imp) {
            return _can_imp->VCI_GetReceiveNum(DeviceType, DeviceInd, CANInd);
        }

        return 0;
    }

    vciReturnType VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd,
        DWORD CANInd) const {
        if (_can_imp) {
            return _can_imp->VCI_ClearBuffer(DeviceType, DeviceInd, CANInd);
        }

        return vciReturnType::STATUS_ERR;
    }

    vciReturnType VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd,
        DWORD CANInd) const {
        if (_can_imp) {
            return _can_imp->VCI_StartCAN(DeviceType, DeviceInd, CANInd);
        }

        return vciReturnType::STATUS_ERR;
    }

    vciReturnType VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd,
        DWORD CANInd) const {
        if (_can_imp) {
            return _can_imp->VCI_ResetCAN(DeviceType, DeviceInd, CANInd);
        }

        return vciReturnType::STATUS_ERR;
    }

    vciReturnType VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
        PVCI_CAN_OBJ pSend, ULONG Len) const {
        if (_can_imp) {
            return _can_imp->VCI_Transmit(DeviceType, DeviceInd, CANInd, pSend, Len);
        }

        return vciReturnType::STATUS_ERR;
    }

    ULONG VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
        PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime) const {
        if (_can_imp) {
            return _can_imp->VCI_Receive(DeviceType, DeviceInd, CANInd, pReceive, Len,
                WaitTime);
        }

        return 0;
    }

private:
    std::unique_ptr<CanImpInterface> _can_imp;
};

// static std::unique_ptr<CanAbstract> gLibCC;
static CanAbstract gLibCC;

DWORD __stdcall VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd,
    DWORD Reserved) {
    // if ( !gLibCC ) gLibCC = make_unique<CanImpDirectCan>();
    return static_cast<DWORD>(
        gLibCC.VCI_OpenDevice(DeviceType, DeviceInd, Reserved));
}

DWORD __stdcall VCI_DeirectOpenCan(DWORD DeviceType, DWORD DeviceInd,
    DWORD Reserved) {
    return static_cast<DWORD>(
        gLibCC.VCI_DeirectOpenCan(DeviceType, DeviceInd, Reserved));
}

DWORD __stdcall VCI_OpenNetCan(DWORD DeviceType, DWORD DeviceInd,
    DWORD Reserved) {
    return static_cast<DWORD>(
        gLibCC.VCI_OpenNetCan(DeviceType, DeviceInd, Reserved));
}

DWORD __stdcall VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd) {
    return static_cast<DWORD>(gLibCC.VCI_CloseDevice(DeviceType, DeviceInd));
}

DWORD __stdcall VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd,
    PVCI_BOARD_INFO pInfo) {
    return static_cast<DWORD>(
        gLibCC.VCI_ReadBoardInfo(DeviceType, DeviceInd, pInfo));
}

DWORD __stdcall VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
    PVCI_INIT_CONFIG pInitConfig) {
    return static_cast<DWORD>(
        gLibCC.VCI_InitCAN(DeviceType, DeviceInd, CANInd, pInitConfig));
}

DWORD __stdcall VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
    PVCI_ERR_INFO pErrInfo) {
    return static_cast<DWORD>(
        gLibCC.VCI_ReadErrInfo(DeviceType, DeviceInd, CANInd, pErrInfo));
}

DWORD __stdcall VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd,
    DWORD CANInd, PVCI_CAN_STATUS pCANStatus) {
    return static_cast<DWORD>(
        gLibCC.VCI_ReadCANStatus(DeviceType, DeviceInd, CANInd, pCANStatus));
}

DWORD __stdcall VCI_GetReference(DWORD DeviceType, DWORD DeviceInd,
    DWORD CANInd, DWORD RefType, PVOID pData) {
    return static_cast<DWORD>(
        gLibCC.VCI_GetReference(DeviceType, DeviceInd, CANInd, RefType, pData));
}

DWORD __stdcall VCI_SetReference(DWORD DeviceType, DWORD DeviceInd,
    DWORD CANInd, DWORD RefType, PVOID pData) {
    return static_cast<DWORD>(
        gLibCC.VCI_SetReference(DeviceType, DeviceInd, CANInd, RefType, pData));
}

ULONG __stdcall VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd,
    DWORD CANInd) {
    return static_cast<ULONG>(
        gLibCC.VCI_GetReceiveNum(DeviceType, DeviceInd, CANInd));
}

DWORD __stdcall VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd,
    DWORD CANInd) {
    return static_cast<DWORD>(
        gLibCC.VCI_ClearBuffer(DeviceType, DeviceInd, CANInd));
}

DWORD __stdcall VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
    return static_cast<DWORD>(gLibCC.VCI_StartCAN(DeviceType, DeviceInd, CANInd));
}

DWORD __stdcall VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd) {
    return static_cast<DWORD>(gLibCC.VCI_ResetCAN(DeviceType, DeviceInd, CANInd));
}

ULONG __stdcall VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
    PVCI_CAN_OBJ pSend, ULONG Len) {
    return static_cast<ULONG>(
        gLibCC.VCI_Transmit(DeviceType, DeviceInd, CANInd, pSend, Len));
}

ULONG __stdcall VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd,
    PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime) {
    return gLibCC.VCI_Receive(DeviceType, DeviceInd, CANInd, pReceive, Len,
        WaitTime);
}