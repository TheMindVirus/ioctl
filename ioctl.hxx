#ifndef _IOCTL_HXX_
#define _IOCTL_HXX_

#define IOCTL_LENGTH   1500

#define DEVICE_NAME      "IOCTLMBOX"
#define DEVICE_DOSNAME   L"\\DosDevices\\" DEVICE_NAME
#define DEVICE_SYMLINK   L"\\\\.\\" DEVICE_NAME

#define FILE_DEVICE_VENDOR   0x8888   //====[ Vendor Device Types are limited to 31-bit (0x8000->0x7FFFFFFF) ]====//

#define IOCTL_COMMON_BIT   0x80000000
#define IOCTL_CUSTOM_BIT   0x00002000
#define VENDOR_BITMASK     IOCTL_COMMON_BIT | IOCTL_CUSTOM_BIT

#define VENDOR_CTL_CODE(CODE, METHOD)   CTL_CODE(FILE_DEVICE_VENDOR, CODE, METHOD, FILE_ANY_ACCESS) | VENDOR_BITMASK

//=========================[ Vendor Function Codes are limited to 11-bit (0x800->0xFFF) ]=========================//

#define IOCTL_MAILBOX   VENDOR_CTL_CODE(0x808, METHOD_BUFFERED)

//================================================================================================================//

#endif//_IOCTL_HXX_