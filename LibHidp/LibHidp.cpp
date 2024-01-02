#include "pch.h"
#include "LibHidp.h"
#include "../HidProxy/hidp.h"
#include <cfgmgr32.h>
#include <winrt/windows.storage.streams.h>
#include <winioctl.h>
#include <queue>

using namespace winrt;

struct NotifyThreadParams
{
	HANDLE CompletionPort;
	HANDLE DeviceHandle;
};

struct HidpHandleImpl
{
	HANDLE DeviceHandle;
	HANDLE NofiyThread;
	HANDLE NotificationResultQueueMutex;
	std::queue<Windows::Storage::Streams::Buffer> NotifyResultQueue;
};

DWORD NotifyThreadProc(
	_In_ LPVOID lpParameter
)
{
	NotifyThreadParams* params = reinterpret_cast<NotifyThreadParams*>(lpParameter);

	BOOL bStatus = TRUE;
	DWORD bytesTransferred = 0;
	ULONG_PTR completionKey = NULL;
	Windows::Storage::Streams::Buffer buffer{ 1024 };
	OVERLAPPED overlapped{};
	bool needRegister = true;
	while (true)
	{
		if (needRegister)
		{
			ZeroMemory(&overlapped, sizeof(OVERLAPPED));
			ZeroMemory(buffer.data(), buffer.Capacity());
			buffer.Length(0);
			// BOOL registerResult = WriteFile(params->DeviceHandle, buffer.data(), buffer.Length(), NULL, &overlapped);
			DWORD bytesReturn = 0;
			bStatus = DeviceIoControl(params->DeviceHandle, IOCTL_REGISTER_NOTIFICATION, NULL, 0, buffer.data(), buffer.Capacity(), &bytesReturn, &overlapped);
			if (!bStatus)
			{
				auto err = GetLastError();
				if (err != ERROR_IO_PENDING)
				{
					return err;
				}
			}
			needRegister = false;
		}
		
		LPOVERLAPPED getQueuedCompletionStatusOverlapped = NULL;
		bStatus = GetQueuedCompletionStatus(params->CompletionPort, &bytesTransferred, &completionKey, &getQueuedCompletionStatusOverlapped, INFINITE);
		if (!bStatus)
		{
			auto err = GetLastError();
			return err;
		}
		if (&overlapped == getQueuedCompletionStatusOverlapped)
		{
			needRegister = true;
			DWORD bytesTransferred = 0;
			bStatus = GetOverlappedResult(params->DeviceHandle, &overlapped, &bytesTransferred, FALSE);
			if (!bStatus)
			{
				auto err = GetLastError();
				return err;
			}
			Windows::Storage::Streams::Buffer receivedBuffer{ bytesTransferred };
			CopyMemory(receivedBuffer.data(), buffer.data(), bytesTransferred);
			receivedBuffer.Length(bytesTransferred);
			HidpNotificationHeader* notification = reinterpret_cast<HidpNotificationHeader*>(receivedBuffer.data());
		}
	}
	return ERROR_SUCCESS;
}

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
		HANDLE deviceHandle = CreateFile(reinterpret_cast<LPCWSTR>(deviceInterfaceListBuffer.data()), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		HANDLE completionPort = CreateIoCompletionPort(deviceHandle, NULL, NULL, 0);
		if (completionPort == NULL)
		{
			auto err = GetLastError();
			hr = winrt::impl::hresult_from_win32(err);
		}
		NotifyThreadParams* threadParams = new NotifyThreadParams();
		threadParams->CompletionPort = completionPort;
		threadParams->DeviceHandle = deviceHandle;
		HANDLE notifyThread = CreateThread(NULL, 0, NotifyThreadProc, threadParams, 0, NULL);
		if (notifyThread == INVALID_HANDLE_VALUE)
		{
			hr = E_UNEXPECTED;
			break;
		}

		Windows::Storage::Streams::Buffer createVHidRequestBuffer{ static_cast<uint32_t>(nReportDescriptorSize + sizeof(HidpQueueRequestHeader)) };
		createVHidRequestBuffer.Length(createVHidRequestBuffer.Capacity());
		ZeroMemory(createVHidRequestBuffer.data(), createVHidRequestBuffer.Capacity());
		reinterpret_cast<HidpQueueRequestHeader*>(createVHidRequestBuffer.data())->RequestType = HidpQueueWriteRequestType::CreateVHid;
		reinterpret_cast<HidpQueueRequestHeader*>(createVHidRequestBuffer.data())->Size = nReportDescriptorSize;
		CopyMemory(createVHidRequestBuffer.data() + offsetof(HidpQueueRequestHeader, Data), lpReportDescriptor, nReportDescriptorSize);
		OVERLAPPED overlapped{};
		overlapped.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
		if (overlapped.hEvent == NULL)
		{
			auto error = GetLastError();
			hr = winrt::impl::hresult_from_win32(error);
			break;
		}
		BOOL writeResult = WriteFile(deviceHandle, createVHidRequestBuffer.data(), createVHidRequestBuffer.Length(), NULL, &overlapped);
		
		if (!writeResult)
		{
			auto err = GetLastError();
			if (err != ERROR_IO_PENDING)
			{
				hr = E_UNEXPECTED;
				break;
			}
		}

		auto waitResult = WaitForSingleObject(overlapped.hEvent, INFINITE);
		if (waitResult != ERROR_SUCCESS)
		{
			hr = winrt::impl::hresult_from_win32(waitResult);
			break;
		}

		*pHidpHandle = new HidpHandleImpl();
		reinterpret_cast<HidpHandleImpl*>(*pHidpHandle)->DeviceHandle = deviceHandle;
		reinterpret_cast<HidpHandleImpl*>(*pHidpHandle)->NofiyThread = notifyThread;

	} while (false);

	if (FAILED(hr))
	{
		*pHidpHandle = NULL;
	}

	return hr;
}

BOOL HidpClose(HIDPHANDLE hidpHandle)
{
	return CloseHandle(reinterpret_cast<HidpHandleImpl*>(hidpHandle)->DeviceHandle);
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

	OVERLAPPED overlapped{};
	overlapped.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
	if (overlapped.hEvent == NULL)
	{
		auto error = GetLastError();
		return winrt::impl::hresult_from_win32(error);
	}
	BOOL writeResult = WriteFile(reinterpret_cast<HidpHandleImpl*>(hidpHandle)->DeviceHandle, submitReportRequest.data(), submitReportRequest.Length(), NULL, &overlapped);
	if (!writeResult)
	{
		auto err = GetLastError();
		if (err != ERROR_IO_PENDING)
		{
			return winrt::impl::hresult_from_win32(err);
		}
	}
	auto waitResult = WaitForSingleObject(overlapped.hEvent, INFINITE);
	if (waitResult != ERROR_SUCCESS)
	{
		return winrt::impl::hresult_from_win32(waitResult);
	}

	return S_OK;
}
