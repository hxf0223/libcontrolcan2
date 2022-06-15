#ifndef USB_CAN_H
#define USB_CAN_H

#if defined WIN32 || defined _WIN32 || defined _WINDOWS
#include <ostream>
#include <stdint.h>
#include <windows.h>
#else
#include <ostream>
#include <stdint.h>

typedef uint16_t USHORT;
typedef uint8_t UCHAR;
typedef uint8_t BYTE;
typedef uint32_t DWORD;
typedef uint32_t ULONG;
typedef uint32_t UINT;

typedef int16_t SHORT;
typedef int8_t CHAR;
typedef int32_t INT;
typedef void *PVOID;

#ifndef __stdcall
#define __stdcall
#endif
#endif

// device type enum
enum class vciDevType : DWORD {
  VCI_PCI5121 = 1,
  VCI_PCI9810 = 2,
  VCI_USBCAN1 = 3,
  VCI_USBCAN2 = 4,
  VCI_USBCAN2A = 4,
  VCI_PCI9820 = 5,
  VCI_CAN232 = 6,
  VCI_PCI5110 = 7,
  VCI_CANLITE = 8,
  VCI_ISA9620 = 9,
  VCI_ISA5420 = 10,
  VCI_PC104CAN = 11,
  VCI_CANETUDP = 12,
  VCI_CANETE = 12,
  VCI_DNP9810 = 13,
  VCI_PCI9840 = 14,
  VCI_PC104CAN2 = 15,
  VCI_PCI9820I = 16,
  VCI_CANETTCP = 17,
  VCI_PEC9920 = 18,
  VCI_PCIE_9220 = 18,
  VCI_PCI5010U = 19,
  VCI_USBCAN_E_U = 20,
  VCI_USBCAN_2E_U = 21,
  VCI_PCI5020U = 22,
  VCI_EG20T_CAN = 23,
  VCI_PCIE9221 = 24,
  VCI_WIFICAN_TCP = 25,
  VCI_WIFICAN_UDP = 26,
  VCI_PCIe9120 = 27,
  VCI_PCIe9110 = 28,
  VCI_PCIe9140 = 29
};

// usbcan error define
#define ERR_CAN_OVERFLOW 0x0001
#define ERR_CAN_ERRALARM 0x0002
#define ERR_CAN_PASSIVE 0x0004
#define ERR_CAN_LOSE 0x0008
#define ERR_CAN_BUSERR 0x0010

// usbcan error define
#define ERR_DEVICEOPENED 0x0100
#define ERR_DEVICEOPEN 0x0200
#define ERR_DEVICENOTOPEN 0x0400
#define ERR_BUFFEROVERFLOW 0x0800
#define ERR_DEVICENOTEXIST 0x1000
#define ERR_LOADKERNELDLL 0x2000
#define ERR_CMDFAILED 0x4000
#define ERR_BUFFERCREATE 0x8000

// return error
enum class vciReturnType { STATUS_OK = 1, STATUS_ERR = 0, STATUS_NET_CONN_FAIL = -1 };

inline bool operator!=(const vciReturnType &lhs, const DWORD &rhs) { return (static_cast<DWORD>(lhs) != rhs); }

template <typename T>
inline std::ostream &operator<<(typename std::enable_if<std::is_enum<T>::value, std::ostream>::type &stream,
                                const T &e) {
  return stream << static_cast<typename std::underlying_type<T>::type>(e);
}

typedef struct _VCI_BOARD_INFO {
  USHORT hw_Version;
  USHORT fw_Version;
  USHORT dr_Version;
  USHORT in_Version;
  USHORT irq_Num;
  BYTE can_Num;
  CHAR str_Serial_Num[20];
  CHAR str_hw_Type[40];
  USHORT Reserved[4];
} VCI_BOARD_INFO, *PVCI_BOARD_INFO;

typedef struct _VCI_CAN_OBJ {
  UINT ID;
  UINT TimeStamp;
  BYTE TimeFlag;
  BYTE SendType;
  BYTE RemoteFlag;
  BYTE ExternFlag;
  BYTE DataLen;
  BYTE Data[8];
  BYTE Reserved[3];
} VCI_CAN_OBJ, *PVCI_CAN_OBJ;

typedef struct _VCI_CAN_STATUS {
  UCHAR ErrInterrupt;
  UCHAR regMode;
  UCHAR regStatus;
  UCHAR regALCapture;
  UCHAR regECCapture;
  UCHAR regEWLimit;
  UCHAR regRECounter;
  UCHAR regTECounter;
  DWORD Reserved;
} VCI_CAN_STATUS, *PVCI_CAN_STATUS;

typedef struct _ERR_INFO {
  UINT ErrCode;
  BYTE Passive_ErrData[3];
  BYTE ArLost_ErrData;
} VCI_ERR_INFO, *PVCI_ERR_INFO;

typedef struct _INIT_CONFIG {
  DWORD AccCode;
  DWORD AccMask;
  DWORD Reserved;
  UCHAR Filter;
  UCHAR Timing0;
  UCHAR Timing1;
  UCHAR Mode;
} VCI_INIT_CONFIG, *PVCI_INIT_CONFIG;

/*#ifdef __cplusplus
#define VCI_EXTERN		extern "C"
#else
#define VCI_EXTERN		extern
#endif


VCI_EXTERN DWORD __stdcall VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved);
VCI_EXTERN DWORD __stdcall VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd);
VCI_EXTERN DWORD __stdcall VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo);

VCI_EXTERN DWORD __stdcall VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig);
VCI_EXTERN DWORD __stdcall VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo);
VCI_EXTERN DWORD __stdcall VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_STATUS
pCANStatus);

VCI_EXTERN DWORD __stdcall VCI_GetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID
pData); VCI_EXTERN DWORD __stdcall VCI_SetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType,
PVOID pData);

VCI_EXTERN ULONG __stdcall VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
VCI_EXTERN DWORD __stdcall VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);

VCI_EXTERN DWORD __stdcall VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
VCI_EXTERN DWORD __stdcall VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);

VCI_EXTERN ULONG __stdcall VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, ULONG Len);
VCI_EXTERN ULONG __stdcall VCI_Receive(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pReceive, ULONG
Len, INT WaitTime);*/

#endif