////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_Implementation.cpp
//
// Purpose: Main implementation of the Wii Remote
//			Core API
//
// History:
//	- 11/1/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "WR_Implementation.h"

// Include implementation files
#include "WR_CHIDController.h"
#include "WR_CTimer.h"

CWR_GlobalInstance CWR_GlobalInstance::m_Instance;
CWR_GlobalInstance *g_pWR = GetWiiRemoteSystem();

////////////////////////////////////////////////////
CWR_GlobalInstance* GetWiiRemoteSystem(void)
{
	return CWR_GlobalInstance::GetInstance();
}

////////////////////////////////////////////////////
CWR_GlobalInstance::CWR_GlobalInstance(void)
{
	m_ErrorDef.nModule = m_ErrorDef.nError = 0;
	m_ErrorDef.szErrorMsgs = NULL;

	pHIDController = new CWR_HIDController;
	pTimer = new CWR_Timer;
}

////////////////////////////////////////////////////
CWR_GlobalInstance::~CWR_GlobalInstance(void)
{
	Shutdown();
}

////////////////////////////////////////////////////
CWR_GlobalInstance* CWR_GlobalInstance::GetInstance(void)
{
	return &m_Instance;
}

////////////////////////////////////////////////////
void CWR_GlobalInstance::GetLastError(int &nModule, int &nError)
{
	nModule = m_ErrorDef.nModule;
	nError = m_ErrorDef.nError;
}

////////////////////////////////////////////////////
int CWR_GlobalInstance::GetLastErrorString(char *szError, int nSize)
{
	memset(szError, 0, nSize);
	if (NULL == m_ErrorDef.szErrorMsgs || true == m_ErrorDef.szErrorMsg.empty())
		return 0;
	if (NULL == m_ErrorDef.szErrorMsgs)
		strncpy_s(szError, nSize, m_ErrorDef.szErrorMsg.c_str(), nSize);
	else
		strncpy_s(szError, nSize, m_ErrorDef.szErrorMsgs[m_ErrorDef.nError], nSize);
	return (int)strlen(szError);
}

////////////////////////////////////////////////////
void CWR_GlobalInstance::RaiseError(int nModule, int nError, char const** szErrorMsgs)
{
	m_ErrorDef.nModule = nModule;
	m_ErrorDef.nError = nError;
	m_ErrorDef.szErrorMsg.clear();
	m_ErrorDef.szErrorMsgs = szErrorMsgs;

	// Signal
	for (Listeners::iterator itI = m_ErrorListeners.begin(); itI != m_ErrorListeners.end(); itI++)
		(*itI)->OnError(nModule, nError, szErrorMsgs[nError]);
}

////////////////////////////////////////////////////
void CWR_GlobalInstance::RaiseManualError(char const* szError, int nError)
{
	m_ErrorDef.nModule = 0;
	m_ErrorDef.nError = nError;
	m_ErrorDef.szErrorMsg = szError;
	m_ErrorDef.szErrorMsgs = NULL;

	// Signal
	for (Listeners::iterator itI = m_ErrorListeners.begin(); itI != m_ErrorListeners.end(); itI++)
		(*itI)->OnError(0, nError, szError);
}

////////////////////////////////////////////////////
void CWR_GlobalInstance::AddErrorListener(IWR_ErrorListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_ErrorListeners.begin(); itI != m_ErrorListeners.end(); itI++)
		if (*itI == pListener)
			return;
	m_ErrorListeners.push_back(pListener);
}

////////////////////////////////////////////////////
void CWR_GlobalInstance::RemoveErrorListener(IWR_ErrorListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_ErrorListeners.begin(); itI != m_ErrorListeners.end(); itI++)
		if (*itI == pListener)
		{
			m_ErrorListeners.erase(itI);
			return;
		}
}

////////////////////////////////////////////////////
bool CWR_GlobalInstance::Initialize(void)
{
	// Initialize timer
	if (WR_FAIL(pTimer->Initialize()))
		return false;

	// Initialize the HID controller
	if (WR_FAIL(pHIDController->Initialize()))
		return false;

	return true;
}

////////////////////////////////////////////////////
void CWR_GlobalInstance::Shutdown(void)
{
	// Destroy the HID controller
	if (NULL != pHIDController)
	{
		pHIDController->Shutdown();
		SAFE_DELETE(pHIDController);
	}

	// Destroy the timer
	if (NULL != pTimer)
	{
		pTimer->Shutdown();
		SAFE_DELETE(pTimer);
	}

	// Clean out listeners
	m_ErrorListeners.clear();
}

////////////////////////////////////////////////////
void CWR_GlobalInstance::Update(void)
{
	// Update timer
	assert(pTimer);
	pTimer->Update();

	// Update HID controller
	assert(pHIDController);
	pHIDController->UpdateRemotes();
}