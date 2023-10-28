// HidProxyTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>


#include <Windows.h>
#include <cfgmgr32.h>
#include <vector>
#include <winrt/base.h>
#include "../HidProxy/hidp.h"

struct HidpCreateVHidRequest
{
    HidpQueueWriteRequestType RequestType = HidpQueueWriteRequestType::CreateVHid;
    USHORT Size = sizeof(ReportDescriptor);
    UCHAR ReportDescriptor[27] =
    {
        0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
        0x09, 0x06,                    // USAGE (Keyboard)
        0xa1, 0x01,                    // COLLECTION (Application)
        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
        0x09, 0x04,                    //   USAGE (Keyboard a and A)
        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
        0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
        0x75, 0x01,                    //   REPORT_SIZE (1)
        0x95, 0x01,                    //   REPORT_COUNT (1)
        0x81, 0x02,                    //   INPUT (Data,Var,Abs)
        0x75, 0x01,                    //   REPORT_SIZE (1)
        0x95, 0x07,                    //   REPORT_COUNT (7)
        0x81, 0x01,                    //   INPUT (Cnst,Ary,Abs)
        0xc0                           // END_COLLECTION
    };
};

struct HidpSubmitReportRequest
{
    HidpQueueWriteRequestType RequestType = HidpQueueWriteRequestType::SendReport;
    USHORT Size = sizeof(Report);
    struct
    {
        UCHAR ReportId = 0;
        UCHAR ReportData[1] = { 1 };
    } Report;
};



int main()
{
    std::cout << "Hello World!\n";

    ULONG deviceInterfaceListLength = 0;
    CONFIGRET cr = CR_SUCCESS;
    cr = CM_Get_Device_Interface_List_Size(&deviceInterfaceListLength, (LPGUID)(&GUID_DEVINTERFACE_HIDP), nullptr, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

    PWSTR deviceInterfaceList = (PWSTR)malloc(deviceInterfaceListLength * sizeof(WCHAR));
    ZeroMemory(deviceInterfaceList, deviceInterfaceListLength * sizeof(WCHAR));
    cr = CM_Get_Device_Interface_List((LPGUID)(&GUID_DEVINTERFACE_HIDP), nullptr, deviceInterfaceList, deviceInterfaceListLength, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

    // Only have 1 interface
    assert(*(deviceInterfaceList + wcslen(deviceInterfaceList) + 1) == UNICODE_NULL);

    winrt::hstring deviceInterface{ deviceInterfaceList };
    free(deviceInterfaceList);
    deviceInterfaceList = nullptr;
    winrt::handle device{ CreateFile(deviceInterface.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL) };

    do
    {
        HidpCreateVHidRequest createVHidRequest{};
        ULONG_PTR ioWriteInfo = 0;
        bool writeResult = false;
        writeResult = WriteFile(device.get(), &createVHidRequest, sizeof(createVHidRequest), reinterpret_cast<LPDWORD>(&ioWriteInfo), nullptr);
        if (!writeResult)
        {
            break;
        }

        HidpSubmitReportRequest submitReportRequest{};
        writeResult = WriteFile(device.get(), &submitReportRequest, sizeof(submitReportRequest), reinterpret_cast<LPDWORD>(&ioWriteInfo), nullptr);
        if (!writeResult)
        {
            break;
        }

    } while (false);
    
    Sleep(5000);

    assert(CloseHandle(device.detach()));
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
