using LibHidpRT;
using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;

namespace LibHidpTestSharp
{
    internal class Program
    {
        const Byte REPORTID_TOUCHPAD = 0x01;
        const Byte REPORTID_MAX_COUNT = 0x02;
        const Byte REPORTID_PTPHQA = 0x03;
        const Byte REPORTID_FEATURE = 0x04;
        const Byte REPORTID_FUNCTION_SWITCH = 0x05;
        const Byte REPORTID_MOUSE = 0x06;
        const Byte REPORTID_PEN = 0x07;
        static Byte[] PenTouchpadDescriptor =
        {
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
        static async Task Main(string[] args)
        {
            var reportDescriptorBuffer = new Windows.Storage.Streams.Buffer((uint)(PenTouchpadDescriptor.Length));
            reportDescriptorBuffer.Length = (uint)PenTouchpadDescriptor.Length;
            using(var stream = reportDescriptorBuffer.AsStream())
            {
                await stream.WriteAsync(PenTouchpadDescriptor);
            }
            var hidp = await Hidp.CreateAsync(reportDescriptorBuffer);

            UInt16 x = 0;
            UInt16 y = 0;

            while(x <= 21241)
            {
                x += 1;
                y += 1;
                y %= 15981;

                var penReportBuffer = writePenReportDatabuffer(1, false, false, false, false, true, x, y, 0, 0, 0);
                await hidp.SubmitReportAsync(1, penReportBuffer);
                await Task.Delay(1);
            }

            hidp.Dispose();
            
        }

        static IBuffer writePenReportDatabuffer(Byte reportId, bool tipSwitch, bool barrelSwitch, bool invert, bool eraserSwitch, bool inRange, UInt16 x, UInt16 y, UInt16 tipPressure, Byte xTilt, Byte yTilt)
        {
            using(var writer = new DataWriter())
            {
                writer.ByteOrder = ByteOrder.LittleEndian;
                Byte reportByte1 = 0;
                reportByte1 |= (byte)(tipSwitch ? 0x1 : 0);
                reportByte1 |= (byte)(barrelSwitch ? 0x2 : 0);
                reportByte1 |= (byte)(invert ? 0x4 : 0);
                reportByte1 |= (byte)(eraserSwitch ? 0x8 : 0);
                reportByte1 |= (byte)(inRange ? 0x20 : 0);

                writer.WriteByte(reportId);
                writer.WriteByte(reportByte1);
                writer.WriteUInt16(x);
                writer.WriteUInt16(y);
                writer.WriteUInt16(tipPressure);
                writer.WriteByte(xTilt);
                writer.WriteByte(yTilt);
                return writer.DetachBuffer();
            }
            
        }
    }
}
