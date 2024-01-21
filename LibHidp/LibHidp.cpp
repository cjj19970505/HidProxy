#include "pch.h"
#include "LibHidp.h"
#include "../HidProxy/hidp.h"
#include <cfgmgr32.h>
#include <winrt/windows.storage.streams.h>
#include <winioctl.h>
#include <queue>

using namespace winrt;



struct HidpHandleImpl
{
	HANDLE DeviceHandle;
	HANDLE NofiyThread;
	PSET_FEATURE_ROUTINE SetFeatureRoutine;
	PGET_FEATURE_ROUTINE GetFeatureRoutine;
};

struct NotifyThreadParams
{
	HANDLE CompletionPort;
	HIDPHANDLE HidpHandle;
	HANDLE NotifyThreadReadyEvent;
};

DWORD NotifyThreadProc(
	_In_ LPVOID lpParameter
)
{
	NotifyThreadParams* params = reinterpret_cast<NotifyThreadParams*>(lpParameter);
	HidpHandleImpl* hidpHandle = reinterpret_cast<HidpHandleImpl*>(params->HidpHandle);

	BOOL bStatus = TRUE;
	DWORD bytesTransferred = 0;
	ULONG_PTR completionKey = NULL;
	Windows::Storage::Streams::Buffer buffer{ 1024 };
	OVERLAPPED overlapped{};
	overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (overlapped.hEvent == NULL)
	{
		auto err = GetLastError();
		return err;
	}
	bool needRegister = true;
	HANDLE notifyThreadReadyEvent = params->NotifyThreadReadyEvent;
	while (true)
	{
		if (needRegister)
		{
			ZeroMemory(buffer.data(), buffer.Capacity());
			buffer.Length(0);
			bStatus = ResetEvent(overlapped.hEvent);
			if (!bStatus)
			{
				auto err = GetLastError();
				return err;
			}
			DWORD bytesReturn = 0;
			bStatus = DeviceIoControl(hidpHandle->DeviceHandle, IOCTL_HIDPROXY_REGISTER_NOTIFICATION, NULL, 0, buffer.data(), buffer.Capacity(), &bytesReturn, &overlapped);
			if (!bStatus)
			{
				auto err = GetLastError();
				if (err != ERROR_IO_PENDING)
				{
					return err;
				}
			}
			if (notifyThreadReadyEvent != NULL)
			{
				bStatus = SetEvent(notifyThreadReadyEvent);
				if (!bStatus)
				{
					auto err = GetLastError();
					return err;
				}
				notifyThreadReadyEvent = NULL;
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
			bStatus = GetOverlappedResult(hidpHandle->DeviceHandle, &overlapped, &bytesTransferred, FALSE);
			if (!bStatus)
			{
				auto err = GetLastError();
				return err;
			}
			Windows::Storage::Streams::Buffer receivedBuffer{ bytesTransferred };
			CopyMemory(receivedBuffer.data(), buffer.data(), bytesTransferred);
			receivedBuffer.Length(bytesTransferred);
			HidpNotificationHeader* notification = reinterpret_cast<HidpNotificationHeader*>(receivedBuffer.data());

			if (notification->NotificationType == HidpNotificationType::SetFeature)
			{
				Windows::Storage::Streams::Buffer completeNotificationBuffer{ sizeof(HidpCompleteNotificationHeader) };
				ZeroMemory(completeNotificationBuffer.data(), completeNotificationBuffer.Capacity());
				completeNotificationBuffer.Length(completeNotificationBuffer.Capacity());
				reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->NotificationType = notification->NotificationType;
				reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->HidTransferPacket = notification->HidTransferPacket;
				reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->VhfOperationHandle = notification->VhfOperationHandle;
				reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->ReportId = notification->ReportId;
				reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->ReportBufferLen = 0;

				if (hidpHandle->SetFeatureRoutine != nullptr)
				{
					hidpHandle->SetFeatureRoutine(hidpHandle, notification->ReportId, notification->ReportBufferLen, notification->Data);
					reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->CompletionStatus = 0L;
				}
				else
				{
					reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->CompletionStatus = 0xC00000BBL;
				}
				bStatus = DeviceIoControl(hidpHandle->DeviceHandle, IOCTL_HIDPROXY_COMPLETE_NOTIFIACTION, completeNotificationBuffer.data(), completeNotificationBuffer.Length(), NULL, 0, NULL, NULL);
			}
			else if (notification->NotificationType == HidpNotificationType::GetFeature)
			{
				Windows::Storage::Streams::Buffer completeNotificationBuffer{ static_cast<uint32_t>(sizeof(HidpCompleteNotificationHeader) + notification->ReportBufferLen) };
				ZeroMemory(completeNotificationBuffer.data(), completeNotificationBuffer.Capacity());
				completeNotificationBuffer.Length(completeNotificationBuffer.Capacity());
				reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->NotificationType = notification->NotificationType;
				reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->HidTransferPacket = notification->HidTransferPacket;
				reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->VhfOperationHandle = notification->VhfOperationHandle;
				reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->ReportId = notification->ReportId;
				

				if (hidpHandle->GetFeatureRoutine != nullptr)
				{
					hidpHandle->GetFeatureRoutine(
						hidpHandle, 
						notification->ReportId, 
						notification->ReportBufferLen, 
						&(reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->ReportBufferLen), 
						reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->Data
					);
					reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->CompletionStatus = 0L;
				}
				else
				{
					reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->CompletionStatus = 0xC00000BBL;
				}
				bStatus = DeviceIoControl(hidpHandle->DeviceHandle, IOCTL_HIDPROXY_COMPLETE_NOTIFIACTION, completeNotificationBuffer.data(), completeNotificationBuffer.Length(), NULL, 0, NULL, NULL);
			}
			
			if (!bStatus)
			{
				auto err = GetLastError();
				return err;
			}
		}
	}
	return ERROR_SUCCESS;
}

HRESULT HidpCreate(DWORD nReportDescriptorSize, LPCVOID lpReportDescriptor, PSET_FEATURE_ROUTINE setFeatureRoutine, PGET_FEATURE_ROUTINE getFeatureRoutine, PHIDPHANDLE pHidpHandle)
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
	*pHidpHandle = new HidpHandleImpl();
	ZeroMemory(*pHidpHandle, sizeof(HidpHandleImpl));
	
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
		threadParams->HidpHandle = *pHidpHandle;
		HANDLE notifyThreadReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		threadParams->NotifyThreadReadyEvent = notifyThreadReadyEvent;
		HANDLE notifyThread = CreateThread(NULL, 0, NotifyThreadProc, threadParams, 0, NULL);
		if (notifyThread == INVALID_HANDLE_VALUE || notifyThread == NULL)
		{
			hr = E_UNEXPECTED;
			break;
		}

		reinterpret_cast<HidpHandleImpl*>(*pHidpHandle)->DeviceHandle = deviceHandle;
		reinterpret_cast<HidpHandleImpl*>(*pHidpHandle)->NofiyThread = notifyThread;
		reinterpret_cast<HidpHandleImpl*>(*pHidpHandle)->SetFeatureRoutine = setFeatureRoutine;
		reinterpret_cast<HidpHandleImpl*>(*pHidpHandle)->GetFeatureRoutine = getFeatureRoutine;

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
		

		// TODO
		// resume thread error check

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

		waitResult = WaitForSingleObject(notifyThreadReadyEvent, INFINITE);
		if (waitResult != ERROR_SUCCESS)
		{
			hr = winrt::impl::hresult_from_win32(waitResult);
			break;
		}

	} while (false);

	if (FAILED(hr))
	{
		*pHidpHandle = NULL;
	}

	return hr;
}

LIBHIDP_API BOOL HidpStart(HIDPHANDLE hidpHandle)
{
	HidpHandleImpl* hidpHandleImpl = reinterpret_cast<HidpHandleImpl*>(hidpHandle);
	return DeviceIoControl(hidpHandleImpl->DeviceHandle, IOCTL_HIDPROXY_START_VHID, NULL, 0, NULL, 0, NULL, NULL);
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
