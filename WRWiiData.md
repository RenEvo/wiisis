# Files #

  * Core\Interfaces\WR\_IWiiData.h
  * Core\WR\_CWiiData.h
  * Core\WR\_CWiiData.cpp

# Description #

This is a helper class created by each Wii Remote instance. It is used to manage reading and writing data to/from the Wii Remote's flash memory and registers. It is used by some of the other helpers as well including the Motion helper (for getting calibration data), the Expansion helpers and the IR Sensor helper.

Its listener will report when data has been read or written.