#include "FileQueue.h"
#include "device.h"
#include "hidp.h"
#include <vhf.h>

EVT_WDF_IO_QUEUE_IO_READ HidProxyFileQueueIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE HidProxyFileQueueIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL HidProxyFileQueueIoDeviceControl;

EVT_VHF_ASYNC_OPERATION VhfAsyncOperationGetFeature;
EVT_VHF_ASYNC_OPERATION VhfAsyncOperationSetFeature;
EVT_VHF_CLEANUP VhfCleanup;


typedef struct _VHF_CLIENT_CONTEXT
{
	WDFFILEOBJECT FileObject;
} VHF_CLIENT_CONTEXT, * PVHF_CLIENT_CONTEXT;

typedef struct _PENDING_NOTIFICATION_CONTEXT
{
	HidpNotificationType NotificationType;
	VHFOPERATIONHANDLE  VhfOperationHandle;
	PHID_XFER_PACKET    HidTransferPacket;
} PENDING_NOTIFICATION_CONTEXT, * PPENDING_NOTIFICATION_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE(PENDING_NOTIFICATION_CONTEXT);

NTSTATUS HidProxyFileQueueInitialize(WDFFILEOBJECT FileObject, WDFQUEUE* Queue)
{
	NTSTATUS status = STATUS_SUCCESS;

	WDFDEVICE device = WdfFileObjectGetDevice(FileObject);
	WDF_IO_QUEUE_CONFIG queueConfig;
	WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchParallel);
	queueConfig.EvtIoWrite = HidProxyFileQueueIoWrite;
	queueConfig.EvtIoRead = HidProxyFileQueueIoRead;
	queueConfig.EvtIoDeviceControl = HidProxyFileQueueIoDeviceControl;
	WDF_OBJECT_ATTRIBUTES queueAttributes;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&queueAttributes, FILE_QUEUE_CONTEXT);

	queueAttributes.ParentObject = FileObject;
	queueAttributes.ExecutionLevel = WdfExecutionLevelPassive;
	status = WdfIoQueueCreate(device, &queueConfig, &queueAttributes, Queue);

	if (!NT_SUCCESS(status))
	{
		return status;
	}
	return STATUS_SUCCESS;
}

VOID
HidProxyFileQueueIoRead(
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
HidProxyFileQueueIoWrite(
	_In_
	WDFQUEUE Queue,
	_In_
	WDFREQUEST Request,
	_In_
	size_t Length
)
{
	UNREFERENCED_PARAMETER(Queue);

	PAGED_CODE();
	NTSTATUS status = STATUS_SUCCESS;

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
	if (!NT_SUCCESS(status))
	{
		WdfRequestComplete(Request, status);
	}
	WDFFILEOBJECT file = WdfRequestGetFileObject(Request);
	PFILE_CONTEXT fileContext = WdfObjectGet_FILE_CONTEXT(file);

	do
	{
		HidpQueueRequestHeader* header = (HidpQueueRequestHeader*)buffer;
		if (header->RequestType == HidpQueueWriteRequestType::CreateVHid)
		{
			if (fileContext->VhfHandle != NULL)
			{
				WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
				break;
			}
			VHF_CONFIG vhfConfig;
			VHF_CONFIG_INIT(&vhfConfig, WdfDeviceWdmGetDeviceObject(fileContext->Device), header->Size, header->Data);
			vhfConfig.EvtVhfAsyncOperationGetFeature = VhfAsyncOperationGetFeature;
			vhfConfig.EvtVhfAsyncOperationSetFeature = VhfAsyncOperationSetFeature;
			vhfConfig.EvtVhfCleanup = VhfCleanup;
			vhfConfig.VhfClientContext = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(VHF_CLIENT_CONTEXT), 'sam1');
			reinterpret_cast<PVHF_CLIENT_CONTEXT>(vhfConfig.VhfClientContext)->FileObject = file;

			VHFHANDLE vhfHandle = NULL;
			status = VhfCreate(&vhfConfig, &vhfHandle);
			if (!NT_SUCCESS(status))
			{
				WdfRequestComplete(Request, status);
				break;
			}
			fileContext->VhfHandle = vhfHandle;
			if (!NT_SUCCESS(status))
			{
				WdfRequestComplete(Request, status);
				break;
			}
			WdfRequestComplete(Request, STATUS_SUCCESS);
			break;
		}
		else if (header->RequestType == HidpQueueWriteRequestType::SendReport)
		{
			HidQueueRequestSubmitReport* report = reinterpret_cast<HidQueueRequestSubmitReport*>(&header->Data);
			if (header->Size < sizeof(HidQueueRequestSubmitReport))
			{
				WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
				break;
			}
			ULONG reportLength = header->Size - sizeof(HidQueueRequestSubmitReport);

			HID_XFER_PACKET packet;
			packet.reportBuffer = reinterpret_cast<PUCHAR>(&report->ReportData);
			packet.reportBufferLen = reportLength;
			packet.reportId = report->ReportId;

			status = VhfReadReportSubmit(fileContext->VhfHandle, &packet);
			if (!NT_SUCCESS(status))
			{
				WdfRequestComplete(Request, status);
				break;
			}
			WdfRequestComplete(Request, STATUS_SUCCESS);
			break;
		}
		else
		{
			WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
			break;
		}
	} while (false);

	ExFreePool(buffer);
}


NTSTATUS HandleNotification(
	WDFREQUEST NotificationRegisterRequest, 
	HidpNotificationType NotificationType,
	VHFOPERATIONHANDLE VhfOperationHandle,
	PHID_XFER_PACKET HidTransferPacket
)
{
	PAGED_CODE();
	NTSTATUS status = STATUS_SUCCESS;
	WDFMEMORY outputMemory;
	status = WdfRequestRetrieveOutputMemory(NotificationRegisterRequest, &outputMemory);
	if (!NT_SUCCESS(status))
	{
		WdfRequestComplete(NotificationRegisterRequest, status);
		VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
		return status;
	}

	size_t outputBufferSize;
	HidpNotificationHeader* outputBuffer = reinterpret_cast<HidpNotificationHeader*>(WdfMemoryGetBuffer(outputMemory, &outputBufferSize));

	if (outputBuffer == NULL)
	{
		// TODO
		// Bug check
		WdfRequestComplete(NotificationRegisterRequest, STATUS_INVALID_PARAMETER);
		VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
		return STATUS_INVALID_PARAMETER;
	}
	size_t outputLength = 0;
	if (NotificationType == HidpNotificationType::GetFeature || NotificationType == HidpNotificationType::GetInputReport)
	{
		outputLength = sizeof(HidpNotificationHeader);
	}
	else if (NotificationType == HidpNotificationType::SetFeature || NotificationType == HidpNotificationType::WriteReport)
	{
		outputLength = sizeof(HidpNotificationHeader) + HidTransferPacket->reportBufferLen;
	}
	else
	{
		WdfRequestComplete(NotificationRegisterRequest, STATUS_INVALID_PARAMETER);
		VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
		return STATUS_INVALID_PARAMETER;
	}

	if (outputBufferSize < outputLength)
	{
		WdfRequestComplete(NotificationRegisterRequest, STATUS_BUFFER_TOO_SMALL);
		VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
		return STATUS_INVALID_PARAMETER;
	}
	RtlZeroMemory(outputBuffer, outputBufferSize);
	outputBuffer->NotificationType = NotificationType;
	outputBuffer->HidTransferPacket = HidTransferPacket;
	outputBuffer->VhfOperationHandle = VhfOperationHandle;
	outputBuffer->ReportId = HidTransferPacket->reportId;
	
	if (NotificationType == HidpNotificationType::SetFeature || NotificationType == HidpNotificationType::WriteReport)
	{
		outputBuffer->ReportBufferLen = HidTransferPacket->reportBufferLen;
		status = WdfMemoryCopyFromBuffer(outputMemory, sizeof(HidpNotificationHeader), HidTransferPacket->reportBuffer, HidTransferPacket->reportBufferLen);
		if (!NT_SUCCESS(status))
		{
			WdfRequestComplete(NotificationRegisterRequest, status);
			VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
			return status;
		}
	}

	if (!NT_SUCCESS(status))
	{
		WdfRequestComplete(NotificationRegisterRequest, status);
		status = VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
		return status;
	}
	WdfRequestCompleteWithInformation(NotificationRegisterRequest, STATUS_SUCCESS, outputLength);
	return STATUS_SUCCESS;
}

VOID
HidProxyFileQueueIoDeviceControl(
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

	PAGED_CODE();

	NTSTATUS status = STATUS_SUCCESS;

	WDFFILEOBJECT file = WdfRequestGetFileObject(Request);
	PFILE_CONTEXT fileContext = WdfObjectGet_FILE_CONTEXT(file);
	if (IoControlCode == IOCTL_HIDPROXY_START_VHID) 
	{
		if (fileContext->VhfHandle == NULL)
		{
			WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
			return;
		}
		status = VhfStart(fileContext->VhfHandle);
		WdfRequestComplete(Request, status);
	}
	else if (IoControlCode == IOCTL_HIDPROXY_REGISTER_NOTIFICATION)
	{
		status = WdfWaitLockAcquire(fileContext->RegisterNotificationLock, NULL);
		if (!NT_SUCCESS(status))
		{
			WdfRequestComplete(Request, status);
			return;
		}
		WDFOBJECT pendingNotification = WdfCollectionGetFirstItem(fileContext->PendingNotificationCollection);
		if (pendingNotification != NULL)
		{
			WdfCollectionRemove(fileContext->PendingNotificationCollection, pendingNotification);
		}
		WdfWaitLockRelease(fileContext->RegisterNotificationLock);
		if (pendingNotification != NULL)
		{
			PPENDING_NOTIFICATION_CONTEXT pendingNotificationContext = WdfObjectGet_PENDING_NOTIFICATION_CONTEXT(pendingNotification);
			HandleNotification(Request, pendingNotificationContext->NotificationType, pendingNotificationContext->VhfOperationHandle, pendingNotificationContext->HidTransferPacket);
			WdfObjectDelete(pendingNotification);
		}
		else
		{
			status = WdfRequestForwardToIoQueue(Request, fileContext->NotificationQueue);
			if (!NT_SUCCESS(status))
			{
				WdfRequestComplete(Request, status);
				return;
			}
		}
	}
	else if (IoControlCode == IOCTL_HIDPROXY_COMPLETE_NOTIFIACTION)
	{
		WDFMEMORY inputMemory;
		status = WdfRequestRetrieveInputMemory(Request, &inputMemory);
		if (!NT_SUCCESS(status))
		{
			WdfRequestComplete(Request, status);
			return;
		}
		HidpCompleteNotificationHeader* inputBuffer = reinterpret_cast<HidpCompleteNotificationHeader*>(ExAllocatePool2(POOL_FLAG_NON_PAGED, InputBufferLength, 'sam1'));
		if (inputBuffer == NULL)
		{
			WdfRequestComplete(Request, STATUS_INSUFFICIENT_RESOURCES);
			return;
		}
		status = WdfMemoryCopyToBuffer(inputMemory, 0, inputBuffer, InputBufferLength);
		if (!NT_SUCCESS(status))
		{
			WdfRequestComplete(Request, status);
			return;
		}
		if (inputBuffer->NotificationType == HidpNotificationType::SetFeature || inputBuffer->NotificationType == HidpNotificationType::WriteReport)
		{
			VhfAsyncOperationComplete(inputBuffer->VhfOperationHandle, inputBuffer->CompletionStatus);
			WdfRequestComplete(Request, STATUS_SUCCESS);
		}
		else if (inputBuffer->NotificationType == HidpNotificationType::GetFeature || inputBuffer->NotificationType == HidpNotificationType::GetInputReport)
		{
			if (NT_SUCCESS(inputBuffer->CompletionStatus))
			{
				RtlCopyMemory(reinterpret_cast<PHID_XFER_PACKET>(inputBuffer->HidTransferPacket)->reportBuffer, inputBuffer->Data, inputBuffer->ReportBufferLen);
				reinterpret_cast<PHID_XFER_PACKET>(inputBuffer->HidTransferPacket)->reportBufferLen = inputBuffer->ReportBufferLen;
			}
			VhfAsyncOperationComplete(inputBuffer->VhfOperationHandle, inputBuffer->CompletionStatus);
			WdfRequestComplete(Request, STATUS_SUCCESS);
		}
		else
		{
			WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
		}
	}
	else
	{
		WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
	}
}

VOID
VhfCleanup(
	_In_
	PVOID VhfClientContext
)
{
	ExFreePool(VhfClientContext);
}



NTSTATUS PendingNotificationCreate(WDFFILEOBJECT file, HidpNotificationType notificationType, VHFOPERATIONHANDLE  vhfOperationHandle, PHID_XFER_PACKET hidTransferPacket, WDFOBJECT* PendingNotification)
{
	NTSTATUS status = STATUS_SUCCESS;

	WDF_OBJECT_ATTRIBUTES attributes{};
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, PENDING_NOTIFICATION_CONTEXT);
	attributes.ParentObject = file;

	WDFOBJECT pendingNotification = NULL;
	status = WdfObjectCreate(&attributes, &pendingNotification);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	PPENDING_NOTIFICATION_CONTEXT pendingNotificationContext = WdfObjectGet_PENDING_NOTIFICATION_CONTEXT(pendingNotification);
	pendingNotificationContext->NotificationType = notificationType;
	pendingNotificationContext->VhfOperationHandle = vhfOperationHandle;
	pendingNotificationContext->HidTransferPacket = hidTransferPacket;

	*PendingNotification = pendingNotification;
	return STATUS_SUCCESS;
}

VOID
VhfAsyncOperationSetFeature(
	_In_
	PVOID               VhfClientContext,
	_In_
	VHFOPERATIONHANDLE  VhfOperationHandle,
	_In_opt_
	PVOID               VhfOperationContext,
	_In_
	PHID_XFER_PACKET    HidTransferPacket
)
{
	PAGED_CODE();
	UNREFERENCED_PARAMETER(VhfOperationContext);
	NTSTATUS status = STATUS_SUCCESS;

	WDFFILEOBJECT file = reinterpret_cast<PVHF_CLIENT_CONTEXT>(VhfClientContext)->FileObject;
	PFILE_CONTEXT fileContext = WdfObjectGet_FILE_CONTEXT(file);

	WDFREQUEST notificationRequest = NULL;
	status = WdfIoQueueRetrieveNextRequest(fileContext->NotificationQueue, &notificationRequest);
	if (!NT_SUCCESS(status))
	{
		if (status == STATUS_NO_MORE_ENTRIES)
		{
			WDFOBJECT pendingNotification = NULL;
			status = PendingNotificationCreate(file, HidpNotificationType::SetFeature, VhfOperationHandle, HidTransferPacket, &pendingNotification);
			if (!NT_SUCCESS(status))
			{
				VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
				return;
			}

			// TODO
			// Lock probably should be moved before retriveing next request
			status = WdfWaitLockAcquire(fileContext->RegisterNotificationLock, NULL);
			if (!NT_SUCCESS(status))
			{
				VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
				return;
			}
			do
			{
				status = WdfCollectionAdd(fileContext->PendingNotificationCollection, pendingNotification);
				if (!NT_SUCCESS(status))
				{
					break;
				}
			} while (false);

			WdfWaitLockRelease(fileContext->RegisterNotificationLock);
			if (!NT_SUCCESS(status))
			{
				VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
				return;
			}
		}
		else
		{
			VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
			return;
		}
		
	}
	else
	{
		HandleNotification(notificationRequest, HidpNotificationType::SetFeature, VhfOperationHandle, HidTransferPacket);
	}
}

VOID
VhfAsyncOperationGetFeature(
	_In_
	PVOID               VhfClientContext,
	_In_
	VHFOPERATIONHANDLE  VhfOperationHandle,
	_In_opt_
	PVOID               VhfOperationContext,
	_In_
	PHID_XFER_PACKET    HidTransferPacket
)
{
	PAGED_CODE();
	UNREFERENCED_PARAMETER(VhfOperationContext);
	NTSTATUS status = STATUS_SUCCESS;

	WDFFILEOBJECT file = reinterpret_cast<PVHF_CLIENT_CONTEXT>(VhfClientContext)->FileObject;
	PFILE_CONTEXT fileContext = WdfObjectGet_FILE_CONTEXT(file);

	WDFREQUEST notificationRequest = NULL;
	status = WdfIoQueueRetrieveNextRequest(fileContext->NotificationQueue, &notificationRequest);
	if (!NT_SUCCESS(status))
	{
		if (status == STATUS_NO_MORE_ENTRIES)
		{
			WDFOBJECT pendingNotification = NULL;
			status = PendingNotificationCreate(file, HidpNotificationType::GetFeature, VhfOperationHandle, HidTransferPacket, &pendingNotification);
			if (!NT_SUCCESS(status))
			{
				VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
				return;
			}

			status = WdfWaitLockAcquire(fileContext->RegisterNotificationLock, NULL);
			if (!NT_SUCCESS(status))
			{
				VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
				return;
			}
			do
			{
				status = WdfCollectionAdd(fileContext->PendingNotificationCollection, pendingNotification);
				if (!NT_SUCCESS(status))
				{
					break;
				}
			} while (false);

			WdfWaitLockRelease(fileContext->RegisterNotificationLock);
			if (!NT_SUCCESS(status))
			{
				VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
				return;
			}
		}
		else
		{
			VhfAsyncOperationComplete(VhfOperationHandle, STATUS_NOT_SUPPORTED);
			return;
		}

	}
	else
	{
		HandleNotification(notificationRequest, HidpNotificationType::GetFeature, VhfOperationHandle, HidTransferPacket);
	}
}