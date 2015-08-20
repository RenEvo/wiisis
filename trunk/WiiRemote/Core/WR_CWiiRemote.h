////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiRemote.h
//
// Purpose: Wii Remote communicator
//
// History:
//	- 11/2/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_CWIIREMOTE_H_
#define _WR_CWIIREMOTE_H_

#include "Interfaces\WR_IWiiRemote.h"
class CWR_WiiRemote : public IWR_WiiRemote
{
	SETUP_WR_MODULE();
	
	friend class CWR_WiiButtons;
	friend class CWR_WiiMotion;
	friend class CWR_WiiData;
	friend class CWR_WiiSensor;

protected:
	unsigned int m_nBattery;		// Last known battery life
	unsigned int m_nReportMode;		// Current report mode
	unsigned int m_nFlags;			// Status flags (see WR_WIIREMOTE_FLAGS)
	RemoteID m_nID;					// remote ID
	HANDLE m_hHandle;				// Remote handle

	// Helpers
	CWR_WiiButtons *m_pButtons;		// Button input helper
	CWR_WiiMotion *m_pMotion;		// Motion input helper
	CWR_WiiData *m_pData;			// Data helper
	CWR_WiiSensor *m_pSensor;		// IR Sensor helper

	// Extension helpder
	IWR_WiiExtension *m_pExtension;

	// Connection values
	float m_fAttemptConnectStart;
	float m_fLastRecv;
	float m_fConnectionTimeout;
	float m_fStatusUpdateFreq;
	float m_fNextStatusUpdate;

	// Writing thread information
	HANDLE m_hWROThread;
	OVERLAPPED m_hWROOverlap;
	HANDLE m_hWRODataReadEvent;
	unsigned int m_dwWROThreadID;

	// Reading thread information
	HANDLE m_hWRIThread;
	unsigned int m_dwWRIThreadID;

	// Listeners
	typedef std::list<IWR_WiiRemoteListener*> Listeners;
	Listeners m_Listeners;

protected:
	////////////////////////////////////////////////////
	// CreateButtonHelper
	//
	// Purpose: Called when Button helper is created
	//
	// Note: See IWR_WiiButtons interface
	//		 Overload to create your own
	////////////////////////////////////////////////////
	virtual IWR_WiiButtons* CreateButtonHelper(void) const;

	////////////////////////////////////////////////////
	// CreateMotionHelper
	//
	// Purpose: Called when Motion helper is created
	//
	// Note: See IWR_WiiMotion interface
	//		 Overload to create your own
	////////////////////////////////////////////////////
	virtual IWR_WiiMotion* CreateMotionHelper(void) const;

	////////////////////////////////////////////////////
	// CreateDataHelper
	//
	// Purpose: Called when Data helper is created
	//
	// Note: See IWR_WiiData interface
	//		 Overload to create your own
	////////////////////////////////////////////////////
	virtual IWR_WiiData* CreateDataHelper(void) const;

	////////////////////////////////////////////////////
	// CreateExtensionHelper
	//
	// Purpose: Called when an Extension helper is
	//	created for a given extension type
	//
	// In:	nType - Extension type (see WR_WIIEXTENSION_ID)
	//
	// Note: See IWR_WiiExtension interface
	//		 Overload to create your won
	////////////////////////////////////////////////////
	virtual IWR_WiiExtension* CreateExtensionHelper(int nType) const;

	////////////////////////////////////////////////////
	// CreateSensorHelper
	//
	// Purpose: Called when Sensor helper is created
	//
	// Note: See IWR_WiiSensor interface
	//		 Overload to create your own
	////////////////////////////////////////////////////
	virtual IWR_WiiSensor* CreateSensorHelper(void) const;


public:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWR_WiiRemote(void);
private:
	CWR_WiiRemote(CWR_WiiRemote&) {}
	CWR_WiiRemote& operator =(CWR_WiiRemote&) {return *this;}

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWR_WiiRemote(void);

public:
	////////////////////////////////////////////////////
	// GetButtonHelper
	//
	// Purpose: Return the button helper
	//
	// Note: See IWR_WiiButtons
	////////////////////////////////////////////////////
	virtual IWR_WiiButtons* GetButtonHelper(void) const;

	////////////////////////////////////////////////////
	// GetMotionHelper
	//
	// Purpose: Return the motion helper
	//
	// Note: See IWR_WiiMotion
	////////////////////////////////////////////////////
	virtual IWR_WiiMotion* GetMotionHelper(void) const;

	////////////////////////////////////////////////////
	// GetDataHelper
	//
	// Purpose: Return the data helper
	//
	// Note: See IWR_WiiData
	////////////////////////////////////////////////////
	virtual IWR_WiiData* GetDataHelper(void) const;

	////////////////////////////////////////////////////
	// GetExtensionHelper
	//
	// Purpose: Return the extension helper
	//
	// Note: See IWR_WiiExtension
	////////////////////////////////////////////////////
	virtual IWR_WiiExtension* GetExtensionHelper(void) const;

	////////////////////////////////////////////////////
	// GetSensorHelper
	//
	// Purpose: Return the Sensor helper
	//
	// Note: See IWR_WiiSensor
	////////////////////////////////////////////////////
	virtual IWR_WiiSensor* GetSensorHelper(void) const;

	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_WiiRemoteListener *pListener);

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiRemoteListener *pListener);

	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize the remote
	//
	// In:	szDevicePath - Remote device path
	//		nID - Remote ID
	//
	// Returns error (see WR_WIIREMOTE_ERROR)
	////////////////////////////////////////////////////
	virtual int Initialize(char const* szDevicePath, RemoteID const& nID);

	////////////////////////////////////////////////////
	// Reconnect
	//
	// Purpose: Attempt to reconnect the controller
	//
	// In:	bKeepReport - TRUE to keep the current
	//			report choice, FALSE to use default
	////////////////////////////////////////////////////
	virtual void Reconnect(bool bKeepReport = true);

	////////////////////////////////////////////////////
	// IsConnecting
	//
	// Purpose: Returns TRUE if remote is attempting to
	//	connect
	////////////////////////////////////////////////////
	virtual bool IsConnecting(void) const;

	////////////////////////////////////////////////////
	// IsConnected
	//
	// Purpose: Returns TRUE if remote is connected
	////////////////////////////////////////////////////
	virtual bool IsConnected(void) const;

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Clean up
	////////////////////////////////////////////////////
	virtual void Shutdown(void);

	////////////////////////////////////////////////////
	// Update
	//
	// Purpose: Update the Wii Remote controller
	////////////////////////////////////////////////////
	virtual void Update(void);

	////////////////////////////////////////////////////
	// Reset
	//
	// Purpose: Reset the controller to initial status
	////////////////////////////////////////////////////
	virtual void Reset(void);

	////////////////////////////////////////////////////
	// SetReport
	//
	// Purpose: Set the report choice for the Wii Remote
	//
	// In:	nReport - Report choice (see WR_WIIREMOTE_REPORT)
	//		bContinuous - TRUE if report should be sent
	//			back continuously
	//		bExtension - TRUE if extension reporting should
	//			be sent back continuously
	////////////////////////////////////////////////////
	virtual void SetReport(int nReport, bool bContinuous = false);
protected:
	// Special cause - Used on init and reconnect
	virtual void SetReport_Connect(int nReport);
public:

	////////////////////////////////////////////////////
	// TriggerContinuousReport
	//
	// Purpose: Trigger continuous report while maintaining
	//	the currently selected report method
	//		
	//	In: bContinuous - TRUE if report should be sent
	//			back continuously
	////////////////////////////////////////////////////
	virtual void TriggerContinuousReport(bool bContinuous);

	////////////////////////////////////////////////////
	// GetReport
	//
	// Purpose: Get the report choice for the Wii remote
	//
	// Note: See WR_WIIREMOTE_REPORT
	////////////////////////////////////////////////////
	virtual unsigned int GetReport(void) const;

	////////////////////////////////////////////////////
	// IsContinuousReport
	//
	// Purpose: Returns TRUE if reporting is occuring
	//	continuously or FALSE if only when status changes
	////////////////////////////////////////////////////
	virtual bool IsContinuousReport(void) const;

	////////////////////////////////////////////////////
	// SetStatusUpdate
	//
	// Purpose: Set how often status updates should be
	//	queried by the remote
	//
	// In:	fFreq - Status update frequency
	////////////////////////////////////////////////////
	virtual void SetStatusUpdate(float fFreq);

	////////////////////////////////////////////////////
	// RequestStatusUpdate
	//
	// Purpose: Request a status update now
	////////////////////////////////////////////////////
	virtual void RequestStatusUpdate(void);

	////////////////////////////////////////////////////
	// SetLEDs
	//
	// Purpose: Turn on/off LED lights
	//
	// In:	nOnLEDs - Lights to turn on (see WR_LED)
	////////////////////////////////////////////////////
	virtual void SetLEDs(int nOnLEDs);

	////////////////////////////////////////////////////
	// SetRumble
	//
	// Purpose: Turn on/off the Rumble feature
	//
	// In:	bOn - TRUE to turn on, FALSE to turn off
	////////////////////////////////////////////////////
	virtual void SetRumble(bool bOn);

	////////////////////////////////////////////////////
	// IsRumbleOn
	//
	// Purpose: Returns TRUE if Rumble feature is on
	////////////////////////////////////////////////////
	virtual bool IsRumbleOn(void) const;

	////////////////////////////////////////////////////
	// GetBatteryLife
	//
	// Purpose: Return the battery life of the remote
	////////////////////////////////////////////////////
	virtual unsigned int GetBatteryLife(void) const;

	////////////////////////////////////////////////////
	// GetID
	//
	// Purpose: Returns remote ID
	////////////////////////////////////////////////////
	virtual RemoteID const& GetID(void) const;

	////////////////////////////////////////////////////
	// CheckFlags
	//
	// Purpose: Check to see if the given status flags
	//	are on or off
	//
	// In:	nFlags - Flags to check for
	//
	// Returns TRUE if flags are on, FALSE if they are off
	////////////////////////////////////////////////////
	virtual bool CheckFlags(unsigned int nFlags) const;

	////////////////////////////////////////////////////
	// SetConnectionTimeout
	//
	// Purpose: Set how long before timing out on the
	//	connection attempt
	//
	// In:	fTimeout - Timeout value
	////////////////////////////////////////////////////
	virtual void SetConnectionTimeout(float fTimeout);

protected:
	////////////////////////////////////////////////////
	// SetFlags
	//
	// Purpose: Set status flags on/off
	//
	// In:	nFlags - Flags to trigger
	//		bOn - TRUE to turn them on, FALSE to turn
	//			  them off
	////////////////////////////////////////////////////
	virtual void SetFlags(unsigned int nFlags, bool bOn = true);

	////////////////////////////////////////////////////
	// GetRumbleBit
	//
	// Purpose: Return the rumble bit based on rumble
	//			status
	////////////////////////////////////////////////////
	virtual unsigned char GetRumbleBit(void) const;

	////////////////////////////////////////////////////
	// UpdateStatus
	//
	// Purpose: Update the status of the controller based
	//	on input WR_IN_EXPANSION info
	////////////////////////////////////////////////////
	virtual void UpdateStatus(DataBuffer const& buffer);

	////////////////////////////////////////////////////
	// OnDataRead
	//
	// Purpose: Call when data is being read from remote
	//
	// In:	buffer - Data read
	////////////////////////////////////////////////////
	virtual void OnDataRead(DataBuffer const& buffer);

	////////////////////////////////////////////////////
	// WriteData
	//
	// Purpose: Write data to the write queue for
	//	processing by the WRO thread
	//
	// In:	data - Data buffer to write
	////////////////////////////////////////////////////
	virtual void WriteData(DataBuffer const& data);

	////////////////////////////////////////////////////
	// WROThreadProc
	//
	// Purpose: Wii Remote Output (writing) thread
	//	procedure
	//
	// In:	pThis - Pointer to wii remote controller
	//
	// Returns non-zero on error
	////////////////////////////////////////////////////
	static unsigned int __stdcall WROThreadProc(void *pThis);
	CRITICAL_SECTION _WROCS;
	volatile bool _bWROThreadProcTerminate;
	DataQueue _WriteQueue;

	////////////////////////////////////////////////////
	// WRIThreadProc
	//
	// Purpose: Wii Remote Input (reading) thread
	//	procedure
	//
	// In:	pThis - Pointer to wii remote controller
	//
	// Returns non-zero on error
	////////////////////////////////////////////////////
	static unsigned int __stdcall WRIThreadProc(void *pThis);
	CRITICAL_SECTION _WRICS;
	volatile bool _bWRIThreadProcTerminate;
	DataQueue _ReadQueue;
};

#endif //_WR_CWIIREMOTE_H_