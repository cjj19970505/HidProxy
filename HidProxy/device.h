#pragma once

#include <ntddk.h>
#include <wdf.h>

typedef struct _DEVICE_CONTEXT
{
	DWORD64 privateDeviceData;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE(DEVICE_CONTEXT)

NTSTATUS HidProxyDeviceCreate(PWDFDEVICE_INIT DeviceInit);