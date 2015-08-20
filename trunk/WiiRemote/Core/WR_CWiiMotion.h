////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiMotion.h
//
// Purpose: Helper for the Wii Remote that manages
//	motion detection
//
// History:
//	- 11/3/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_CWIIMOTION_H_
#define _WR_CWIIMOTION_H_

#include "Interfaces\WR_IWiiMotion.h"

class CWR_WiiMotion : public IWR_WiiMotion
{
	friend class CWR_WiiRemote;

protected:
	CWR_WiiRemote *m_pRemote;
	int m_nFlags;
	bool m_bWasUpdated;

	// Calibration data
	SMotionVec3 m_vCalibration_ZeroPoint;
	SMotionVec3 m_vCalibration_1G;
	SMotionVec3F m_vCalibration_Ratio;

	// Current motion values
	SMotionVec3F m_vAccel;
	SMotionVec3F m_vDir;
	float m_fPitch, m_fRoll;

	// Motion queue
	int m_nMinMotionSize;
	int m_nCurrMotionLifetime;
	WiiMotionQueue m_MotionQueue;

	// Listeners
	typedef std::list<IWR_WiiMotionListener*> Listeners;
	Listeners m_Listeners;

public:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWR_WiiMotion(void);
private:
	CWR_WiiMotion(CWR_WiiMotion const&) {}
	CWR_WiiMotion& operator =(CWR_WiiMotion&) { return *this; }

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWR_WiiMotion(void);

protected:
	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize to prepare for button status
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
	// OnMotionUpdate
	//
	// Purpose: Call when motion is updated on the remote
	//
	// In:	buffer - Recv buffer containing motion
	//			status
	//		nOffset - Offset into buffer where data is
	//		fCurrTick - Current tick time
	////////////////////////////////////////////////////
	virtual void OnMotionUpdate(DataBuffer const& buffer, int nOffset, float fCurrTick);

	////////////////////////////////////////////////////
	// OnPostUpdate
	//
	// Purpose: Finalize update
	//
	// Note: Should be called at end of Update irregardless
	//	if OnMotionUpdate is called
	////////////////////////////////////////////////////
	virtual void OnPostUpdate(void);

	////////////////////////////////////////////////////
	// OnCalibrateData
	//
	// Purpose: Callback for when calibration data is read
	////////////////////////////////////////////////////
	static void OnCalibrateData(int nAddr, int nSize, LPWiiIOData pData, WiiIOCallBackParam pParam);

	////////////////////////////////////////////////////
	// StopMotion
	//
	// Purpose: Clean up and report the end of the
	//	current motion
	////////////////////////////////////////////////////
	virtual void StopMotion(void);

public:
	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_WiiMotionListener *pListener);

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiMotionListener *pListener);

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
};

#endif //_WR_CWIIMOTION_H_