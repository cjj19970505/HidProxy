#pragma once
#include <ntddk.h>
#include <wdf.h>

typedef struct _FILE_QUEUE_CONTEXT
{
	DWORD64 privateData;
} FILE_QUEUE_CONTEXT, * PFILE_QUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE(FILE_QUEUE_CONTEXT);

NTSTATUS HidProxyFileQueueInitialize(WDFFILEOBJECT FileObject, WDFQUEUE* Queue);