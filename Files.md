# Wii Remote (WR) Library #
These files make up WR Library and perform the actual communication with the Wii Remote. There are several modules that each control a general aspect of what it takes to talk to the remote, its extensions, and the HID profile on your computer for the remote.

All files are contained in the **Core** sub-folder. If all you need is the underlaying communications, you should only include the following files.

  * [WR\_Implementation Files](WRImplementation.md)
  * [WR\_Timer Files](WRTimer.md)
  * [WR\_HIDController Files](WRHIDController.md)
  * [WR\_WiiRemote Files](WRWiiRemote.md)
  * [WR\_WiiButtons Files](WRWiiButtons.md)
  * [WR\_WiiMotion Files](WRWiiMotion.md)
  * [WR\_WiiData Files](WRWiiData.md)
  * [WR\_WiiSensor Files](WRWiiSensor.md)
  * [WR\_WiiExtension Files (including Nunchuk support)](WRExtension.md)

# Wiisis Source #
These files make up the game logic used in Wiisis. This included utilizing the WR Library and deploying all the logic for the many buttons and motion inputs, parsing the Wii Remote configuration file, and altering some of the functionality in Crysis to better suit the remote.

You are free to include and alter this source in your own modifications, but you do not need to include _all_ of these files in your solution. You should only use what you need.

  * [Remote Manager](WiisisRemoteManager.md)
  * [Profile Manager](WiisisProfileManager.md)
  * [Motion Control](WiisisMotion.md)
  * [Crysis Files](WiisisCrysis.md)