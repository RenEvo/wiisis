////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiData.h
//
// Purpose: Helper for the Wii Remote that manages
//	data transfers to/from flash memory on controller
//
// History:
//	- 11/3/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_CWIIDATA_H_
#define _WR_CWIIDATA_H_

#include "Interfaces\WR_IWiiData.h"

class CWR_WiiData : public IWR_WiiData
{
	friend class CWR_WiiRemote;

protected:
	CWR_WiiRemote *m_pRemote;
	bool m_bWasUpdated;

	// Callback map
	WiiIOCallBackMap m_CallBackmap;

	// Listeners
	typedef std::list<IWR_WiiDataListener*> Listeners;
	Listeners m_Listeners;

public:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWR_WiiData(void);
private:
	CWR_WiiData(CWR_WiiData const&) {}
	CWR_WiiData& operator =(CWR_WiiData&) { return *this; }

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWR_WiiData(void);

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
	virtual bool Initialize(IWR_WiiRemote *pRemote);

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Clean up
	////////////////////////////////////////////////////
	virtual void Shutdown(void);

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
	virtual void OnDataRead(DataBuffer const& buffer, int nOffset, float fCurrTick);

	////////////////////////////////////////////////////
	// OnPostUpdate
	//
	// Purpose: Finalize update
	//
	// Note: Should be called at end of Update irregardless
	//	if OnDataRead is called
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
	virtual void AddListener(IWR_WiiDataListener *pListener);

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiDataListener *pListener);

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
	virtual void ReadData(int nAddr, int nSize, WiiIOCallBack pCallBack = NULL, WiiIOCallBackParam pParam = NULL);

	////////////////////////////////////////////////////
	// WriteData
	//
	// Purpose: Write data to the Wii Remote
	//
	// In:	nAddr - Address where to write data to
	//		nSize - Size (in bytes) of data to write
	//		pData - Data to write
	////////////////////////////////////////////////////
	virtual void WriteData(int nAddr, int nSize, const LPWiiIOData pData) const;
};

#endif //_WR_CWIIDATA_H_