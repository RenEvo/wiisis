////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiMotion.cpp
//
// Purpose: Helper for the Wii Remote that manages
//	motion detection
//
// History:
//	- 11/3/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "WR_Implementation.h"
#include "WR_CWiiMotion.h"
#include "WR_CWiiRemote.h"

////////////////////////////////////////////////////
CWR_WiiMotion::CWR_WiiMotion(void)
{
	m_pRemote = NULL;
	m_bWasUpdated = false;
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	SetMotionSize(0);
	m_nFlags = 0;
	m_nCurrMotionLifetime = 0;
}

////////////////////////////////////////////////////
CWR_WiiMotion::~CWR_WiiMotion(void)
{
	Shutdown();
}

////////////////////////////////////////////////////
bool CWR_WiiMotion::Initialize(IWR_WiiRemote *pRemote)
{
	m_pRemote = (CWR_WiiRemote*)pRemote;
	if (NULL == m_pRemote) return false;

	m_nFlags = 0;

	return true;
}

////////////////////////////////////////////////////
void CWR_WiiMotion::Shutdown(void)
{

}

////////////////////////////////////////////////////
void CWR_WiiMotion::Calibrate(void)
{
	// Read calibration data
	m_pRemote->GetDataHelper()->ReadData(WR_DATAREAD_REMOTE_CALIBRATION, WR_DATAREAD_REMOTE_CALIBRATION_SIZE,
		OnCalibrateData, this);
}

////////////////////////////////////////////////////
bool CWR_WiiMotion::IsCalibrated(void) const
{
	return CHECK_BITS(WMF_ISCALIBRATED,m_nFlags);
}

////////////////////////////////////////////////////
void CWR_WiiMotion::OnCalibrateData(int nAddr, int nSize, LPWiiIOData pData, WiiIOCallBackParam pParam)
{
	CWR_WiiMotion *pMotion = (CWR_WiiMotion*)pParam;
	if (WR_DATAREAD_REMOTE_CALIBRATION != nAddr) return;

	// Extract calibration data
	pMotion->m_vCalibration_ZeroPoint.x = *(pData+0);
	pMotion->m_vCalibration_ZeroPoint.y = *(pData+1);
	pMotion->m_vCalibration_ZeroPoint.z = *(pData+2);
	pMotion->m_vCalibration_1G.x = *(pData+4);
	pMotion->m_vCalibration_1G.y = *(pData+5);
	pMotion->m_vCalibration_1G.z = *(pData+6);

	// Calculate ratio values
	pMotion->m_vCalibration_Ratio.x = 1.0f / ((float)pMotion->m_vCalibration_1G.x - pMotion->m_vCalibration_ZeroPoint.x);
	pMotion->m_vCalibration_Ratio.y = 1.0f / ((float)pMotion->m_vCalibration_1G.y - pMotion->m_vCalibration_ZeroPoint.y);
	pMotion->m_vCalibration_Ratio.z = 1.0f / ((float)pMotion->m_vCalibration_1G.z - pMotion->m_vCalibration_ZeroPoint.z);

	// Set calibrated bit
	pMotion->m_nFlags = SET_BITS(WMF_ISCALIBRATED,pMotion->m_nFlags);
}

////////////////////////////////////////////////////
void CWR_WiiMotion::OnMotionUpdate(DataBuffer const& buffer, int nOffset, float fCurrTick)
{
	assert(m_pRemote);

	m_bWasUpdated = true;

	// Get motion data (acceleration)
	SMotionVec3F vPrev(m_vAccel);
	m_vAccel.x = (float)buffer[nOffset+0]-m_vCalibration_ZeroPoint.x;
	m_vAccel.y = (float)buffer[nOffset+1]-m_vCalibration_ZeroPoint.y;
	m_vAccel.z = (float)buffer[nOffset+2]-m_vCalibration_ZeroPoint.z;
	if (true == CHECK_BITS(WMF_ISCALIBRATED,m_nFlags)) m_vAccel *= m_vCalibration_Ratio;

	// Determine orientation update
	float fLengthSq = (m_vAccel.x*m_vAccel.x)+(m_vAccel.y*m_vAccel.y)+(m_vAccel.z*m_vAccel.z);
	if (fLengthSq >= 1.0f-WR_MOTION_1GEPSILON && fLengthSq <= 1.0f+WR_MOTION_1GEPSILON)
	{
		// Normalize acceleration vector to get orientation direction
		m_vDir = m_vAccel * (1.0f/sqrtf(fLengthSq));

		// Update rotation orientation
		m_fPitch = -asinf(NEGSATURATE(m_vDir.y));
		m_fRoll = asinf(NEGSATURATE(m_vDir.x));

		// Convert to normalized units
		if (m_vDir.z < 0.0f)
		{
			m_fPitch = (m_vDir.y<0.0f)?(PI-m_fPitch):(-PI-m_fPitch);
			m_fRoll  = (m_vDir.x<0.0f)?(-PI-m_fRoll):(PI-m_fRoll);
		}
	}

	// Create an element
	SMotionElement element;
	element.vAccel = m_vAccel;
	element.vDir = m_vDir;
	element.fGForce = fLengthSq;
	element.fPitch = m_fPitch;
	element.fRoll = m_fRoll;
	element.nLifetime = ++m_nCurrMotionLifetime;

	// Report that the motion has been updated
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		(*itI)->OnSingleMotion(m_pRemote, this, element);

	// Was there a change?
	if (fabs(vPrev.x-m_vAccel.x) <= WR_MOTION_GESTUREEPSILON &&
		fabs(vPrev.y-m_vAccel.y) <= WR_MOTION_GESTUREEPSILON &&
		fabs(vPrev.z-m_vAccel.z) <= WR_MOTION_GESTUREEPSILON)
	{
		// If there was an active motion, report the end
		if (true == CHECK_BITS(WMF_ACTIVEMOTION, m_nFlags))
		{
			// Report the end
			for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
				(*itI)->OnMotionEnd(m_pRemote, this, element);
		}

		StopMotion();
		return;
	}

	// If motion is not active, add to queue
	if (false == CHECK_BITS(WMF_ACTIVEMOTION, m_nFlags))
	{
		// Add to queue and check if we have passed the min
		m_MotionQueue.push(element);
		if ((int)m_MotionQueue.size() <= m_nMinMotionSize) return;

		// Report what is in the queue
		SMotionElement const& front = m_MotionQueue.front(); m_MotionQueue.pop();
		for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
			(*itI)->OnMotionStart(m_pRemote, this, front);
		while (false == m_MotionQueue.empty())
		{
			SMotionElement const& next = m_MotionQueue.front(); m_MotionQueue.pop();
			for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
				(*itI)->OnMotionUpdate(m_pRemote, this, next);
		}

		// Set flag
		m_nFlags = SET_BITS(WMF_ACTIVEMOTION, m_nFlags);
	}
	else
	{
		// Just report it
		for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
			(*itI)->OnMotionUpdate(m_pRemote, this, element);
	}
}

////////////////////////////////////////////////////
void CWR_WiiMotion::StopMotion(void)
{
	// Empty the queue
	while (false == m_MotionQueue.empty()) m_MotionQueue.pop();
	m_nCurrMotionLifetime = 0;

	// Set flag
	m_nFlags = CLEAR_BITS(WMF_ACTIVEMOTION, m_nFlags);
}

////////////////////////////////////////////////////
void CWR_WiiMotion::OnPostUpdate(void)
{
	m_bWasUpdated = false;
}

////////////////////////////////////////////////////
void CWR_WiiMotion::SetMotionSize(int nSize)
{
	m_nMinMotionSize = MAX(nSize, 10);
}

////////////////////////////////////////////////////
int CWR_WiiMotion::GetMotionSize(void) const
{
	return m_nMinMotionSize;
}

////////////////////////////////////////////////////
bool CWR_WiiMotion::IsMotionActive(void) const
{
	return CHECK_BITS(WMF_ACTIVEMOTION, m_nFlags);
}

////////////////////////////////////////////////////
int CWR_WiiMotion::GetMotionLifetime(void) const
{
	return m_nCurrMotionLifetime;
}

////////////////////////////////////////////////////
void CWR_WiiMotion::AddListener(IWR_WiiMotionListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		if (*itI == pListener)
			return;
	m_Listeners.push_back(pListener);
}

////////////////////////////////////////////////////
void CWR_WiiMotion::RemoveListener(IWR_WiiMotionListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		if (*itI == pListener)
		{
			m_Listeners.erase(itI);
			return;
		}
}

////////////////////////////////////////////////////
void CWR_WiiMotion::GetAcceleration(SMotionVec3F &v) const
{
	v = m_vAccel;
}

////////////////////////////////////////////////////
float CWR_WiiMotion::GetAccelerationX(void) const
{
	return m_vAccel.x;
}

////////////////////////////////////////////////////
float CWR_WiiMotion::GetAccelerationY(void) const
{
	return m_vAccel.y;
}

////////////////////////////////////////////////////
float CWR_WiiMotion::GetAccelerationZ(void) const
{
	return m_vAccel.z;
}

////////////////////////////////////////////////////
void CWR_WiiMotion::GetDirection(SMotionVec3F &v) const
{
	v = m_vDir;
}

////////////////////////////////////////////////////
float CWR_WiiMotion::GetDirectionX(void) const
{
	return m_vDir.x;
}

////////////////////////////////////////////////////
float CWR_WiiMotion::GetDirectionY(void) const
{
	return m_vDir.y;
}

////////////////////////////////////////////////////
float CWR_WiiMotion::GetDirectionZ(void) const
{
	return m_vDir.z;
}

////////////////////////////////////////////////////
float CWR_WiiMotion::GetPitch(void) const
{
	return m_fPitch;
}

////////////////////////////////////////////////////
float CWR_WiiMotion::GetRoll(void) const
{
	return m_fRoll;
}