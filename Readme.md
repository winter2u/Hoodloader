Hoodloader Alpha

New Hoodloader, dev status. (this repository is a temporary backup for now)
Planned features:
Programm Arduino via usb
Use Arduino as HID
Use Arduino as ISP

This folder is only interesting for developers.
You need to install the gcc-avr toolchain, avr-libc and compile with:
``` bash
$ sudo apt-get install gcc-avr avr-libc
$ cd Hoodloader_Source
$ sudo make clean
$ sudo make
```

changelog:
Removed USBtoUSART buffer (not needed)
Integrated NHP directly
Seperated different modes better to not cause any errors in default mode
Improved writing to CDC Host
fixed a bug in checkNHPProtocol: & needs to be a==