////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_IWiiRemote.h
//
// Purpose: Interface object
//	Describes a Wii Remote communicator
//
// History:
//	- 11/2/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_IWIIREMOTE_H_
#define _WR_IWIIREMOTE_H_

struct IWR_WiiButtons;
struct IWR_WiiMotion;
struct IWR_WiiData;
struct IWR_WiiExtension;
struct IWR_WiiSensor;

// Remote ID
typedef unsigned char RemoteID;
#define REMOTEID_INVALID (0x00)

// Error codes
enum WR_WIIREMOTE_ERROR
{
	WR_WIIREMOTE_OK	= WR_ERROR_SUCCESS,				// No error occured
	WR_WIIREMOTE_BADINIT,							// Bad initialization
	WR_WIIREMOTE_INVALIDHANDLE,						// Invalid or bad handle to Remote
	WR_WIIREMOTE_WRITETHREADFAIL,					// Failed to create writing thread
	WR_WIIREMOTE_READTHREADFAIL,					// Failed to create reading thread
	WR_WIIREMOTE_BADBUTTONS,						// Failed to initialize button helper
	WR_WIIREMOTE_BADMOTION,							// Failed to initialize motion helper
	WR_WIIREMOTE_BADDATA,							// Failed to initialize data helper
	WR_WIIREMOTE_BADSENSOR,							// Failed to initialize the IR Sensor helper
};
static char const* WR_WIIREMOTE_ERRORSTR[] =
{
	"Success",
	"Bad initialization or already initialized",
	"Invalid/Bad handle to Wii Remote",
	"Failed to create Wii Remote Output (writing) thread",
	"Failed to create Wii Remote Input (reading) thread",
	"Bad initialization of Button Helper",
	"Bad initialization of Motion Helper",
	"Bad initialization of Data Helper",
	"Bad initialization of IR Sensor Helper",
};

// WR_WIIREMOTE_FLAGS
//	Wii remote status flags
enum WR_WIIREMOTE_FLAGS
{
	WRF_CONNECTED = 0x01,			// Set when remote has a connection
	WRF_ATTEMPTCONNECT = 0x02,		// Set when remote is attempting a connection
	WRF_RUMBLEON = 0x04,			// Set when remote rumble is enabled
	WRF_UPDATESTATUS = 0x08,		// Set when remote status updates are requested
	WRF_CONTINUOUSREPORT = 0x10,	// Set when remote is sending back reports continuously
	WRF_CHECKEDEXT = 0x20,			// Set when remote has queried for the plugged-in extension type

	// Remote Status flags
	WRF_STATUS_RUMBLE = 0x100,		// Set when rumble is enabled on the remote
	WRF_STATUS_EXPANSION = 0x200,	// Set when an expansion is plugged into the remote
	WRF_STATUS_SPEAKER = 0x400,		// Set when the speaker is turned on on the remote
	WRF_STATUS_IR = 0x800,			// Set when the IR sensor is enabled on the remote
	WRF_STATUS_LED1 = 0x1000,		// Set when the LED P1 light is turned on on the remote
	WRF_STATUS_LED2 = 0x2000,		// Set when the LED P2 light is turned on on the remote
	WRF_STATUS_LED3 = 0x4000,		// Set when the LED P3 light is turned on on the remote
	WRF_STATUS_LED4 = 0x8000,		// Set when the LED P4 light is turned on on the remote

	WRF_STATUS_MASK = 0xFF00,		// Status mask
	WRF_STATUS_SHIFT = 8,			// Status bit shift

	// All bits on
	WRF_ALL = 0xFFFF,
};

// WR_WIIREMOTE_REPORT
//	Used to control what input data is sent back from remote
enum WR_WIIREMOTE_REPORT
{
	WR_REPORT_BUTTONS = 0x30,			// Send back button info
	WR_REPORT_MOTION = 0x01,			// Send back motion input
	WR_REPORT_IR = 0x02,				// Send back IR input
	WR_REPORT_EXTENSION = 0x04,			// Send back extension input
	WR_REPORT_BUTTONACCEL = 0x08,		// Send back acceleration info in the buttons

	WR_REPORT_NOIR = (WR_REPORT_BUTTONS|WR_REPORT_MOTION|WR_REPORT_EXTENSION),
	WR_REPORT_ALL = (WR_REPORT_NOIR|WR_REPORT_IR),
	WR_REPORT_DEFAULT = WR_REPORT_BUTTONS,
};
// TODO Extension input bit, send back Extension Enabled data on GetReport

// WR_WIIREMOTE_LED
//	Used to turn LED lights on/off
enum WR_WIIREMOTE_LED
{
	WR_LED_P1 = 0x10,				// Player 1 LED
	WR_LED_P2 = 0x20,				// Player 2 LED
	WR_LED_P3 = 0x40,				// Player 3 LED
	WR_LED_P4 = 0x80,				// Player 4 LED

	WR_LED_NONE = 0x00,				// No lights
	WR_LED_ALL = 0xF0,				// All lights
};

// WR_WIIREMOTE_RUMBLE
//	Used to turn rumble feature on/off
enum WR_WIIREMOTE_RUMBLE
{
	WR_RUMBLE_ON = 0x01,			// Turn rumble on
	WR_RUMBLE_OFF = 0x00,			// Turn rumble off
};

// WR_WIIREMOTE_EXPANSION
//	Used to extract info from the Expansion input (status)
enum WR_WIIREMOTE_EXPANSION
{
	// Status info
	WR_EXPANSION_STATUSBYTE = 3,
	WR_EXPANSION_CONTROLLER = 0x02,	// Expansion controller (byte 3)
	WR_EXPANSION_SPEAKER = 0x04,	// Speaker enabled (byte 3)
	WR_EXPANSION_IR = 0x08,			// IR enabled (byte 3)
	WR_EXPANSION_LED1 = 0x10,		// LED1 on (byte 3)
	WR_EXPANSION_LED2 = 0x20,		// LED2 on (byte 3)
	WR_EXPANSION_LED3 = 0x40,		// LED3 on (byte 3)
	WR_EXPANSION_LED4 = 0x80,		// LED4 on (byte 3)

	// Other parts
	WR_EXPANSION_BATTERYBYTE = 6,
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// maximum payload size
#define WR_MAX_PAYLOAD (22)

// Input operands
enum WR_WIIREMOTE_INPUT_OPS
{
	WR_IN_EXPANSION = 0x20,			// Expansion report. Also returned from STATUS
	WR_IN_INPUTMASK = 0x3F,			// Input information
	WR_IN_DATAREAD = 0x21,			// Data read from Wii Remote
	WR_IN_DATAWROTE = 0x22,			// Data wrote to Wii Remote
};

// Output operands
enum WR_WIIREMOTE_OUTPUT_OPS
{
	WR_OUT_LED = 0x11,				// Control the LEDs
	WR_OUT_REPORT = 0x12,			// Establish connection
	WR_OUT_IR_RUMBLE = 0x13,		// Control IR and rumble features
	WR_OUT_SPEAKER = 0x14,			// Control speaker (enable)
	WR_OUT_SPEAKER_MUTE = 0x19,		// Control speaker (mute)
	WR_OUT_STATUS = 0x15,			// Request remote status
	WR_OUT_WRITEDATA = 0x16,		// Write data to remote
	WR_OUT_READDATA = 0x17,			// Read data from remote
	WR_OUT_IR2 = 0x1A,				// Control IR
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Remote Listener
struct IWR_WiiRemote;
struct IWR_WiiRemoteListener
{
	////////////////////////////////////////////////////
	// OnConnect
	//
	// Purpose: Signaled when remote has made a connection
	//
	// In:	pRemote - Wii Remote controller
	////////////////////////////////////////////////////
	virtual void OnConnect(IWR_WiiRemote *pRemote) = 0;

	////////////////////////////////////////////////////
	// OnDisconnect
	//
	// Purpose: Signaled when remote has lost or failed
	//	to connect
	//
	// In:	pRemote - Wii Remote controller
	//		bAbnormal - TRUE if disconnect was abnormal
	////////////////////////////////////////////////////
	virtual void OnDisconnect(IWR_WiiRemote *pRemote, bool bAbnormal) = 0;

	////////////////////////////////////////////////////
	// OnConnecting
	//
	// Purpose: Signaled when remote is attempting to
	//	connect
	//
	// In:	pRemote - Wii Remote controller
	////////////////////////////////////////////////////
	virtual void OnConnecting(IWR_WiiRemote *pRemote) = 0;

	////////////////////////////////////////////////////
	// OnReportChanged
	//
	// Purpose: Signaled when the reporting method
	//	changes on the Wii Remote
	//
	// In:	premote - Wii Remote controller
	//		nReport - Report method (see WR_WIIREMOTE_REPORT)
	//		bContinuous - TRUE if continuous reporting is
	//			enabled, FALSE if it is not
	////////////////////////////////////////////////////
	virtual void OnReportChanged(IWR_WiiRemote *pRemote, int nReport, bool bContinuous) = 0;

	////////////////////////////////////////////////////
	// OnStatusUpdate
	//
	// Purpose: Signaled when the status of the remote
	//	has changed, such as rumble or LED lights
	//
	// In:	pRemote - Wii Remote controller
	//		nStatus - Status flags (see WR_WIIREMOTE_FLAGS)
	//		nBattery - Battery life
	////////////////////////////////////////////////////
	virtual void OnStatusUpdate(IWR_WiiRemote *pRemote, int nStatus, int nBattery) = 0;

	////////////////////////////////////////////////////
	// OnExtensionPluggedIn
	//
	// Purpose: Signaled when an extension is plugged in
	//
	// In:	pRemote - Wii Remote controller
	//		pExtension - Extension helper
	////////////////////////////////////////////////////
	virtual void OnExtensionPluggedIn(IWR_WiiRemote *premote, IWR_WiiExtension *pExtension) = 0;

	////////////////////////////////////////////////////
	// OnExtensionUnplugged
	//
	// Purpose: Signaled when an extension is unplugged
	//
	// In:	pRemote - Wii Remote controller
	//		pExtension - Extension helper
	////////////////////////////////////////////////////
	virtual void OnExtensionUnplugged(IWR_WiiRemote *premote, IWR_WiiExtension *pExtension) = 0;
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// DataBuffer - Used for I/O data transmissions
struct DataBuffer
{
	unsigned char data[WR_MAX_PAYLOAD];
	DataBuffer(void) { memset(&data, 0x00, WR_MAX_PAYLOAD); }
	unsigned char& operator [](int n) { return data[n]; }
	unsigned char const& operator [](int n) const { return data[n]; }
	operator LPVOID(void) { return data; }
	operator LPCVOID(void) const { return data; }
	operator DWORD(void) const { return WR_MAX_PAYLOAD; }
	operator int(void) const { return WR_MAX_PAYLOAD; }
};
typedef std::queue<DataBuffer> DataQueue;

// Wii remote interface
struct IWR_WiiRemote
{
protected:
	////////////////////////////////////////////////////
	// CreateButtonHelper
	//
	// Purpose: Called when Button helper is created
	//
	// Note: See IWR_WiiButtons interface
	//		 Overload to create your own
	////////////////////////////////////////////////////
	virtual IWR_WiiButtons* CreateButtonHelper(void) const = 0;

	////////////////////////////////////////////////////
	// CreateMotionHelper
	//
	// Purpose: Called when Motion helper is created
	//
	// Note: See IWR_WiiMotion interface
	//		 Overload to create your own
	////////////////////////////////////////////////////
	virtual IWR_WiiMotion* CreateMotionHelper(void) const = 0;

	////////////////////////////////////////////////////
	// CreateDataHelper
	//
	// Purpose: Called when Data helper is created
	//
	// Note: See IWR_WiiData interface
	//		 Overload to create your own
	////////////////////////////////////////////////////
	virtual IWR_WiiData* CreateDataHelper(void) const = 0;

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
	virtual IWR_WiiExtension* CreateExtensionHelper(int nType) const = 0;

	////////////////////////////////////////////////////
	// CreateSensorHelper
	//
	// Purpose: Called when Sensor helper is created
	//
	// Note: See IWR_WiiSensor interface
	//		 Overload to create your own
	////////////////////////////////////////////////////
	virtual IWR_WiiSensor* CreateSensorHelper(void) const = 0;

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~IWR_WiiRemote(void) {}

	////////////////////////////////////////////////////
	// GetButtonHelper
	//
	// Purpose: Return the button helper
	//
	// Note: See IWR_WiiButtons
	////////////////////////////////////////////////////
	virtual IWR_WiiButtons* GetButtonHelper(void) const = 0;

	////////////////////////////////////////////////////
	// GetMotionHelper
	//
	// Purpose: Return the motion helper
	//
	// Note: See IWR_WiiMotion
	////////////////////////////////////////////////////
	virtual IWR_WiiMotion* GetMotionHelper(void) const = 0;

	////////////////////////////////////////////////////
	// GetDataHelper
	//
	// Purpose: Return the data helper
	//
	// Note: See IWR_WiiData
	////////////////////////////////////////////////////
	virtual IWR_WiiData* GetDataHelper(void) const = 0;

	////////////////////////////////////////////////////
	// GetExtensionHelper
	//
	// Purpose: Return the extension helper
	//
	// Note: See IWR_WiiExtension
	////////////////////////////////////////////////////
	virtual IWR_WiiExtension* GetExtensionHelper(void) const = 0;

	////////////////////////////////////////////////////
	// GetSensorHelper
	//
	// Purpose: Return the Sensor helper
	//
	// Note: See IWR_WiiSensor
	////////////////////////////////////////////////////
	virtual IWR_WiiSensor* GetSensorHelper(void) const = 0;

	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_WiiRemoteListener *pListener) = 0;

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiRemoteListener *pListener) = 0;

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
	virtual int Initialize(char const* szDevicePath, RemoteID const& nID) = 0;

	////////////////////////////////////////////////////
	// Reconnect
	//
	// Purpose: Attempt to reconnect the controller
	//
	// In:	bKeepReport - TRUE to keep the current
	//			report choice, FALSE to use default
	////////////////////////////////////////////////////
	virtual void Reconnect(bool bKeepReport = true) = 0;

	////////////////////////////////////////////////////
	// IsConnecting
	//
	// Purpose: Returns TRUE if remote is attempting to
	//	connect
	////////////////////////////////////////////////////
	virtual bool IsConnecting(void) const = 0;

	////////////////////////////////////////////////////
	// IsConnected
	//
	// Purpose: Returns TRUE if remote is connected
	////////////////////////////////////////////////////
	virtual bool IsConnected(void) const = 0;

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Clean up
	////////////////////////////////////////////////////
	virtual void Shutdown(void) = 0;

	////////////////////////////////////////////////////
	// Update
	//
	// Purpose: Update the Wii Remote controller
	////////////////////////////////////////////////////
	virtual void Update(void) = 0;

	////////////////////////////////////////////////////
	// Reset
	//
	// Purpose: Reset the controller to initial status
	////////////////////////////////////////////////////
	virtual void Reset(void) = 0;

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
	virtual void SetReport(int nReport, bool bContinuous = false) = 0;

	////////////////////////////////////////////////////
	// TriggerContinuousReport
	//
	// Purpose: Trigger continuous report while maintaining
	//	the currently selected report method
	//		
	//	In: bContinuous - TRUE if report should be sent
	//			back continuously
	////////////////////////////////////////////////////
	virtual void TriggerContinuousReport(bool bContinuous) = 0;

	////////////////////////////////////////////////////
	// GetReport
	//
	// Purpose: Get the report choice for the Wii remote
	//
	// Note: See WR_WIIREMOTE_REPORT
	////////////////////////////////////////////////////
	virtual unsigned int GetReport(void) const = 0;

	////////////////////////////////////////////////////
	// IsContinuousReport
	//
	// Purpose: Returns TRUE if reporting is occuring
	//	continuously or FALSE if only when status changes
	////////////////////////////////////////////////////
	virtual bool IsContinuousReport(void) const = 0;

	////////////////////////////////////////////////////
	// SetStatusUpdate
	//
	// Purpose: Set how often status updates should be
	//	queried by the remote
	//
	// In:	fFreq - Status update frequency
	////////////////////////////////////////////////////
	virtual void SetStatusUpdate(float fFreq) = 0;

	////////////////////////////////////////////////////
	// RequestStatusUpdate
	//
	// Purpose: Request a status update now
	////////////////////////////////////////////////////
	virtual void RequestStatusUpdate(void) = 0;

	////////////////////////////////////////////////////
	// SetLEDs
	//
	// Purpose: Turn on/off LED lights
	//
	// In:	nOnLEDs - Lights to turn on (see WR_WIIREMOTE_LED)
	////////////////////////////////////////////////////
	virtual void SetLEDs(int nOnLEDs) = 0;

	////////////////////////////////////////////////////
	// SetRumble
	//
	// Purpose: Turn on/off the Rumble feature
	//
	// In:	bOn - TRUE to turn on, FALSE to turn off
	////////////////////////////////////////////////////
	virtual void SetRumble(bool bOn) = 0;

	////////////////////////////////////////////////////
	// IsRumbleOn
	//
	// Purpose: Returns TRUE if Rumble feature is on
	////////////////////////////////////////////////////
	virtual bool IsRumbleOn(void) const = 0;

	////////////////////////////////////////////////////
	// GetBatteryLife
	//
	// Purpose: Return the battery life of the remote
	////////////////////////////////////////////////////
	virtual unsigned int GetBatteryLife(void) const = 0;

	////////////////////////////////////////////////////
	// GetID
	//
	// Purpose: Returns remote ID
	////////////////////////////////////////////////////
	virtual RemoteID const& GetID(void) const = 0;

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
	virtual bool CheckFlags(unsigned int nFlags) const = 0;

	////////////////////////////////////////////////////
	// SetConnectionTimeout
	//
	// Purpose: Set how long before timing out on the
	//	connection attempt
	//
	// In:	fTimeout - Timeout value
	////////////////////////////////////////////////////
	virtual void SetConnectionTimeout(float fTimeout) = 0;

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
	virtual void SetFlags(unsigned int nFlags, bool bOn = true) = 0;

	////////////////////////////////////////////////////
	// GetRumbleBit
	//
	// Purpose: Return the rumble bit based on rumble
	//			status
	////////////////////////////////////////////////////
	virtual unsigned char GetRumbleBit(void) const = 0;

	////////////////////////////////////////////////////
	// UpdateStatus
	//
	// Purpose: Update the status of the controller based
	//	on input WR_IN_EXPANSION info
	////////////////////////////////////////////////////
	virtual void UpdateStatus(DataBuffer const& buffer) = 0;

	////////////////////////////////////////////////////
	// WriteData
	//
	// Purpose: Write data to the write queue for
	//	processing by the WRO thread
	//
	// In:	data - Data buffer to write
	////////////////////////////////////////////////////
	virtual void WriteData(DataBuffer const& data) = 0;
};

#endif //_WR_IWIIREMOTE_H_