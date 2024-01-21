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

UCHAR PenReportDescriptor[] =
{
    // Integrated Windows Pen TLC
    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)          
    0x09, 0x02,                         // USAGE (Pen)                      
    0xa1, 0x01,                         // COLLECTION (Application)         
    0x85, 0x01,                         //   REPORT_ID (Pen)                
    0x09, 0x20,                         //   USAGE (Stylus)                 
    0xa1, 0x00,                         //   COLLECTION (Physical)          
    0x09, 0x42,                         //     USAGE (Tip Switch)           
    0x09, 0x44,                         //     USAGE (Barrel Switch)        
    0x09, 0x3c,                         //     USAGE (Invert)               
    0x09, 0x45,                         //     USAGE (Eraser Switch)        
    0x15, 0x00,                         //     LOGICAL_MINIMUM (0)          
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          
    0x75, 0x01,                         //     REPORT_SIZE (1)              
    0x95, 0x04,                         //     REPORT_COUNT (4)             
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x95, 0x01,                         //     REPORT_COUNT (1)           
    0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)      
    0x09, 0x32,                         //     USAGE (In Range)             
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x95, 0x02,                         //     REPORT_COUNT (2)             
    0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         
    0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 
    0x09, 0x30,                         //     USAGE (X)                    
    0x75, 0x10,                         //     REPORT_SIZE (16)             
    0x95, 0x01,                         //     REPORT_COUNT (1)             
    0xa4,                               //     PUSH                         
    0x55, 0x0d,                         //     UNIT_EXPONENT (-3)           
    0x65, 0x13,                         //     UNIT (Inch,EngLinear)        
    0x35, 0x00,                         //     PHYSICAL_MINIMUM (0)         
    0x46, 0x3a, 0x20,                   //     PHYSICAL_MAXIMUM (8250)      
    0x26, 0xf8, 0x52,                   //     LOGICAL_MAXIMUM (21240)      
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x09, 0x31,                         //     USAGE (Y)                    
    0x46, 0x2c, 0x18,                   //     PHYSICAL_MAXIMUM (6188)      
    0x26, 0x6c, 0x3e,                   //     LOGICAL_MAXIMUM (15980)      
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0xb4,                               //     POP                          
    0x05, 0x0d,                         //     USAGE_PAGE (Digitizers)      
    0x09, 0x30,                         //     USAGE (Tip Pressure)         
    0x26, 0xff, 0x00,                   //     LOGICAL_MAXIMUM (255)        
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x75, 0x08,                         //     REPORT_SIZE (8)              
    0x09, 0x3d,                         //     USAGE (X Tilt)               
    0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)       
    0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)        
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x09, 0x3e,                         //     USAGE (Y Tilt)               
    0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)       
    0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)        
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0xc0,                               //   END_COLLECTION                 
    0xc0                                // END_COLLECTION
};
struct PenReportByte1
{
    uint8_t TipSwitch : 1;
    uint8_t BarrelSwitch : 1;
    uint8_t Invert : 1;
    uint8_t EraserSwitch : 1;
    uint8_t Reserved1 : 1;
    uint8_t InRange : 1;
    uint8_t Reserved2 : 2;
};

uint8_t PenReportDataBuffer[1 + 1 + 2 + 2 + 2 + 1 + 1];


void GetPenReportDataBuffer(uint8_t* buffer, uint8_t reportId, PenReportByte1 reportByte1, uint16_t x, uint16_t y, uint16_t tipPressure, uint8_t xTilt, uint8_t yTilt)
{
    buffer[0] = reportId;
    *reinterpret_cast<PenReportByte1*>(&buffer[1]) = reportByte1;
    *reinterpret_cast<uint16_t*>(&buffer[2]) = x;
    *reinterpret_cast<uint16_t*>(&buffer[4]) = y;
    *reinterpret_cast<uint16_t*>(&buffer[6]) = tipPressure;
    *reinterpret_cast<uint16_t*>(&buffer[8]) = xTilt;
    *reinterpret_cast<uint16_t*>(&buffer[9]) = yTilt;
}

constexpr uint8_t REPORTID_TOUCHPAD = 0x01;
constexpr uint8_t REPORTID_MAX_COUNT = 0x02;
constexpr uint8_t REPORTID_PTPHQA = 0x03;
constexpr uint8_t REPORTID_FEATURE = 0x04;
constexpr uint8_t REPORTID_FUNCTION_SWITCH = 0x05;
constexpr uint8_t REPORTID_MOUSE = 0x06;
constexpr uint8_t REPORTID_PEN = 0x07;

UCHAR TouchpadDescriptor[]{
    //TOUCH PAD input TLC
    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)          
    0x09, 0x05,                         // USAGE (Touch Pad)             
    0xa1, 0x01,                         // COLLECTION (Application)         
    0x85, REPORTID_TOUCHPAD,            //   REPORT_ID (Touch pad)              
    0x09, 0x22,                         //   USAGE (Finger)                 
    0xa1, 0x02,                         //   COLLECTION (Logical)  
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
    0x25, 0x01,                         //       LOGICAL_MAXIMUM (1)
    0x09, 0x47,                         //       USAGE (Confidence) 
    0x09, 0x42,                         //       USAGE (Tip switch)
    0x95, 0x02,                         //       REPORT_COUNT (2)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x75, 0x02,                         //       REPORT_SIZE (2)
    0x25, 0x02,                         //       LOGICAL_MAXIMUM (2)
    0x09, 0x51,                         //       USAGE (Contact Identifier)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x95, 0x04,                         //       REPORT_COUNT (4)             
    0x81, 0x03,                         //       INPUT (Cnst,Var,Abs)      // byte1 (1 byte)
    0x05, 0x01,                         //       USAGE_PAGE (Generic Desk..
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x0f,                   //       LOGICAL_MAXIMUM (4095)         
    0x75, 0x10,                         //       REPORT_SIZE (16)             
    0x55, 0x0e,                         //       UNIT_EXPONENT (-2)           
    0x65, 0x13,                         //       UNIT(Inch,EngLinear)                  
    0x09, 0x30,                         //       USAGE (X)                    
    0x35, 0x00,                         //       PHYSICAL_MINIMUM (0)         
    0x46, 0x90, 0x01,                   //       PHYSICAL_MAXIMUM (400)
    0x95, 0x01,                         //       REPORT_COUNT (1)         
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)      // x (2 byte)
    0x46, 0x13, 0x01,                   //       PHYSICAL_MAXIMUM (275)
    0x09, 0x31,                         //       USAGE (Y)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)      // y (2 byte)
    0xc0,                               //    END_COLLECTION
    0x55, 0x0C,                         //    UNIT_EXPONENT (-4)           
    0x66, 0x01, 0x10,                   //    UNIT (Seconds)        
    0x47, 0xff, 0xff, 0x00, 0x00,      //     PHYSICAL_MAXIMUM (65535)
    0x27, 0xff, 0xff, 0x00, 0x00,         //  LOGICAL_MAXIMUM (65535) 
    0x75, 0x10,                           //  REPORT_SIZE (16)             
    0x95, 0x01,                           //  REPORT_COUNT (1) 
    0x05, 0x0d,                         //    USAGE_PAGE (Digitizers)
    0x09, 0x56,                         //    USAGE (Scan Time)    
    0x81, 0x02,                           //  INPUT (Data,Var,Abs)         // scan time (2 byte)
    0x09, 0x54,                         //    USAGE (Contact count)
    0x25, 0x7f,                           //  LOGICAL_MAXIMUM (127) 
    0x95, 0x01,                         //    REPORT_COUNT (1)
    0x75, 0x08,                         //    REPORT_SIZE (8)    
    0x81, 0x02,                         //    INPUT (Data,Var,Abs)         // contact count (1 byte)
    0x05, 0x09,                         //    USAGE_PAGE (Button)         
    0x09, 0x01,                         //    USAGE_(Button 1)     
    0x25, 0x01,                         //    LOGICAL_MAXIMUM (1)          
    0x75, 0x01,                         //    REPORT_SIZE (1)              
    0x95, 0x01,                         //    REPORT_COUNT (1)             
    0x81, 0x02,                         //    INPUT (Data,Var,Abs)
    0x95, 0x07,                          //   REPORT_COUNT (7)                 
    0x81, 0x03,                         //    INPUT (Cnst,Var,Abs)         // btn1 (1 byte), end of report REPORTID_TOUCHPAD
    0x05, 0x0d,                         //    USAGE_PAGE (Digitizer)
    0x85, REPORTID_MAX_COUNT,            //   REPORT_ID (Feature)              
    0x09, 0x55,                         //    USAGE (Contact Count Maximum)
    0x09, 0x59,                         //    USAGE (Pad TYpe)
    0x75, 0x04,                         //    REPORT_SIZE (4) 
    0x95, 0x02,                         //    REPORT_COUNT (2)
    0x25, 0x0f,                         //    LOGICAL_MAXIMUM (15)
    0xb1, 0x02,                         //    FEATURE (Data,Var,Abs)
    0x06, 0x00, 0xff,                   //    USAGE_PAGE (Vendor Defined)
    0x85, REPORTID_PTPHQA,               //    REPORT_ID (PTPHQA)  
    0x09, 0xC5,                         //    USAGE (Vendor Usage 0xC5)    
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)          
    0x26, 0xff, 0x00,                   //    LOGICAL_MAXIMUM (0xff) 
    0x75, 0x08,                         //    REPORT_SIZE (8)             
    0x96, 0x00, 0x01,                   //    REPORT_COUNT (0x100 (256))             
    0xb1, 0x02,                         //    FEATURE (Data,Var,Abs)
    0xc0,                               // END_COLLECTION
    //CONFIG TLC
    0x05, 0x0d,                         //    USAGE_PAGE (Digitizer)
    0x09, 0x0E,                         //    USAGE (Configuration)
    0xa1, 0x01,                         //   COLLECTION (Application)
    0x85, REPORTID_FEATURE,             //   REPORT_ID (Feature)              
    0x09, 0x22,                         //   USAGE (Finger)              
    0xa1, 0x02,                         //   COLLECTION (logical)     
    0x09, 0x52,                         //    USAGE (Input Mode)         
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)      
    0x25, 0x0a,                         //    LOGICAL_MAXIMUM (10)
    0x75, 0x08,                         //    REPORT_SIZE (8)         
    0x95, 0x01,                         //    REPORT_COUNT (1)         
    0xb1, 0x02,                         //    FEATURE (Data,Var,Abs    
    0xc0,                               //   END_COLLECTION
    0x09, 0x22,                         //   USAGE (Finger)              
    0xa1, 0x00,                         //   COLLECTION (physical)     
    0x85, REPORTID_FUNCTION_SWITCH,     //     REPORT_ID (Feature)              
    0x09, 0x57,                         //     USAGE(Surface switch)
    0x09, 0x58,                         //     USAGE(Button switch)
    0x75, 0x01,                         //     REPORT_SIZE (1)
    0x95, 0x02,                         //     REPORT_COUNT (2)
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)
    0xb1, 0x02,                         //     FEATURE (Data,Var,Abs)
    0x95, 0x06,                         //     REPORT_COUNT (6)             
    0xb1, 0x03,                         //     FEATURE (Cnst,Var,Abs)
    0xc0,                               //   END_COLLECTION
    0xc0,                               // END_COLLECTION
    //MOUSE TLC
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)     
    0x09, 0x02,                         // USAGE (Mouse)                    
    0xa1, 0x01,                         // COLLECTION (Application)        
    0x85, REPORTID_MOUSE,               //   REPORT_ID (Mouse)              
    0x09, 0x01,                         //   USAGE (Pointer)                
    0xa1, 0x00,                         //   COLLECTION (Physical)          
    0x05, 0x09,                         //     USAGE_PAGE (Button)          
    0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)     
    0x29, 0x02,                         //     USAGE_MAXIMUM (Button 2)     
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          
    0x75, 0x01,                         //     REPORT_SIZE (1)              
    0x95, 0x02,                         //     REPORT_COUNT (2)             
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x95, 0x06,                         //     REPORT_COUNT (6)             
    0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         
    0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 
    0x09, 0x30,                         //     USAGE (X)                    
    0x09, 0x31,                         //     USAGE (Y)                    
    0x75, 0x10,                         //     REPORT_SIZE (16)             
    0x95, 0x02,                         //     REPORT_COUNT (2)             
    0x25, 0x0a,                          //    LOGICAL_MAXIMUM (10)      
    0x81, 0x06,                         //     INPUT (Data,Var,Rel)         
    0xc0,                               //   END_COLLECTION                 
    0xc0,                                //END_COLLECTION
};


UCHAR PenTouchpadDescriptor[]{
    //TOUCH PAD input TLC
    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)          
    0x09, 0x05,                         // USAGE (Touch Pad)             
    0xa1, 0x01,                         // COLLECTION (Application)         
    0x85, REPORTID_TOUCHPAD,            //   REPORT_ID (Touch pad)              
    0x09, 0x22,                         //   USAGE (Finger)                 
    0xa1, 0x02,                         //   COLLECTION (Logical)  
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
    0x25, 0x01,                         //       LOGICAL_MAXIMUM (1)
    0x09, 0x47,                         //       USAGE (Confidence) 
    0x09, 0x42,                         //       USAGE (Tip switch)
    0x95, 0x02,                         //       REPORT_COUNT (2)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x75, 0x02,                         //       REPORT_SIZE (2)
    0x25, 0x02,                         //       LOGICAL_MAXIMUM (2)
    0x09, 0x51,                         //       USAGE (Contact Identifier)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x95, 0x04,                         //       REPORT_COUNT (4)             
    0x81, 0x03,                         //       INPUT (Cnst,Var,Abs)      // byte1 (1 byte)
    0x05, 0x01,                         //       USAGE_PAGE (Generic Desk..
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x0f,                   //       LOGICAL_MAXIMUM (4095)         
    0x75, 0x10,                         //       REPORT_SIZE (16)             
    0x55, 0x0e,                         //       UNIT_EXPONENT (-2)           
    0x65, 0x13,                         //       UNIT(Inch,EngLinear)                  
    0x09, 0x30,                         //       USAGE (X)                    
    0x35, 0x00,                         //       PHYSICAL_MINIMUM (0)         
    0x46, 0x90, 0x01,                   //       PHYSICAL_MAXIMUM (400)
    0x95, 0x01,                         //       REPORT_COUNT (1)         
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)      // x (2 byte)
    0x46, 0x13, 0x01,                   //       PHYSICAL_MAXIMUM (275)
    0x09, 0x31,                         //       USAGE (Y)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)      // y (2 byte)
    0xc0,                               //    END_COLLECTION
    0x55, 0x0C,                         //    UNIT_EXPONENT (-4)           
    0x66, 0x01, 0x10,                   //    UNIT (Seconds)        
    0x47, 0xff, 0xff, 0x00, 0x00,      //     PHYSICAL_MAXIMUM (65535)
    0x27, 0xff, 0xff, 0x00, 0x00,         //  LOGICAL_MAXIMUM (65535) 
    0x75, 0x10,                           //  REPORT_SIZE (16)             
    0x95, 0x01,                           //  REPORT_COUNT (1) 
    0x05, 0x0d,                         //    USAGE_PAGE (Digitizers)
    0x09, 0x56,                         //    USAGE (Scan Time)    
    0x81, 0x02,                           //  INPUT (Data,Var,Abs)         // scan time (2 byte)
    0x09, 0x54,                         //    USAGE (Contact count)
    0x25, 0x7f,                           //  LOGICAL_MAXIMUM (127) 
    0x95, 0x01,                         //    REPORT_COUNT (1)
    0x75, 0x08,                         //    REPORT_SIZE (8)    
    0x81, 0x02,                         //    INPUT (Data,Var,Abs)         // contact count (1 byte)
    0x05, 0x09,                         //    USAGE_PAGE (Button)         
    0x09, 0x01,                         //    USAGE_(Button 1)     
    0x25, 0x01,                         //    LOGICAL_MAXIMUM (1)          
    0x75, 0x01,                         //    REPORT_SIZE (1)              
    0x95, 0x01,                         //    REPORT_COUNT (1)             
    0x81, 0x02,                         //    INPUT (Data,Var,Abs)
    0x95, 0x07,                          //   REPORT_COUNT (7)                 
    0x81, 0x03,                         //    INPUT (Cnst,Var,Abs)         // btn1 (1 byte), end of report REPORTID_TOUCHPAD
    0x05, 0x0d,                         //    USAGE_PAGE (Digitizer)
    0x85, REPORTID_MAX_COUNT,            //   REPORT_ID (Feature)              
    0x09, 0x55,                         //    USAGE (Contact Count Maximum)
    0x09, 0x59,                         //    USAGE (Pad TYpe)
    0x75, 0x04,                         //    REPORT_SIZE (4) 
    0x95, 0x02,                         //    REPORT_COUNT (2)
    0x25, 0x0f,                         //    LOGICAL_MAXIMUM (15)
    0xb1, 0x02,                         //    FEATURE (Data,Var,Abs)
    0x06, 0x00, 0xff,                   //    USAGE_PAGE (Vendor Defined)
    0x85, REPORTID_PTPHQA,               //    REPORT_ID (PTPHQA)  
    0x09, 0xC5,                         //    USAGE (Vendor Usage 0xC5)    
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)          
    0x26, 0xff, 0x00,                   //    LOGICAL_MAXIMUM (0xff) 
    0x75, 0x08,                         //    REPORT_SIZE (8)             
    0x96, 0x00, 0x01,                   //    REPORT_COUNT (0x100 (256))             
    0xb1, 0x02,                         //    FEATURE (Data,Var,Abs)
    0xc0,                               // END_COLLECTION
    //CONFIG TLC
    0x05, 0x0d,                         //    USAGE_PAGE (Digitizer)
    0x09, 0x0E,                         //    USAGE (Configuration)
    0xa1, 0x01,                         //   COLLECTION (Application)
    0x85, REPORTID_FEATURE,             //   REPORT_ID (Feature)              
    0x09, 0x22,                         //   USAGE (Finger)              
    0xa1, 0x02,                         //   COLLECTION (logical)     
    0x09, 0x52,                         //    USAGE (Input Mode)         
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)      
    0x25, 0x0a,                         //    LOGICAL_MAXIMUM (10)
    0x75, 0x08,                         //    REPORT_SIZE (8)         
    0x95, 0x01,                         //    REPORT_COUNT (1)         
    0xb1, 0x02,                         //    FEATURE (Data,Var,Abs    
    0xc0,                               //   END_COLLECTION
    0x09, 0x22,                         //   USAGE (Finger)              
    0xa1, 0x00,                         //   COLLECTION (physical)     
    0x85, REPORTID_FUNCTION_SWITCH,     //     REPORT_ID (Feature)              
    0x09, 0x57,                         //     USAGE(Surface switch)
    0x09, 0x58,                         //     USAGE(Button switch)
    0x75, 0x01,                         //     REPORT_SIZE (1)
    0x95, 0x02,                         //     REPORT_COUNT (2)
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)
    0xb1, 0x02,                         //     FEATURE (Data,Var,Abs)
    0x95, 0x06,                         //     REPORT_COUNT (6)             
    0xb1, 0x03,                         //     FEATURE (Cnst,Var,Abs)
    0xc0,                               //   END_COLLECTION
    0xc0,                               // END_COLLECTION
    //MOUSE TLC
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)     
    0x09, 0x02,                         // USAGE (Mouse)                    
    0xa1, 0x01,                         // COLLECTION (Application)        
    0x85, REPORTID_MOUSE,               //   REPORT_ID (Mouse)              
    0x09, 0x01,                         //   USAGE (Pointer)                
    0xa1, 0x00,                         //   COLLECTION (Physical)          
    0x05, 0x09,                         //     USAGE_PAGE (Button)          
    0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)     
    0x29, 0x02,                         //     USAGE_MAXIMUM (Button 2)     
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          
    0x75, 0x01,                         //     REPORT_SIZE (1)              
    0x95, 0x02,                         //     REPORT_COUNT (2)             
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x95, 0x06,                         //     REPORT_COUNT (6)             
    0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         
    0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 
    0x09, 0x30,                         //     USAGE (X)                    
    0x09, 0x31,                         //     USAGE (Y)                    
    0x75, 0x10,                         //     REPORT_SIZE (16)             
    0x95, 0x02,                         //     REPORT_COUNT (2)             
    0x25, 0x0a,                          //    LOGICAL_MAXIMUM (10)      
    0x81, 0x06,                         //     INPUT (Data,Var,Rel)         
    0xc0,                               //   END_COLLECTION                 
    0xc0,                                //END_COLLECTION

    // Integrated Windows Pen TLC
    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)          
    0x09, 0x02,                         // USAGE (Pen)                      
    0xa1, 0x01,                         // COLLECTION (Application)         
    0x85, REPORTID_PEN,                 //   REPORT_ID (Pen)                
    0x09, 0x20,                         //   USAGE (Stylus)                 
    0xa1, 0x00,                         //   COLLECTION (Physical)          
    0x09, 0x42,                         //     USAGE (Tip Switch)           
    0x09, 0x44,                         //     USAGE (Barrel Switch)        
    0x09, 0x3c,                         //     USAGE (Invert)               
    0x09, 0x45,                         //     USAGE (Eraser Switch)        
    0x15, 0x00,                         //     LOGICAL_MINIMUM (0)          
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          
    0x75, 0x01,                         //     REPORT_SIZE (1)              
    0x95, 0x04,                         //     REPORT_COUNT (4)             
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x95, 0x01,                         //     REPORT_COUNT (1)           
    0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)      
    0x09, 0x32,                         //     USAGE (In Range)             
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x95, 0x02,                         //     REPORT_COUNT (2)             
    0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         
    0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 
    0x09, 0x30,                         //     USAGE (X)                    
    0x75, 0x10,                         //     REPORT_SIZE (16)             
    0x95, 0x01,                         //     REPORT_COUNT (1)             
    0xa4,                               //     PUSH                         
    0x55, 0x0d,                         //     UNIT_EXPONENT (-3)           
    0x65, 0x13,                         //     UNIT (Inch,EngLinear)        
    0x35, 0x00,                         //     PHYSICAL_MINIMUM (0)         
    0x46, 0x3a, 0x20,                   //     PHYSICAL_MAXIMUM (8250)      
    0x26, 0xf8, 0x52,                   //     LOGICAL_MAXIMUM (21240)      
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x09, 0x31,                         //     USAGE (Y)                    
    0x46, 0x2c, 0x18,                   //     PHYSICAL_MAXIMUM (6188)      
    0x26, 0x6c, 0x3e,                   //     LOGICAL_MAXIMUM (15980)      
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0xb4,                               //     POP                          
    0x05, 0x0d,                         //     USAGE_PAGE (Digitizers)      
    0x09, 0x30,                         //     USAGE (Tip Pressure)         
    0x26, 0xff, 0x00,                   //     LOGICAL_MAXIMUM (255)        
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x75, 0x08,                         //     REPORT_SIZE (8)              
    0x09, 0x3d,                         //     USAGE (X Tilt)               
    0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)       
    0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)        
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0x09, 0x3e,                         //     USAGE (Y Tilt)               
    0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)       
    0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)        
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
    0xc0,                               //   END_COLLECTION                 
    0xc0                                // END_COLLECTION
};

struct TouchReportByte1
{
    uint8_t Confidence : 1;
    uint8_t TipSwitch : 1;
    uint8_t ContactId : 2;
    uint8_t reserved1 : 4;
};

uint8_t TouchReportDataBuffer[1 + 1 + 2 + 2 + 2 + 1 + 1];
void GetTouchReportDataBuffer(uint8_t* buffer, uint8_t reportId, TouchReportByte1 reportByte1, uint16_t x, uint16_t y, uint16_t scanTime, uint8_t contactCount, uint8_t btn1)
{
    buffer[0] = reportId;
    *reinterpret_cast<TouchReportByte1*>(&buffer[1]) = reportByte1;
    *reinterpret_cast<uint16_t*>(&buffer[2]) = x;
    *reinterpret_cast<uint16_t*>(&buffer[4]) = y;
    *reinterpret_cast<uint16_t*>(&buffer[6]) = scanTime;
    *reinterpret_cast<uint8_t*>(&buffer[8]) = contactCount;
    *reinterpret_cast<uint8_t*>(&buffer[9]) = btn1;
}

uint8_t mouseReportDataBuffer[1 + 1 + 2 + 2];
void GetMouseReportDataBuffer(uint8_t* buffer, uint8_t reportId, bool btn1, bool btn2, uint16_t x, uint16_t y)
{
    buffer[0] = reportId;
    buffer[1] = (btn1 ? 0x1 : 0) | (btn2 ? 0x2 : 0);
    *reinterpret_cast<uint16_t*>(&buffer[2]) = x;
    *reinterpret_cast<uint16_t*>(&buffer[4]) = y;
}

void OnSetFeature(HIDPHANDLE hidpHandle, UCHAR reportId, ULONG reportBufferLen, UCHAR* data)
{
    std::cout << "SetFeature:" << static_cast<int32_t>(reportId) << std::endl;
    return;
}

void OnGetFeature(HIDPHANDLE hidpHandle, UCHAR reportId, ULONG bufferLength, PULONG pReportBufferLen, UCHAR* data)
{
    std::cout << "GetFeature:" << static_cast<int32_t>(reportId) << std::endl;
    UCHAR featureReport[2] = { reportId, 0x15 };
    *pReportBufferLen = 2;
    CopyMemory(data, featureReport, sizeof(featureReport));
}


int main()
{
    std::cout << "Hello World!\n";
    HRESULT hr;

    HIDPHANDLE penTouchHidp = nullptr;
    
    hr = HidpCreate(sizeof(PenTouchpadDescriptor), PenTouchpadDescriptor, OnSetFeature, OnGetFeature, &penTouchHidp);
    BOOL b = HidpStart(penTouchHidp);
    do
    {
        
        PenReportByte1 penReportByte1{};
        penReportByte1.TipSwitch = 0;
        penReportByte1.BarrelSwitch = 0;
        penReportByte1.Invert = 0;
        penReportByte1.EraserSwitch = 0;
        penReportByte1.Reserved1 = 0;
        penReportByte1.InRange = 1;
        penReportByte1.Reserved2 = 0;
        uint16_t x = 0;
        uint16_t y = 0;
        while (x < 3000)
        {
            BYTE data = 1;
            // According to HID speicification 5.8
            // The first byte should be Report ID if defined in descriptor
            // uint8_t payload[4] = { 2, 10, 10, 0 };
            x += 1;
            x %= 21241;
            y += 1;
            y %= 15981;
            GetPenReportDataBuffer(PenReportDataBuffer, REPORTID_PEN, penReportByte1, x, y, 0, 0, 0);
            hr = HidpSubmitReport(penTouchHidp, 1, sizeof(PenReportDataBuffer), PenReportDataBuffer);
            // std::cout << x << std::endl;
            Sleep(1);
        }
        
    } while (false);

    Sleep(2000);
    
    do
    {
        TouchReportByte1 touchReportByte1{};
        touchReportByte1.Confidence = 1;
        touchReportByte1.TipSwitch = 1;
        touchReportByte1.ContactId = 0;
        uint16_t x = 0;
        uint16_t y = 0;
        
        while (x < 3000)
        {
            BYTE data = 1;
            x += 1;
            x %= 4096;
            y += 1;
            y %= 4096;
            GetTouchReportDataBuffer(TouchReportDataBuffer, REPORTID_TOUCHPAD, touchReportByte1, x, y, 10, 1, 0);
            hr = HidpSubmitReport(penTouchHidp, 1, sizeof(TouchReportDataBuffer), TouchReportDataBuffer);
            // std::cout << x << std::endl;
            Sleep(1);
        }
    } while (false);

    HidpClose(penTouchHidp);
    
    
    
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
