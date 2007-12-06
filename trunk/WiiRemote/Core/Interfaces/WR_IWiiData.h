////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_IWiiData.h
//
// Purpose: Interface object
//	Describes a helper for the Wii Remote that manages
//	data transfers to/from flash memory on controller
//
// History:
//	- 11/7/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_IWIIDATA_H_
#define _WR_IWIIDATA_H_

struct IWR_WiiRemote;
struct IWR_WiiData;

// Data I/O Error Codes
enum WR_DATA_ERROR
{
	WR_DATAERROR_SUCCESS = 0,					// No error occured
	WR_DATAERROR_BADREAD = 8,					// Attempted to read from data that is not there
	WR_DATAERROR_WRITEONLY = 7,					// Attempted to read from write-only memory
};

// Common read/write points
enum WR_DATA_READADDR
{
	WR_DATAREAD_REMOTE_CALIBRATION = 0x16,		// Where calibration data is stored on Wii Remote
	WR_DATAREAD_REMOTE_CALIBRATION_SIZE = 8,
};
enum WR_DATA_WRITEADDR
{

};

// Read/Write Callback buffer
typedef unsigned char WiiIOData;
typedef unsigned char* LPWiiIOData;
typedef void* WiiIOCallBackParam;
typedef void (*WiiIOCallBack)(int nAddr, int nSize, LPWiiIOData pData, WiiIOCallBackParam pParam);
typedef std::pair<WiiIOCallBack, WiiIOCallBackParam> WiiIOCallBackEntry;
typedef std::list<WiiIOCallBackEntry> WiiIOCallBackList;
typedef std::map<int, WiiIOCallBackList> WiiIOCallBackMap;

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Data Listener
struct IWR_WiiDataListener
{
	////////////////////////////////////////////////////
	// OnDataRead
	//
	// Purpose: Called when data is read from the remote
	//
	// In:	pRemote - Wii Remote controller
	//		pHelper - Data helper
	//		nAddr - Data location
	//		nSize - Size of data that was read
	//		pData - Data that was read
	////////////////////////////////////////////////////
	virtual void OnDataRead(IWR_WiiRemote *pRemote, IWR_WiiData *pHelper, int nAddr, int nSize, LPWiiIOData pData) = 0;

	////////////////////////////////////////////////////
	// OnDataError
	//
	// Purpose: Called when an error occured during the
	//	last I/O process
	//
	// In:	pRemote - Wii Remote controller
	//		pHelper - Data helper
	//		nAddr - Data location
	//		nError - Error code (see WR_DATA_ERROR)
	////////////////////////////////////////////////////
	virtual void OnDataError(IWR_WiiRemote *pRemote, IWR_WiiData *pHelper, int nAddr, int nError) = 0;
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

struct IWR_WiiData
{
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~IWR_WiiData(void) { }

	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_WiiDataListener *pListener) = 0;

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiDataListener *pListener) = 0;

	////////////////////////////////////////////////////
	// ReadData
	//
	// Purpose: Read data from the Wii Remote
	//
	// In:	nAddr - Address where to read data from
	//		nSize - Size (in bytes) to read
	//		pCallBack - Optional callback function to call
	//			when this data has been read
	//		pParam - Optional param data to pass along
	//			to callback
	////////////////////////////////////////////////////
	virtual void ReadData(int nAddr, int nSize, WiiIOCallBack pCallBack = NULL, WiiIOCallBackParam pParam = NULL) = 0;

	////////////////////////////////////////////////////
	// WriteData
	//
	// Purpose: Write data to the Wii Remote
	//
	// In:	nAddr - Address where to write data to
	//		nSize - Size (in bytes) of data to write
	//		pData - Data to write
	////////////////////////////////////////////////////
	virtual void WriteData(int nAddr, int nSize, const LPWiiIOData pData) const = 0;

protected:
	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize to prepare for data read/writes
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
	// OnDataRead
	//
	// Purpose: Call when data is coming in from the remote
	//
	// In:	buffer - Recv buffer containing motion
	//			status
	//		nOffset - Offset into buffer where data is
	//		fCurrTick - Current tick time
	////////////////////////////////////////////////////
	virtual void OnDataRead(DataBuffer const& buffer, int nOffset, float fCurrTick) = 0;

	////////////////////////////////////////////////////
	// OnPostUpdate
	//
	// Purpose: Finalize update
	//
	// Note: Should be called at end of Update irregardless
	//	if OnDataRead is called
	////////////////////////////////////////////////////
	virtual void OnPostUpdate(void) = 0;
};

#endif //_WR_IWIIDATA_H_