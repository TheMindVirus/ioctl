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

	PHYSICAL_ADDRESS base;
	PHYSICAL_ADDRESS max;
	PUINT32 buffer;

	PULONG mbox_base;
	PULONG mbox_packet;

	KSPIN_LOCK lock;
	KIRQL prevState;

	UINT32 attempt;
	BOOLEAN critical;

	UINT32 checked;
	UINT32 mail;

	UINT32 timeout1;
	UINT32 timeout2;

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
	UINT32 MailboxExchange(UINT8 channel);
}

#define PERI_BASE      0xFE000000
#define PERI_MASK      0x3FFFFFFF
#define MBOX_BASE      0x0000B880 + PERI_BASE
#define MBOX_SIZE      0x24
#define MBOX_DATA      0x10000000

#define MBOX_READ      0 //0x00
#define MBOX_POLL      4 //0x10
#define MBOX_SENDER    5 //0x14
#define MBOX_STATUS    6 //0x18
#define MBOX_CONFIG    7 //0x1C
#define MBOX_WRITE     8 //0x20

#define MBOX_REQUEST   0x00000000
#define MBOX_EMPTY     0x40000000
#define MBOX_FULL      0x80000000
#define MBOX_SUCCESS   0x80000000
#define MBOX_FAILURE   0x80000001

#define MBOX_TIMEOUT   1000000 //microseconds
#define MBOX_RETRIES   10 //times

#define MBOX_CHANNEL   8

#define mmio_read(base, offset)           (READ_REGISTER_ULONG((PULONG)(base + offset)))
#define mmio_write(base, offset, value)   (WRITE_REGISTER_ULONG((PULONG)(base + offset), value))

#define mbox_peek()                       (mmio_read((ext.mbox_base), MBOX_STATUS))
#define mbox_read()                       (mmio_read((ext.mbox_base), MBOX_READ))
#define mbox_write(addrech)               (mmio_write((ext.mbox_base), MBOX_WRITE, addrech))

#endif//_DRIVER_HXX_