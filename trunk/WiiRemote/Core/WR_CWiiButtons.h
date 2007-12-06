////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiButtons.h
//
// Purpose: Helper for the Wii Remote that manages
//	button input
//
// History:
//	- 11/3/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_CWIIBUTTONS_H_
#define _WR_CWIIBUTTONS_H_

#include "Interfaces\WR_IWiiButtons.h"

class CWR_WiiButtons : public IWR_WiiButtons
{
	friend class CWR_WiiRemote;

protected:
	CWR_WiiRemote *m_pRemote;
	bool m_bWasUpdated;

	// Button status
	unsigned int m_nButtons;
	int m_pButtonStatus[WR_WIIREMOTE_BUTTONS_MAX];
	float m_pButtonBufferedTime[WR_WIIREMOTE_BUTTONS_MAX];

	// Actions
	ActionID m_nActionIDSeed;
	ActionMap m_ActionMap;

	// Listeners
	typedef std::list<IWR_WiiButtonsListener*> Listeners;
	Listeners m_Listeners;

public:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWR_WiiButtons(void);
private:
	CWR_WiiButtons(CWR_WiiButtons const&) {}
	CWR_WiiButtons& operator =(CWR_WiiButtons&) { return *this; }

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWR_WiiButtons(void);

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
	virtual bool Initialize(IWR_WiiRemote *pRemote);

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Clean up
	////////////////////////////////////////////////////
	virtual void Shutdown(void);

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
	virtual void OnButtonUpdate(DataBuffer const& buffer, int nOffset, float fCurrTick);

	////////////////////////////////////////////////////
	// OnPostUpdate
	//
	// Purpose: Finalize update for buffered input
	//
	// Note: Should be called at end of Update irregardless
	//	if OnButtonUpdate is called
	////////////////////////////////////////////////////
	virtual void OnPostUpdate(void);

public:
	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_WiiButtonsListener *pListener);

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiButtonsListener *pListener);

	////////////////////////////////////////////////////
	// EnableBufferedInput
	//
	// Purpose: Turn buffered input on. Continuous update
	//	must be set for buffered input to work on the remote
	//
	// In:	bEnable - TRUE to enable it, FALSE to disable it
	////////////////////////////////////////////////////
	virtual void EnableBufferedInput(bool bEnable);

	////////////////////////////////////////////////////
	// IsBufferedInputEnabled
	//
	// Purpose: Returns TRUE if buffered input is enabled
	//	or FALSE if it is disabled
	//
	// Note: Checks if continuous update is enabled on
	//	the remote
	////////////////////////////////////////////////////
	virtual bool IsBufferedInputEnabled(void) const;

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
	virtual int GetButtonStatus(unsigned int nButtonMask, float fError = 0.0f) const;

	////////////////////////////////////////////////////
	// IsButtonDown
	//
	// Purpose: Returns TRUE if the given button is down
	//
	// In:	nButtonMask - Buttons to check (see WR_WIIREMOTE_BUTTONS)
	//		fError - Error time for buffered checking
	////////////////////////////////////////////////////
	virtual bool IsButtonDown(unsigned int nButtonMask, float fError = 0.0f) const;

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
	virtual int RegisterAction(char const* szName, unsigned int nButtonMask);

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
	virtual int GetActionStatus(char const* szName, float fError = 0.0f) const;
	virtual int GetActionStatus(int nID, float fError = 0.0f) const;

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
	virtual bool IsActionDown(char const* szName, float fError = 0.0f) const;
	virtual bool IsActionDown(int nID, float fError = 0.0f) const;
};

#endif //_WR_CWIIBUTTONS_H_