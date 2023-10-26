#pragma once


enum class HidpQueueWriteRequestType
{
	CreateVHid
};

#pragma warning(disable : 4200)
struct HidpQueueRequestHeader
{
	HidpQueueWriteRequestType RequestType;
	USHORT Size;
	UCHAR Data[0];
};