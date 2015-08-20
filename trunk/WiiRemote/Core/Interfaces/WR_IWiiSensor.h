////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_IWiiSensor.h
//
// Purpose: Interface object
//	Describes a helper for the Wii Remote that manages
//	IR sensor readings/updating
//
// History:
//	- 11/23/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_IWIISENSOR_H_
#define _WR_IWIISENSOR_H_

// WR_WIISENSOR_MODE
//	Possible modes used for IR sensor input
enum WR_WIISENSOR_MODE
{
	WR_SENSORMODE_POS = 0x01,		// Report only pos data (10 bytes)
	WR_SENSORMODE_POSSIZE = 0x02,	// Report pos and size data (12 bytes, cannot be used with extension)
	WR_SENSORMODE_ALL = 0x04,		// Report everything (16 bytes, cannot be used with motion or extension)
};

// WR_WIISENSOR_REG
//	Registery locations for Sensor initialization
enum WR_WIISENSOR_REG
{
	WR_SENSORREG_MASTER = 0x04b00030,				// Must write 0x01 first
	WR_SENSORREG_SENSITIVITY_1 = 0x04b00000,		// Set 9-byte sensitivity data
	WR_SENSORREG_SENSITIVITY_2 = 0x04b0001a,		// Set 2-byte sensitivity data
	WR_SENSORREG_MODE = 0x04b00033,					// Set mode (see WR_WIISENSOR_MODE)
};

// Camera dimensions
#define WR_WIISENSOR_DZ (8.0f) // Deadzone value
#define WR_WIISENSOR_MAX_X (1024.0f-WR_WIISENSOR_DZ)
#define WR_WIISENSOR_MAX_Y (768.0f-WR_WIISENSOR_DZ)

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Sensor Listener
struct IWR_WiiRemote;
struct IWR_WiiSensor;
struct IWR_WiiSensorListener
{
	////////////////////////////////////////////////////
	// OnEnterScreen
	//
	// Purpose: Called when cursor enters the screen after
	//	being off for at least one frame
	//
	// In:	pRemote - Wii Remote controller
	//		pSensor - Wii Remote Sensor helper
	//		fX - Cursor X position
	//		fY - Cursor Y position
	////////////////////////////////////////////////////
	virtual void OnEnterScreen(IWR_WiiRemote *pRemote, IWR_WiiSensor *pSensor, float fX, float fY) = 0;

	////////////////////////////////////////////////////
	// OnLeaveScreen
	//
	// Purpose: Called when cursor leaves the screen after
	//	being on for at least one frame
	//
	// In:	pRemote - Wii Remote controller
	//		pSensor - Wii Remote Sensor helper
	//		fX - Cursor X position
	//		fY - Cursor Y position
	////////////////////////////////////////////////////
	virtual void OnLeaveScreen(IWR_WiiRemote *pRemote, IWR_WiiSensor *pSensor, float fX, float fY) = 0;

	////////////////////////////////////////////////////
	// OnCursorUpdate
	//
	// Purpose: Called when cursor is updated and is on
	//	the screen
	//
	// In:	pRemote - Wii Remote controller
	//		pSensor - Wii Remote Sensor helper
	//		fX - Cursor X position
	//		fY - Cursor Y position
	////////////////////////////////////////////////////
	virtual void OnCursorUpdate(IWR_WiiRemote *pRemote, IWR_WiiSensor *pSensor, float fX, float fY) = 0;
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

struct IWR_WiiSensor
{
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~IWR_WiiSensor(void) { }

	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_WiiSensorListener *pListener) = 0;

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiSensorListener *pListener) = 0;

	////////////////////////////////////////////////////
	// IsEnabled
	//
	// Purpose: Returns TRUE if enabled, FALSE on error
	////////////////////////////////////////////////////
	virtual bool IsEnabled(void) const = 0;

protected:
	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize to prepare for sensor status
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
	// OnSetReport
	//
	// Purpose: Called when report type is about to change
	//
	// In:	nMode - Report type
	////////////////////////////////////////////////////
	virtual void OnSetReport(int nMode) = 0;

	////////////////////////////////////////////////////
	// OnSensorUpdate
	//
	// Purpose: Call when sensor readings are picked up
	//	from the remote
	//
	// In:	buffer - Recv buffer containing sensor
	//			status
	//		nOffset - Offset into buffer where data is
	//		fCurrTick - Current tick time
	////////////////////////////////////////////////////
	virtual void OnSensorUpdate(struct DataBuffer const& buffer, int nOffset, float fCurrTick) = 0;

	////////////////////////////////////////////////////
	// OnPostUpdate
	//
	// Purpose: Finalize update for sensor input
	//
	// Note: Should be called at end of Update irregardless
	//	if OnSensorUpdate is called
	////////////////////////////////////////////////////
	virtual void OnPostUpdate(void) = 0;
};

#endif //_WR_IWIISENSOR_H_