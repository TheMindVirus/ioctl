""" ### Linux
import fcntl, ctypes, termios

ioctl = fcntl.ioctl

SOUND_MIXER_READ_VOLUME = 0x80044D00
SOUND_MIXER_WRITE_VOLUME = 0xC0044D00

volume = ioctl(0, SOUND_MIXER_READ_VOLUME)
print(volume)
"""

IOCTL_COMMON_BIT = 0x80000000
IOCTL_CUSTOM_BIT = 0x00002000
VENDOR_BITMASK = IOCTL_COMMON_BIT | IOCTL_CUSTOM_BIT

FILE_DEVICE_VENDOR = 0x8888
FILE_ANY_ACCESS = 0
FILE_READ_ACCESS = 1
FILE_WRITE_ACCESS = 2

METHOD_BUFFERED = 0
METHOD_IN_DIRECT = 1
METHOD_OUT_DIRECT = 2
METHOD_NEITHER = 3

def CTL_CODE(DeviceType, Function, Method, Access):
    return ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method)

def VENDOR_CTL_CODE(CODE, METHOD):
    return CTL_CODE(FILE_DEVICE_VENDOR, CODE, METHOD, FILE_ANY_ACCESS) | VENDOR_BITMASK

IOCTL_SOCKET = VENDOR_CTL_CODE(0x800, METHOD_BUFFERED)
IOCTL_HWTEST = VENDOR_CTL_CODE(0x801, METHOD_NEITHER)
print("[IOCTL_SOCKET]: 0x{:08x}".format(IOCTL_SOCKET))
print("[IOCTL_HWTEST]: 0x{:08x}".format(IOCTL_HWTEST))
#[IOCTL_SOCKET]: 0x88882000
#[IOCTL_HWTEST]: 0x88882007

FILE_DEVICE_BATTERY = 0x00000029
IOCTL_QUERY_DEVICE_POWER_STATE = CTL_CODE(FILE_DEVICE_BATTERY, 0x0,
                                          METHOD_BUFFERED, FILE_READ_ACCESS)

FILE_DEVICE_USB = 34
USB_GET_HUB_CAPABILITIES_EX = 276
IOCTL_USB_GET_HUB_CAPABILITIES_EX = CTL_CODE(FILE_DEVICE_USB,
    USB_GET_HUB_CAPABILITIES_EX, METHOD_BUFFERED, FILE_ANY_ACCESS)

#IOCTL_CUSTOM = IOCTL_QUERY_DEVICE_POWER_STATE
IOCTL_CUSTOM = IOCTL_USB_GET_HUB_CAPABILITIES_EX

#pip3 install pywin32
from win32 import win32file

print(win32file.QueryDosDevice)
print(win32file.DeviceIoControl)

# List Devices
DOSdevices = win32file.QueryDosDevice(None).split("\0")
#print(DOSdevices)
print("Discovered:")
MIXER_PATH = "USB#VID_0D8C&PID_0014"
for i in range(0, len(DOSdevices)):
    if MIXER_PATH in DOSdevices[i]:
        print(DOSdevices[i])
print("Selected:")
MIXER_PATH = "USB#VID_0D8C&PID_0014#6&1e95266d&0&3#{a5dcbf10-6530-11d2-901f-00c04fb951ed}"
for i in range(0, len(DOSdevices)):
    if MIXER_PATH in DOSdevices[i]:
        print(DOSdevices[i])
print("Handle:")
MIXER_PATH = "\\??\\" + MIXER_PATH #win32file.QueryDosDevice(MIXER_PATH)
#MIXER_PATH = "\\\\.\\0000008E"
#MIXER_PATH = "\\\\.\\Device\\0000008E"
#MIXER_PATH = "DosDevices/0000008E"
#MIXER_PATH = "\\Device\\0000003D"
#MIXER_PATH = "./pyioctl.txt"
#MIXER_PATH = "\\??\\PCI#VEN_8086&DEV_8D31&SUBSYS_86001043&REV_05#3&11583659&0&A0#{3abf6f2d-71c4-462a-8a92-1e6861e6af27}"
print(MIXER_PATH) # Tries to resolve via network

FILE_READ_WRITE = win32file.FILE_SHARE_READ | win32file.FILE_SHARE_WRITE
MIXER_HANDLE = win32file.CreateFile(MIXER_PATH, 0, FILE_READ_WRITE, None,
                                    win32file.OPEN_EXISTING, 0, None)

# Get Device Capabilities
SOUND_MIXER_READ_VOLUME = 0x80044D00
SOUND_MIXER_WRITE_VOLUME = 0xC0044D00
#IOCTL_HELP / IOCTL_DOCS / IOCTL_ENUM

# Communicate with Device
device_handle = MIXER_HANDLE
device_method = IOCTL_CUSTOM # SOUND_MIXER_READ_VOLUME # IOCTL Code
input_buffer = bytes()
output_buffer = win32file.AllocateReadBuffer(4096)
for i in range(0, 1500):
    output_buffer[i] = 0
volume = win32file.DeviceIoControl(device_handle, device_method,
                                   input_buffer, output_buffer)
volume = volume.tobytes()
print("[INFO]: Result:", volume)

# It did not read the volume, instead it queried the USB Sound Card
# for USB Hub Capabilities and it returned an empty string b'' on Windows 10
