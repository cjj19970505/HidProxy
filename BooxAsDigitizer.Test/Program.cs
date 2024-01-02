using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks.Dataflow;
using Windows.Storage.Streams;
using AoaRT;
using LibHidpRT;
using System.Net;
using System;

namespace BooxAsDigitizer.Test
{
    internal class Program
    {
        static Byte REPORTID_TOUCHPAD = 0x01;
        static Byte REPORTID_MAX_COUNT = 0x02;
        static Byte REPORTID_PTPHQA = 0x03;
        static Byte REPORTID_FEATURE = 0x04;
        static Byte REPORTID_FUNCTION_SWITCH = 0x05;
        static Byte REPORTID_MOUSE = 0x06;
        static Byte REPORTID_PEN = 0x07;
        static UInt16 TOUCHPAD_PHY_WIDTH = 6181 / 4;
        static UInt16 TOUCHPAD_PHY_HEIGHT = 4606 / 4;

        static UInt16 PENPAD_PHY_WIDTH = 6181;
        static UInt16 PENPAD_PHY_HEIGHT = 4606;

        static Byte[] HidDescriptor = new byte[]
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
            0x75, 0x04,                         //       REPORT_SIZE (4)
            0x25, 0x05,                         //       LOGICAL_MAXIMUM (5)
            0x09, 0x51,                         //       USAGE (Contact Identifier)
            0x81, 0x02,                         //       INPUT (Data,Var,Abs)
            0x75, 0x01,                         //       REPORT_SIZE (1)
            0x95, 0x02,                         //       REPORT_COUNT (2)             
            0x81, 0x03,                         //       INPUT (Cnst,Var,Abs)      // byte1 (1 byte)
            0x05, 0x01,                         //       USAGE_PAGE (Generic Desk..
            0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
            0x26, 0xff, 0x0f,                   //       LOGICAL_MAXIMUM (4095)         
            0x75, 0x10,                         //       REPORT_SIZE (16)             
            0x55, 0x0e,                         //       UNIT_EXPONENT (-2)           
            0x65, 0x13,                         //       UNIT(Inch,EngLinear)                  
            0x09, 0x30,                         //       USAGE (X)                    
            0x35, 0x00,                         //       PHYSICAL_MINIMUM (0)         
            0x46, (byte)(TOUCHPAD_PHY_WIDTH & 0x00FF), (byte)((TOUCHPAD_PHY_WIDTH >> 8) & 0xFF),                   //       PHYSICAL_MAXIMUM (TOUCHPAD_PHY_WIDTH)
            0x95, 0x01,                         //       REPORT_COUNT (1)         
            0x81, 0x02,                         //       INPUT (Data,Var,Abs)      // x (2 byte)
            0x46, (byte)(TOUCHPAD_PHY_HEIGHT & 0x00FF), (byte)((TOUCHPAD_PHY_HEIGHT >> 8) & 0xFF),                   //       PHYSICAL_MAXIMUM (TOUCHPAD_PHY_HEIGHT)
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
            0x46, (byte)(PENPAD_PHY_WIDTH & 0x00FF), (byte)((PENPAD_PHY_WIDTH >> 8) & 0xFF),                   //     PHYSICAL_MAXIMUM (8250)      
            0x26, 0xf8, 0x52,                   //     LOGICAL_MAXIMUM (21240)      
            0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
            0x09, 0x31,                         //     USAGE (Y)                    
            0x46, (byte)(PENPAD_PHY_HEIGHT & 0x00FF), (byte)((PENPAD_PHY_HEIGHT >> 8) & 0xFF),                   //     PHYSICAL_MAXIMUM (6188)      
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

        static BufferBlock<Windows.Storage.Streams.Buffer> PendingBuffers = new();
        static async Task Main(string[] args)
        {
            var receiveBufferTask = ReceiveBufferAsync();
            var consumeBufferTask = ConsumeBufferAsync();
            await Task.WhenAll(receiveBufferTask, consumeBufferTask);
        }

        static async Task ReceiveBufferAsync()
        {
            AoaCredential credential = new AoaCredential()
            {
                manufacturer = "XeonCJJ",
                modelName = "BooxAsDigitizer",
                description = "BooxAsDigitizer Description",
                version = "0.1",
                Uri = "https://dev.azure.com/xeonj/Misc/_git/BooxAsDigitizer",
                serialNumber = "00000000"
            };
            using (var device = await AoaDevice.CreateAsync(credential))
            {
                var preReadBuffer = new Windows.Storage.Streams.Buffer(device.InputMaxPacketSize);
                while (true)
                {
                    await device.ReadToBufferAsync(preReadBuffer);
                    var readBuffer = new Windows.Storage.Streams.Buffer(preReadBuffer.Length);
                    readBuffer.Length = readBuffer.Capacity;
                    preReadBuffer.CopyTo(readBuffer);
                    await PendingBuffers.SendAsync(readBuffer);
                }
            }
        }

        static async Task ConsumeBufferAsync()
        {
            var penReportDescriptorBuffer = new Windows.Storage.Streams.Buffer((uint)(HidDescriptor.Length));
            penReportDescriptorBuffer.Length = (uint)HidDescriptor.Length;
            using (var stream = penReportDescriptorBuffer.AsStream())
            {
                await stream.WriteAsync(HidDescriptor);
            }
            using (var hidp = await Hidp.CreateAsync(penReportDescriptorBuffer))
            {
                Task submitReportTask = null;
                while (true)
                {

                    var msg = await PendingBuffers.ReceiveAsync();
                    Byte reportId = 0;
                    using (var reader = DataReader.FromBuffer(msg))
                    {
                        reader.ByteOrder = ByteOrder.LittleEndian;
                        reportId = reader.ReadByte();
                        var byte1 = reader.ReadByte();
                        var x = reader.ReadUInt16();
                        var y = reader.ReadUInt16();
                        //var byte1 = reader.ReadByte();
                        //var x = reader.ReadUInt16();
                        //var y = reader.ReadUInt16();
                        //var tipPressure = reader.ReadUInt16();
                        //var xTilt = reader.ReadByte();
                        //var yTilt = reader.ReadByte();
                        // Console.WriteLine($"msg.Length={msg.Length}, reportId={reportId}, pointerId={byte1 >> 2} b1={byte1 & 0x3}, x={x}, y={y}");
                    }
                    if(submitReportTask != null)
                    {
                        await submitReportTask;
                    }
                    submitReportTask = hidp.SubmitReportAsync(reportId, msg).AsTask();
                }
            }
        }
    }
}
