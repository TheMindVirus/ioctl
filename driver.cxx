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
        case (IOCTL_MAILBOX):
        {
            debug("[INFO]: Begin I/O Operation");
            //ext.nBytes = (InputBufferLength < OutputBufferLength)
            //           ?  InputBufferLength : OutputBufferLength;
            ext.nBytes = 0;
        
            status = WdfRequestRetrieveInputMemory(Request, &(ext.inMemory));
            if (NT_ERROR(status)) { debug("[WARN]: WdfRequestRetrieveInputMemory Failed (0x%08lX)", status); return; }

            //status = WdfRequestRetrieveOutputMemory(Request, &(ext.outMemory));
            //if (NT_ERROR(status)) { debug("[WARN]: WdfRequestRetrieveOutputMemory Failed (0x%08lX)", status); return; }

            ext.buffer = (PUINT32)WdfMemoryGetBuffer(ext.inMemory, NULL);
            if (!(ext.buffer)) { debug("[WARN]: WdfMemoryGetBuffer Returned NULL"); return; }

            for (SIZE_T i = 0; i < InputBufferLength; ++i) { ext.mbox_packet[i] = ext.buffer[i]; debug("[INFO]: %u", (UINT8)(ext.buffer[i])); }
            if (MBOX_FAILURE == MailboxExchange(MBOX_CHANNEL)) { debug("[WARN]: Mailbox Exchange Returned Failure"); return; }
            ext.nBytes = ext.mbox_packet[0]; if (ext.nBytes > OutputBufferLength) { ext.nBytes = OutputBufferLength; }
           
            status = WdfMemoryAssignBuffer(ext.outMemory, ext.buffer, ext.nBytes);
            if (NT_ERROR(status)) { debug("[WARN]: WdfMemoryAssignBuffer Failed (0x%08lX)", status); return; }

            //status = WdfMemoryCopyToBuffer(ext.inMemory, 0, WdfMemoryGetBuffer(ext.outMemory, NULL), ext.nBytes);
            //if (NT_ERROR(status)) { debug("[WARN]: WdfMemoryCopyToBuffer Failed (0x%08lX)", status); return; }

            information = ext.nBytes;
            status = STATUS_SUCCESS;
            debug("[INFO]: I/O Operation Complete");
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

UINT32 MailboxExchange(UINT8 channel)
{
    UINT32 status = MBOX_FAILURE;

    ext.base.QuadPart = MBOX_BASE;
    ext.mbox_base = (PULONG)MmMapIoSpace(ext.base, MBOX_SIZE, MmNonCached);
    if (!(ext.mbox_base)) { debug("[WARN]: Failed to Memory-Map Mailbox"); goto cleanup; }

    ext.max.QuadPart = PERI_MASK;
    ext.mbox_packet = (PULONG)MmAllocateContiguousMemory(IOCTL_LENGTH, ext.max);
    if (!(ext.mbox_packet)) { debug("[WARN]: Failed to Memory-Map Mailbox Packet"); goto cleanup; }

    ext.checked = 0;
    ext.mail = ((((ULONGLONG)(MmGetPhysicalAddress(ext.mbox_packet).QuadPart)) & ~0xF) | (channel & 0xF)); //0xF reserved for 4-bit channel //<--Needs to be a physical address so the VC knows where it is

    ext.critical = TRUE;
    KeEnterCriticalRegion();

    ext.lock = NULL;
    ext.prevState = NULL;
    KeInitializeSpinLock(&(ext.lock));
    KeAcquireSpinLock(&(ext.lock), &(ext.prevState));

    mmio_write((ext.mbox_base), MBOX_READ, 0x00000000);
    mmio_write((ext.mbox_base), 1, 0x00000000);
    mmio_write((ext.mbox_base), 2, 0x00000000);
    mmio_write((ext.mbox_base), 3, 0x00000000);
    mmio_write((ext.mbox_base), MBOX_POLL, 0x00000000);
    mmio_write((ext.mbox_base), MBOX_SENDER, 0x00000000);
    //mmio_write((ext.mbox_base), MBOX_STATUS, 0x00000000);
    mmio_write((ext.mbox_base), MBOX_CONFIG, 0x00000400);
    //mmio_write((ext.mbox_base), MBOX_WRITE,  0x00000000);
    KeStallExecutionProcessor(MBOX_TIMEOUT);

    ext.attempt = 0;
    while ((mbox_peek() & MBOX_FULL) != 0)
    {
        debug("[HANG]: Mailbox Write");
        KeStallExecutionProcessor(MBOX_TIMEOUT);
        ++(ext.attempt);
        if ((ext.attempt) > MBOX_RETRIES) { debug("[WARN]: Mailbox Write Error"); goto cleanup; }
    }
    mbox_write(ext.mail);
    KeStallExecutionProcessor(MBOX_TIMEOUT);

    ext.timeout1 = 0;
    ext.timeout2 = 0;
    while (TRUE)
    {
        while ((mbox_peek() & MBOX_EMPTY) != 0)
        {
            debug("[HANG]: Mailbox Read");
            KeStallExecutionProcessor(MBOX_TIMEOUT);
            ++(ext.timeout2);
            if ((ext.timeout2) > MBOX_RETRIES) { debug("[MBOX]: Mailbox Read Error"); goto cleanup; }
        }

        ext.checked = mbox_read();
        if ((ext.mail) == (ext.checked)) { status = MBOX_SUCCESS; goto cleanup; }

        debug("[HANG]: Mailbox Check");
        KeStallExecutionProcessor(MBOX_TIMEOUT);
        ++(ext.timeout1);
        if ((ext.timeout1) > MBOX_RETRIES) { debug("[MBOX]: Mailbox Check Error"); goto cleanup; }
    }

cleanup:
    if ((ext.critical) == TRUE)
    {
        KeLeaveCriticalRegion();
        ext.critical = FALSE;
    }
    if ((ext.prevState != NULL) && (ext.lock != NULL))
    {
        KeReleaseSpinLock(&(ext.lock), ext.prevState);
        ext.prevState = NULL;
        ext.lock = NULL;
    }
    if ((ext.mbox_packet) && (ext.mbox_packet != (PULONG)0xCDCDCDCDCDCDCDCD))
    {
        MmFreeContiguousMemory(ext.mbox_packet);
        ext.mbox_packet = NULL;
    }
    if ((ext.mbox_base) && (ext.mbox_base != (PULONG)0xCDCDCDCDCDCDCDCD))
    {
        MmUnmapIoSpace(ext.mbox_base, MBOX_SIZE);
        ext.mbox_base = NULL;
    }
    return status;
}