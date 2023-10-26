#pragma once

#include <initguid.h>
#include <ntddk.h>
#include <wdf.h>

typedef struct _DEVICE_CONTEXT
{
	DWORD64 privateDeviceData;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE(DEVICE_CONTEXT)

// {EE1A9EFD-71AC-492E-9EED-B71BB4BD1E27}
DEFINE_GUID(GUID_DEVINTERFACE_HIDP ,
	0xee1a9efd, 0x71ac, 0x492e, 0x9e, 0xed, 0xb7, 0x1b, 0xb4, 0xbd, 0x1e, 0x27);


NTSTATUS HidProxyDeviceCreate(PWDFDEVICE_INIT DeviceInit);