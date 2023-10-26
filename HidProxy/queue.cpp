#include "queue.h"
#include "hidp.h"

typedef struct _QUEUE_CONTEXT
{
	WDFDEVICE Device;
} QUEUE_CONTEXT, * PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE(QUEUE_CONTEXT);

EVT_WDF_IO_QUEUE_IO_READ HidProxyEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE HidProxyEvtIoWrite;
EVT_WDF_OBJECT_CONTEXT_DESTROY HidProxyEvtIoQueueContextDestroy;

NTSTATUS HidProxyQueueInitialize(WDFDEVICE Device)
{
	PAGED_CODE();
	NTSTATUS status = STATUS_SUCCESS;
	PQUEUE_CONTEXT queueContext;
	WDF_IO_QUEUE_CONFIG queueConfig;
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchSequential);

	queueConfig.EvtIoRead = HidProxyEvtIoRead;
	queueConfig.EvtIoWrite = HidProxyEvtIoWrite;
	
	
	WDF_OBJECT_ATTRIBUTES queueAttributes;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&queueAttributes, QUEUE_CONTEXT);
	queueAttributes.SynchronizationScope = WdfSynchronizationScopeQueue;
	queueAttributes.EvtDestroyCallback = HidProxyEvtIoQueueContextDestroy;
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
	UNREFERENCED_PARAMETER(Request);
	UNREFERENCED_PARAMETER(Length);
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
	NTSTATUS status;
	WDFMEMORY memory;
	
	status = WdfRequestRetrieveInputMemory(Request, &memory);
	if (!NT_SUCCESS(status))
	{
		WdfRequestComplete(Request, status);
		return;
	}

	PVOID buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, Length, 'sam1');
	if (buffer == NULL)
	{
		WdfRequestComplete(Request, STATUS_INSUFFICIENT_RESOURCES);
		return;
	}
	status = WdfMemoryCopyToBuffer(memory, 0, buffer, Length);


	PQUEUE_CONTEXT queueContext = WdfObjectGet_QUEUE_CONTEXT(Queue);
	HidpQueueRequestHeader* header = (HidpQueueRequestHeader*)buffer;
	
	VHF_CONFIG vhfConfig;
	VHF_CONFIG_INIT(&vhfConfig, WdfDeviceWdmGetDeviceObject(queueContext->Device), header->Size, header->Data);
	
	VHFHANDLE vhfHandle;
	status = VhfCreate(&vhfConfig, &vhfHandle);
	if (!NT_SUCCESS(status))
	{
		WdfRequestComplete(Request, status);
	}
	
	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, reinterpret_cast<ULONG_PTR>(vhfHandle));
	ExFreePool(buffer);
}

VOID HidProxyEvtIoQueueContextDestroy(_In_ WDFOBJECT Object)
{
	UNREFERENCED_PARAMETER(Object);
}

