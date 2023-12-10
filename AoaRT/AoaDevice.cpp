#include "pch.h"
#include "AoaDevice.h"
#include "AoaDevice.g.cpp"
#include "AoaInputStream.h"
#include <cfgmgr32.h>
#include <SetupAPI.h>
#include "Utils.h"
#include <chrono>
using namespace std::chrono;

namespace winrt::AoaRT::implementation
{
	winrt::Windows::Foundation::IAsyncOperation<winrt::AoaRT::AoaDevice> AoaDevice::CreateAsync(winrt::AoaRT::AoaCredential aoaCredential)
	{
		co_await resume_background();
		auto aoaDevicePaths = Utils::GetDevicePathFromVidPid(0x18d1, 0x2d01);
		if (aoaDevicePaths.size() == 0)
		{
			auto adbDevicePaths = Utils::GetDevicePathFromVidPid(0x18d1, 0x4ee2);
			if (adbDevicePaths.size() == 0)
			{
				throw hresult(HRESULT_FROM_NT(ERROR_DEVICE_NOT_CONNECTED));
			}
			auto adbDeviceHandle = CreateFileW(
				reinterpret_cast<LPCWSTR>(adbDevicePaths[0].c_str()),
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_WRITE | FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
				NULL
			);
			if (adbDeviceHandle == INVALID_HANDLE_VALUE)
			{
				auto err = GetLastError();
				throw hresult(HRESULT_FROM_NT(err));
			}
			WINUSB_INTERFACE_HANDLE hWinUsbInterface = INVALID_HANDLE_VALUE;
			if (!WinUsb_Initialize(adbDeviceHandle, &hWinUsbInterface))
			{
				auto err = GetLastError();

				if (!CloseHandle(adbDeviceHandle))
				{
					auto err = GetLastError();
					throw hresult(HRESULT_FROM_NT(err));
				}

				throw hresult(HRESULT_FROM_NT(err));
			}
			co_await StartAccessoryModeAsync(hWinUsbInterface, aoaCredential);
			if (!WinUsb_Free(hWinUsbInterface))
			{
				auto err = GetLastError();

				if (!CloseHandle(adbDeviceHandle))
				{
					auto err = GetLastError();
					throw hresult(HRESULT_FROM_NT(err));
				}

				throw hresult(HRESULT_FROM_NT(err));

			}
			if (!CloseHandle(adbDeviceHandle))
			{
				auto err = GetLastError();
				throw hresult(HRESULT_FROM_NT(err));
			}
			co_await 3s;
			aoaDevicePaths = Utils::GetDevicePathFromVidPid(0x18d1, 0x2d01);
		}

		HANDLE aoaDeviceHandle = CreateFileW(
			reinterpret_cast<LPCWSTR>(aoaDevicePaths[0].c_str()),
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL
		);

		if (aoaDeviceHandle == INVALID_HANDLE_VALUE)
		{
			auto err = GetLastError();
			throw hresult(HRESULT_FROM_NT(err));
		}

		WINUSB_INTERFACE_HANDLE aoaInterfaceHandle = INVALID_HANDLE_VALUE;
		if (!WinUsb_Initialize(aoaDeviceHandle, &aoaInterfaceHandle))
		{
			auto err = GetLastError();
			if (!CloseHandle(aoaDeviceHandle))
			{
				auto err = GetLastError();
				throw hresult(HRESULT_FROM_NT(err));
			}
			throw hresult(HRESULT_FROM_NT(err));
		}

		{
			auto getAoaAlterSetting = [](WINUSB_INTERFACE_HANDLE oldInterface, PWINUSB_INTERFACE_HANDLE newInterface)->DWORD
				{
					BOOL bStatus = TRUE;
					UCHAR settingNum = 0;
					bStatus = WinUsb_GetCurrentAlternateSetting(oldInterface, &settingNum);
					if (!bStatus)
					{
						auto err = GetLastError();
						return err;
					}
					USB_INTERFACE_DESCRIPTOR desc{};
					bStatus = WinUsb_QueryInterfaceSettings(oldInterface, settingNum, &desc);
					if (!bStatus)
					{
						auto err = GetLastError();
						return err;
					}

					// Not tested yet
					// 0x2D01 has two interfaces with two bulk endpoints each for input and output communication. The first interface handles standard communication and the second interface handles ADB communication.
					if (desc.bInterfaceNumber != 0)
					{
						bStatus = WinUsb_GetAssociatedInterface(oldInterface, 0, newInterface);
						if (!bStatus)
						{
							auto err = GetLastError();
							return err;
						}
					}
					else
					{
						*newInterface = oldInterface;
					}

					return ERROR_SUCCESS;
				};

			WINUSB_INTERFACE_HANDLE newAoaInterfaceHandle = INVALID_HANDLE_VALUE;
			auto ntStatus = getAoaAlterSetting(aoaInterfaceHandle, &newAoaInterfaceHandle);
			if (ntStatus != ERROR_SUCCESS)
			{
				if (!WinUsb_Free(aoaInterfaceHandle))
				{
					auto err = GetLastError();
					throw hresult(HRESULT_FROM_NT(err));
				}
				if (!CloseHandle(aoaDeviceHandle))
				{
					auto err = GetLastError();
					throw hresult(HRESULT_FROM_NT(err));
				}
				throw hresult(HRESULT_FROM_NT(ntStatus));
			}
			if (newAoaInterfaceHandle != aoaInterfaceHandle)
			{
				if (!WinUsb_Free(aoaInterfaceHandle))
				{
					auto err = GetLastError();
					throw hresult(HRESULT_FROM_NT(err));
				}
				aoaInterfaceHandle = newAoaInterfaceHandle;
			}
		}

		{
			BOOL bStatus = TRUE;
			WINUSB_SETUP_PACKET packet{};
			packet.RequestType = 0x00;
			packet.Request = USB_REQUEST_SET_CONFIGURATION;
			packet.Value = 1;
			packet.Index = 0;
			packet.Length = 0;
			co_await Utils::WinUsbControlTransferAsync(aoaInterfaceHandle, packet, nullptr);
		}

		USB_ENDPOINT_DESCRIPTOR inEndPointDescriptor{};
		USB_ENDPOINT_DESCRIPTOR outEndPointDescriptor{};
		{
			BOOL bStatus = TRUE;
			ULONG lengthTransfered = 0;
			LONG bufferSize = sizeof(USB_CONFIGURATION_DESCRIPTOR) + sizeof(USB_INTERFACE_DESCRIPTOR) + 2 * sizeof(USB_ENDPOINT_DESCRIPTOR);
			std::unique_ptr<uint8_t[]> buffer{ new uint8_t[bufferSize] };
			ZeroMemory(buffer.get(), bufferSize);
			bStatus = WinUsb_GetDescriptor(aoaInterfaceHandle, USB_CONFIGURATION_DESCRIPTOR_TYPE, 0, 0, reinterpret_cast<PUCHAR>(buffer.get()), bufferSize, &lengthTransfered);
			if (!bStatus)
			{
				auto err = GetLastError();
				throw hresult(HRESULT_FROM_NT(err));
			}
			std::array<USB_ENDPOINT_DESCRIPTOR, 2> endPointDescriptors{};
			CopyMemory(endPointDescriptors.data(), buffer.get() + sizeof(USB_CONFIGURATION_DESCRIPTOR) + sizeof(USB_INTERFACE_DESCRIPTOR), 2 * sizeof(USB_ENDPOINT_DESCRIPTOR));
			if ((endPointDescriptors[0].bEndpointAddress & 0x80) == 0)
			{
				outEndPointDescriptor = endPointDescriptors[0];
				inEndPointDescriptor = endPointDescriptors[1];
			}
			else
			{
				outEndPointDescriptor = endPointDescriptors[1];
				inEndPointDescriptor = endPointDescriptors[0];
			}
		}

	}
	winrt::Windows::Storage::Streams::IInputStream AoaDevice::InputStream()
	{
		com_ptr<implementation::AoaDevice> abiThis{ nullptr };
		check_hresult(this->QueryInterface(guid_of<implementation::AoaDevice>(), abiThis.put_void()));
		return make<implementation::AoaInputStream>(abiThis);
	}
	winrt::Windows::Storage::Streams::IOutputStream AoaDevice::OutputStream()
	{
		throw hresult_not_implemented();
	}
	void AoaDevice::Close()
	{
		if (!WinUsb_Free(_AoaInterfaceHandle))
		{
			auto err = GetLastError();
			throw hresult(HRESULT_FROM_NT(err));
		}
		if (!CloseHandle(_DeviceHandle))
		{
			auto err = GetLastError();
			throw hresult(HRESULT_FROM_NT(err));
		}
	}
	winrt::Windows::Foundation::IAsyncAction AoaDevice::StartAccessoryModeAsync(WINUSB_INTERFACE_HANDLE interfaceHandle, const winrt::AoaRT::AoaCredential& aoaCredential)
	{
		BOOL bStatus = TRUE;
		uint16_t protocolVersionNumber = 0;
		{
			WINUSB_SETUP_PACKET p1{};
			p1.RequestType = 0xc0;
			p1.Request = 51;
			p1.Value = 0;
			p1.Index = 0;
			p1.Length = sizeof(protocolVersionNumber);
			ULONG lengthTransferred = 0;

			bStatus = WinUsb_ControlTransfer(interfaceHandle, p1, reinterpret_cast<PUCHAR>(&protocolVersionNumber), 2, &lengthTransferred, NULL);
			if (!bStatus)
			{
				auto err = GetLastError();
				check_nt(err);
			}

			if (protocolVersionNumber == 0)
			{
				check_nt(ERROR_NOT_SUPPORTED);
			}
		}
		{
			auto manufacturerBuffer = Utils::WideCharToUtf8Buffer(aoaCredential.manufacturer);
			auto modelNameBuffer = Utils::WideCharToUtf8Buffer(aoaCredential.modelName);
			auto descriptionBuffer = Utils::WideCharToUtf8Buffer(aoaCredential.description);
			auto versionBuffer = Utils::WideCharToUtf8Buffer(aoaCredential.version);
			auto uriBuffer = Utils::WideCharToUtf8Buffer(aoaCredential.Uri);
			auto serialNumberBuffer = Utils::WideCharToUtf8Buffer(aoaCredential.serialNumber);

			WINUSB_SETUP_PACKET p2{};
			ULONG lengthTransferred = 0;
			p2.RequestType = 0x40;
			p2.Request = 52;
			p2.Value = 0;

			p2.Index = 0;
			p2.Length = manufacturerBuffer.Length();
			co_await Utils::WinUsbControlTransferAsync(interfaceHandle, p2, manufacturerBuffer);

			p2.Index = 1;
			p2.Length = modelNameBuffer.Length();
			co_await Utils::WinUsbControlTransferAsync(interfaceHandle, p2, modelNameBuffer);

			p2.Index = 2;
			p2.Length = descriptionBuffer.Length();
			co_await Utils::WinUsbControlTransferAsync(interfaceHandle, p2, descriptionBuffer);

			p2.Index = 3;
			p2.Length = versionBuffer.Length();
			co_await Utils::WinUsbControlTransferAsync(interfaceHandle, p2, versionBuffer);

			p2.Index = 4;
			p2.Length = uriBuffer.Length();
			co_await Utils::WinUsbControlTransferAsync(interfaceHandle, p2, uriBuffer);

			p2.Index = 5;
			p2.Length = serialNumberBuffer.Length();
			co_await Utils::WinUsbControlTransferAsync(interfaceHandle, p2, serialNumberBuffer);
		}
		{
			WINUSB_SETUP_PACKET p3{};
			p3.RequestType = 0x40;
			p3.Request = 53;
			p3.Value = 0;
			p3.Index = 0;
			p3.Length = 0;
			co_await Utils::WinUsbControlTransferAsync(interfaceHandle, p3, nullptr);
		}
	}
}
