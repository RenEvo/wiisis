# Files #

  * Core\Interfaces\WR\_IWiiExtension.h
  * Core\WR\_CWiiNunchuk.h
  * Core\WR\_CWiiNunchuk.cpp

# Description #

This is a helper class created by each Wii Remote instance. It is used to manage extensions plugged in to the Wii Remote. It is a generic template; derived forms are created for each supported extension type. Currently, only the Wii Nunchuk can be used.

Calling either **GetType** or **GetName** will return information that you can use to determine which extension is currently plugged in. Type-casting should then be used to interface with the actual extension's class.

The Nunchuk supports button input, motion control and analog stick control. The input manager and motion control operate exactly the same as the helpers do on the Wii Remote. This means you should **Calibrate** the motion before relying on the reported data and **EnableBufferedInput** if you wish to use buffered input.

Its listener will report the same data as both the Buttons and Motion helpers. It will also report when the analog stick is updated.