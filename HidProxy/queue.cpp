#include "queue.h"
#include "hidp.h"
#include "device.h"

EVT_WDF_IO_QUEUE_IO_READ HidProxyEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE HidProxyEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL HidProxyEvtIoDeviceControl;

typedef struct _VHF_CLIENT_CONTEXT
{
	WDFFILEOBJECT FileObject;
} VHF_CLIENT_CONTEXT, * PVHF_CLIENT_CONTEXT;

NTSTATUS HidProxyQueueInitialize(WDFDEVICE Device)
{
	PAGED_CODE();
	NTSTATUS status = STATUS_SUCCESS;
	PQUEUE_CONTEXT queueContext;
	WDF_IO_QUEUE_CONFIG queueConfig;
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);

	queueConfig.EvtIoRead = HidProxyEvtIoRead;
	queueConfig.EvtIoWrite = HidProxyEvtIoWrite;
	queueConfig.EvtIoDeviceControl = HidProxyEvtIoDeviceControl;
	
	WDF_OBJECT_ATTRIBUTES queueAttributes;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&queueAttributes, QUEUE_CONTEXT);
	queueAttributes.SynchronizationScope = WdfSynchronizationScopeQueue;
	queueAttributes.ExecutionLevel = WdfExecutionLevelPassive;

	WDFQUEUE queue;
	status = WdfIoQueueCreate(Device, &queueConfig, &queueAttributes, &queue);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	queueContext = WdfObjectGet_QUEUE_CONTEXT(queue);
	queueContext->Device = Device;

	return STATUS_SUCCESS;
}

VOID
HidProxyEvtIoDeviceControl(
	_In_
	WDFQUEUE Queue,
	_In_
	WDFREQUEST Request,
	_In_
	size_t OutputBufferLength,
	_In_
	size_t InputBufferLength,
	_In_
	ULONG IoControlCode
)
{
	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);
	UNREFERENCED_PARAMETER(IoControlCode);

	NTSTATUS status = STATUS_SUCCESS;
	WDFFILEOBJECT file = WdfRequestGetFileObject(Request);
	PFILE_CONTEXT fileContext = WdfObjectGet_FILE_CONTEXT(file);
	status = WdfRequestForwardToIoQueue(Request, fileContext->FileQueue);
	if (!NT_SUCCESS(status))
	{
		WdfRequestComplete(Request, status);
	}
}

VOID
HidProxyEvtIoRead(
	_In_
	WDFQUEUE Queue,
	_In_
	WDFREQUEST Request,
	_In_
	size_t Length
)
{
	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(Length);

	NTSTATUS status = STATUS_SUCCESS;
	WDFFILEOBJECT file = WdfRequestGetFileObject(Request);
	PFILE_CONTEXT fileContext = WdfObjectGet_FILE_CONTEXT(file);
	status = WdfRequestForwardToIoQueue(Request, fileContext->FileQueue);
	if (!NT_SUCCESS(status))
	{
		WdfRequestComplete(Request, status);
	}
}

VOID
HidProxyEvtIoWrite(
	_In_
	WDFQUEUE Queue,
	_In_
	WDFREQUEST Request,
	_In_
	size_t Length
)
{
	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(Length);

	NTSTATUS status = STATUS_SUCCESS;
	WDFFILEOBJECT file = WdfRequestGetFileObject(Request);
	PFILE_CONTEXT fileContext = WdfObjectGet_FILE_CONTEXT(file);
	status = WdfRequestForwardToIoQueue(Request, fileContext->FileQueue);
	if (!NT_SUCCESS(status))
	{
		WdfRequestComplete(Request, status);
	}
}

