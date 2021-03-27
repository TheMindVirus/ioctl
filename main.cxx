#include <stdio.h>
#include <Windows.h>
#include "ioctl.hxx"

#define debug(...)   \
	printf(__VA_ARGS__); \
	printf("\n");

#define OpenHandle(SYMLINK)                                  CreateFileW(SYMLINK, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)
#define DeviceControl(DEVICE, INBUFFER, OUTBUFFER, NBYTES)   DeviceIoControl(DEVICE, IOCTL_SOCKET, INBUFFER, IOCTL_LENGTH, OUTBUFFER, IOCTL_LENGTH, NBYTES, NULL);

int main()
{
	debug("[INFO]: Begin Device I/O Control");
	BOOL result = FALSE;
	CHAR inBuffer[IOCTL_LENGTH] = "Hello World";
	CHAR outBuffer[IOCTL_LENGTH] = "";
	DWORD nBytes = 0;

	HANDLE device = OpenHandle(DEVICE_SYMLINK);
	if (device == INVALID_HANDLE_VALUE) { debug("[WARN]: OpenHandle Failed (0x%08lX)", GetLastError()); return -1; }

	result = DeviceControl(device, inBuffer, outBuffer, &nBytes);
	if (!result) { debug("[WARN]: DeviceControl Failed (0x%08lX)", GetLastError()); return -2; }

	debug("[INFO]: inBuffer = %s", inBuffer);
	debug("[INFO]: outBuffer = %s", outBuffer);
	debug("[INFO]: nBytes = %d", nBytes);

	CloseHandle(device);
	debug("[INFO]: Device I/O Control Complete");
	system("pause");
	return 0;
}