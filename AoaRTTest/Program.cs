using AoaRT;

namespace AoaRTTest
{
    internal class Program
    {
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
            var device = await AoaDevice.CreateAsync(credential);
        }
    }
}
