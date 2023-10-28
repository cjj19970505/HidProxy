#include "pch.h"
#include "LibHidp.h"
#include "../HidProxy/hidp.h"
#include <cfgmgr32.h>
#include <winrt/windows.storage.streams.h>

using namespace winrt;

HRESULT HidpCreate(DWORD nReportDescriptorSize, LPCVOID lpReportDescriptor, PHIDPHANDLE pHidpHandle)
{
	ULONG deviceInterfaceListLength = 0;
	CONFIGRET cr = CR_SUCCESS;
	cr = CM_Get_Device_Interface_List_Size(&deviceInterfaceListLength, (LPGUID)(&GUID_DEVINTERFACE_HIDP), nullptr, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
	
	if (cr != CR_SUCCESS)
	{
		return E_UNEXPECTED;
	}
	if (deviceInterfaceListLength == 0)
	{
		return E_UNEXPECTED;
	}
	Windows::Storage::Streams::Buffer deviceInterfaceListBuffer{ static_cast<uint32_t>(deviceInterfaceListLength * sizeof(WCHAR))};
	deviceInterfaceListBuffer.Length(deviceInterfaceListLength * sizeof(WCHAR));
	ZeroMemory(deviceInterfaceListBuffer.data(), deviceInterfaceListBuffer.Capacity());

	HRESULT hr = S_OK;
	do
	{
		cr = CM_Get_Device_Interface_List((LPGUID)(&GUID_DEVINTERFACE_HIDP), nullptr, reinterpret_cast<PZZWSTR>(deviceInterfaceListBuffer.data()), deviceInterfaceListLength, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

		// Should only have 1 interface
		if (*(reinterpret_cast<LPCWSTR>(deviceInterfaceListBuffer.data()) + wcslen(reinterpret_cast<LPCWSTR>(deviceInterfaceListBuffer.data())) + 1) != UNICODE_NULL)
		{
			hr = E_UNEXPECTED;
			break;
		}
		HANDLE deviceHandle = CreateFile(reinterpret_cast<LPCWSTR>(deviceInterfaceListBuffer.data()), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (deviceHandle == INVALID_HANDLE_VALUE)
		{
			hr = E_UNEXPECTED;
			break;
		}
		*pHidpHandle = deviceHandle;

		Windows::Storage::Streams::Buffer createVHidRequestBuffer{ static_cast<uint32_t>(nReportDescriptorSize + sizeof(HidpQueueRequestHeader)) };
		createVHidRequestBuffer.Length(createVHidRequestBuffer.Capacity());
		ZeroMemory(createVHidRequestBuffer.data(), createVHidRequestBuffer.Capacity());
		reinterpret_cast<HidpQueueRequestHeader*>(createVHidRequestBuffer.data())->RequestType = HidpQueueWriteRequestType::CreateVHid;
		reinterpret_cast<HidpQueueRequestHeader*>(createVHidRequestBuffer.data())->Size = nReportDescriptorSize;
		CopyMemory(createVHidRequestBuffer.data() + offsetof(HidpQueueRequestHeader, Data), lpReportDescriptor, nReportDescriptorSize);

		BOOL writeResult = WriteFile(deviceHandle, createVHidRequestBuffer.data(), createVHidRequestBuffer.Length(), NULL, NULL);
		if (!writeResult)
		{
			hr = E_UNEXPECTED;
			break;
		}

	} while (false);

	if (FAILED(hr))
	{
		*pHidpHandle = NULL;
	}

	return hr;
}

BOOL HidpClose(HIDPHANDLE hidpHandle)
{
	return CloseHandle(hidpHandle);
}

HRESULT HidpSubmitReport(HIDPHANDLE hidpHandle, UCHAR reportId, SIZE_T reportSize, PBYTE reportData)
{
	Windows::Storage::Streams::Buffer submitReportRequest{ static_cast<uint32_t>(reportSize + sizeof(HidQueueRequestSubmitReport) + sizeof(HidpQueueRequestHeader)) };
	submitReportRequest.Length(submitReportRequest.Capacity());
	ZeroMemory(submitReportRequest.data(), submitReportRequest.Capacity());
	
	reinterpret_cast<HidpQueueRequestHeader*>(submitReportRequest.data())->RequestType = HidpQueueWriteRequestType::SendReport;
	reinterpret_cast<HidpQueueRequestHeader*>(submitReportRequest.data())->Size = sizeof(HidQueueRequestSubmitReport) + reportSize;
	reinterpret_cast<HidQueueRequestSubmitReport*>(&reinterpret_cast<HidpQueueRequestHeader*>(submitReportRequest.data())->Data)->ReportId = reportId;
	CopyMemory(submitReportRequest.data() + offsetof(HidpQueueRequestHeader, Data) + offsetof(HidQueueRequestSubmitReport, ReportData), reportData, reportSize);

	bool writeResult = WriteFile(hidpHandle, submitReportRequest.data(), submitReportRequest.Length(), NULL, NULL);
	if (!writeResult)
	{
		return E_UNEXPECTED;
	}

	return S_OK;
}
