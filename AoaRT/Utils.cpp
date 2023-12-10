#include "pch.h"
#include "Utils.h"
#include <cfgmgr32.h>
#include <SetupAPI.h>
#include <winusb.h>

#define GUID_STR_LEN 38

using namespace winrt;

winrt::Windows::Storage::Streams::IBuffer Utils::WideCharToUtf8Buffer(const winrt::hstring& wstr)
{
	auto bufferSize = WideCharToMultiByte(CP_UTF8, NULL, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	if (bufferSize == 0)
	{
		check_nt(GetLastError());
	}
	winrt::Windows::Storage::Streams::Buffer buffer{ static_cast<uint32_t>(bufferSize) };
	bufferSize = WideCharToMultiByte(CP_UTF8, NULL, wstr.c_str(), -1, reinterpret_cast<LPSTR>(buffer.data()), buffer.Capacity(), NULL, NULL);
	if (bufferSize == 0)
	{
		check_nt(GetLastError());
	}
	buffer.Length(static_cast<uint32_t>(bufferSize));
	return buffer;
}

std::optional<std::wstring> Utils::GetDevicePathFromGuid(LPGUID guid)
{
	ULONG deviceInterfaceListLength = 0;
	CONFIGRET cr = CR_SUCCESS;
	cr = CM_Get_Device_Interface_List_Size(&deviceInterfaceListLength, guid, nullptr, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

	if (cr != CR_SUCCESS)
	{
		return std::nullopt;
	}
	if (deviceInterfaceListLength == 0)
	{
		return std::nullopt;
	}
	Windows::Storage::Streams::Buffer deviceInterfaceListBuffer{ static_cast<uint32_t>(deviceInterfaceListLength * sizeof(WCHAR)) };
	deviceInterfaceListBuffer.Length(deviceInterfaceListLength * sizeof(WCHAR));
	ZeroMemory(deviceInterfaceListBuffer.data(), deviceInterfaceListBuffer.Capacity());
	HRESULT hr = S_OK;
	do
	{
		cr = CM_Get_Device_Interface_List(guid, nullptr, reinterpret_cast<PZZWSTR>(deviceInterfaceListBuffer.data()), deviceInterfaceListLength, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

		// Assume only have 1 interface
		if (*(reinterpret_cast<LPCWSTR>(deviceInterfaceListBuffer.data()) + wcslen(reinterpret_cast<LPCWSTR>(deviceInterfaceListBuffer.data())) + 1) != UNICODE_NULL)
		{
			hr = E_UNEXPECTED;
			break;
		}
		return std::wstring{ reinterpret_cast<LPCWSTR>(deviceInterfaceListBuffer.data()) };
	} while (false);
	return std::nullopt;
}

std::vector<std::wstring> Utils::GetDevicePathFromVidPid(USHORT vid, USHORT pid)
{
	HDEVINFO devInfoSet = SetupDiGetClassDevsW(NULL, L"USB", NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	DWORD devInfoIndex = 0;
	std::vector<std::wstring> devicePaths{};
	while (true)
	{
		BOOL bResult = TRUE;
		LSTATUS lStatus = ERROR_SUCCESS;
		HRESULT hResult = S_OK;
		SP_DEVINFO_DATA devInfoData{};
		{
			devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			bResult = SetupDiEnumDeviceInfo(devInfoSet, devInfoIndex, &devInfoData);
			++devInfoIndex;
			if (!bResult)
			{
				auto err = GetLastError();
				assert(err == ERROR_NO_MORE_ITEMS);
				break;
			}
		}

		bResult = SetupDiGetDeviceRegistryPropertyW(devInfoSet, &devInfoData, SPDRP_DRIVER, NULL, NULL, 0, NULL);
		if (!bResult)
		{
			auto err = GetLastError();
			if (err != ERROR_INSUFFICIENT_BUFFER)
			{
				continue;
			}
			else
			{
				err = ERROR_SUCCESS;
			}
		}

		// Copied from libusb windows_winusb.c
		HKEY key = SetupDiOpenDevRegKey(devInfoSet, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
		if (key == INVALID_HANDLE_VALUE)
		{
			continue;
		}

		GUID deviceInterfaceGuid{};
		{
			bool useDeviceInterfaceGUIDs = true;
			DWORD type = 0;
			DWORD dataSize = 0;

			lStatus = RegQueryValueExW(key, L"DeviceInterfaceGUIDs", NULL, &type, NULL, &dataSize);
			if (lStatus == ERROR_FILE_NOT_FOUND)
			{
				lStatus = RegQueryValueExW(key, L"DeviceInterfaceGUID", NULL, &type, NULL, &dataSize);
				useDeviceInterfaceGUIDs = false;
			}
			if (lStatus != ERROR_SUCCESS)
			{
				continue;
			}

			// value returned is NOT guaranteed to be null-terminated.
			// https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryvalueexw
			DWORD allocateSize = dataSize;
			if (dataSize < sizeof(WCHAR) * (GUID_STR_LEN + 1))
			{
				allocateSize = sizeof(WCHAR) * (GUID_STR_LEN + 1);
			}
			std::unique_ptr<uint8_t[]> guidStr{ new uint8_t[allocateSize] };
			ZeroMemory(guidStr.get(), allocateSize);
			if (useDeviceInterfaceGUIDs)
			{
				lStatus = RegQueryValueExW(key, L"DeviceInterfaceGUIDs", NULL, &type, guidStr.get(), &dataSize);
			}
			else
			{
				lStatus = RegQueryValueExW(key, L"DeviceInterfaceGUID", NULL, &type, guidStr.get(), &dataSize);
			}
			if (lStatus != ERROR_SUCCESS)
			{
				continue;
			}
			// It just works.
			hResult = IIDFromString(reinterpret_cast<LPCOLESTR>(guidStr.get()), &deviceInterfaceGuid);
			if (FAILED(hResult))
			{
				continue;
			}
		}

		auto devicePath = GetDevicePathFromGuid(&deviceInterfaceGuid);
		if (!devicePath.has_value())
		{
			continue;
		}

		HANDLE hDevice = CreateFileW(
			reinterpret_cast<LPCWSTR>(devicePath.value().c_str()),
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL
		);

		if (hDevice == INVALID_HANDLE_VALUE)
		{
			continue;
		}

		WINUSB_INTERFACE_HANDLE hWinUsbInterface{ nullptr };
		if (!WinUsb_Initialize(hDevice, &hWinUsbInterface))
		{
			CloseHandle(hDevice);
			continue;
		}
		USB_DEVICE_DESCRIPTOR deviceDescriptor{};
		ULONG lengthTransfered = 0;
		if (!WinUsb_GetDescriptor(hWinUsbInterface, USB_DEVICE_DESCRIPTOR_TYPE, 0, 0, reinterpret_cast<PUCHAR>(&deviceDescriptor), sizeof(USB_DEVICE_DESCRIPTOR), &lengthTransfered))
		{
			CloseHandle(hDevice);
			continue;
		}
		if (deviceDescriptor.idVendor == vid && deviceDescriptor.idProduct == pid)
		{
			devicePaths.emplace_back(std::move(devicePath.value()));
		}
		CloseHandle(hDevice);
	}
	return devicePaths;
}

winrt::Windows::Foundation::IAsyncAction Utils::WinUsbControlTransferAsync(WINUSB_INTERFACE_HANDLE interfaceHandle, WINUSB_SETUP_PACKET setupPacket, winrt::Windows::Storage::Streams::IBuffer buffer)
{
	ULONG lengthTransfered = 0;
	OVERLAPPED overlapped{};
	overlapped.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
	BOOL bStatus = TRUE;

	if (buffer == nullptr)
	{
		bStatus = WinUsb_ControlTransfer(interfaceHandle, setupPacket, NULL, 0, &lengthTransfered, &overlapped);
	}
	else
	{
		if ((setupPacket.RequestType | 0x80) == 0)
		{
			// Host to device
			bStatus = WinUsb_ControlTransfer(interfaceHandle, setupPacket, reinterpret_cast<PUCHAR>(buffer.data()), buffer.Length(), &lengthTransfered, &overlapped);
		}
		else
		{
			// Device to host
			bStatus = WinUsb_ControlTransfer(interfaceHandle, setupPacket, reinterpret_cast<PUCHAR>(buffer.data()), buffer.Capacity(), &lengthTransfered, &overlapped);
		}
	}
	if (!bStatus)
	{
		auto err = GetLastError();
		if (err != ERROR_IO_PENDING)
		{
			bStatus = CloseHandle(overlapped.hEvent);
			throw hresult(HRESULT_FROM_WIN32(err));
		}
	}
	co_await resume_on_signal(overlapped.hEvent);
	bStatus = CloseHandle(overlapped.hEvent);
	if (!bStatus)
	{
		auto err = GetLastError();
		throw hresult(HRESULT_FROM_WIN32(err));
	}
	if (buffer != nullptr && (setupPacket.RequestType | 0x80) != 0)
	{
		DWORD numOfBytesTransferred = 0;
		bStatus = WinUsb_GetOverlappedResult(interfaceHandle, &overlapped, &numOfBytesTransferred, FALSE);
		if (!bStatus)
		{
			auto err = GetLastError();
			throw hresult(HRESULT_FROM_WIN32(err));
		}
		buffer.Length(numOfBytesTransferred);
	}
}
