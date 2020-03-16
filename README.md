# macrodevice
Turn any input device into a dedicated macrodevice. This is a rewrite of [macroKeyboard](https://github.com/dokutan/macroKeyboard).

## What is the purpose of this program?
This program is designed to execute a command at the press of a button (or other input) from a dedicated device (e.g keyboard, gamepad, etc.). The configuration is done in Lua, so a complex configuration is possible.

## Installing
- Install all dependencies (on some distros you might need a dev package for the header files)
  - Lua
  - Take a look at the available [backends](https://github.com/dokutan/macrodevice/blob/master/documentation/backends.md) and install the dependencies for the backends you want. Or install all of them: libevdev, libusb, hidapi
- Clone this repository
- If you don't want all backends, comment out or remove the appropriate lines at the beginning of the makefile
- Build and install with
```
make
sudo make install
```

## Running

### Basic usage

Get a list of all options
```
macrodevice-lua -h
```

Start the program
```
macrodevice-lua -c your-config.lua
```

Run in the background
```
macrodevice-lua -c your-config.lua -f
```

### Lua config

To handle the incoming events and execute commands a Lua script is needed. For the details look at example.lua .

### Dealing with permissions
In most cases root privileges are needed to directly open an input device, however running this program as root creates a major security risk, as all macros are executed with root privileges as well. There are multiple ways to deal with this problem.

1. Start the program with root privileges, and drop these as soon as the device has been opened. This works in all cases where no root privileges are needed once the device has been opened. For this you need to [know your user id and group id](https://wiki.archlinux.org/index.php/Users_and_groups#User_database).
```
sudo macrodevice-lua -c your-config.lua -u your_user_id -g your_group_id
```
2. If the device is represented by a file, as with the libevdev backend, you can change the file permissions, or add your user to a group with access to the files of interest.
3. In case of the libusb and hidapi backends, you can create a [udev](https://wiki.archlinux.org/index.php/Udev) rule that allows read and write access to the device from your normal user.

## License
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
