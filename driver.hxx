#ifndef _DRIVER_HXX_
#define _DRIVER_HXX_

#include <ntddk.h>
#include <wdf.h>
#include "ioctl.hxx"

#define debug(...)   \
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__)); \
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n"));

typedef struct _EXTENSION
{
	SIZE_T nBytes;
	WDFMEMORY inMemory;
	WDFMEMORY outMemory;

	SIZE_T bytes;
	SIZE_T offset;
	SIZE_T min;
	PHYSICAL_ADDRESS base;
	PULONG address;
	PULONG reg;
	ULONG value;
	ULONG tmp;

}   EXTENSION, *PEXTENSION;
static EXTENSION ext;
#define KTAG_EXTENSION   (PVOID)1

extern "C"
{
	DRIVER_INITIALIZE DriverEntry;

	EVT_WDF_DRIVER_DEVICE_ADD DriverDeviceAdd;
	EVT_WDF_DRIVER_UNLOAD     DriverUnload;

	EVT_WDF_DEVICE_PREPARE_HARDWARE DevicePrepareHardware;
	EVT_WDF_DEVICE_RELEASE_HARDWARE DeviceReleaseHardware;
	EVT_WDF_DEVICE_D0_ENTRY         DevicePowerUp;
	EVT_WDF_DEVICE_D0_EXIT          DevicePowerDown;

	EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL DeviceIoControl;
}

#endif//_DRIVER_HXX_