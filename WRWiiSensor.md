# Files #

  * Core\Interfaces\WR\_IWiiSensor.h
  * Core\WR\_CWiiSensor.h
  * Core\WR\_CWiiSensor.cpp

# Description #

This is a helper class created by each Wii Remote instance. It is used to manage IR sensor control on the Wii Remote.

The Sensor helper will listen for the remote's reporting on Sensor beads through its IR camera. It will perform simple triangulation to calculate the location of the "cursor" on the screen.

Its listener will report when the cursor enters/leaves the screen and when it moves to a new location on the screen.