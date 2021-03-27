#include "driver.hxx"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (INIT, DriverEntry)
#endif

NTSTATUS DriverEntry
(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
)
{
    debug("[CALL]: DriverEntry");
    NTSTATUS status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG DriverConfig = { 0 };

    status = IoAllocateDriverObjectExtension(DriverObject, KTAG_EXTENSION, sizeof(EXTENSION), (PVOID*)&ext);
    if (NT_ERROR(status)) { debug("[WARN]: IoAllocateDriverObjectExtension Failed (0x%08lX)", status); return status; }

    WDF_DRIVER_CONFIG_INIT(&DriverConfig, DriverDeviceAdd);
    DriverConfig.EvtDriverUnload = DriverUnload;

    status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &DriverConfig, WDF_NO_HANDLE);
    if (NT_ERROR(status)) { debug("[WARN]: WdfDriverCreate Failed (0x%08lX)", status); return status; }
    return STATUS_SUCCESS;
}

NTSTATUS DriverDeviceAdd
(
    WDFDRIVER Driver,
    PWDFDEVICE_INIT DeviceInit
)
{
    debug("[CALL]: DriverDeviceAdd");
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING symlink = { 0 };
    WDF_PNPPOWER_EVENT_CALLBACKS configPnP = { 0 };
    WDF_IO_QUEUE_CONFIG configIO = { 0 };
    WDFDEVICE device = { 0 };
    WDFQUEUE queue = { 0 };
    
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&configPnP);
    configPnP.EvtDevicePrepareHardware = DevicePrepareHardware;
    configPnP.EvtDeviceReleaseHardware = DeviceReleaseHardware;
    configPnP.EvtDeviceD0Entry = DevicePowerUp;
    configPnP.EvtDeviceD0Exit = DevicePowerDown;
    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &configPnP);

    status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &device);
    if (NT_ERROR(status)) { debug("[WARN]: WdfDeviceCreate Failed (0x%08lX)", status); return status; }

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&configIO, WdfIoQueueDispatchParallel);
    configIO.EvtIoDeviceControl = DeviceIoControl;
    configIO.EvtIoInternalDeviceControl = DeviceIoControl;

    status = WdfIoQueueCreate(device, &configIO, WDF_NO_OBJECT_ATTRIBUTES, &queue);
    if (NT_ERROR(status)) { debug("[WARN]: WdfIoQueueCreate Failed (0x%08lX)", status); return status; }

    RtlInitUnicodeString(&symlink, DEVICE_DOSNAME);
    status = WdfDeviceCreateSymbolicLink(device, &symlink);
    if (NT_ERROR(status)) { debug("[WARN]: WdfDeviceCreateSymbolicLink Failed (0x%08lX)", status); return status; }

    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(Driver);
}

void DriverUnload
(
    WDFDRIVER Driver
)
{
    debug("[CALL]: DriverUnload");
    UNREFERENCED_PARAMETER(Driver);
}

NTSTATUS DevicePrepareHardware
(
    WDFDEVICE Device,
    WDFCMRESLIST ResourcesRaw,
    WDFCMRESLIST ResourcesTranslated
)
{
    debug("[CALL]: DevicePrepareHardware");
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(ResourcesRaw);
    UNREFERENCED_PARAMETER(ResourcesTranslated);
}

NTSTATUS DeviceReleaseHardware
(
    WDFDEVICE Device,
    WDFCMRESLIST ResourcesTranslated
)
{
    debug("[CALL]: DeviceReleaseHardware");
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(ResourcesTranslated);
}

NTSTATUS DevicePowerUp
(
    WDFDEVICE Device,
    WDF_POWER_DEVICE_STATE PreviousState
)
{
    debug("[CALL]: DevicePowerUp");
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(PreviousState);
}

NTSTATUS DevicePowerDown
(
    WDFDEVICE Device,
    WDF_POWER_DEVICE_STATE TargetState
)
{
    debug("[CALL]: DevicePowerDown");
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(TargetState);
}

void DeviceIoControl
(
    WDFQUEUE Queue,
    WDFREQUEST Request,
    SIZE_T OutputBufferLength,
    SIZE_T InputBufferLength,
    ULONG IoControlCode
)
{
    debug("[CALL]: DeviceIoControl");
    NTSTATUS status = STATUS_SUCCESS;
    SIZE_T information = 0;

    switch (IoControlCode)
    {
        case (IOCTL_SOCKET):
        {
            debug("[INFO]: Begin I/O Operation");
            ext.nBytes = (InputBufferLength < OutputBufferLength)
                       ?  InputBufferLength : OutputBufferLength;
        
            status = WdfRequestRetrieveInputMemory(Request, &(ext.inMemory));
            if (NT_ERROR(status)) { debug("[WARN]: WdfRequestRetrieveInputMemory Failed (0x%08lX)", status); return; }

            status = WdfRequestRetrieveOutputMemory(Request, &(ext.outMemory));
            if (NT_ERROR(status)) { debug("[WARN]: WdfRequestRetrieveOutputMemory Failed (0x%08lX)", status); return; }

            status = WdfMemoryCopyToBuffer(ext.inMemory, 0, WdfMemoryGetBuffer(ext.outMemory, NULL), ext.nBytes);
            if (NT_ERROR(status)) { debug("[WARN]: WdfMemoryCopyToBuffer Failed (0x%08lX)", status); return; }

            information = ext.nBytes;
            status = STATUS_SUCCESS;
            debug("[INFO]: I/O Operation Complete");
            goto complete;
        }
        break;
        case (IOCTL_HWTEST):
        {
            ext.bytes = 1;
            ext.offset = 0;
            ext.base.QuadPart = HWTEST_ADDRESS;
           
            ext.address = (PULONG)MmMapIoSpace(ext.base, ext.bytes, MmNonCached);
            if (!ext.address) { debug("[WARN]: MmMapIoSpace Failed"); status = STATUS_UNSUCCESSFUL; goto complete; }
            
            ext.reg = ext.address + (ext.offset / sizeof(ULONG));
            ext.value = 0;
            ext.tmp = 0;

            ext.value = ext.tmp;
            ext.tmp = READ_REGISTER_ULONG(ext.reg);
            debug("[MMIO]: Stage %d | Address = 0x%016llX | Value = 0x%08lX", 1, MmGetPhysicalAddress(ext.reg).QuadPart, ext.value);

            ext.value |= HWTEST_MASK;
            WRITE_REGISTER_ULONG(ext.reg, ext.value);
            KeStallExecutionProcessor((HWTEST_DELAY) / 2);

            ext.value = READ_REGISTER_ULONG(ext.reg);
            debug("[MMIO]: Stage %d | Address = 0x%016llX | Value = 0x%08lX", 2, MmGetPhysicalAddress(ext.reg).QuadPart, ext.value);

            ext.value = ext.tmp;
            WRITE_REGISTER_ULONG(ext.reg, ext.value);
            KeStallExecutionProcessor((HWTEST_DELAY) / 2);

            ext.value = READ_REGISTER_ULONG(ext.reg);
            debug("[MMIO]: Stage %d | Address = 0x%016llX | Value = 0x%08lX", 3, MmGetPhysicalAddress(ext.reg).QuadPart, ext.value);

            MmUnmapIoSpace(ext.address, ext.bytes);
            status = STATUS_SUCCESS;
            goto complete;
        }
        break;
        default:
        {
            debug("[WARN]: Unknown IoControlCode (%lu)", IoControlCode);
            status = STATUS_INVALID_DEVICE_REQUEST;
            goto complete;
        } 
        break;
    }

complete:
    WdfRequestCompleteWithInformation(Request, status, information);
    return;
    UNREFERENCED_PARAMETER(Queue);
}
