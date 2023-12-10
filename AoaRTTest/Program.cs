using AoaRT;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks.Dataflow;
using Windows.Storage.Streams;

namespace AoaRTTest
{
    internal class Program
    {
        static BufferBlock<Windows.Storage.Streams.Buffer> PendingBuffers = new();
        static async Task Main(string[] args)
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

            _ = ConsumeBufferTask();

            using (var device = await AoaDevice.CreateAsync(credential))
            {
                var preReadBuffer = new Windows.Storage.Streams.Buffer(device.InputMaxPacketSize);
                while(true)
                {
                    await device.ReadToBufferAsync(preReadBuffer);
                    var readBuffer = new Windows.Storage.Streams.Buffer(preReadBuffer.Length);
                    readBuffer.Length = readBuffer.Capacity;
                    preReadBuffer.CopyTo(readBuffer);
                    await PendingBuffers.SendAsync(readBuffer);
                }

            }
        }
        static async Task ConsumeBufferTask()
        {
            while (true)
            {
                var msg = await PendingBuffers.ReceiveAsync();
                using (var reader = DataReader.FromBuffer(msg))
                {
                    reader.ByteOrder = ByteOrder.LittleEndian;
                    var reportId = reader.ReadByte();
                    var byte1 = reader.ReadByte();
                    var x = reader.ReadUInt16();
                    var y = reader.ReadUInt16();
                    var tipPressure = reader.ReadUInt16();
                    var xTilt = reader.ReadByte();
                    var yTilt = reader.ReadByte();
                    Console.WriteLine($"msg.Length={msg.Length}, reportId={reportId}, b1={byte1}, x={x}, y={y}");
                }
            }
        }
    }
}
