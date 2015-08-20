# Files #

  * Core\Interfaces\WR\_IHIDController.h
  * Core\WR\_CHIDController.h
  * Core\WR\_CHIDController.cpp

# Description #

The HID Controller detects and prepares communication through the Wii Remote through its HID Profile. The Windows Driver Development Kit it utilized in this module to achieve this result.

You should first poll for devices, then initialize each one that you want to start. The controller will create and return a Wii Remote interface object which you can use to talk to the remote.

Its listener control will report back when an HID device is found and/or initialized.