////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CHIDController.h
//
// Purpose: HID Bluetooth controller which
//			detects the remote's device
//
// History:
//	- 11/1/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_CHIDCONTROLLER_H_
#define _WR_CHIDCONTROLLER_H_

#include "Interfaces\WR_IHIDController.h"
#include "Interfaces\WR_IWiiRemote.h"

class CWR_HIDController : public IWR_HIDController
{
	SETUP_WR_MODULE();

protected:
	GUID m_GUID;			// HID GUID

	// Controller map
	RemoteMap m_Remotes;
	RemoteFoundMap m_FoundRemotes;
	int m_nFoundCount;

	// Listeners
	typedef std::list<IWR_HIDControllerListener*> Listeners;
	Listeners m_Listeners;

public:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWR_HIDController(void);
private:
	CWR_HIDController(CWR_HIDController const&) {}
	CWR_HIDController& operator =(CWR_HIDController const&) {return *this;}

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWR_HIDController(void);

	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize the HID system
	//
	// Returns error status (see WR_HIDCONTROLLER_ERROR)
	////////////////////////////////////////////////////
	virtual int Initialize(void);

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Clean up
	////////////////////////////////////////////////////
	virtual void Shutdown(void);

	////////////////////////////////////////////////////
	// ClearFoundRemotes
	//
	// Purpose: Clear all remotes found so far
	////////////////////////////////////////////////////
	virtual void ClearFoundRemotes(void);

	////////////////////////////////////////////////////
	// GetFoundRemoteCount
	//
	// Purpose: Return how many remotes have been found
	////////////////////////////////////////////////////
	virtual int GetFoundRemoteCount(void);

	////////////////////////////////////////////////////
	// InitializeRemote
	//
	// Purpose: Initialize the given remote
	//
	// In:	nID - Remote ID
	//
	// Returns remote object or NULL on error
	////////////////////////////////////////////////////
	virtual IWR_WiiRemote* InitializeRemote(int nID);

	////////////////////////////////////////////////////
	// ShutdownRemotes
	//
	// Purpose: Destroy all connected remotes
	////////////////////////////////////////////////////
	virtual void ShutdownRemotes(void);

	////////////////////////////////////////////////////
	// UpdateRemotes
	//
	// Purpose: Update all remotes
	////////////////////////////////////////////////////
	virtual void UpdateRemotes(void);

	////////////////////////////////////////////////////
	// PoolRemoteDevices
	//
	// Purpose: Pool for the Wii Remote device
	//
	// In:	nCount - Number of controllers to attempt
	//				 to find
	//		fTimeout - Timeout value (0 for async)
	//
	// Returns number of devices found
	////////////////////////////////////////////////////
	virtual int PoolRemoteDevices(int nCount = 1, float fTimeout = 0);

	////////////////////////////////////////////////////
	// GetRemote
	//
	// Purpose: Return the remote with the given ID
	//
	// In:	nID - Remote ID
	//
	// Returns remote object or NULL if invalid ID
	////////////////////////////////////////////////////
	virtual IWR_WiiRemote *GetRemote(RemoteID const& nID) const;

	////////////////////////////////////////////////////
	// GetRemotes
	//
	// Purpose: Returns map of remotes
	//
	// Out:	remotes - Map of remotes
	//
	// Returns TRUE if remotes were returned, FALSE if not
	////////////////////////////////////////////////////
	virtual bool GetRemotes(RemoteMap &remotes) const;

	////////////////////////////////////////////////////
	// GetRemoteCount
	//
	// Purpose: Returns how many remotes have been found
	////////////////////////////////////////////////////
	virtual int GetRemoteCount(void) const;

	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_HIDControllerListener *pListener);

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_HIDControllerListener *pListener);
};

#endif //_WR_CHIDCONTROLLER_H_