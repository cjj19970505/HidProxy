#pragma once
#include "pch.h"
#include <codecvt>
#include <winusb.h>

class Utils
{
public:
    // https://stackoverflow.com/a/18374698/11879605

    static winrt::Windows::Storage::Streams::IBuffer WideCharToUtf8Buffer(const winrt::hstring& wstr);

    static std::optional<std::wstring> GetDevicePathFromGuid(LPGUID guid);
    static std::vector<std::wstring> GetDevicePathFromVidPid(USHORT vid, USHORT pid);
    static winrt::Windows::Foundation::IAsyncAction WinUsbControlTransferAsync(WINUSB_INTERFACE_HANDLE InterfaceHandle, WINUSB_SETUP_PACKET SetupPacket, winrt::Windows::Storage::Streams::IBuffer buffer);
};

