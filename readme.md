This project implements a virtual HID Framework for user mode applications.

Orignally used for BooxAsDigitizer project, but I decided to single the KMDF part out since there could be more use to it.

# Getting Started
1. Build and install the HidProxy kernel mode driver.

2. Using `LibHidpRT` WinRT library
    - Currently we have a `LibHidpRT` which can be used for WinRT applications (No UWP). Check `LibHidpTestSharp` for usages.

# How it works
This project creates a kernel mode driver that utilizes the [VHF](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/vhf/).

# Risk
You need to install the kernel mode driver built from this libaries, which if not properly signed, can only ran when you set your PC to test-sign mode.

- [How to Test-Sign a Driver Package](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/how-to-test-sign-a-driver-package)

# Reference
- [USB 2.0 Specification](https://www.usb.org/document-library/usb-20-specification)
    - [Device Class Definition for HID 1.11](https://www.usb.org/document-library/device-class-definition-hid-111)
    - [HID Usage Tables 1.5](https://www.usb.org/document-library/hid-usage-tables-15)

# TODO

# Prerequisite
- [Zadig](https://zadig.akeo.ie/) for installing USB driver for Android 

# Related Projects:
- [BooxAsDigitizer](https://xeonj.visualstudio.com/Misc/_git/BooxAsDigitizer)

# Debug

# Reference
- [Android Open Accessory (AOA)](https://source.android.com/docs/core/interaction/accessories/protocol)
    - [adk2012_demo](https://android.googlesource.com/device/google/accessory/adk2012_demo)
    - [Android accessory support - demo kit.](https://android.googlesource.com/device/google/accessory/demokit)
- [USB 2.0 Specification](https://www.usb.org/document-library/usb-20-specification)
    - [Device Class Definition for HID 1.11](https://www.usb.org/document-library/device-class-definition-hid-111)
    - [HID Usage Tables 1.5](https://www.usb.org/document-library/hid-usage-tables-15)