#pragma once
#include "pch.h"
#include "AoaDevice.h"

namespace winrt::AoaRT::implementation
{
	using namespace winrt;
	using namespace winrt::Windows::Storage::Streams;
	using namespace winrt::Windows::Foundation;
	struct AoaInputStream:implements<AoaInputStream, IInputStream>
	{
		AoaInputStream(com_ptr<implementation::AoaDevice> abiAoaDevice);
		IAsyncOperationWithProgress<IBuffer, uint32_t> ReadAsync(IBuffer const& buffer, uint32_t count, InputStreamOptions const& options) const;
		void Close();

	private:
		com_ptr<implementation::AoaDevice> _AbiAoaDevice;
	};
}


