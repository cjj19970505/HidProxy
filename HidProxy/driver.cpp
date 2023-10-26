#include "driver.h"
#include "device.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, HidProxyEvtDeviceAdd)
#endif

NTSTATUS
DriverEntry(
    _In_ struct _DRIVER_OBJECT* DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;

    WDF_DRIVER_CONFIG_INIT(&config, HidProxyEvtDeviceAdd);

    status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);
    
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    return status;
}

NTSTATUS
HidProxyEvtDeviceAdd(
    _In_
    WDFDRIVER Driver,
    _Inout_
    PWDFDEVICE_INIT DeviceInit
)
{
    UNREFERENCED_PARAMETER(Driver);
    return HidProxyDeviceCreate(DeviceInit);
}

