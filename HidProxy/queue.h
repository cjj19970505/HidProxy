#pragma once
#include <ntddk.h>
#include <wdf.h>
#include <vhf.h>

typedef struct _QUEUE_CONTEXT
{
	WDFDEVICE Device;
} QUEUE_CONTEXT, * PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE(QUEUE_CONTEXT);



NTSTATUS HidProxyQueueInitialize(WDFDEVICE Device);
