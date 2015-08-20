////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_IWiiButtons.h
//
// Purpose: Interface object
//	Describes a helper for the Wii Remote that manages
//	button input
//
// History:
//	- 11/3/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_IWIIBUTTONS_H_
#define _WR_IWIIBUTTONS_H_

// WR_WIIREMOTE_BUTTONS
//	Mapping of buttons on the Wii Remote
enum WR_WIIREMOTE_BUTTONS
{
	// Direction pad controls
	WR_BUTTON_DPAD_UP = 0x0800,		// Up on the DPad
	WR_BUTTON_DPAD_DOWN = 0x0400,	// Down on the DPad
	WR_BUTTON_DPAD_LEFT = 0x0100,	// Left on the DPad
	WR_BUTTON_DPAD_RIGHT = 0x0200,	// Right on the DPad
	WR_BUTTON_DPAD_MASK = 0x0F00,
	WR_BUTTON_DPAD_SHIFT = 8,

	// Button controls
	WR_BUTTON_A = 0x0008,			// 'A' button
	WR_BUTTON_B = 0x0004,			// 'B' or Trigger button
	WR_BUTTON_TRIGGER = WR_BUTTON_B,
	WR_BUTTON_MINUS = 0x0010,		// '-' or Minus button
	WR_BUTTON_PLUS = 0x1000,		// '+' or Plus button
	WR_BUTTON_ONE = 0x0002,			// '1' or One button
	WR_BUTTON_TWO = 0x0001,			// '2' or Two button

	WR_BUTTON_HOME = 0x0080,		// Home button
};

// WR_WIIREMOTE_BUTTONS_INDEX
//	Index value of buttons
enum WR_WIIREMOTE_BUTTONS_INDEX
{
	WR_BUTTONINDEX_DPAD_UP = 0,
	WR_BUTTONINDEX_DPAD_DOWN,
	WR_BUTTONINDEX_DPAD_LEFT,
	WR_BUTTONINDEX_DPAD_RIGHT,
	WR_BUTTONINDEX_A,
	WR_BUTTONINDEX_B,
	WR_BUTTONINDEX_MINUS,
	WR_BUTTONINDEX_PLUS,
	WR_BUTTONINDEX_ONE,
	WR_BUTTONINDEX_TWO,
	WR_BUTTONINDEX_HOME,

	// Max
	WR_WIIREMOTE_BUTTONS_MAX,

	WR_BUTTONINDEX_TRIGGER = WR_BUTTONINDEX_B,
};
static const unsigned int WR_WIIREMOTE_BUTTONS_INDEX_VALUE[WR_WIIREMOTE_BUTTONS_MAX] =
{
	WR_BUTTON_DPAD_UP, WR_BUTTON_DPAD_DOWN, WR_BUTTON_DPAD_LEFT, WR_BUTTON_DPAD_RIGHT,
	WR_BUTTON_A, WR_BUTTON_B, WR_BUTTON_MINUS, WR_BUTTON_PLUS, WR_BUTTON_ONE, WR_BUTTON_TWO,
	WR_BUTTON_HOME,
};

// WR_WIIREMOTE_BUTTONSTATUS
//	Input status of button including buffered checks
enum WR_WIIREMOTE_BUTTONSTATUS
{
	WR_BUTTONSTATUS_UP = 0,			// Button is currently up
	WR_BUTTONSTATUS_PUSHED,			// Button has been pushed down (buffered)
	WR_BUTTONSTATUS_DOWN,			// Button is currently held down
	WR_BUTTONSTATUS_RELEASED,		// Button is currently being released

	WR_BUTTONSTATUS_COUNT,
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Input 
typedef unsigned short ActionID;
#define ACTIONID_INVALID (0)
struct SWR_WiiButton_Action
{
	ActionID nID;
	unsigned int nButtonMask;
	int nLastStatus;
	std::string szName;

	SWR_WiiButton_Action(void) : nID(ACTIONID_INVALID), nButtonMask(0), nLastStatus(0) {}
};
typedef std::map<ActionID, SWR_WiiButton_Action> ActionMap;

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Button Listener
struct IWR_WiiRemote;
struct IWR_WiiButtons;
struct IWR_WiiButtonsListener
{
	////////////////////////////////////////////////////
	// OnButton
	//
	// Purpose: Signaled when a button has been altered
	//
	// In:	pRemote - Wii Remote controller
	//		pButtons - Wii Remote button helper
	//		nButton - Button that was altered
	//			(see WR_WIIREMOTE_BUTTONS)
	//		nStatus - Button status
	//			(see WR_WIIREMOTE_BUTTONSTATUS)
	//		bDown - TRUE if button is down, FALSE if it
	//			is up
	////////////////////////////////////////////////////
	virtual void OnButton(IWR_WiiRemote *pRemote, IWR_WiiButtons *pButtons,
		unsigned int nButton, int nStatus, bool bDown) = 0;

	////////////////////////////////////////////////////
	// OnAction
	//
	// Purpose: Signaled when an action has been altered
	//
	// In:	pRemote - Wii Remote controller
	//		pButton - Wii Remote button helper
	//		szAction - Action name
	//		nActionID - ID of the action
	//		nStatus - Button status
	//			(see WR_WIIREMOTE_BUTTONSTATUS)
	//		bDown - TRUE if button is down, FALSE if it
	//			is up
	////////////////////////////////////////////////////
	virtual void OnAction(IWR_WiiRemote *pRemote, IWR_WiiButtons *pButtons, char const* szAction, 
		ActionID nActionID, int nStatus, bool bDown) = 0;
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

struct IWR_WiiButtons
{
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~IWR_WiiButtons(void) { }

	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_WiiButtonsListener *pListener) = 0;

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiButtonsListener *pListener) = 0;

	////////////////////////////////////////////////////
	// EnableBufferedInput
	//
	// Purpose: Turn buffered input on. Continuous update
	//	must be set for buffered input to work on the remote
	//
	// In:	bEnable - TRUE to enable it, FALSE to disable it
	////////////////////////////////////////////////////
	virtual void EnableBufferedInput(bool bEnable) = 0;

	////////////////////////////////////////////////////
	// IsBufferedInputEnabled
	//
	// Purpose: Returns TRUE if buffered input is enabled
	//	or FALSE if it is disabled
	//
	// Note: Checks if continuous update is enabled on
	//	the remote
	////////////////////////////////////////////////////
	virtual bool IsBufferedInputEnabled(void) const = 0;

	////////////////////////////////////////////////////
	// GetButtonStatus
	//
	// Purpose: Return the status of a button
	//
	// In:	nButtonMask - Buttons to check (see WR_WIIREMOTE_BUTTONS)
	//		fError - Error time for buffered checking
	//
	// Returns the status of the button (see WR_WIIREMOTE_BUTTONSTATUS)
	////////////////////////////////////////////////////
	virtual int GetButtonStatus(unsigned int nButtonMask, float fError = 0.0f) const = 0;

	////////////////////////////////////////////////////
	// IsButtonDown
	//
	// Purpose: Returns TRUE if the given button is down
	//		fError - Error time for buffered checking
	//
	// In:	nButtonMask - Buttons to check (see WR_WIIREMOTE_BUTTONS)
	//		fError - Error time for buffered checking
	////////////////////////////////////////////////////
	virtual bool IsButtonDown(unsigned int nButtonMask, float fError = 0.0f) const = 0;

	////////////////////////////////////////////////////
	// RegisterAction
	//
	// Purpose: Register an input action
	//
	// In:	szName - Name of the action
	//		nButtonMask - Buttons that make up the action
	//			(see WR_WIIREMOTE_BUTTONS)
	//
	// Returns ID of action or 0 on error
	////////////////////////////////////////////////////
	virtual int RegisterAction(char const* szName, unsigned int nButtonMask) = 0;

	////////////////////////////////////////////////////
	// GetActionStatus
	//
	// Purpose: Return the status of an action
	//
	// In:	szName - Name of the action
	//		nID - ID of the action
	//		fError - Error time for buffered checking
	//
	// Returns the status of the action (see WR_WIIREMOTE_BUTTONSTATUS)
	////////////////////////////////////////////////////
	virtual int GetActionStatus(char const* szName, float fError = 0.0f) const = 0;
	virtual int GetActionStatus(int nID, float fError = 0.0f) const = 0;

	////////////////////////////////////////////////////
	// IsActionDown
	//
	// Purpose: Returns TRUE if the given action is down
	//		fError - Error time for buffered checking
	//
	// In:	szName - Name of the action
	//		nID - ID of the action
	//		fError - Error time for buffered checking
	//
	// Note: An action is down if all its buttons are down
	////////////////////////////////////////////////////
	virtual bool IsActionDown(char const* szName, float fError = 0.0f) const = 0;
	virtual bool IsActionDown(int nID, float fError = 0.0f) const = 0;

protected:
	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize to prepare for button status
	//	reports
	//
	// In:	pRemote - Owning remote controller
	//
	// Returns TRUE on success, FALSE on error
	////////////////////////////////////////////////////
	virtual bool Initialize(IWR_WiiRemote *pRemote) = 0;

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Clean up
	////////////////////////////////////////////////////
	virtual void Shutdown(void) = 0;

	////////////////////////////////////////////////////
	// OnButtonUpdate
	//
	// Purpose: Call when buttons are updated on the remote
	//
	// In:	buffer - Recv buffer containing button
	//			status
	//		nOffset - Offset into buffer where data is
	//		fCurrTick - Current tick time
	////////////////////////////////////////////////////
	virtual void OnButtonUpdate(struct DataBuffer const& buffer, int nOffset, float fCurrTick) = 0;

	////////////////////////////////////////////////////
	// OnPostUpdate
	//
	// Purpose: Finalize update for buffered input
	//
	// Note: Should be called at end of Update irregardless
	//	if OnButtonUpdate is called
	////////////////////////////////////////////////////
	virtual void OnPostUpdate(void) = 0;
};

#endif //_WR_IWIIBUTTONS_H_