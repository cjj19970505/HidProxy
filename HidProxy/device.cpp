#include "device.h"
#include "queue.h"

NTSTATUS HidProxyDeviceCreate(PWDFDEVICE_INIT DeviceInit)
{
	PAGED_CODE();
	NTSTATUS status;
	WDF_OBJECT_ATTRIBUTES deviceAttributes;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);
	
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

	return STATUS_SUCCESS;
}
