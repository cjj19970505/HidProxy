#include "pch.h"
#include "AoaInputStream.h"

using namespace winrt;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::Foundation;

winrt::AoaRT::implementation::AoaInputStream::AoaInputStream(com_ptr<implementation::AoaDevice> abiAoaDevice):_AbiAoaDevice(abiAoaDevice)
{
}

IAsyncOperationWithProgress<IBuffer, uint32_t> winrt::AoaRT::implementation::AoaInputStream::ReadAsync(IBuffer const& buffer, uint32_t count, InputStreamOptions const& options) const
{
	if (options != InputStreamOptions::None)
	{
		throw hresult_invalid_argument();
	}

	Buffer outputBuffer{ count };

	ZeroMemory(outputBuffer.data(), outputBuffer.Capacity());
	
	ULONG lengthTransferred = 0;
	BOOL bStatus = TRUE;
	OVERLAPPED overlapped{};
	overlapped.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
	bStatus = WinUsb_ReadPipe(_AbiAoaDevice->AoaInterfaceHandle(), _AbiAoaDevice->InEndpointDescriptor().bEndpointAddress, reinterpret_cast<PUCHAR>(outputBuffer.data()), count, &lengthTransferred, &overlapped);
	if (!bStatus)
	{
		auto err = GetLastError();
		if (err != ERROR_IO_PENDING)
		{
			throw hresult{ HRESULT_FROM_WIN32(err) };
		}
	}
	co_await resume_on_signal(overlapped.hEvent);
	bStatus = CloseHandle(overlapped.hEvent);
	if (!bStatus)
	{
		auto err = GetLastError();
		throw hresult{ HRESULT_FROM_WIN32(err) };
	}
	DWORD numOfBytesTransferred = 0;
	bStatus = WinUsb_GetOverlappedResult(_AbiAoaDevice->AoaInterfaceHandle(), &overlapped, &numOfBytesTransferred, FALSE);
	if (!bStatus)
	{
		auto err = GetLastError();
		throw hresult{ HRESULT_FROM_WIN32(err) };
	}
	
	outputBuffer.Length(numOfBytesTransferred);
	co_return outputBuffer;
}

void winrt::AoaRT::implementation::AoaInputStream::Close()
{
}
