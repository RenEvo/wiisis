////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_IWiiExtension.h
//
// Purpose: Interface object
//	Describes an extension wrapper for the Wii Remote
//
// History:
//	- 11/12/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_IWIIEXTENSION_H_
#define _WR_IWIIEXTENSION_H_

// Extension IDs
enum WR_WIIEXTENSION_ID
{
	WR_EXTENSION_NONE = 0x0000,			// No extension
	WR_EXTENSION_ERROR = 0xFFFF,		// Bad extension (may be partially inserted)
	WR_EXTENSION_NUNCHUK = 0xFEFE,		// The Nunchuk's ID
	WR_EXTENSION_CLASSIC = 0xFDFD,		// The Classic Controller's ID
};

// Extension-specific read locations
enum WR_WIIEXTENSION_DATALOC
{
	// Register location. Write 0x00 here to init the extension
	WR_EXTENSION_REGISTERLOC = 0x04a40040,

	// Where to find the type of extension plugged in (see WR_WIIEXTENSION_ID)
	WR_EXTENSION_TYPELOC = 0x04a400fe,
	WR_EXTENSION_TYPESIZE = 2,
};

// Update IDs
enum WR_WIIEXTENSION_UPDATE
{
	WR_EXTENSION_UPDATE_REPORT = 1,		// Sending info back from Wii Remote's extension input report
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Extension Listener
struct IWR_WiiExtension;
struct IWR_WiiExtensionListener
{
	////////////////////////////////////////////////////
	// OnExtensionButton
	//
	// Purpose: Signaled when a button has been altered
	//	on the extension remote
	//
	// In:	pRemote - Wii Remote controller
	//		pExtension - Extension object
	//		nButton - Button that was altered
	//			(see WR_NUNCHUK_BUTTONS)
	//		nStatus - Button status
	//			(see WR_WIIREMOTE_BUTTONSTATUS)
	//		bDown - TRUE if button is down, FALSE if it
	//			is up
	////////////////////////////////////////////////////
	virtual void OnExtensionButton(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension,
		unsigned int nButton, int nStatus, bool bDown) = 0;

	////////////////////////////////////////////////////
	// OnExtensionAction
	//
	// Purpose: Signaled when an action has been altered
	//
	// In:	pRemote - Wii Remote controller
	//		pExtension - Extension object
	//		szAction - Action name
	//		nActionID - ID of the action
	//		nStatus - Button status
	//			(see WR_WIIREMOTE_BUTTONSTATUS)
	//		bDown - TRUE if button is down, FALSE if it
	//			is up
	////////////////////////////////////////////////////
	virtual void OnExtensionAction(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension,
		char const* szAction, ActionID nActionID, int nStatus, bool bDown) = 0;

	////////////////////////////////////////////////////
	// OnExtensionSingleMotion
	//
	// Purpose: Called when the controller has moved
	//
	// In:	pRemote - Controller object
	//		pExtension - Extension object
	//		motion - Motion element
	////////////////////////////////////////////////////
	virtual void OnExtensionSingleMotion(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion) = 0;

	////////////////////////////////////////////////////
	// OnExtensionMotionStart
	//
	// Purpose: Called when a motion has started
	//
	// In:	pRemote - Controller object
	//		pExtension - Extension object
	//		motion - Motion element
	////////////////////////////////////////////////////
	virtual void OnExtensionMotionStart(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion) = 0;

	////////////////////////////////////////////////////
	// OnExtensionMotionUpdate
	//
	// Purpose: Called when a motion has been updated
	//
	// In:	pRemote - Controller object
	//		pExtension - Extension object
	//		motion - Motion element
	////////////////////////////////////////////////////
	virtual void OnExtensionMotionUpdate(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion) = 0;

	////////////////////////////////////////////////////
	// OnExtensionMotionEnd
	//
	// Purpose: Called when a motion has ended
	//
	// In:	pRemote - Controller object
	//		pExtension - Extension object
	//		motion - Motion element
	////////////////////////////////////////////////////
	virtual void OnExtensionMotionEnd(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion) = 0;

	////////////////////////////////////////////////////
	// OnExtensionAnalogUpdate
	//
	// Purpose: Called when an analog stick is updated
	//
	// In:	pRemote - Controller object
	//		pExtension - Extension object
	//		nStickID - ID of the stick
	//		fX - Analog stick's X position
	//		fY - Analog stick's Y position
	////////////////////////////////////////////////////
	virtual void OnExtensionAnalogUpdate(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, int nStickID, float fX, float fY) = 0;
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

struct IWR_WiiExtension
{
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~IWR_WiiExtension(void) { }

	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_WiiExtensionListener *pListener) = 0;

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiExtensionListener *pListener) = 0;

	////////////////////////////////////////////////////
	// GetType
	//
	// Purpose: Return extension type ID
	////////////////////////////////////////////////////
	virtual int GetType(void) const = 0;

	////////////////////////////////////////////////////
	// GetName
	//
	// Purpose: Return extension name
	////////////////////////////////////////////////////
	virtual char const* GetName(void) const = 0;

	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize the extension
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
	// OnUpdate
	//
	// Purpose: Call to update the extension
	//
	// In:	nID - Update ID (what is being updated)
	//		buffer - Recv buffer containing motion
	//			status
	//		nOffset - Offset into buffer where data is
	//		fCurrTick - Current tick time
	////////////////////////////////////////////////////
	virtual void OnUpdate(int nID, DataBuffer const& buffer, int nOffset, float fCurrTick) = 0;

	////////////////////////////////////////////////////
	// OnPostUpdate
	//
	// Purpose: Finalize update
	//
	// Note: Should be called at end of Update
	////////////////////////////////////////////////////
	virtual void OnPostUpdate(void) = 0;
};

#endif //_WR_IWIIEXTENSION_H_