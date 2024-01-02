#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <vhf.h>

typedef struct _DEVICE_CONTEXT
{
	DWORD64 privateDeviceData;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE(DEVICE_CONTEXT);

typedef struct _FILE_CONTEXT
{
	VHFHANDLE VhfHandle;
	WDFQUEUE FileQueue;
	WDFQUEUE NotificationQueue;
	WDFDEVICE Device;

} FILE_CONTEXT, * PFILE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE(FILE_CONTEXT);

NTSTATUS HidProxyDeviceCreate(PWDFDEVICE_INIT DeviceInit);