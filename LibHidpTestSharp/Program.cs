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
        static Byte[] PenReportDescriptor =
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
        static async Task Main(string[] args)
        {
            var penReportDescriptorBuffer = new Windows.Storage.Streams.Buffer((uint)(PenReportDescriptor.Length));
            penReportDescriptorBuffer.Length = (uint)PenReportDescriptor.Length;
            using(var stream = penReportDescriptorBuffer.AsStream())
            {
                await stream.WriteAsync(PenReportDescriptor);
            }
            var hidp = await Hidp.CreateAsync(penReportDescriptorBuffer);

            UInt16 x = 0;
            UInt16 y = 0;

            while(x <= 21241)
            {
                x += 1;
                y += 1;
                y %= 15981;

                var penReportBuffer = writePenReportDatabuffer(1, false, false, false, false, true, x, y, 0, 0, 0);
                await hidp.SubmitReportAsync(1, penReportBuffer);
                await Task.Delay(16);
            }

            hidp.Dispose();
            
        }

        static IBuffer writePenReportDatabuffer(Byte reportId, bool tipSwitch, bool barrelSwitch, bool invert, bool eraserSwitch, bool inRange, UInt16 x, UInt16 y, UInt16 tipPressure, Byte xTilt, Byte yTilt)
        {
            using(var writer = new DataWriter())
            {
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
