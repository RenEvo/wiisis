////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiNunchuk.h
//
// Purpose: Nunchuk extension for the Wii Remote.
//	Complete with its own motion and button data
//
// History:
//	- 11/12/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_CWIINUNCHUK_H_
#define _WR_CWIINUNCHUK_H_

#include "Interfaces\WR_IWiiExtension.h"

// Location of where calibration data is stored
#define WR_NUNCHUK_CALIBRATION_LOC (0x04a40020)
#define WR_NUNCHUK_CALIBRATION_SIZE (16)

// WR_NUNCHUK_OFFSETS
//	Location into buffer where data is for the Nunchuk
enum WR_NUNCHUK_OFFSETS
{
	WR_NCDATA_ANALOG_X = 0,		// X analog stick
	WR_NCDATA_ANALOG_Y = 1,		// Y analog stick
	WR_NCDATA_BUTTONS = 5,		// Button status
	WR_NCDATA_MOTION_X = 2,		// X motion
	WR_NCDATA_MOTION_Y = 3,		// Y motion
	WR_NCDATA_MOTION_Z = 4,		// Z motion
};

// WR_NUNCHUK_BUTTONS
//	Mapping of buttons on the Wii Nunchuk
enum WR_NUNCHUK_BUTTONS
{
	WR_NCBUTTON_Z = 0x01,			// 'Z' button
	WR_NCBUTTON_C = 0x02,			// 'C' button
};

// WR_NUNCHUK_BUTTONS_INDEX
//	Index value of buttons
enum WR_NUNCHUK_BUTTONS_INDEX
{
	WR_NCBUTTONINDEX_Z = 0,
	WR_NCBUTTONINDEX_C,

	// Max
	WR_NUNCHUK_BUTTONS_MAX,
};
static const unsigned int WR_NUNCHUK_BUTTONS_INDEX_VALUE[WR_NUNCHUK_BUTTONS_MAX] =
{
	WR_NCBUTTON_Z, WR_NCBUTTON_C,
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// TODO Analog stick accessors
class CWR_WiiNunchuk : public IWR_WiiExtension
{
protected:
	// Parent remote
	IWR_WiiRemote *m_pRemote;
	int m_nFlags;
	bool m_bWasUpdated;

	// Button status
	unsigned int m_nButtons;
	int m_pButtonStatus[WR_NUNCHUK_BUTTONS_MAX];
	float m_pButtonBufferedTime[WR_NUNCHUK_BUTTONS_MAX];

	// Actions
	ActionID m_nActionIDSeed;
	ActionMap m_ActionMap;

	// Calibration data
	SMotionVec3 m_vCalibration_ZeroPoint;
	SMotionVec3 m_vCalibration_1G;
	SMotionVec3F m_vCalibration_Ratio;
	SMotionVec3 m_vAnalogCalibration_Max; // Z = X's center
	SMotionVec3 m_vAnalogCalibration_Min; // Z = Y's center
	SMotionVec3F m_vAnalogCalibration_Ratio;

	// Current motion values
	SMotionVec3F m_vAccel;
	SMotionVec3F m_vDir;
	float m_fPitch, m_fRoll;
	
	// Analog stick
	float m_fAnalogX, m_fAnalogY;

	// Motion queue
	int m_nMinMotionSize;
	int m_nCurrMotionLifetime;
	WiiMotionQueue m_MotionQueue;

	// Listeners
	typedef std::list<IWR_WiiExtensionListener*> Listeners;
	Listeners m_Listeners;

public:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWR_WiiNunchuk(void);
private:
	CWR_WiiNunchuk(CWR_WiiNunchuk const&) {}
	CWR_WiiNunchuk& operator =(CWR_WiiNunchuk const&) {return *this;}

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWR_WiiNunchuk(void);

	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_WiiExtensionListener *pListener);

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiExtensionListener *pListener);

	////////////////////////////////////////////////////
	// GetType
	//
	// Purpose: Return extension type ID
	////////////////////////////////////////////////////
	static const int TYPE = WR_EXTENSION_NUNCHUK;
	virtual int GetType(void) const;

	////////////////////////////////////////////////////
	// GetName
	//
	// Purpose: Return extension name
	////////////////////////////////////////////////////
	virtual char const* GetName(void) const;

	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize the extension
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
	virtual void OnUpdate(int nID, DataBuffer const& buffer, int nOffset, float fCurrTick);

	////////////////////////////////////////////////////
	// OnPostUpdate
	//
	// Purpose: Finalize update
	//
	// Note: Should be called at end of Update
	////////////////////////////////////////////////////
	virtual void OnPostUpdate(void);

public:
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
	// In:	nButtonMask - Buttons to check (see WR_NUNCHUK_BUTTONS)
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
	// In:	nButtonMask - Buttons to check (see WR_NUNCHUK_BUTTONS)
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
	//			(see WR_NUNCHUK_BUTTONS)
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

	////////////////////////////////////////////////////
	// Calibrate
	//
	// Purpose: Call to calibrate the motion
	////////////////////////////////////////////////////
	virtual void Calibrate(void);

	////////////////////////////////////////////////////
	// IsCalibrated
	//
	// Purpose: Returns TRUE if the remote has been
	//	calibrated yet
	////////////////////////////////////////////////////
	virtual bool IsCalibrated(void) const;

	////////////////////////////////////////////////////
	// SetMotionSize
	//
	// Purpose: Set the minimum size (consecutive updates)
	//	for a motion to be reported
	//
	// In:	nSize - Size (in number of consecutive updates)
	////////////////////////////////////////////////////
	virtual void SetMotionSize(int nSize);

	////////////////////////////////////////////////////
	// GetMotionSize
	//
	// Purpose: Get the minimum size (consecutive updates)
	//	for a motion to be reported
	////////////////////////////////////////////////////
	virtual int GetMotionSize(void) const;

	////////////////////////////////////////////////////
	// IsMotionActive
	//
	// Purpose: Returns TRUE if a motion is active
	////////////////////////////////////////////////////
	virtual bool IsMotionActive(void) const;

	////////////////////////////////////////////////////
	// GetMotionLifetime
	//
	// Returns the current motion's lifetime or 0 if no
	//	motion is active right now
	////////////////////////////////////////////////////
	virtual int GetMotionLifetime(void) const;

	////////////////////////////////////////////////////
	// GetAcceleration
	//
	// Purpose: Get the acceleration vector on the remote
	//	as it was on the last update
	//
	// Out: v - Acceleration vector
	////////////////////////////////////////////////////
	virtual void GetAcceleration(SMotionVec3F &v) const;

	////////////////////////////////////////////////////
	// GetAccelerationXYZ
	//
	// Purpose: Get a component of the acceleration vector
	////////////////////////////////////////////////////
	virtual float GetAccelerationX(void) const;
	virtual float GetAccelerationY(void) const;
	virtual float GetAccelerationZ(void) const;

	////////////////////////////////////////////////////
	// GetDirection
	//
	// Purpose: Get the direction vector on the remote
	//	as it was on the last update
	//
	// Out: v - Direction vector
	////////////////////////////////////////////////////
	virtual void GetDirection(SMotionVec3F &v) const;

	////////////////////////////////////////////////////
	// GetDirectionXYZ
	//
	// Purpose: Get a component of the direction vector
	////////////////////////////////////////////////////
	virtual float GetDirectionX(void) const;
	virtual float GetDirectionY(void) const;
	virtual float GetDirectionZ(void) const;

	////////////////////////////////////////////////////
	// GetPitch
	//
	// Purpose: Get pitch component of rotation
	////////////////////////////////////////////////////
	virtual float GetPitch(void) const;

	////////////////////////////////////////////////////
	// GetRoll
	//
	// Purpose: Get roll component of rotation
	////////////////////////////////////////////////////
	virtual float GetRoll(void) const;

	////////////////////////////////////////////////////
	// GetAnalog
	//
	// Purpose: Get the analog stick vector on the remote
	//	as it was on the last update
	//
	// Out: v - Analog vector
	////////////////////////////////////////////////////
	virtual void GetAnalog(SMotionVec3F &v) const;

	////////////////////////////////////////////////////
	// GetAnalogX
	//
	// Purpose: Return the analog stick's position in
	//	the X axis
	////////////////////////////////////////////////////
	virtual float GetAnalogX(void) const;

	////////////////////////////////////////////////////
	// GetAnalogY
	//
	// Purpose: Return the analog stick's position in
	//	the Y axis
	////////////////////////////////////////////////////
	virtual float GetAnalogY(void) const;

protected:
	////////////////////////////////////////////////////
	// OnCalibrateData
	//
	// Purpose: Callback for when calibration data is read
	////////////////////////////////////////////////////
	static void OnCalibrateData(int nAddr, int nSize, LPWiiIOData pData, WiiIOCallBackParam pParam);

	////////////////////////////////////////////////////
	// DecryptBuffer
	//
	// Purpose: Take the data in the buffer and decrypt
	//	it
	//
	// In:	buffer - Buffer to decrypt
	//
	// Out: out - Decrypted buffer
	////////////////////////////////////////////////////
	virtual void DecryptBuffer(DataBuffer const& buffer, DataBuffer &out);

	////////////////////////////////////////////////////
	// StopMotion
	//
	// Purpose: Clean up and report the end of the
	//	current motion
	////////////////////////////////////////////////////
	virtual void StopMotion(void);
};

#endif //_WR_CWIINUNCHUK_H_