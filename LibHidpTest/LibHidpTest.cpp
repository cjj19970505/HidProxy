// LibHidpTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <iostream>
#include "../LibHidp/LibHidp.h"

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

UCHAR MouseReportDescriptor[] =
{
    0x05, 0x01,    // UsagePage(Generic Desktop[0x0001])
    0x09, 0x02,    // UsageId(Mouse[0x0002])
    0xA1, 0x01,    // Collection(Application)
    0x85, 0x02,    //     ReportId(1)
    0x09, 0x01,    //     UsageId(Pointer[0x0001])
    0xA1, 0x00,    //     Collection(Physical)
    0x09, 0x30,    //         UsageId(X[0x0030])
    0x09, 0x31,    //         UsageId(Y[0x0031])
    0x15, 0x80,    //         LogicalMinimum(-128)
    0x25, 0x7F,    //         LogicalMaximum(127)
    0x95, 0x02,    //         ReportCount(2)
    0x75, 0x08,    //         ReportSize(8)
    0x81, 0x06,    //         Input(Data, Variable, Relative, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0x05, 0x09,    //         UsagePage(Button[0x0009])
    0x19, 0x01,    //         UsageIdMin(Button 1[0x0001])
    0x29, 0x03,    //         UsageIdMax(Button 3[0x0003])
    0x15, 0x00,    //         LogicalMinimum(0)
    0x25, 0x01,    //         LogicalMaximum(1)
    0x95, 0x03,    //         ReportCount(3)
    0x75, 0x01,    //         ReportSize(1)
    0x81, 0x02,    //         Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0xC0,          //     EndCollection()
    0x95, 0x01,    //     ReportCount(1)
    0x75, 0x05,    //     ReportSize(5)
    0x81, 0x03,    //     Input(Constant, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
    0xC0,          // EndCollection()
};

//UCHAR ff[] =
//{
//    //
//    // Dummy mouse collection starts here
//    //
//    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)     0
//    0x09, 0x02,                         // USAGE (Mouse)                    2
//    0xa1, 0x01,                         // COLLECTION (Application)         4
//    //0x85, 0x01,                         //   REPORT_ID (Mouse)              6
//    0x09, 0x01,                         //   USAGE (Pointer)                8
//    0xa1, 0x00,                         //   COLLECTION (Physical)          10
//    0x05, 0x09,                         //     USAGE_PAGE (Button)          12
//    0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)     14
//    0x29, 0x02,                         //     USAGE_MAXIMUM (Button 2)     16
//    0x15, 0x00,                         //     LOGICAL_MINIMUM (0)          18
//    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          20
//    0x75, 0x01,                         //     REPORT_SIZE (1)              22
//    0x95, 0x02,                         //     REPORT_COUNT (2)             24
//    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         26
//    0x95, 0x06,                         //     REPORT_COUNT (6)             28
//    0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         30
//    0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 32
//    0x09, 0x30,                         //     USAGE (X)                    34
//    0x09, 0x31,                         //     USAGE (Y)                    36
//    0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)       38
//    0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)        40
//    0x75, 0x08,                         //     REPORT_SIZE (8)              42
//    0x95, 0x02,                         //     REPORT_COUNT (2)             44
//    0x81, 0x06,                         //     INPUT (Data,Var,Rel)         46
//    0xc0,                               //   END_COLLECTION                 48
//    0xc0                                // END_COLLECTION                   49/50
//};

int main()
{
    std::cout << "Hello World!\n";
    HRESULT hr;
    HIDPHANDLE hidp = nullptr;
    do
    {
        hr = HidpCreate(sizeof(MouseReportDescriptor), MouseReportDescriptor, &hidp);

        while (true)
        {
            BYTE data = 1;

            // According to HID speicification 5.8
            // The first byte should be Report ID if defined in descriptor
            uint8_t payload[4] = { 2, 10, 10, 0 };
            hr = HidpSubmitReport(hidp, 2, 4, payload);
            Sleep(500);
        }
        
        bool b = HidpClose(hidp);
    } while (false);
    
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
