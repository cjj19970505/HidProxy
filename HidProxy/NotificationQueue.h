#pragma once
#include <ntddk.h>
#include <wdf.h>

NTSTATUS HidProxyNotificationQueueInitialize(WDFFILEOBJECT FileObject, WDFQUEUE* Queue);