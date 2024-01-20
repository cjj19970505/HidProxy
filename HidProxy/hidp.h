#pragma once
#include <initguid.h>

// {EE1A9EFD-71AC-492E-9EED-B71BB4BD1E27}
DEFINE_GUID(GUID_DEVINTERFACE_HIDP,
	0xee1a9efd, 0x71ac, 0x492e, 0x9e, 0xed, 0xb7, 0x1b, 0xb4, 0xbd, 0x1e, 0x27);

enum class HidpQueueWriteRequestType
{
	CreateVHid = 0, SendReport = 1, SetFeature = 2, RegisterNotification = 100
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

struct HidQueueCompleteSetFeatureInfo
{
	VOID* VhfOperationHandle;
	LONG Status;
};

enum class HidpNotificationType
{
	GetFeature = 0, SetFeature = 1, WriteReport = 2, GetInputReport = 3
};

#pragma warning(disable : 4200)
struct HidpNotificationHeader
{
	// Theses 3 fields should be transferred back to driver when completing notifications.
	HidpNotificationType NotificationType;
	VOID* HidTransferPacket;
	VOID* VhfOperationHandle;
	
	UCHAR ReportId;
	ULONG ReportBufferLen;
	UCHAR Data[0];
};

struct HidpCompleteNotificationHeader
{
	HidpNotificationType NotificationType;
	VOID* HidTransferPacket;
	VOID* VhfOperationHandle;
	LONG CompletionStatus;
	UCHAR ReportId;
	ULONG ReportBufferLen;
	UCHAR Data[0];
};

//
// The following value is arbitrarily chosen from the space defined by Microsoft
// as being "for non-Microsoft use"
//
#define FILE_DEVICE_INVERTED 0xCF54
#define IOCTL_REGISTER_NOTIFICATION CTL_CODE(FILE_DEVICE_INVERTED, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_COMPLETE_NOTIFIACTION CTL_CODE(FILE_DEVICE_INVERTED, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)