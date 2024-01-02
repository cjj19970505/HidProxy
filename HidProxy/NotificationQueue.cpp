#include "NotificationQueue.h"

NTSTATUS HidProxyNotificationQueueInitialize(WDFFILEOBJECT FileObject, WDFQUEUE* Queue)
{
	NTSTATUS status = STATUS_SUCCESS;

	WDFDEVICE device = WdfFileObjectGetDevice(FileObject);
	WDF_IO_QUEUE_CONFIG queueConfig;
	WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);
	WDF_OBJECT_ATTRIBUTES queueAttributes;
	WDF_OBJECT_ATTRIBUTES_INIT(&queueAttributes);
	queueAttributes.ParentObject = FileObject;
	queueAttributes.ExecutionLevel = WdfExecutionLevelPassive;
	status = WdfIoQueueCreate(device, &queueConfig, &queueAttributes, Queue);
	
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	return STATUS_SUCCESS;
}
