#include "device.h"
#include "queue.h"
#include "hidp.h"

EVT_WDF_DEVICE_SELF_MANAGED_IO_INIT  HidProxyEvtDeviceSelfManagedIoInit;
EVT_WDF_DEVICE_SELF_MANAGED_IO_SUSPEND  HidProxyEvtDeviceSelfManagedIoSuspend;
EVT_WDF_DEVICE_FILE_CREATE HidProxyEvtDeviceFileCreate;
EVT_WDF_FILE_CLOSE HidProxyEvtDeviceFileClose;
EVT_WDF_FILE_CLEANUP HidProxyEvtFileCleanup;


NTSTATUS HidProxyDeviceCreate(PWDFDEVICE_INIT DeviceInit)
{
	PAGED_CODE();
	NTSTATUS status;

	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);

	pnpPowerCallbacks.EvtDeviceSelfManagedIoInit = HidProxyEvtDeviceSelfManagedIoInit;
	pnpPowerCallbacks.EvtDeviceSelfManagedIoSuspend = HidProxyEvtDeviceSelfManagedIoSuspend;
	pnpPowerCallbacks.EvtDeviceSelfManagedIoRestart = HidProxyEvtDeviceSelfManagedIoInit;
	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

	WDF_OBJECT_ATTRIBUTES deviceAttributes;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

	WDF_FILEOBJECT_CONFIG fileObjectConfig;
	WDF_FILEOBJECT_CONFIG_INIT(&fileObjectConfig, &HidProxyEvtDeviceFileCreate, &HidProxyEvtDeviceFileClose, &HidProxyEvtFileCleanup);
	WDF_OBJECT_ATTRIBUTES fileObjectAttributes;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&fileObjectAttributes, FILE_CONTEXT);
	//fileObjectAttributes.SynchronizationScope = WdfSynchronizationScopeDevice;
	WdfDeviceInitSetFileObjectConfig(DeviceInit, &fileObjectConfig, &fileObjectAttributes);

	WDFDEVICE device;
	status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	PDEVICE_CONTEXT deviceContext = WdfObjectGet_DEVICE_CONTEXT(device);
	deviceContext->privateDeviceData = 0;

	status = WdfDeviceCreateDeviceInterface(device, &GUID_DEVINTERFACE_HIDP, NULL);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = HidProxyQueueInitialize(device);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	return STATUS_SUCCESS;
}

NTSTATUS
HidProxyEvtDeviceSelfManagedIoInit(
	_In_
	WDFDEVICE Device
)
{
	PAGED_CODE();
	WDFQUEUE defaultQueue = WdfDeviceGetDefaultQueue(Device);
	WdfIoQueueStart(defaultQueue);
	return STATUS_SUCCESS;
}

NTSTATUS
HidProxyEvtDeviceSelfManagedIoSuspend(
	_In_
	WDFDEVICE Device
)
{
	PAGED_CODE();
	WDFQUEUE defaultQueue = WdfDeviceGetDefaultQueue(Device);
	WdfIoQueueStopSynchronously(defaultQueue);
	return STATUS_SUCCESS;
}

VOID
HidProxyEvtDeviceFileCreate(
	_In_
	WDFDEVICE Device,
	_In_
	WDFREQUEST Request,
	_In_
	WDFFILEOBJECT FileObject
)
{
	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(Request);
	PAGED_CODE();
	PFILE_CONTEXT pFileContext = WdfObjectGet_FILE_CONTEXT(FileObject);
	pFileContext->VhfHandle = NULL;
	WdfRequestComplete(Request, STATUS_SUCCESS);
}

VOID
HidProxyEvtDeviceFileClose(
	_In_
	WDFFILEOBJECT FileObject
)
{
	UNREFERENCED_PARAMETER(FileObject);
	//PAGED_CODE();
	//PFILE_CONTEXT pFileContext = WdfObjectGet_FILE_CONTEXT(FileObject);
	//if (pFileContext->VhfHandle != NULL)
	//{
	//	VhfDelete(pFileContext->VhfHandle, TRUE);
	//	pFileContext->VhfHandle = NULL;
	//}
	
}

VOID
HidProxyEvtFileCleanup(
	_In_
	WDFFILEOBJECT FileObject
)
{
	PAGED_CODE();
	PFILE_CONTEXT pFileContext = WdfObjectGet_FILE_CONTEXT(FileObject);
	if (pFileContext->VhfHandle != NULL)
	{
		VhfDelete(pFileContext->VhfHandle, TRUE);
		pFileContext->VhfHandle = NULL;
	}
}