////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_IHIDController.h
//
// Purpose: Interface object
//	Describes the HID Bluetooth controller which
//	detects the remote's device
//
// History:
//	- 11/1/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_IHIDCONTROLLER_H_
#define _WR_IHIDCONTROLLER_H_

#include "WR_IWiiRemote.h"

// Error codes
enum WR_HIDCONTROLLER_ERROR
{
	WR_HIDCONTROLLER_OK	= WR_ERROR_SUCCESS,			// No error occured
	WR_HIDCONTROLLER_NODEVICES,						// No HID device list was returned
	WR_HIDCONTROLLER_UPDATETHREADFAIL,				// Failed to create update thread
};
static char const* WR_HIDCONTROLLER_ERRORSTR[] =
{
	"Success",
	"No HID device list was returned",
	"Failed to create Update Thread",
};

#define MAX_REMOTES 4
#define WII_REMOTE_P1	(1)
#define WII_REMOTE_P2	(2)
#define WII_REMOTE_P3	(3)
#define WII_REMOTE_P4	(4)
typedef IWR_WiiRemote* RemoteMap[MAX_REMOTES];
typedef std::string RemoteFoundMap[MAX_REMOTES];

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// HID Listener
struct IWR_HIDControllerListener
{
	////////////////////////////////////////////////////
	// OnFoundRemoteDevice
	//
	// Purpose: Called when a device has been found
	//
	// In:	szDevice - Path to device
	//		hHandle - Handle to device
	////////////////////////////////////////////////////
	virtual void OnFoundRemoteDevice(char const* szDevice, HANDLE hHandle) = 0;

	////////////////////////////////////////////////////
	// OnRemoteInitialized
	//
	// Purpose: Signaled when remote has been initialized
	//
	// In:	pRemote - Wii Remote controller
	//		nID - Wii Remote ID
	////////////////////////////////////////////////////
	virtual void OnRemoteInitialized(IWR_WiiRemote* pRemote, RemoteID nID) = 0;
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

struct IWR_HIDController
{
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~IWR_HIDController(void) {}

	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize the HID system
	//
	// Returns error status (see WR_HIDCONTROLLER_ERROR)
	////////////////////////////////////////////////////
	virtual int Initialize(void) = 0;

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Clean up
	////////////////////////////////////////////////////
	virtual void Shutdown(void) = 0;

	////////////////////////////////////////////////////
	// ClearFoundRemotes
	//
	// Purpose: Clear all remotes found so far
	////////////////////////////////////////////////////
	virtual void ClearFoundRemotes(void) = 0;

	////////////////////////////////////////////////////
	// GetFoundRemoteCount
	//
	// Purpose: Return how many remotes have been found
	////////////////////////////////////////////////////
	virtual int GetFoundRemoteCount(void) = 0;

	////////////////////////////////////////////////////
	// InitializeRemote
	//
	// Purpose: Initialize the given remote
	//
	// In:	nID - Remote ID
	//
	// Returns remote object or NULL on error
	////////////////////////////////////////////////////
	virtual IWR_WiiRemote* InitializeRemote(int nID) = 0;

	////////////////////////////////////////////////////
	// ShutdownRemotes
	//
	// Purpose: Destroy all connected remotes
	////////////////////////////////////////////////////
	virtual void ShutdownRemotes(void) = 0;

	////////////////////////////////////////////////////
	// UpdateRemotes
	//
	// Purpose: Update all remotes
	////////////////////////////////////////////////////
	virtual void UpdateRemotes(void) = 0;

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
	virtual int PoolRemoteDevices(int nCount = 1, float fTimeout = 0) = 0;

	////////////////////////////////////////////////////
	// GetRemote
	//
	// Purpose: Return the remote with the given ID
	//
	// In:	nID - Remote ID
	//
	// Returns remote object or NULL if invalid ID
	////////////////////////////////////////////////////
	virtual IWR_WiiRemote *GetRemote(RemoteID const& nID) const = 0;

	////////////////////////////////////////////////////
	// GetRemotes
	//
	// Purpose: Returns map of remotes
	//
	// Out:	remotes - Map of remotes
	//
	// Returns TRUE if remotes were returned, FALSE if not
	////////////////////////////////////////////////////
	virtual bool GetRemotes(RemoteMap &pRemotes) const = 0;

	////////////////////////////////////////////////////
	// GetRemoteCount
	//
	// Purpose: Returns how many remotes have been found
	////////////////////////////////////////////////////
	virtual int GetRemoteCount(void) const = 0;

	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_HIDControllerListener *pListener) = 0;

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_HIDControllerListener *pListener) = 0;
};

#endif //_WR_IHIDCONTROLLER_H_