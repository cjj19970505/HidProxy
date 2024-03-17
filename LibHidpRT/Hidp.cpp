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
	struct Hidp::NotifyThreadParams
	{
		implementation::Hidp* Hidp;
	};
	DWORD Hidp::NotifyThreadProc(
		_In_ LPVOID lpParameter
	)
	{
		NotifyThreadParams* params = reinterpret_cast<NotifyThreadParams*>(lpParameter);
		BOOL bStatus = TRUE;
		DWORD bytesTransferred = 0;
		Windows::Storage::Streams::Buffer buffer{ 1024 };
		OVERLAPPED overlapped{};
		DWORD win32Status = 0;
		// although we don't use this event, but document says:
		// https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-deviceiocontrol#parameters
		// If hDevice was opened with the FILE_FLAG_OVERLAPPED flag, the operation is performed as an overlapped (asynchronous) operation. In this case, lpOverlapped must point to a valid OVERLAPPED structure that contains a handle to an event object. Otherwise, the function fails in unpredictable ways.
		overlapped.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
		bool needRegister = true;
		while (true)
		{
			if (needRegister)
			{
				ZeroMemory(buffer.data(), buffer.Capacity());
				buffer.Length(0);
				DWORD bytesReturn = 0;
				ResetEvent(overlapped.hEvent);
				bStatus = DeviceIoControl(params->Hidp->_HidpHandle, IOCTL_HIDPROXY_REGISTER_NOTIFICATION, NULL, 0, buffer.data(), buffer.Capacity(), &bytesReturn, &overlapped);
				if (!bStatus)
				{
					win32Status = GetLastError();
					if (win32Status != ERROR_IO_PENDING)
					{
						break;
					}
					win32Status = ERROR_SUCCESS;
				}

				needRegister = false;
			}

			ULONG_PTR completionKey = NULL;
			LPOVERLAPPED getQueuedCompletionStatusOverlapped = NULL;
			bStatus = GetQueuedCompletionStatus(params->Hidp->_NotificationCompletionPort, &bytesTransferred, &completionKey, &getQueuedCompletionStatusOverlapped, INFINITE);
			if (!bStatus)
			{
				win32Status = GetLastError();
				break;
			}
			if (&overlapped == getQueuedCompletionStatusOverlapped)
			{
				needRegister = true;
				DWORD bytesTransferred = 0;
				bStatus = GetOverlappedResult(params->Hidp->_HidpHandle, &overlapped, &bytesTransferred, FALSE);
				if (!bStatus)
				{
					win32Status = GetLastError();
					break;
				}

				Windows::Storage::Streams::Buffer receivedBuffer{ bytesTransferred };
				CopyMemory(receivedBuffer.data(), buffer.data(), bytesTransferred);
				receivedBuffer.Length(bytesTransferred);
				HidpNotificationHeader* notification = reinterpret_cast<HidpNotificationHeader*>(receivedBuffer.data());

				if (notification->NotificationType == HidpNotificationType::SetFeature)
				{
					Windows::Storage::Streams::Buffer reportBuffer{ notification->ReportBufferLen };
					CopyMemory(reportBuffer.data(), notification->Data, notification->ReportBufferLen);
					reportBuffer.Length(reportBuffer.Capacity());
					auto request = make<implementation::SetFeatureRequest>(notification->ReportId, notification->HidTransferPacket, notification->VhfOperationHandle, reportBuffer);
					if (params->Hidp->_OnSetFeatureRequestEvent)
					{
						params->Hidp->_OnSetFeatureRequestEvent(*params->Hidp, request);
					}
					else
					{
						params->Hidp->CompleteSetFeatureRequest(request, false);
					}
				}
				else if (notification->NotificationType == HidpNotificationType::GetFeature)
				{
					auto request = make<implementation::GetFeatureRequest>(notification->ReportId, notification->HidTransferPacket, notification->VhfOperationHandle);
					if (params->Hidp->_OnGetFeatureRequestEvent)
					{
						params->Hidp->_OnGetFeatureRequestEvent(*params->Hidp, request);
					}
					else
					{
						params->Hidp->CompleteGetFeatureRequest(request, nullptr, false);
					}
				}
				else
				{
					win32Status = ERROR_NOT_SUPPORTED;
				}
			}
		}
		CloseHandle(overlapped.hEvent);
		return win32Status;
	}

	Windows::Foundation::IAsyncAction WriteFileAsync(HANDLE handle, const Buffer buffer)
	{
		BOOL bStatus = TRUE;
		OVERLAPPED overlapped{};
		overlapped.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
		bStatus = WriteFile(handle, buffer.data(), buffer.Length(), NULL, &overlapped);
		if (!bStatus)
		{
			auto err = GetLastError();
			if (err != ERROR_IO_PENDING)
			{
				bStatus = CloseHandle(overlapped.hEvent);
				check_win32(err);
			}
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
			HANDLE deviceHandle = CreateFile(reinterpret_cast<LPCWSTR>(deviceInterfaceListBuffer.data()), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
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
	winrt::Windows::Foundation::IAsyncAction Hidp::StartAsync()
	{
		co_await DeviceIoControlAsync(_HidpHandle, IOCTL_HIDPROXY_START_VHID, nullptr, nullptr);
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
			co_await DeviceIoControlAsync(_HidpHandle, IOCTL_HIDPROXY_COMPLETE_NOTIFIACTION, completeNotificationBuffer, nullptr);
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
			co_await DeviceIoControlAsync(_HidpHandle, IOCTL_HIDPROXY_COMPLETE_NOTIFIACTION, completeNotificationBuffer, nullptr);
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
		co_await DeviceIoControlAsync(_HidpHandle, IOCTL_HIDPROXY_COMPLETE_NOTIFIACTION, completeNotificationBuffer, nullptr);
	}

    void Hidp::Close()
    {
		BOOL bStatus = TRUE;
		DWORD win32Status = 0;

		if (_HidpHandle != NULL)
		{
			bStatus = CloseHandle(_HidpHandle);
			if (!bStatus)
			{
				check_win32(GetLastError());
			}
			_HidpHandle = NULL;
		}

		// File handle must be close before closing _NotificationCompletionPort
		// https://learn.microsoft.com/en-us/windows/win32/fileio/createiocompletionport#remarks
		if (_NotificationCompletionPort != NULL)
		{
			bStatus = CloseHandle(_NotificationCompletionPort);
			if (!bStatus)
			{
				check_win32(GetLastError());
			}
			_NotificationCompletionPort = NULL;
		}
		
		
		if (_NotifyThread != NULL)
		{
			win32Status = WaitForSingleObject(_NotifyThread, INFINITE);
			if (win32Status != WAIT_OBJECT_0)
			{
				check_win32(win32Status);
			}
			DWORD exitCode = 0;
			bStatus = GetExitCodeThread(_NotifyThread, &exitCode);
			if (!bStatus)
			{
				check_win32(GetLastError());
			}
			if (exitCode != ERROR_SUCCESS && exitCode != ERROR_OPERATION_ABORTED)
			{
				check_win32(exitCode);
			}

			CloseHandle(_NotifyThread);
			_NotifyThread = NULL;
		}
		if (_NotifyThreadParams != NULL) {
			delete _NotifyThreadParams;
			_NotifyThreadParams = NULL;
		}

		_OnGetFeatureRequestEvent.clear();
		_OnSetFeatureRequestEvent.clear();
    }

	void Hidp::CompleteSetFeatureRequest(winrt::LibHidpRT::SetFeatureRequest request, bool supported)
	{
		BOOL bStatus = TRUE;
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
		bStatus = DeviceIoControl(_HidpHandle, IOCTL_HIDPROXY_COMPLETE_NOTIFIACTION, completeNotificationBuffer.data(), completeNotificationBuffer.Length(), NULL, 0, NULL, NULL);
		if (!bStatus)
		{
			auto err = GetLastError();
			check_win32(err);
		}
	}

	void Hidp::CompleteGetFeatureRequest(winrt::LibHidpRT::GetFeatureRequest request, winrt::Windows::Storage::Streams::IBuffer reportBuffer, bool supported)
	{
		BOOL bStatus = TRUE;
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
			bStatus = DeviceIoControl(_HidpHandle, IOCTL_HIDPROXY_COMPLETE_NOTIFIACTION, completeNotificationBuffer.data(), completeNotificationBuffer.Length(), NULL, 0, NULL, NULL);
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
			bStatus = DeviceIoControl(_HidpHandle, IOCTL_HIDPROXY_COMPLETE_NOTIFIACTION, completeNotificationBuffer.data(), completeNotificationBuffer.Length(), NULL, 0, NULL, NULL);
		}
		if (!bStatus)
		{
			auto err = GetLastError();
			check_win32(err);
		}
	}

	Windows::Foundation::IAsyncAction Hidp::InitAsync(winrt::Windows::Storage::Streams::Buffer reportDescriptor)
	{

		// TODO
		// need to delete params
		_NotifyThreadParams = new NotifyThreadParams();
		_NotifyThreadParams->Hidp = this;

		// TODO
		// need to stop the thread when closing the hidp
		_NotifyThread = CreateThread(NULL, 0, NotifyThreadProc, _NotifyThreadParams, 0, NULL);

		Windows::Storage::Streams::Buffer createVHidRequestBuffer{ static_cast<uint32_t>(reportDescriptor.Length() + sizeof(HidpQueueRequestHeader)) };
		createVHidRequestBuffer.Length(createVHidRequestBuffer.Capacity());
		ZeroMemory(createVHidRequestBuffer.data(), createVHidRequestBuffer.Capacity());
		reinterpret_cast<HidpQueueRequestHeader*>(createVHidRequestBuffer.data())->RequestType = HidpQueueWriteRequestType::CreateVHid;
		reinterpret_cast<HidpQueueRequestHeader*>(createVHidRequestBuffer.data())->Size = static_cast<USHORT>(reportDescriptor.Length());
		CopyMemory(createVHidRequestBuffer.data() + offsetof(HidpQueueRequestHeader, Data), reportDescriptor.data(), reportDescriptor.Length());
		
		co_await WriteFileAsync(_HidpHandle, createVHidRequestBuffer);
	}

}
