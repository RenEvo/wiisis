# Files #

  * Core\Interfaces\WR\_IWiiMotion.h
  * Core\WR\_CWiiMotion.h
  * Core\WR\_CWiiMotion.cpp

# Description #

This is a helper class created by each Wii Remote instance. It is used to manage motion control on the Wii Remote.

The Motion helper monitors the accelerometer on the Wii Remote and detects the G forces being applied in all three cardinal directions. For it to report the correct data, you will need to first calibrate the remote. This can be done by calling **Calibrate** before requesting any of the motion data.

The Motion helper will attempt to calculate both the directional orientation, pitch and roll of the remote by relying on the concept that gravity is universal here on Earth. This will only work if the remote's acceleration is normalized. This means if you jerk it, it will not attempt to calculate this data.

The Motion helper can also group several motion updates into one uniform gesture. It does this by determining if the remote has moved beyond an epsilon value on any axis. Once it has stopped, it terminates the line. You can set how many updates must past before a gesture is determined by calling **SetMotionSize**.

Its listener will report back when the remote has experienced a motion update. It will also report the starting of a gesture, an update frame in the active gesture, and the ending of the current gesture as explained above.