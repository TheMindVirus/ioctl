#include <stdio.h>
#include <Windows.h>
#include "ioctl.hxx"

#define debug(...)   \
	printf(__VA_ARGS__); \
	printf("\n");

#define OpenHandle(SYMLINK)                                  CreateFileW(SYMLINK, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)
#define DeviceControl(DEVICE, INBUFFER, OUTBUFFER, NBYTES)   DeviceIoControl(DEVICE, IOCTL_MAILBOX, INBUFFER, IOCTL_LENGTH, OUTBUFFER, IOCTL_LENGTH, NBYTES, NULL);

int main()
{
	debug("[INFO]: Begin Device I/O Control");
	BOOL result = FALSE;
	UINT32 inPacket[IOCTL_LENGTH] = { 0 };
	UINT32 outPacket[IOCTL_LENGTH] = { 0 };
	DWORD inSize = 0;
	DWORD outSize = 0;

	HANDLE device = OpenHandle(DEVICE_SYMLINK);
	if (device == INVALID_HANDLE_VALUE) { debug("[WARN]: OpenHandle Failed (0x%08lX)", GetLastError()); return -1; }

	inPacket[inSize++] = 0x00000000; //Mailbox Request
	inPacket[inSize++] = 0x00010006; //RPI_MBOX_GET_VC_MEMSIZE
	inPacket[inSize++] = 0x00000000; //Request Data Length
	inPacket[inSize++] = 0x00000008; //Response Data Length

	inPacket[inSize++] = 0x00000000; //End Mark
	inPacket[0] = inSize * 4;        //Update Packet Size

	result = DeviceControl(device, inPacket, outPacket, &outSize);
	if (!result) { debug("[WARN]: DeviceControl Failed (0x%08lX)", GetLastError()); return -2; }
	outSize /= 4;

	debug("[INFO]: inSize = %d", inSize);
	debug("[INFO]: outSize = %d", outSize);
	for (SIZE_T i = 0; i < inSize; ++i) { debug("[INFO]: inPacket[%llu] = 0x%08lX", i, inPacket[i]); }
	for (SIZE_T o = 0; o < outSize; ++o) { debug("[INFO]: outPacket[%llu] = 0x%08lX", o, outPacket[o]); }

	CloseHandle(device);
	debug("[INFO]: Device I/O Control Complete");
	system("pause");
	return 0;
}