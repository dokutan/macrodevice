# Backend documentation

This is a list of all backends and their features.

## libevdev
### Dependencies
[libevdev](https://www.freedesktop.org/software/libevdev/doc/latest/)
### Supported devices
Any supported by Linux as input devices
### Notes and Limitations
This is the recommended backend if available.
### Settings
setting key | description |  required? | default
---|---|---|---
eventfile | the path to the eventfile, e.g. /dev/input/event1 | required | 
grab | block input from the device to other programs, "true" or "false" | optional | true
numbers | don't convert the numeric event values to strings, "true" or "false" | optional | false
### Event description
1. event type
2. event code
3. event value

## libusb
### Dependencies
[libusb](https://github.com/libusb/libusb)
### Supported devices
USB keyboards
### Notes and Limitations
This is the recommended backend for keyboards, as an alternative to libevdev.
### Settings
setting key | description |  required? | default
---|---|---|---
use_bus_device | open the device with the usb bus id and device address instead oft the vid and pid | optional | false
vid | usb product id | required when use_bus_device = "false" | 
pid | usb vendor id | required when use_bus_device = "false" | 
bus | usb bus id | required when use_bus_device = "true" | 
device | usb device address | required when use_bus_device = "true" | 
### Event description
1. modifiers
2. key

## hidapi
### Dependencies
[hidapi](https://github.com/libusb/hidapi)
### Supported devices
USB keyboards
### Notes and Limitations
Not recommended, try libusb instead. Included for compatibility.
After closing the program, the keyboard needs to be removed and plugged back in for it to work again. This is because the kernel driver remains detached.
### Settings
setting key | description |  required? | default
---|---|---|---
vid | usb product id | required | 
pid | usb vendor id | required | 
### Event description
1. modifiers
2. key

## serial
### Dependencies
None
### Supported devices
Any device sending over serial, e.g. Arduino.
### Notes and Limitations
This backend reads from the serial interface, until a newline ('\n') is received. The received message up to that point (excluding the newline) is passed to Lua. Care must be taken to handle any potential carriage returns ('\r') at the end of the message. A minimal Arduino example can be found in documentation/arduino-button-serial.ino.
### Settings
setting key | description |  required? | default
---|---|---|---
port | the path to the serial port, e.g. /dev/ttyUSB0 | required |  false
### Event description
1. serial message

## xindicator
### Dependencies
Xlib (libx11)
### Supported devices
Xorg keyboard indicators (caps lock, num lock, etc.).
### Notes and Limitations

### Settings
None
### Event description
1. A number corresponding to the active keyboard indicators
