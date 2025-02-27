#pragma once
#include <ntddk.h>
#include <wdf.h>

// Inspired by Inverted call model
// https://www.osr.com/nt-insider/2013-issue1/inverted-call-model-kmdf/
// https://github.com/OSRDrivers/Inverted
NTSTATUS HidProxyNotificationQueueInitialize(WDFFILEOBJECT FileObject, WDFQUEUE* Queue);