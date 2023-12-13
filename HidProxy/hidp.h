#pragma once
#include <initguid.h>

// {EE1A9EFD-71AC-492E-9EED-B71BB4BD1E27}
DEFINE_GUID(GUID_DEVINTERFACE_HIDP,
	0xee1a9efd, 0x71ac, 0x492e, 0x9e, 0xed, 0xb7, 0x1b, 0xb4, 0xbd, 0x1e, 0x27);

enum class HidpQueueWriteRequestType
{
	CreateVHid = 0, SendReport = 1, GetFeature = 2
};

#pragma warning(disable : 4200)
struct HidpQueueRequestHeader
{
	HidpQueueWriteRequestType RequestType;
	USHORT Size;
	UCHAR Data[0];
};

#pragma warning(disable : 4200)
struct HidQueueRequestSubmitReport
{
	UCHAR ReportId;
	UCHAR ReportData[0];
};