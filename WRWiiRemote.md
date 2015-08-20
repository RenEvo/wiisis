# Files #

  * Core\Interfaces\WR\_IWiiRemote.h
  * Core\WR\_CWiiRemote.h
  * Core\WR\_CWiiRemotecpp

# Description #

The Wii Remote class is the central communication point for an initialized Wii Remote. One is created through the HID Controller when an HID profile is detected and initialized.

This module runs in multiple threads - it is multithread-safe. This allows it to asynchronously write out and read packets along the HID channel through the remote's Bluetooth control.

Several helper modules are created and maintained through this module and handle input management, motion control, data transferring, IR sensor control and extension control. Each of these are described in their own File Descriptions section.

The remote relies on constant communication to confirm its connection status. You should set up a valid status update timer via **SetStatusUpdate** before connecting the remote. The remote is automatically initialized for you after it has been created. If it disconnects, simply calling **Reconnect** should cause the remote to attempt a new connection. You can set the connection timeout value via **SetConnectionTimeout**.

Its listener will report when the remote is connected, attempting to connect, or disconnected. It will also report when the remote's reporting type has changed, when a status update is received, or when an extension is plugged in/out of the Expansion port.