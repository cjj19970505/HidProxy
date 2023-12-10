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
	throw hresult_not_implemented();
}

void winrt::AoaRT::implementation::AoaInputStream::Close()
{
}
