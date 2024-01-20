#include "pch.h"
#include "Hidp.h"
#include "Hidp.g.cpp"
#include "..\HidProxy\hidp.h"
#include <cfgmgr32.h>
#include <winioctl.h>
#include "SetFeatureRequest.h"
#include "GetFeatureRequest.h"

#define REQUEST_NOT_SUPPORTED 0xC00000BBL
using namespace winrt::Windows::Storage::Streams;
namespace winrt::LibHidpRT::implementation
{

	Windows::Foundation::IAsyncAction WriteFileAsync(HANDLE handle, const Buffer buffer)
	{
		co_await resume_background();
		BOOL bStatus = TRUE;
		OVERLAPPED overlapped{};
		overlapped.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);

		auto overlappedRoutine = [](
			_In_    DWORD dwErrorCode,
			_In_    DWORD dwNumberOfBytesTransfered,
			_Inout_ LPOVERLAPPED lpOverlapped
			)
			{
				BOOL bResult = TRUE;
				bResult = SetEvent(lpOverlapped->hEvent);
				if (!bResult)
				{
					check_win32(GetLastError());
				}
			};
		bStatus = WriteFileEx(handle, buffer.data(), buffer.Length(), &overlapped, overlappedRoutine);
		if (!bStatus)
		{
			bStatus = CloseHandle(overlapped.hEvent);
		}
		co_await resume_on_signal(overlapped.hEvent);
		bStatus = CloseHandle(overlapped.hEvent);
		if (!bStatus)
		{
			check_win32(GetLastError());
		}
	}

	Windows::Foundation::IAsyncAction DeviceIoControlAsync(HANDLE deviceHandle, DWORD ioControlCode, Buffer inBuffer, Buffer outBuffer)
	{
		BOOL bStatus = TRUE;
		OVERLAPPED overlapped{};
		overlapped.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
		LPVOID lpInBuffer = NULL;
		DWORD nInBufferSize = 0;
		LPVOID lpOutBuffer = NULL;
		DWORD nOutBufferSize = 0;
		if (inBuffer != nullptr)
		{
			lpInBuffer = inBuffer.data();
			nInBufferSize = inBuffer.Length();
		}
		if (outBuffer != nullptr)
		{
			lpOutBuffer = outBuffer.data();
			nOutBufferSize = outBuffer.Capacity();
		}
		bStatus = DeviceIoControl(deviceHandle, ioControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, NULL, &overlapped);
		if (!bStatus)
		{
			auto err = GetLastError();
			if (err != ERROR_IO_PENDING)
			{
				check_win32(err);
			}
		}
		co_await resume_on_signal(overlapped.hEvent);
		
		DWORD bytesTransferred = 0;
		bStatus = GetOverlappedResult(deviceHandle, &overlapped, &bytesTransferred, FALSE);
		if (!bStatus)
		{
			auto err = GetLastError();
			check_win32(err);
		}
		if (outBuffer != nullptr)
		{
			outBuffer.Length(bytesTransferred);
		}
	}

	Hidp::Hidp(HANDLE hidpHandle, HANDLE notificationCompletionPort): _HidpHandle(hidpHandle), _NotificationCompletionPort(notificationCompletionPort)
	{
		
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LibHidpRT::Hidp> Hidp::CreateAsync(winrt::Windows::Storage::Streams::Buffer reportDescriptor)
    {
		co_await resume_background();
		ULONG deviceInterfaceListLength = 0;
		CONFIGRET cr = CR_SUCCESS;
		cr = CM_Get_Device_Interface_List_Size(&deviceInterfaceListLength, (LPGUID)(&GUID_DEVINTERFACE_HIDP), nullptr, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

		if (cr != CR_SUCCESS)
		{
			check_hresult(E_UNEXPECTED);
		}
		if (deviceInterfaceListLength == 0)
		{
			check_hresult(E_UNEXPECTED);
		}
		Windows::Storage::Streams::Buffer deviceInterfaceListBuffer{ static_cast<uint32_t>(deviceInterfaceListLength * sizeof(WCHAR)) };
		deviceInterfaceListBuffer.Length(deviceInterfaceListLength * sizeof(WCHAR));
		ZeroMemory(deviceInterfaceListBuffer.data(), deviceInterfaceListBuffer.Capacity());
		HANDLE hidpHandle = INVALID_HANDLE_VALUE;
		HANDLE notificationCompletionPort = NULL;
		HRESULT hr = S_OK;
		DWORD win32Err = 0;
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
			hidpHandle = deviceHandle;

			notificationCompletionPort = CreateIoCompletionPort(deviceHandle, NULL, NULL, 0);
			if (notificationCompletionPort == NULL)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				break;
			}

		} while (false);
		
		if (FAILED(hr))
		{
			if (hidpHandle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hidpHandle);
			}
			if (notificationCompletionPort != INVALID_HANDLE_VALUE)
			{
				CloseHandle(notificationCompletionPort);
			}
			check_hresult(hr);
		}

		auto hidp = make<implementation::Hidp>(hidpHandle, notificationCompletionPort);
		auto hidpAbi = hidp.as<implementation::Hidp>();
		co_await hidpAbi->InitAsync(reportDescriptor);
		co_return hidp;
    }
    winrt::Windows::Foundation::IAsyncAction Hidp::SubmitReportAsync(uint8_t reportId, winrt::Windows::Storage::Streams::IBuffer reportData)
    {
		Windows::Storage::Streams::Buffer submitReportRequest{ static_cast<uint32_t>(reportData.Length() + sizeof(HidQueueRequestSubmitReport) + sizeof(HidpQueueRequestHeader))};
		submitReportRequest.Length(submitReportRequest.Capacity());
		ZeroMemory(submitReportRequest.data(), submitReportRequest.Capacity());
		reinterpret_cast<HidpQueueRequestHeader*>(submitReportRequest.data())->RequestType = HidpQueueWriteRequestType::SendReport;
		reinterpret_cast<HidpQueueRequestHeader*>(submitReportRequest.data())->Size = static_cast<USHORT>(sizeof(HidQueueRequestSubmitReport) + reportData.Length());
		reinterpret_cast<HidQueueRequestSubmitReport*>(&reinterpret_cast<HidpQueueRequestHeader*>(submitReportRequest.data())->Data)->ReportId = reportId;
		CopyMemory(submitReportRequest.data() + offsetof(HidpQueueRequestHeader, Data) + offsetof(HidQueueRequestSubmitReport, ReportData), reportData.data(), reportData.Length());
		co_await WriteFileAsync(_HidpHandle, submitReportRequest);
    }

	winrt::event_token Hidp::OnGetFeatureRequest(winrt::Windows::Foundation::EventHandler<winrt::LibHidpRT::GetFeatureRequest> const& handler)
	{
		return _OnGetFeatureRequestEvent.add(handler);
	}

	void Hidp::OnGetFeatureRequest(winrt::event_token const& token) noexcept
	{
		_OnGetFeatureRequestEvent.remove(token);
	}

	winrt::event_token Hidp::OnSetFeatureRequest(winrt::Windows::Foundation::EventHandler<winrt::LibHidpRT::SetFeatureRequest> const& handler)
	{
		return _OnSetFeatureRequestEvent.add(handler);
	}

	void Hidp::OnSetFeatureRequest(winrt::event_token const& token) noexcept
	{
		_OnSetFeatureRequestEvent.remove(token);
	}

	winrt::Windows::Foundation::IAsyncAction Hidp::CompleteGetFeatureRequestAsync(winrt::LibHidpRT::GetFeatureRequest request, winrt::Windows::Storage::Streams::IBuffer reportBuffer, bool supported)
	{
		auto requestAbi = request.as<implementation::GetFeatureRequest>();
		if (request == nullptr || !supported)
		{
			Buffer completeNotificationBuffer{ static_cast<uint32_t>(sizeof(HidpCompleteNotificationHeader)) };
			ZeroMemory(completeNotificationBuffer.data(), completeNotificationBuffer.Capacity());
			completeNotificationBuffer.Length(completeNotificationBuffer.Capacity());
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->NotificationType = HidpNotificationType::GetFeature;
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->HidTransferPacket = requestAbi->HidTransferPacket();
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->VhfOperationHandle = requestAbi->VhfOperationHandle();
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->ReportId = requestAbi->ReportId();
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->ReportBufferLen = 0;
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->CompletionStatus = REQUEST_NOT_SUPPORTED;
			co_await DeviceIoControlAsync(_HidpHandle, IOCTL_COMPLETE_NOTIFIACTION, completeNotificationBuffer, nullptr);
		}
		else
		{
			Buffer completeNotificationBuffer{ static_cast<uint32_t>(sizeof(HidpCompleteNotificationHeader) + reportBuffer.Length()) };
			ZeroMemory(completeNotificationBuffer.data(), completeNotificationBuffer.Capacity());
			completeNotificationBuffer.Length(completeNotificationBuffer.Capacity());
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->NotificationType = HidpNotificationType::GetFeature;
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->HidTransferPacket = requestAbi->HidTransferPacket();
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->VhfOperationHandle = requestAbi->VhfOperationHandle();
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->ReportId = requestAbi->ReportId();
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->ReportBufferLen = reportBuffer.Length();
			CopyMemory(reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->Data, reportBuffer.data(), reportBuffer.Length());
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->CompletionStatus = 0;
			co_await DeviceIoControlAsync(_HidpHandle, IOCTL_COMPLETE_NOTIFIACTION, completeNotificationBuffer, nullptr);
		}

	}
	winrt::Windows::Foundation::IAsyncAction Hidp::CompleteSetFeatureRequestAsync(winrt::LibHidpRT::SetFeatureRequest request, bool supported)
	{
		Buffer completeNotificationBuffer{ sizeof(HidpCompleteNotificationHeader) };
		ZeroMemory(completeNotificationBuffer.data(), completeNotificationBuffer.Capacity());
		completeNotificationBuffer.Length(completeNotificationBuffer.Capacity());
		auto requestAbi = request.as<implementation::SetFeatureRequest>();
		reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->NotificationType = HidpNotificationType::SetFeature;
		reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->HidTransferPacket = requestAbi->HidTransferPacket();
		reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->VhfOperationHandle = requestAbi->VhfOperationHandle();
		reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->ReportId = requestAbi->ReportId();
		reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->ReportBufferLen = 0;
		if (supported)
		{
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->CompletionStatus = 0L;
		}
		else
		{
			reinterpret_cast<HidpCompleteNotificationHeader*>(completeNotificationBuffer.data())->CompletionStatus = REQUEST_NOT_SUPPORTED;
		}
		co_await DeviceIoControlAsync(_HidpHandle, IOCTL_COMPLETE_NOTIFIACTION, completeNotificationBuffer, nullptr);
	}

    void Hidp::Close()
    {
		BOOL bStatus = CloseHandle(_HidpHandle);
		if (!bStatus)
		{
			check_win32(GetLastError());
		}
    }

	Windows::Foundation::IAsyncAction Hidp::InitAsync(winrt::Windows::Storage::Streams::Buffer reportDescriptor)
	{
		BOOL bStatus = TRUE;

		HANDLE notificationRegisteredHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (notificationRegisteredHandle == NULL)
		{
			check_win32(GetLastError());
		}
		_NotificationTask = _RegisterForNotificationAsync(notificationRegisteredHandle);
		

		Windows::Storage::Streams::Buffer createVHidRequestBuffer{ static_cast<uint32_t>(reportDescriptor.Length() + sizeof(HidpQueueRequestHeader)) };
		createVHidRequestBuffer.Length(createVHidRequestBuffer.Capacity());
		ZeroMemory(createVHidRequestBuffer.data(), createVHidRequestBuffer.Capacity());
		reinterpret_cast<HidpQueueRequestHeader*>(createVHidRequestBuffer.data())->RequestType = HidpQueueWriteRequestType::CreateVHid;
		reinterpret_cast<HidpQueueRequestHeader*>(createVHidRequestBuffer.data())->Size = static_cast<USHORT>(reportDescriptor.Length());
		CopyMemory(createVHidRequestBuffer.data() + offsetof(HidpQueueRequestHeader, Data), reportDescriptor.data(), reportDescriptor.Length());

		// Start Vhid after IOCTL_REGISTER_NOTIFICATION
		co_await resume_on_signal(notificationRegisteredHandle);
		bStatus = CloseHandle(notificationRegisteredHandle);
		if (!bStatus)
		{
			check_win32(GetLastError());
		}
		
		co_await WriteFileAsync(_HidpHandle, createVHidRequestBuffer);
	}

	winrt::Windows::Foundation::IAsyncAction Hidp::_RegisterForNotificationAsync(HANDLE notificationRegisteredHandle)
	{
		co_await resume_background();
		BOOL bStatus = TRUE;
		HANDLE initNotificationRegisteredHandle = notificationRegisteredHandle;
		DWORD bytesTransferred = 0;
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
				DWORD bytesReturn = 0;
				bStatus = DeviceIoControl(_HidpHandle, IOCTL_REGISTER_NOTIFICATION, NULL, 0, buffer.data(), buffer.Capacity(), &bytesReturn, &overlapped);
				if (!bStatus)
				{
					auto err = GetLastError();
					if (err != ERROR_IO_PENDING)
					{
						check_win32(err);
					}
				}
				if (initNotificationRegisteredHandle != NULL)
				{
					bStatus = SetEvent(initNotificationRegisteredHandle);
					initNotificationRegisteredHandle = NULL;
				}
				if (!bStatus)
				{
					auto err = GetLastError();
					check_win32(err);
				}
				needRegister = false;
			}

			ULONG_PTR completionKey = NULL;
			LPOVERLAPPED getQueuedCompletionStatusOverlapped = NULL;
			bStatus = GetQueuedCompletionStatus(_NotificationCompletionPort, &bytesTransferred, &completionKey, &getQueuedCompletionStatusOverlapped, INFINITE);
			if (!bStatus)
			{
				auto err = GetLastError();
				check_win32(err);
			}
			if (&overlapped == getQueuedCompletionStatusOverlapped)
			{
				needRegister = true;
				DWORD bytesTransferred = 0;
				bStatus = GetOverlappedResult(_HidpHandle, &overlapped, &bytesTransferred, FALSE);
				if (!bStatus)
				{
					auto err = GetLastError();
					check_win32(err);
				}

				Windows::Storage::Streams::Buffer receivedBuffer{ bytesTransferred };
				CopyMemory(receivedBuffer.data(), buffer.data(), bytesTransferred);
				receivedBuffer.Length(bytesTransferred);
				HidpNotificationHeader* notification = reinterpret_cast<HidpNotificationHeader*>(receivedBuffer.data());
				
				if (notification->NotificationType == HidpNotificationType::SetFeature)
				{
					if (_OnGetFeatureRequestEvent)
					{
						Windows::Storage::Streams::Buffer reportBuffer{ notification->ReportBufferLen };
						CopyMemory(reportBuffer.data(), notification->Data, notification->ReportBufferLen);
						reportBuffer.Length(reportBuffer.Capacity());
						auto request = make<implementation::SetFeatureRequest>(notification->ReportId, notification->HidTransferPacket, notification->VhfOperationHandle, reportBuffer);
						_OnSetFeatureRequestEvent(*this, request);
					}
				}
				else if (notification->NotificationType == HidpNotificationType::GetFeature)
				{
					if (_OnGetFeatureRequestEvent)
					{
						auto request = make<implementation::GetFeatureRequest>(notification->ReportId, notification->HidTransferPacket, notification->VhfOperationHandle);
						_OnGetFeatureRequestEvent(*this, request);
					}
				}
			}
		}
	}

}
