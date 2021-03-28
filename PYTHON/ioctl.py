from win32 import win32file

IOCTL_DEVICE = "\\\\.\\IOCTL"
IOCTL_SOCKET = 0x88882000
IOCTL_LENGTH = 1500

inBuffer = bytes("Hello, World", "UTF-8")
outBuffer = win32file.AllocateReadBuffer(IOCTL_LENGTH)
for i in range(0, IOCTL_LENGTH): #IMPORTANT ZEROMEMORY OPERATION
    outBuffer[i] = 0

FILE_READ_WRITE = win32file.FILE_SHARE_READ | win32file.FILE_SHARE_WRITE

params = (0, FILE_READ_WRITE, None, win32file.OPEN_EXISTING, 0, None)
handle = win32file.CreateFileW(IOCTL_DEVICE, *(params))

try:
    params = (handle, IOCTL_SOCKET, inBuffer, outBuffer, None)
    win32file.DeviceIoControl(*(params))
except:
    raise
finally:
    win32file.CloseHandle(handle)
    print(outBuffer.tobytes().decode())
