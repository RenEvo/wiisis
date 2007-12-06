////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiSensor.h
//
// Purpose: Helper for the Wii Remote that manages
//	IR sensor readings/updating
//
// History:
//	- 11/23/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_CWIISENSOR_H_
#define _WR_CWIISENSOR_H_

#include "Interfaces\WR_IWiiSensor.h"

class CWR_WiiSensor : public IWR_WiiSensor
{
	friend class CWR_WiiRemote;

protected:
	CWR_WiiRemote *m_pRemote;
	bool m_bWasUpdated;
	bool m_bEnabled;

	// Raw dot data
	struct tSensorDot
	{
		bool bTargeted;
		int nRawX;
		int nRawY;
	} m_SensorDots[4];
	float m_fLastOnScreenTime;

	// Cursor data
	bool m_bOnScreen;
	float m_fX;
	float m_fY;

	// Listeners
	typedef std::list<IWR_WiiSensorListener*> Listeners;
	Listeners m_Listeners;

public:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWR_WiiSensor(void);
private:
	CWR_WiiSensor(CWR_WiiSensor const&) {}
	CWR_WiiSensor& operator =(CWR_WiiSensor&) { return *this; }

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWR_WiiSensor(void);

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
	virtual bool Initialize(IWR_WiiRemote *pRemote);

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Clean up
	////////////////////////////////////////////////////
	virtual void Shutdown(void);

	////////////////////////////////////////////////////
	// OnSetReport
	//
	// Purpose: Called when report type is about to change
	//
	// In:	nMode - Report type
	////////////////////////////////////////////////////
	virtual void OnSetReport(int nMode);

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
	virtual void OnSensorUpdate(struct DataBuffer const& buffer, int nOffset, float fCurrTick);

	////////////////////////////////////////////////////
	// OnPostUpdate
	//
	// Purpose: Finalize update for sensor input
	//
	// Note: Should be called at end of Update irregardless
	//	if OnSensorUpdate is called
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
	virtual void AddListener(IWR_WiiSensorListener *pListener);

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiSensorListener *pListener);

	////////////////////////////////////////////////////
	// IsEnabled
	//
	// Purpose: Returns TRUE if enabled, FALSE on error
	////////////////////////////////////////////////////
	virtual bool IsEnabled(void) const;
};

#endif //_WR_CWIISENSOR_H_