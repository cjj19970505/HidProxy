#include "pch.h"
#include "Hidp.h"
#include "Hidp.g.cpp"
#include "..\HidProxy\hidp.h"
#include <cfgmgr32.h>

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

	Hidp::Hidp(HANDLE hidpHandle): _HidpHandle(hidpHandle)
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
			hidpHandle = deviceHandle;

			Windows::Storage::Streams::Buffer createVHidRequestBuffer{ static_cast<uint32_t>(reportDescriptor.Length() + sizeof(HidpQueueRequestHeader))};
			createVHidRequestBuffer.Length(createVHidRequestBuffer.Capacity());
			ZeroMemory(createVHidRequestBuffer.data(), createVHidRequestBuffer.Capacity());
			reinterpret_cast<HidpQueueRequestHeader*>(createVHidRequestBuffer.data())->RequestType = HidpQueueWriteRequestType::CreateVHid;
			reinterpret_cast<HidpQueueRequestHeader*>(createVHidRequestBuffer.data())->Size = static_cast<USHORT>(reportDescriptor.Length());
			CopyMemory(createVHidRequestBuffer.data() + offsetof(HidpQueueRequestHeader, Data), reportDescriptor.data(), reportDescriptor.Length());
			//BOOL writeResult = WriteFile(deviceHandle, createVHidRequestBuffer.data(), createVHidRequestBuffer.Length(), NULL, NULL);
			//if (!writeResult)
			//{
			//	hr = E_UNEXPECTED;
			//	break;
			//}
			co_await WriteFileAsync(deviceHandle, createVHidRequestBuffer);

		} while (false);
		
		if (FAILED(hr))
		{
			check_hresult(hr);
		}
		else
		{
			co_return make<implementation::Hidp>(hidpHandle);
		}
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

    void Hidp::Close()
    {
		BOOL bStatus = CloseHandle(_HidpHandle);
		if (!bStatus)
		{
			check_win32(GetLastError());
		}
    }

}
