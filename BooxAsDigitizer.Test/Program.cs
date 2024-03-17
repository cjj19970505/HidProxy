using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks.Dataflow;
using Windows.Storage.Streams;
using AoaRT;
using LibHidpRT;
using System.Net;
using System;
using System.Diagnostics;
using System.Numerics;
using Windows.UI.Input.Preview.Injection;

namespace BooxAsDigitizer.Test
{
    internal class Program
    {
        // First byte 0..0x7f for report id
        static Byte REPORTID_TOUCHPAD = 0x01;
        static Byte REPORTID_MAX_COUNT = 0x02;
        static Byte REPORTID_PTPHQA = 0x03;
        static Byte REPORTID_FEATURE = 0x04;
        static Byte REPORTID_FUNCTION_SWITCH = 0x05;
        static Byte REPORTID_MOUSE = 0x06;
        static Byte REPORTID_PEN = 0x07;

        static Byte SETUPID_RESET_DESCRIPTOR = 0x80;

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
            var resetHidDescriptorCts = new CancellationTokenSource();
            var receiveBufferTask = ReceiveBufferAsync();
            var consumeBufferTask = ConsumeBufferAsync(resetHidDescriptorCts, resetHidDescriptorCts.Token);
            
            while(true)
            {
                try
                {
                    await consumeBufferTask;
                }
                catch (OperationCanceledException ex)
                {
                    resetHidDescriptorCts = new CancellationTokenSource();
                    consumeBufferTask = ConsumeBufferAsync(resetHidDescriptorCts, resetHidDescriptorCts.Token);
                }
            }

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

        static async Task ConsumeBufferAsync(CancellationTokenSource resetHidDescriptorCts, CancellationToken token)
        {
            var penReportDescriptorBuffer = new Windows.Storage.Streams.Buffer((uint)(HidDescriptor.Length));
            penReportDescriptorBuffer.Length = (uint)HidDescriptor.Length;
            using (var stream = penReportDescriptorBuffer.AsStream())
            {
                await stream.WriteAsync(HidDescriptor);
            }
            using (var hidp = await Hidp.CreateAsync(penReportDescriptorBuffer))
            {
                hidp.OnSetFeatureRequest += Hidp_OnSetFeatureRequest;
                hidp.OnGetFeatureRequest += Hidp_OnGetFeatureRequest;
                await hidp.StartAsync();

                Task? submitReportTask = null;
                (UInt16 X, uint Y)? latestPenPos = null;
                Dictionary<byte, bool> presentedContacts = new Dictionary<byte, bool>();
                var mouseCorrectInputInjector = InputInjector.TryCreate();
                while (true)
                {
                    bool skip = false;
                    var msg = await PendingBuffers.ReceiveAsync();
                    Byte reportId = 0;
                    using (var reader = DataReader.FromBuffer(msg))
                    {
                        reader.ByteOrder = ByteOrder.LittleEndian;
                        reportId = reader.ReadByte();
                        if(reportId == SETUPID_RESET_DESCRIPTOR)
                        {
                            HidDescriptor = new byte[msg.Length - 1];
                            reader.ReadBytes(HidDescriptor);
                            resetHidDescriptorCts.Cancel();
                        }
                        token.ThrowIfCancellationRequested();
                        if (reportId == REPORTID_PEN)
                        {
                            var byte1 = reader.ReadByte();
                            bool tipSwitch = (byte1 & 1) != 0;
                            bool barrelSwitch = (byte1 & (1 << 1)) != 0;
                            bool inverted = (byte1 & (1 << 2)) != 0;
                            bool eraserSwitch = (byte1 & (1 << 3)) != 0;
                            bool inRange = (byte1 & (1 << 5)) != 0;

                            UInt16 x = reader.ReadUInt16();
                            UInt16 y = reader.ReadUInt16();
                            Byte tipPressure = reader.ReadByte();
                            Byte xTilt = reader.ReadByte();
                            Byte yTilt = reader.ReadByte();
                            latestPenPos = (x, y);
                            presentedContacts.Clear();
                        }
                        else if(reportId == REPORTID_TOUCHPAD)
                        {
                            var byte1 = reader.ReadByte();
                            bool confidence = (byte1 & (1 << 0)) != 0;
                            bool tipSwitch = (byte1 & (1 << 1)) != 0;
                            byte contactId = (byte)((byte1 & (0xf << 2)) >> 2);
                            UInt16 x = reader.ReadUInt16();
                            UInt16 y = reader.ReadUInt16();
                            UInt16 scanTime = reader.ReadUInt16();
                            byte contactCount = reader.ReadByte();
                            if(tipSwitch)
                            {
                                if(presentedContacts.Count == 0 && latestPenPos != null)
                                {
                                    double normalizedLatestPenX = ((double)latestPenPos.Value.X) / 21240;
                                    double normalizedLatestPenY = ((double)latestPenPos.Value.Y) / 15980;


                                    latestPenPos = null;
                                    var injectedInputMouseInfo = new InjectedInputMouseInfo()
                                    {
                                        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-mouseinput#remarks
                                        // about 65535
                                        DeltaX = (int)(normalizedLatestPenX * 65535),
                                        DeltaY = (int)(normalizedLatestPenY * 65535),
                                        MouseData = 0,
                                        MouseOptions = InjectedInputMouseOptions.Absolute,
                                        TimeOffsetInMilliseconds = 0
                                    };
                                    mouseCorrectInputInjector.InjectMouseInput(new InjectedInputMouseInfo[] { injectedInputMouseInfo });

                                }
                                presentedContacts[contactId] = true;
                                if(contactCount == 1)
                                {
                                    // skip = true;
                                }
                            }
                            else
                            {
                                presentedContacts.Remove(contactId);
                            }
                        }
                    }
                    if(submitReportTask != null)
                    {
                        await submitReportTask;
                        submitReportTask = null;
                    }
                    if(!skip)
                    {
                        submitReportTask = hidp.SubmitReportAsync(reportId, msg).AsTask();
                    }
                }
            }
        }

        private async static void Hidp_OnGetFeatureRequest(object? sender, GetFeatureRequest e)
        {
            Hidp? hidp = sender as Hidp;
            Debug.Assert(hidp != null);
            if(e.ReportId == REPORTID_MAX_COUNT)
            {
                byte buttonType = 1;
                byte maxContactCount = 5;
                using(var writer = new DataWriter())
                {
                    writer.WriteByte(e.ReportId);
                    writer.WriteByte((byte)(buttonType << 4 | maxContactCount));
                    await hidp.CompleteGetFeatureRequestAsync(e, writer.DetachBuffer(), true);
                }
                Console.WriteLine("GetFeature: MAX_COUNT and BUTTON_TYPE");
            }
            else if(e.ReportId == REPORTID_PTPHQA)
            {
                // This is not a valid blob
                // It's just a sample blob from https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/touchscreen-required-hid-top-level-collections
                var pthqaBlob = new byte[]
                {
                    0xfc, 0x28, 0xfe, 0x84, 0x40, 0xcb, 0x9a, 0x87, 0x0d, 0xbe, 0x57, 0x3c, 0xb6, 0x70, 0x09, 0x88, 0x07,
                    0x97, 0x2d, 0x2b, 0xe3, 0x38, 0x34, 0xb6, 0x6c, 0xed, 0xb0, 0xf7, 0xe5, 0x9c, 0xf6, 0xc2, 0x2e, 0x84,
                    0x1b, 0xe8, 0xb4, 0x51, 0x78, 0x43, 0x1f, 0x28, 0x4b, 0x7c, 0x2d, 0x53, 0xaf, 0xfc, 0x47, 0x70, 0x1b,
                    0x59, 0x6f, 0x74, 0x43, 0xc4, 0xf3, 0x47, 0x18, 0x53, 0x1a, 0xa2, 0xa1, 0x71, 0xc7, 0x95, 0x0e, 0x31,
                    0x55, 0x21, 0xd3, 0xb5, 0x1e, 0xe9, 0x0c, 0xba, 0xec, 0xb8, 0x89, 0x19, 0x3e, 0xb3, 0xaf, 0x75, 0x81,
                    0x9d, 0x53, 0xb9, 0x41, 0x57, 0xf4, 0x6d, 0x39, 0x25, 0x29, 0x7c, 0x87, 0xd9, 0xb4, 0x98, 0x45, 0x7d,
                    0xa7, 0x26, 0x9c, 0x65, 0x3b, 0x85, 0x68, 0x89, 0xd7, 0x3b, 0xbd, 0xff, 0x14, 0x67, 0xf2, 0x2b, 0xf0,
                    0x2a, 0x41, 0x54, 0xf0, 0xfd, 0x2c, 0x66, 0x7c, 0xf8, 0xc0, 0x8f, 0x33, 0x13, 0x03, 0xf1, 0xd3, 0xc1, 0x0b,
                    0x89, 0xd9, 0x1b, 0x62, 0xcd, 0x51, 0xb7, 0x80, 0xb8, 0xaf, 0x3a, 0x10, 0xc1, 0x8a, 0x5b, 0xe8, 0x8a,
                    0x56, 0xf0, 0x8c, 0xaa, 0xfa, 0x35, 0xe9, 0x42, 0xc4, 0xd8, 0x55, 0xc3, 0x38, 0xcc, 0x2b, 0x53, 0x5c,
                    0x69, 0x52, 0xd5, 0xc8, 0x73, 0x02, 0x38, 0x7c, 0x73, 0xb6, 0x41, 0xe7, 0xff, 0x05, 0xd8, 0x2b, 0x79,
                    0x9a, 0xe2, 0x34, 0x60, 0x8f, 0xa3, 0x32, 0x1f, 0x09, 0x78, 0x62, 0xbc, 0x80, 0xe3, 0x0f, 0xbd, 0x65,
                    0x20, 0x08, 0x13, 0xc1, 0xe2, 0xee, 0x53, 0x2d, 0x86, 0x7e, 0xa7, 0x5a, 0xc5, 0xd3, 0x7d, 0x98, 0xbe,
                    0x31, 0x48, 0x1f, 0xfb, 0xda, 0xaf, 0xa2, 0xa8, 0x6a, 0x89, 0xd6, 0xbf, 0xf2, 0xd3, 0x32, 0x2a, 0x9a,
                    0xe4, 0xcf, 0x17, 0xb7, 0xb8, 0xf4, 0xe1, 0x33, 0x08, 0x24, 0x8b, 0xc4, 0x43, 0xa5, 0xe5, 0x24, 0xc2
                };
                using(var writer = new DataWriter())
                {
                    writer.WriteBytes(pthqaBlob);
                    await hidp.CompleteGetFeatureRequestAsync(e, writer.DetachBuffer(), true);
                }
                Console.WriteLine("GetFeature: MAX_COUNT and BUTTON_TYPE");
            }
            else
            {
                Console.WriteLine("Unknown GetFeature");
                await hidp.CompleteGetFeatureRequestAsync(e, null, false);
            }

        }

        private static async void Hidp_OnSetFeatureRequest(object? sender, SetFeatureRequest e)
        {
            Hidp? hidp = sender as Hidp;
            Debug.Assert(hidp != null);
            // Input mode
            if(e.ReportId == REPORTID_FEATURE)
            {
                await hidp.CompleteSetFeatureRequestAsync(e, true);
                Console.WriteLine($"Input Mode: {e.ReportBuffer.ToArray()[1]}");
            }
            else if(e.ReportId == REPORTID_FUNCTION_SWITCH)
            {
                var functionSwitch = e.ReportBuffer.ToArray()[1];
                var surfaceSwitch = (functionSwitch & 1) != 0;
                var buttonSwitch = (functionSwitch & 2) != 0;
                await hidp.CompleteSetFeatureRequestAsync(e, true);
                Console.WriteLine($"Surface Switch: {surfaceSwitch}, Button Switch: {buttonSwitch}");
            }
            else
            {
                Console.WriteLine("Unknown SetFeature");
                await hidp.CompleteSetFeatureRequestAsync(e, false);
            }
        }
    }
}
