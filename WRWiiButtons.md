# Files #

  * Core\Interfaces\WR\_IWiiButtons.h
  * Core\WR\_CWiiButtons.h
  * Core\WR\_CWiiButtons.cpp

# Description #

This is a helper class created by each Wii Remote instance. It is used to manage input control on the Wii Remote.

The Buttons helper supports buffered input control, but this must first be turned on by calling **EnableBufferedInput**. You can then poll for the status of a button by:
  * Calling **IsButtonDown** to return TRUE if the button is down or FALSE if it is up
  * Calling **GetButtonStatus** to return a value indicating its current status, including:
    1. Up - Button is up
    1. Pushed - Buffered Input; Button is being pushed down
    1. Down - Button is down
    1. Released - Buffered Input; Button is being released

The Buttons helper also has a built in action mapping system. You may create an action by calling **RegisterAction** and giving it a name and any combination of buttons (OR values together). You can then check the status of the action using either **IsActionDown** or **GetActionStatus** which operate the same as above.

Its listener will report back when the status of a button or an action has changed.