////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiButtons.cpp
//
// Purpose: Helper for the Wii Remote that manages
//	button input
//
// History:
//	- 11/3/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "WR_Implementation.h"
#include "WR_CWiiButtons.h"
#include "WR_CWiiRemote.h"

////////////////////////////////////////////////////
CWR_WiiButtons::CWR_WiiButtons(void)
{
	m_pRemote = NULL;
	m_bWasUpdated = false;
	m_nButtons = 0;
	m_nActionIDSeed = 0;
	memset(m_pButtonStatus, 0, sizeof(int)*WR_WIIREMOTE_BUTTONS_MAX);
	memset(m_pButtonBufferedTime, 0, sizeof(float)*WR_WIIREMOTE_BUTTONS_MAX);
}

////////////////////////////////////////////////////
CWR_WiiButtons::~CWR_WiiButtons(void)
{
	Shutdown();
}

////////////////////////////////////////////////////
bool CWR_WiiButtons::Initialize(IWR_WiiRemote *pRemote)
{
	m_pRemote = (CWR_WiiRemote*)pRemote;
	if (NULL == m_pRemote) return false;

	// Reset status of buttons
	m_nButtons = 0;
	for (int i = 0; i < WR_WIIREMOTE_BUTTONS_MAX; i++)
	{
		m_pButtonStatus[i] = WR_BUTTONSTATUS_UP;
		m_pButtonBufferedTime[i] = 0;
	}

	m_nActionIDSeed = ACTIONID_INVALID;

	return true;
}

////////////////////////////////////////////////////
void CWR_WiiButtons::Shutdown(void)
{
	// Clear action map
	m_ActionMap.clear();
}

////////////////////////////////////////////////////
void CWR_WiiButtons::OnButtonUpdate(DataBuffer const& buffer, int nOffset, float fCurrTick)
{
	assert(m_pRemote);
	bool bBuffered = IsBufferedInputEnabled();
	unsigned int nPrevButtons = m_nButtons;
	int nPrev, nCurr, nDown;

	m_bWasUpdated = true;

	// Update m_nButtons and look for changes
	m_nButtons = ((unsigned int)buffer[nOffset+0]<<8)|buffer[nOffset+1];
	for (int nButton = 0; nButton < WR_WIIREMOTE_BUTTONS_MAX; nButton++)
	{
		// Get previous and current changes
		nDown = m_nButtons & WR_WIIREMOTE_BUTTONS_INDEX_VALUE[nButton];
		nPrev = m_pButtonStatus[nButton];

		// Pick new state for it based on previous state
		switch (m_pButtonStatus[nButton])
		{
			case WR_BUTTONSTATUS_UP:
			{
				// If the button is now down...
				if (nDown)
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
				if (nDown)
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
				if (!nDown)
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
				if (nDown)
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
				(*itI)->OnButton(m_pRemote, this, WR_WIIREMOTE_BUTTONS_INDEX_VALUE[nButton],
					m_pButtonStatus[nButton], (m_pButtonStatus[nButton]==WR_BUTTONSTATUS_UP?false:true));
		}
	}
}

////////////////////////////////////////////////////
void CWR_WiiButtons::OnPostUpdate(void)
{
	if (false == m_bWasUpdated)
	{
		// Go through and push buffered input to next stage, to prevent spamming due to lack of
		//	comm from the remote
		int nPrev, nCurr;
		for (int nButton = 0; nButton < WR_WIIREMOTE_BUTTONS_MAX; nButton++)
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
					(*itI)->OnButton(m_pRemote, this, WR_WIIREMOTE_BUTTONS_INDEX_VALUE[nButton],
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
					(*itI)->OnAction(m_pRemote, this, itAction->second.szName.c_str(), itAction->first,
						itAction->second.nLastStatus, (itAction->second.nLastStatus==WR_BUTTONSTATUS_UP?false:true));
		}
	}

	m_bWasUpdated = false;
}

////////////////////////////////////////////////////
void CWR_WiiButtons::AddListener(IWR_WiiButtonsListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		if (*itI == pListener)
			return;
	m_Listeners.push_back(pListener);
}

////////////////////////////////////////////////////
void CWR_WiiButtons::RemoveListener(IWR_WiiButtonsListener *pListener)
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
void CWR_WiiButtons::EnableBufferedInput(bool bEnable)
{
	// Trigger continuous reporting
	assert(m_pRemote);
	m_pRemote->TriggerContinuousReport(bEnable);
}

////////////////////////////////////////////////////
bool CWR_WiiButtons::IsBufferedInputEnabled(void) const
{
	assert(m_pRemote);
	return m_pRemote->IsContinuousReport();
}

////////////////////////////////////////////////////
int CWR_WiiButtons::GetButtonStatus(unsigned int nButtonMask, float fError) const
{
	int nStatus = -1;

	// If it is a pow2 mask (meaning only one button is requested), just return it
	if (IS_POW2(nButtonMask))
	{
		for (int nButton = 0; nButton < WR_WIIREMOTE_BUTTONS_MAX; nButton++)
		{
			if (nButtonMask == WR_WIIREMOTE_BUTTONS_INDEX_VALUE[nButton])
				return m_pButtonStatus[nButton];
		}
		return WR_BUTTONSTATUS_UP;
	}

	// Check each button in the mask
	int nButtonValue, nButtonStatus;
	float fCurrTick = (fError > 0.0f ? g_pWR->pTimer->GetCurrTime() : 0.0f);
	for (int nButton = 0; nButton < WR_WIIREMOTE_BUTTONS_MAX; nButton++)
	{
		nButtonValue = WR_WIIREMOTE_BUTTONS_INDEX_VALUE[nButton];
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
bool CWR_WiiButtons::IsButtonDown(unsigned int nButtonMask, float fError) const
{
	int nStatus = GetButtonStatus(nButtonMask, fError);
	return (WR_BUTTONSTATUS_UP == nStatus ? false : true);
}

////////////////////////////////////////////////////
int CWR_WiiButtons::RegisterAction(char const* szName, unsigned int nButtonMask)
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
int CWR_WiiButtons::GetActionStatus(char const* szName, float fError) const
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
int CWR_WiiButtons::GetActionStatus(int nID, float fError) const
{
	// Find the entry
	ActionMap::const_iterator itAction = m_ActionMap.find(nID);
	if (m_ActionMap.end() == itAction) return WR_BUTTONSTATUS_UP;

	// Check its status
	return GetButtonStatus(itAction->second.nButtonMask, fError);
}

////////////////////////////////////////////////////
bool CWR_WiiButtons::IsActionDown(char const* szName, float fError) const
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
bool CWR_WiiButtons::IsActionDown(int nID, float fError) const
{
	// Find the entry
	ActionMap::const_iterator itAction = m_ActionMap.find(nID);
	if (m_ActionMap.end() == itAction) return WR_BUTTONSTATUS_UP;

	// Check its status
	return IsButtonDown(itAction->second.nButtonMask, fError);
}