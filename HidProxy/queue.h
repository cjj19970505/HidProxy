#pragma once
#include <ntddk.h>
#include <wdf.h>
#include <vhf.h>


NTSTATUS HidProxyQueueInitialize(WDFDEVICE Device);
