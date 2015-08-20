////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiNunchuk.cpp
//
// Purpose: Nunchuk extension for the Wii Remote.
//	Complete with its own motion and button data
//
// History:
//	- 11/12/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "WR_Implementation.h"
#include "WR_CWiiNunchuk.h"

////////////////////////////////////////////////////
CWR_WiiNunchuk::CWR_WiiNunchuk(void)
{
	m_pRemote = NULL;
	m_bWasUpdated = false;

	m_nButtons = 0;
	m_nActionIDSeed = 0;
	memset(m_pButtonStatus, 0, sizeof(int)*WR_NUNCHUK_BUTTONS_MAX);
	memset(m_pButtonBufferedTime, 0, sizeof(float)*WR_NUNCHUK_BUTTONS_MAX);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	SetMotionSize(0);
	m_nFlags = 0;
	m_nCurrMotionLifetime = 0;

	m_fAnalogX = 0.0f;
	m_fAnalogY = 0.0f;
}

////////////////////////////////////////////////////
CWR_WiiNunchuk::~CWR_WiiNunchuk(void)
{
	Shutdown();
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::AddListener(IWR_WiiExtensionListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		if (*itI == pListener)
			return;
	m_Listeners.push_back(pListener);
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::RemoveListener(IWR_WiiExtensionListener *pListener)
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
int CWR_WiiNunchuk::GetType(void) const
{
	return CWR_WiiNunchuk::TYPE;
}

////////////////////////////////////////////////////
char const* CWR_WiiNunchuk::GetName(void) const
{
	return "Wii Nunchuk";
}

////////////////////////////////////////////////////
bool CWR_WiiNunchuk::Initialize(IWR_WiiRemote *pRemote)
{
	m_pRemote = pRemote;
	if (NULL == m_pRemote) return false;

	m_nFlags = 0;

	// TODO Setup analog stick


	// Reset status of buttons
	m_nButtons = 0;
	for (int i = 0; i < WR_NUNCHUK_BUTTONS_MAX; i++)
	{
		m_pButtonStatus[i] = WR_BUTTONSTATUS_UP;
		m_pButtonBufferedTime[i] = 0;
	}

	m_nActionIDSeed = ACTIONID_INVALID;
	return true;
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::OnCalibrateData(int nAddr, int nSize, LPWiiIOData pData, WiiIOCallBackParam pParam)
{
	CWR_WiiNunchuk *pThis = (CWR_WiiNunchuk*)pParam;
	if (NULL == pThis) return;

	// Extract calibration data
	pThis->m_vCalibration_ZeroPoint.x = (*(pData+0)^0x17)+0x17;
	pThis->m_vCalibration_ZeroPoint.y = (*(pData+1)^0x17)+0x17;
	pThis->m_vCalibration_ZeroPoint.z = (*(pData+2)^0x17)+0x17;
	pThis->m_vCalibration_1G.x = (*(pData+4)^0x17)+0x17;
	pThis->m_vCalibration_1G.y = (*(pData+5)^0x17)+0x17;
	pThis->m_vCalibration_1G.z = (*(pData+6)^0x17)+0x17;

	pThis->m_vAnalogCalibration_Max.x = (*(pData+8)^0x17)+0x17;
	pThis->m_vAnalogCalibration_Max.y = (*(pData+11)^0x17)+0x17;
	pThis->m_vAnalogCalibration_Max.z = (*(pData+10)^0x17)+0x17;
	pThis->m_vAnalogCalibration_Min.x = (*(pData+9)^0x17)+0x17;
	pThis->m_vAnalogCalibration_Min.y = (*(pData+12)^0x17)+0x17;
	pThis->m_vAnalogCalibration_Min.z = (*(pData+13)^0x17)+0x17;

	// Calculate ratio values
	pThis->m_vCalibration_Ratio.x = 1.0f / ((float)pThis->m_vCalibration_1G.x - pThis->m_vCalibration_ZeroPoint.x);
	pThis->m_vCalibration_Ratio.y = 1.0f / ((float)pThis->m_vCalibration_1G.y - pThis->m_vCalibration_ZeroPoint.y);
	pThis->m_vCalibration_Ratio.z = 1.0f / ((float)pThis->m_vCalibration_1G.z - pThis->m_vCalibration_ZeroPoint.z);
	
	pThis->m_vAnalogCalibration_Ratio.x = 2.0f / ((float)pThis->m_vAnalogCalibration_Max.x - pThis->m_vAnalogCalibration_Min.x);
	pThis->m_vAnalogCalibration_Ratio.y = 2.0f / ((float)pThis->m_vAnalogCalibration_Max.y - pThis->m_vAnalogCalibration_Min.y);

	// Set calibrated bit
	pThis->m_nFlags = SET_BITS(WMF_ISCALIBRATED,pThis->m_nFlags);
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::Shutdown(void)
{
	m_pRemote = NULL;

	// Clear action map
	m_ActionMap.clear();
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::DecryptBuffer(DataBuffer const& buffer, DataBuffer &out)
{
	for (int i = 0; i < (int)buffer; i++)
	{
		out[i] = (buffer[i]^0x17)+0x17;
	}
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::OnUpdate(int nID, DataBuffer const& buffer, int nOffset, float fCurrTick)
{
	// Decrypt data first
	DataBuffer decryptBuffer;
	DecryptBuffer(buffer, decryptBuffer);

	if (WR_EXTENSION_UPDATE_REPORT == nID)
	{
		m_bWasUpdated = true;

		// Update input
		assert(m_pRemote);
		bool bBuffered = IsBufferedInputEnabled();
		unsigned int nPrevButtons = m_nButtons;
		int nPrev, nCurr, nUp;

		// Update m_nButtons and look for changes
		m_nButtons = (unsigned int)decryptBuffer[nOffset+WR_NCDATA_BUTTONS];
		for (int nButton = 0; nButton < WR_NUNCHUK_BUTTONS_MAX; nButton++)
		{
			// Get previous and current changes
			nUp = m_nButtons & WR_NUNCHUK_BUTTONS_INDEX_VALUE[nButton];
			nPrev = m_pButtonStatus[nButton];

			// Pick new state for it based on previous state
			switch (m_pButtonStatus[nButton])
			{
				case WR_BUTTONSTATUS_UP:
				{
					// If the button is now down...
					if (!nUp)
					{
						// Go to pushed on buffered, otherwise go to down
						m_pButtonStatus[nButton] = (bBuffered?WR_BUTTONSTATUS_PUSHED:WR_BUTTONSTATUS_DOWN);
						if (true == bBuffered) m_pButtonBufferedTime[nButton] = fCurrTick;
					}
				}
				break;
				case WR_BUTTONSTATUS_PUSHED:
				{
					// If the button is still down...
					if (!nUp)
					{
						// Go to down state
						m_pButtonStatus[nButton] = WR_BUTTONSTATUS_DOWN;
					}
					else
					{
						// Otherwise go to up state
						m_pButtonStatus[nButton] = WR_BUTTONSTATUS_RELEASED;
					}
				}
				break;
				case WR_BUTTONSTATUS_DOWN:
				{
					// If the button is coming back up...
					if (nUp)
					{
						// Go to released on buffered, otherwise go to up
						m_pButtonStatus[nButton] = (bBuffered?WR_BUTTONSTATUS_RELEASED:WR_BUTTONSTATUS_UP);
						if (true == bBuffered) m_pButtonBufferedTime[nButton] = fCurrTick;
					}
				}
				break;
				case WR_BUTTONSTATUS_RELEASED:
				{
					// If the button is now down...
					if (!nUp)
					{
						// Go back to down state
						m_pButtonStatus[nButton] = WR_BUTTONSTATUS_PUSHED;
					}
					else
					{
						// Otherwise finish going to up state
						m_pButtonStatus[nButton] = WR_BUTTONSTATUS_UP;
					}
				}
				break;
			}

			// Did the status change?
			nCurr = m_pButtonStatus[nButton];
			if (nPrev != nCurr)
			{
				// Signal the button status changed
				for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
					(*itI)->OnExtensionButton(m_pRemote, this, WR_NUNCHUK_BUTTONS_INDEX_VALUE[nButton],
						m_pButtonStatus[nButton], (m_pButtonStatus[nButton]==WR_BUTTONSTATUS_UP?false:true));
			}
		}

		// Update motion
		SMotionVec3F vPrev(m_vAccel);
		m_vAccel.x = (float)decryptBuffer[nOffset+WR_NCDATA_MOTION_X]-m_vCalibration_ZeroPoint.x;
		m_vAccel.y = (float)decryptBuffer[nOffset+WR_NCDATA_MOTION_Y]-m_vCalibration_ZeroPoint.y;
		m_vAccel.z = (float)decryptBuffer[nOffset+WR_NCDATA_MOTION_Z]-m_vCalibration_ZeroPoint.z;
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
					(*itI)->OnExtensionMotionEnd(m_pRemote, this, element);
			}

			StopMotion();
		}
		else
		{
			// If motion is not active, add to queue
			if (false == CHECK_BITS(WMF_ACTIVEMOTION, m_nFlags))
			{
				// Add to queue and check if we have passed the min
				m_MotionQueue.push(element);
				if ((int)m_MotionQueue.size() > m_nMinMotionSize)
				{
					// Report what is in the queue
					SMotionElement const& front = m_MotionQueue.front(); m_MotionQueue.pop();
					for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
						(*itI)->OnExtensionMotionStart(m_pRemote, this, front);
					while (false == m_MotionQueue.empty())
					{
						SMotionElement const& next = m_MotionQueue.front(); m_MotionQueue.pop();
						for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
							(*itI)->OnExtensionMotionUpdate(m_pRemote, this, next);
					}

					// Set flag
					m_nFlags = SET_BITS(WMF_ACTIVEMOTION, m_nFlags);
				}
			}
			else
			{
				// Just report it
				for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
					(*itI)->OnExtensionMotionUpdate(m_pRemote, this, element);
			}
		}

		// Report that the motion has been updated
		for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
			(*itI)->OnExtensionSingleMotion(m_pRemote, this, element);

		// Update analog sticks
		float fPrevX = m_fAnalogX;
		float fPrevY = m_fAnalogY;
		m_fAnalogX = float(decryptBuffer[nOffset+WR_NCDATA_ANALOG_X]) - m_vAnalogCalibration_Max.z;
		m_fAnalogY = float(decryptBuffer[nOffset+WR_NCDATA_ANALOG_Y]) - m_vAnalogCalibration_Min.z;
		if (true == CHECK_BITS(WMF_ISCALIBRATED,m_nFlags))
		{
			m_fAnalogX *= m_vAnalogCalibration_Ratio.x;
			m_fAnalogY *= m_vAnalogCalibration_Ratio.y;
		}
		if (m_fAnalogX != fPrevX || m_fAnalogY != fPrevY)
		{
			for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
				(*itI)->OnExtensionAnalogUpdate(m_pRemote, this, 1, m_fAnalogX, m_fAnalogY);
		}
	}
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::StopMotion(void)
{
	// Empty the queue
	while (false == m_MotionQueue.empty()) m_MotionQueue.pop();
	m_nCurrMotionLifetime = 0;

	// Set flag
	m_nFlags = CLEAR_BITS(WMF_ACTIVEMOTION, m_nFlags);
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::OnPostUpdate(void)
{
	if (false == m_bWasUpdated)
	{
		// Go through and push buffered input to next stage, to prevent spamming due to lack of
		//	comm from the remote
		int nPrev, nCurr;
		for (int nButton = 0; nButton < WR_NUNCHUK_BUTTONS_MAX; nButton++)
		{
			nPrev = m_pButtonStatus[nButton];
			switch (m_pButtonStatus[nButton])
			{
				case WR_BUTTONSTATUS_PUSHED: m_pButtonStatus[nButton] = WR_BUTTONSTATUS_DOWN; break;
				case WR_BUTTONSTATUS_RELEASED: m_pButtonStatus[nButton] = WR_BUTTONSTATUS_UP; break;
			}
			nCurr = m_pButtonStatus[nButton];

			// Did the status change?
			nCurr = m_pButtonStatus[nButton];
			if (nPrev != nCurr)
			{
				// Signal the button status changed
				for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
					(*itI)->OnExtensionButton(m_pRemote, this, WR_NUNCHUK_BUTTONS_INDEX_VALUE[nButton],
						m_pButtonStatus[nButton], (m_pButtonStatus[nButton]==WR_BUTTONSTATUS_UP?false:true));
			}
		}
	}

	// Update action status
	for (ActionMap::iterator itAction = m_ActionMap.begin(); itAction != m_ActionMap.end(); itAction++)
	{
		int nLastStatus = itAction->second.nLastStatus;
		itAction->second.nLastStatus = GetActionStatus(itAction->first, g_pWR->pTimer->GetDeltaTime());
		if (itAction->second.nLastStatus != nLastStatus)
		{
			// Report the change
			for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
					(*itI)->OnExtensionAction(m_pRemote, this, itAction->second.szName.c_str(), itAction->first,
						itAction->second.nLastStatus, (itAction->second.nLastStatus==WR_BUTTONSTATUS_UP?false:true));
		}
	}

	m_bWasUpdated = false;
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::EnableBufferedInput(bool bEnable)
{
	// Trigger continuous reporting
	assert(m_pRemote);
	m_pRemote->TriggerContinuousReport(bEnable);
}

////////////////////////////////////////////////////
bool CWR_WiiNunchuk::IsBufferedInputEnabled(void) const
{
	assert(m_pRemote);
	return m_pRemote->IsContinuousReport();
}

////////////////////////////////////////////////////
int CWR_WiiNunchuk::GetButtonStatus(unsigned int nButtonMask, float fError) const
{
	int nStatus = -1;

	// If it is a pow2 mask (meaning only one button is requested), just return it
	if (IS_POW2(nButtonMask))
	{
		for (int nButton = 0; nButton < WR_NUNCHUK_BUTTONS_MAX; nButton++)
		{
			if (nButtonMask == WR_NUNCHUK_BUTTONS_INDEX_VALUE[nButton])
				return m_pButtonStatus[nButton];
		}
		return WR_BUTTONSTATUS_UP;
	}

	// Check each button in the mask
	int nButtonValue, nButtonStatus;
	float fCurrTick = (fError > 0.0f ? g_pWR->pTimer->GetCurrTime() : 0.0f);
	for (int nButton = 0; nButton < WR_NUNCHUK_BUTTONS_MAX; nButton++)
	{
		nButtonValue = WR_NUNCHUK_BUTTONS_INDEX_VALUE[nButton];
		if (nButtonValue == (nButtonMask&nButtonValue))
		{
			nButtonStatus = m_pButtonStatus[nButton];

			// Account for error
			if (fError > 0.0f &&
				(WR_BUTTONSTATUS_DOWN == nButtonStatus || WR_BUTTONSTATUS_UP == nButtonStatus))
			{
				if (fCurrTick - m_pButtonBufferedTime[nButton] <= fError)
				{
					if (WR_BUTTONSTATUS_DOWN == nButtonStatus)
						nButtonStatus = WR_BUTTONSTATUS_PUSHED;
					else
						nButtonStatus = WR_BUTTONSTATUS_RELEASED;
				}
			}

			// They want the status of this button...
			if (-1 == nStatus)
			{
				// First request, so just take the value
				nStatus = nButtonStatus;
			}
			else
			{
				// Take status based on overall result
				// Rules:
				//	1. If a button is up, all are up
				//	2. Up and down status take presedence over pushed and releasing
				//  3. If just one button is being released and none others are, all are up
				if (WR_BUTTONSTATUS_UP == nButtonStatus)
					nStatus = WR_BUTTONSTATUS_UP;
				else if (WR_BUTTONSTATUS_PUSHED == nStatus && WR_BUTTONSTATUS_DOWN == nButtonStatus)
					nStatus = WR_BUTTONSTATUS_DOWN;
				else if (WR_BUTTONSTATUS_RELEASED == nButtonStatus && WR_BUTTONSTATUS_RELEASED != nStatus)
					nStatus = WR_BUTTONSTATUS_UP;
			}
		}
	}
	
	// Return status
	return (-1 == nStatus ? WR_BUTTONSTATUS_UP : nStatus);
}

////////////////////////////////////////////////////
bool CWR_WiiNunchuk::IsButtonDown(unsigned int nButtonMask, float fError) const
{
	int nStatus = GetButtonStatus(nButtonMask, fError);
	return (WR_BUTTONSTATUS_UP == nStatus ? false : true);
}

////////////////////////////////////////////////////
int CWR_WiiNunchuk::RegisterAction(char const* szName, unsigned int nButtonMask)
{
	// Check to make sure there isn't already an action named this
	for (ActionMap::iterator itI = m_ActionMap.begin(); itI != m_ActionMap.end(); itI++)
	{
		if (itI->second.szName == szName)
		{
			// Update its mask
			itI->second.nButtonMask = nButtonMask;
			return itI->first;
		}
	}

	// Make a new entry
	SWR_WiiButton_Action action;
	action.nID = ++m_nActionIDSeed;
	action.szName = szName;
	action.nButtonMask = nButtonMask;

	// Return it
	m_ActionMap[action.nID] = action;
	return action.nID;
}

////////////////////////////////////////////////////
int CWR_WiiNunchuk::GetActionStatus(char const* szName, float fError) const
{
	// Find the entry
	for (ActionMap::const_iterator itI = m_ActionMap.begin(); itI != m_ActionMap.end(); itI++)
	{
		if (itI->second.szName == szName)
		{
			return GetActionStatus(itI->first, fError);
		}
	}
	return WR_BUTTONSTATUS_UP;
}

////////////////////////////////////////////////////
int CWR_WiiNunchuk::GetActionStatus(int nID, float fError) const
{
	// Find the entry
	ActionMap::const_iterator itAction = m_ActionMap.find(nID);
	if (m_ActionMap.end() == itAction) return WR_BUTTONSTATUS_UP;

	// Check its status
	return GetButtonStatus(itAction->second.nButtonMask, fError);
}

////////////////////////////////////////////////////
bool CWR_WiiNunchuk::IsActionDown(char const* szName, float fError) const
{
	// Find the entry
	for (ActionMap::const_iterator itI = m_ActionMap.begin(); itI != m_ActionMap.end(); itI++)
	{
		if (itI->second.szName == szName)
		{
			return IsActionDown(itI->first, fError);
		}
	}
	return WR_BUTTONSTATUS_UP;
}

////////////////////////////////////////////////////
bool CWR_WiiNunchuk::IsActionDown(int nID, float fError) const
{
	// Find the entry
	ActionMap::const_iterator itAction = m_ActionMap.find(nID);
	if (m_ActionMap.end() == itAction) return WR_BUTTONSTATUS_UP;

	// Check its status
	return IsButtonDown(itAction->second.nButtonMask, fError);
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::Calibrate(void)
{
	// Read calibration data
	m_pRemote->GetDataHelper()->ReadData(WR_NUNCHUK_CALIBRATION_LOC, WR_NUNCHUK_CALIBRATION_SIZE, OnCalibrateData, this);
}

////////////////////////////////////////////////////
bool CWR_WiiNunchuk::IsCalibrated(void) const
{
	return CHECK_BITS(WMF_ISCALIBRATED,m_nFlags);
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::SetMotionSize(int nSize)
{
	m_nMinMotionSize = MAX(nSize, 10);
}

////////////////////////////////////////////////////
int CWR_WiiNunchuk::GetMotionSize(void) const
{
	return m_nMinMotionSize;
}

////////////////////////////////////////////////////
bool CWR_WiiNunchuk::IsMotionActive(void) const
{
	return CHECK_BITS(WMF_ACTIVEMOTION, m_nFlags);
}

////////////////////////////////////////////////////
int CWR_WiiNunchuk::GetMotionLifetime(void) const
{
	return m_nCurrMotionLifetime;
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::GetAcceleration(SMotionVec3F &v) const
{
	v = m_vAccel;
}

////////////////////////////////////////////////////
float CWR_WiiNunchuk::GetAccelerationX(void) const
{
	return m_vAccel.x;
}

////////////////////////////////////////////////////
float CWR_WiiNunchuk::GetAccelerationY(void) const
{
	return m_vAccel.y;
}

////////////////////////////////////////////////////
float CWR_WiiNunchuk::GetAccelerationZ(void) const
{
	return m_vAccel.z;
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::GetDirection(SMotionVec3F &v) const
{
	v = m_vDir;
}

////////////////////////////////////////////////////
float CWR_WiiNunchuk::GetDirectionX(void) const
{
	return m_vDir.x;
}

////////////////////////////////////////////////////
float CWR_WiiNunchuk::GetDirectionY(void) const
{
	return m_vDir.y;
}

////////////////////////////////////////////////////
float CWR_WiiNunchuk::GetDirectionZ(void) const
{
	return m_vDir.z;
}

////////////////////////////////////////////////////
float CWR_WiiNunchuk::GetPitch(void) const
{
	return m_fPitch;
}

////////////////////////////////////////////////////
float CWR_WiiNunchuk::GetRoll(void) const
{
	return m_fRoll;
}

////////////////////////////////////////////////////
void CWR_WiiNunchuk::GetAnalog(SMotionVec3F &v) const
{
	v.x = m_fAnalogX;
	v.y = m_fAnalogY;
	v.z = 0.0f;
}

////////////////////////////////////////////////////
float CWR_WiiNunchuk::GetAnalogX(void) const
{
	return m_fAnalogX;
}

////////////////////////////////////////////////////
float CWR_WiiNunchuk::GetAnalogY(void) const
{
	return m_fAnalogY;
}