////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiRemote.cpp
//
// Purpose: Wii Remote communicator
//
// History:
//	- 11/2/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "WR_Implementation.h"
#include "WR_CWiiRemote.h"

// Helpers
#include "WR_CWiiButtons.h"
#include "WR_CWiiMotion.h"
#include "WR_CWiiData.h"
#include "WR_CWiiSensor.h"

// Extensions
#include "WR_CWiiNunchuk.h"

REGISTER_WR_MODULE(CWR_WiiRemote, WIIREMOTE);

////////////////////////////////////////////////////
IWR_WiiButtons* CWR_WiiRemote::CreateButtonHelper(void) const
{
	return new CWR_WiiButtons;
}

////////////////////////////////////////////////////
IWR_WiiMotion* CWR_WiiRemote::CreateMotionHelper(void) const
{
	return new CWR_WiiMotion;
}

////////////////////////////////////////////////////
IWR_WiiData* CWR_WiiRemote::CreateDataHelper(void) const
{
	return new CWR_WiiData;
}

////////////////////////////////////////////////////
IWR_WiiExtension* CWR_WiiRemote::CreateExtensionHelper(int nType) const
{
	switch (nType)
	{
		case CWR_WiiNunchuk::TYPE: return new CWR_WiiNunchuk; break;
	}
	return NULL;
}

////////////////////////////////////////////////////
IWR_WiiSensor* CWR_WiiRemote::CreateSensorHelper(void) const
{
	return new CWR_WiiSensor;
}


////////////////////////////////////////////////////
CWR_WiiRemote::CWR_WiiRemote(void)
{
	m_nID = REMOTEID_INVALID;
	m_hHandle = INVALID_HANDLE_VALUE;
	m_nFlags = 0;
	m_nReportMode = 0;
	m_nBattery = 0;

	m_pButtons = NULL;
	m_pMotion = NULL;
	m_pData = NULL;
	m_pExtension = NULL;
	m_pSensor = NULL;

	m_hWROThread = INVALID_HANDLE_VALUE;
	m_hWRODataReadEvent = INVALID_HANDLE_VALUE;
	m_dwWROThreadID = 0;
	_bWROThreadProcTerminate = false;

	m_hWRIThread = INVALID_HANDLE_VALUE;
	m_dwWRIThreadID = 0;
	_bWRIThreadProcTerminate = false;

	m_fAttemptConnectStart = 0.0f;
	m_fLastRecv = 0.0f;
	m_fConnectionTimeout = 0.0f;
	m_fStatusUpdateFreq = 0.0f;
	m_fNextStatusUpdate = 0.0f;

	InitializeCriticalSection(&_WROCS);
	InitializeCriticalSection(&_WRICS);
}

////////////////////////////////////////////////////
CWR_WiiRemote::~CWR_WiiRemote(void)
{
	Shutdown();
	DeleteCriticalSection(&_WROCS);
	DeleteCriticalSection(&_WRICS);
}

////////////////////////////////////////////////////
IWR_WiiButtons* CWR_WiiRemote::GetButtonHelper(void) const
{
	return m_pButtons;
}

////////////////////////////////////////////////////
IWR_WiiMotion* CWR_WiiRemote::GetMotionHelper(void) const
{
	return m_pMotion;
}

////////////////////////////////////////////////////
IWR_WiiData* CWR_WiiRemote::GetDataHelper(void) const
{
	return m_pData;
}

////////////////////////////////////////////////////
IWR_WiiExtension* CWR_WiiRemote::GetExtensionHelper(void) const
{
	return m_pExtension;
}

////////////////////////////////////////////////////
IWR_WiiSensor* CWR_WiiRemote::GetSensorHelper(void) const
{
	return m_pSensor;
}

////////////////////////////////////////////////////
void CWR_WiiRemote::AddListener(IWR_WiiRemoteListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		if (*itI == pListener)
			return;
	m_Listeners.push_back(pListener);
}

////////////////////////////////////////////////////
void CWR_WiiRemote::RemoveListener(IWR_WiiRemoteListener *pListener)
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
int CWR_WiiRemote::Initialize(char const* szDevicePath, RemoteID const& nID)
{
	if (REMOTEID_INVALID != m_nID || REMOTEID_INVALID == nID)
		WR_RAISEERROR(WR_WIIREMOTE_BADINIT);
	m_nID = nID;

	// Create helpers
	m_pButtons = (CWR_WiiButtons*)CreateButtonHelper();
	if (NULL == m_pButtons || false == m_pButtons->Initialize(this))
		WR_RAISEERROR(WR_WIIREMOTE_BADBUTTONS);
	m_pMotion = (CWR_WiiMotion*)CreateMotionHelper();
	if (NULL == m_pMotion || false == m_pMotion->Initialize(this))
		WR_RAISEERROR(WR_WIIREMOTE_BADMOTION);
	m_pData = (CWR_WiiData*)CreateDataHelper();
	if (NULL == m_pData || false == m_pData->Initialize(this))
		WR_RAISEERROR(WR_WIIREMOTE_BADDATA);
	m_pSensor = (CWR_WiiSensor*)CreateSensorHelper();
	if (NULL == m_pSensor || false == m_pSensor->Initialize(this))
		WR_RAISEERROR(WR_WIIREMOTE_BADSENSOR);

	// Create handle to device
	m_hHandle = CreateFile(szDevicePath, (GENERIC_READ|GENERIC_WRITE),
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (INVALID_HANDLE_VALUE == m_hHandle) return WR_WIIREMOTE_INVALIDHANDLE;

	// Create writing thread
	m_hWROThread = (HANDLE)_beginthreadex(NULL, 0, WROThreadProc, this, 0, &m_dwWROThreadID);
	if (INVALID_HANDLE_VALUE == m_hWROThread) WR_RAISEERROR(WR_WIIREMOTE_WRITETHREADFAIL);
	//SetThreadPriority(m_hWROThread, THREAD_PRIORITY_HIGHEST);
	m_hWRODataReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (INVALID_HANDLE_VALUE == m_hWRODataReadEvent) WR_RAISEERROR(WR_WIIREMOTE_WRITETHREADFAIL);

	// Setup overlapped
	memset(&m_hWROOverlap, 0, sizeof(OVERLAPPED));
	m_hWROOverlap.hEvent = m_hWRODataReadEvent;
	m_hWROOverlap.Offset = 0;
	m_hWROOverlap.OffsetHigh = 0;

	// Create reading thread
	m_hWRIThread = (HANDLE)_beginthreadex(NULL, 0, WRIThreadProc, this, 0, &m_dwWRIThreadID);
	if (INVALID_HANDLE_VALUE == m_hWRIThread) WR_RAISEERROR(WR_WIIREMOTE_READTHREADFAIL);
	//SetThreadPriority(m_hWRIThread, THREAD_PRIORITY_HIGHEST);

	Reconnect(false);
	return WR_WIIREMOTE_OK;
}

////////////////////////////////////////////////////
void CWR_WiiRemote::Reconnect(bool bKeepReport)
{
	// Attempt a connect
	SetFlags(WRF_ATTEMPTCONNECT);
	m_fAttemptConnectStart = g_pWR->pTimer->GetCurrTime();
	SetReport_Connect(bKeepReport?m_nReportMode:WR_REPORT_DEFAULT);

	// Report we are trying to connect
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		(*itI)->OnConnecting(this);
}

////////////////////////////////////////////////////
bool CWR_WiiRemote::IsConnecting(void) const
{
	return CheckFlags(WRF_ATTEMPTCONNECT);
}

////////////////////////////////////////////////////
bool CWR_WiiRemote::IsConnected(void) const
{
	return CheckFlags(WRF_CONNECTED);
}

////////////////////////////////////////////////////
void CWR_WiiRemote::Shutdown(void)
{
	// Reset status
	Reset();

	// Close the threads
	if (INVALID_HANDLE_VALUE != m_hWROThread)
	{
		_bWROThreadProcTerminate = true;
		WaitForSingleObject(m_hWROThread, INFINITE);
		CloseHandle(m_hWROThread);
		m_hWROThread = INVALID_HANDLE_VALUE;
	}
	if (INVALID_HANDLE_VALUE != m_hWRIThread)
	{
		_bWRIThreadProcTerminate = true;
		WaitForSingleObject(m_hWRIThread, INFINITE);
		CloseHandle(m_hWRIThread);
		m_hWRIThread = INVALID_HANDLE_VALUE;
	}

	// Tidy up critical sections and events
	if (INVALID_HANDLE_VALUE != m_hWRODataReadEvent)
	{
		CloseHandle(m_hWRODataReadEvent);
		m_hWRODataReadEvent = INVALID_HANDLE_VALUE;
	}

	// Close the handle
	if (INVALID_HANDLE_VALUE != m_hHandle)
	{
		CloseHandle(m_hHandle);
		m_hHandle = INVALID_HANDLE_VALUE;
	}

	// Kill flags
	m_nFlags = 0;

	// Kill helpers
	if (NULL != m_pButtons)
	{
		m_pButtons->Shutdown();
		SAFE_DELETE(m_pButtons);
	}
	if (NULL != m_pMotion)
	{
		m_pMotion->Shutdown();
		SAFE_DELETE(m_pMotion);
	}
	if (NULL != m_pData)
	{
		m_pData->Shutdown();
		SAFE_DELETE(m_pData);
	}
	if (NULL != m_pExtension)
	{
		m_pExtension->Shutdown();
		SAFE_DELETE(m_pExtension);
	}
	if (NULL != m_pSensor)
	{
		m_pSensor->Shutdown();
		SAFE_DELETE(m_pSensor);
	}

	// Report we disconnected
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		(*itI)->OnDisconnect(this, false);
}

////////////////////////////////////////////////////
void CWR_WiiRemote::Reset(void)
{
	// Turn off LEDs
	SetLEDs(WR_LED_NONE);
	SetRumble(false);

	// Set next status update
	if (true == CheckFlags(WRF_UPDATESTATUS))
		m_fNextStatusUpdate = g_pWR->pTimer->GetCurrTime() + m_fStatusUpdateFreq;

	// Let it go through
	Sleep(100);
}

////////////////////////////////////////////////////
RemoteID const& CWR_WiiRemote::GetID(void) const
{
	return m_nID;
}

////////////////////////////////////////////////////
void CWR_WiiRemote::SetReport(int nReport, bool bContinuous)
{
	// Must be connected
	if (false == CheckFlags(WRF_CONNECTED))
		return;

	// See if report changed
	int nPrevReport = m_nReportMode;
	bool bPrevContinuous = CheckFlags(WRF_CONTINUOUSREPORT);

	// Update
	m_nReportMode = nReport;
	SetFlags(WRF_CONTINUOUSREPORT, bContinuous);
	if (m_nReportMode != nPrevReport || bContinuous != bPrevContinuous)
	{
		// Changed
		for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
			(*itI)->OnReportChanged(this, m_nReportMode, bContinuous);
	}

	// Enable IR?
	if (m_pSensor != NULL) m_pSensor->OnSetReport(nReport);

	DataBuffer buffer;
	buffer[0] = WR_OUT_REPORT;
	buffer[1] = (true==bContinuous?0x04:0x00)|GetRumbleBit();
	buffer[2] = (nReport&0x3F);
	WriteData(buffer);
}

////////////////////////////////////////////////////
void CWR_WiiRemote::SetReport_Connect(int nReport)
{
	// Hack to bypass safety check
	SetFlags(WRF_CONNECTED, true);
	SetReport(nReport);
	SetFlags(WRF_CONNECTED, false);
}

////////////////////////////////////////////////////
void CWR_WiiRemote::TriggerContinuousReport(bool bContinuous)
{
	SetReport(m_nReportMode, bContinuous);
}

////////////////////////////////////////////////////
unsigned int CWR_WiiRemote::GetReport(void) const
{
	return m_nReportMode;
}

////////////////////////////////////////////////////
bool CWR_WiiRemote::IsContinuousReport(void) const
{
	return CheckFlags(WRF_CONTINUOUSREPORT);
}

////////////////////////////////////////////////////
void CWR_WiiRemote::SetStatusUpdate(float fFreq)
{
	m_fStatusUpdateFreq = fFreq;
	if (fFreq > 0.0f)
	{
		SetFlags(WRF_UPDATESTATUS, true);
		m_fNextStatusUpdate = g_pWR->pTimer->GetCurrTime() + fFreq;
	}
	else
	{
		SetFlags(WRF_UPDATESTATUS, false);
	}
}

////////////////////////////////////////////////////
void CWR_WiiRemote::RequestStatusUpdate(void)
{
	// Must be connected
	if (false == CheckFlags(WRF_CONNECTED))
		return;

	DataBuffer buffer;
	buffer[0] = WR_OUT_STATUS;
	buffer[1] = GetRumbleBit();
	WriteData(buffer);
}

////////////////////////////////////////////////////
void CWR_WiiRemote::SetLEDs(int nOnLEDs)
{
	// Must be connected
	if (false == CheckFlags(WRF_CONNECTED))
		return;

	DataBuffer buffer;
	buffer[0] = WR_OUT_LED;
	buffer[1] = (nOnLEDs&0xF0) | GetRumbleBit();
	WriteData(buffer);
}

////////////////////////////////////////////////////
void CWR_WiiRemote::SetRumble(bool bOn)
{
	// Must be connected
	if (false == CheckFlags(WRF_CONNECTED))
		return;

	// Set flag
	SetFlags(WRF_RUMBLEON, bOn);
	SetFlags(WRF_STATUS_RUMBLE, bOn);

	// Send packet through IR channel now
	DataBuffer buffer;
	buffer[0] = WR_OUT_SPEAKER;
	buffer[1] = GetRumbleBit();
	WriteData(buffer);

	// Signal a change
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		(*itI)->OnStatusUpdate(this, m_nFlags, m_nBattery);
}

////////////////////////////////////////////////////
bool CWR_WiiRemote::IsRumbleOn(void) const
{
	return CheckFlags(WRF_RUMBLEON);
}

////////////////////////////////////////////////////
unsigned int CWR_WiiRemote::GetBatteryLife(void) const
{
	return m_nBattery;
}

////////////////////////////////////////////////////
void CWR_WiiRemote::Update(void)
{
	float fCurrTick = g_pWR->pTimer->GetCurrTime();

	while (true)
	{
		// Anything to parse in read buffer?
		DataBuffer buffer;
		bool bParse = false;
		EnterCriticalSection(&_WRICS);
		if (false == _ReadQueue.empty())
		{
			// Get it
			bParse = true;
			buffer = _ReadQueue.front();
			_ReadQueue.pop();
		}
		LeaveCriticalSection(&_WRICS);

		// Handle the data
		if (true == bParse)
		{
			m_fLastRecv = fCurrTick;

			// If we were attempting a connection, we succedded
			if (true == CheckFlags(WRF_ATTEMPTCONNECT))
			{
				SetFlags(WRF_ATTEMPTCONNECT, false);
				SetFlags(WRF_CONNECTED, true);
				m_fAttemptConnectStart = 0;

				// Reset controller
				Reset();

				// Report we connected
				for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
					(*itI)->OnConnect(this);
			}

			// Handle input data based on op code
			BYTE nOpCode = buffer[0];
			if (WR_IN_EXPANSION == nOpCode)
			{
				// Status update
				UpdateStatus(buffer);
			}
			else if (WR_IN_DATAREAD == nOpCode)
			{
				// Read Update
				m_pButtons->OnButtonUpdate(buffer, 1, fCurrTick);
				m_pData->OnDataRead(buffer, 3, fCurrTick);
				OnDataRead(buffer); // Used for various things
			}
			else if (nOpCode&WR_IN_INPUTMASK)
			{
				// Check against mask
				int nOffset = 1;
				if (WR_REPORT_BUTTONS == (nOpCode&WR_REPORT_BUTTONS) && NULL != m_pButtons)
				{
					m_pButtons->OnButtonUpdate(buffer, nOffset, fCurrTick);
					nOffset += 2; // 2 bytes used for button data
				}
				if (WR_REPORT_MOTION == (nOpCode&WR_REPORT_MOTION) && NULL != m_pMotion)
				{
					m_pMotion->OnMotionUpdate(buffer, nOffset, fCurrTick);
					nOffset += 3; // 3 bytes used for motion data
				}
				if (WR_REPORT_IR == (nOpCode&WR_REPORT_IR) && NULL != m_pSensor)
				{
					m_pSensor->OnSensorUpdate(buffer, nOffset, fCurrTick);
					nOffset += 10; // 10 bytes used for IR data
				}
				if (WR_REPORT_EXTENSION == (nOpCode&WR_REPORT_EXTENSION) && NULL != m_pExtension)
				{
					m_pExtension->OnUpdate(WR_EXTENSION_UPDATE_REPORT, buffer, nOffset, fCurrTick);
				}
			}
		}
		else
			break; // Nothing left to read
	}

	// Finalize updates
	m_pButtons->OnPostUpdate();
	m_pMotion->OnPostUpdate();
	m_pData->OnPostUpdate();
	m_pSensor->OnPostUpdate();
	if (NULL != m_pExtension) m_pExtension->OnPostUpdate();

	// Check for connect timeout
	if (m_fConnectionTimeout > 0)
	{
		bool bLostConnection = false;
		if (true == CheckFlags(WRF_ATTEMPTCONNECT) && fCurrTick - m_fAttemptConnectStart > m_fConnectionTimeout)
		{
			bLostConnection = true;
		}
		if (true == CheckFlags(WRF_CONNECTED) && fCurrTick - m_fLastRecv > m_fConnectionTimeout)
		{
			bLostConnection = true;
		}

		// Did it happen?
		if (true == bLostConnection)
		{
			SetFlags(WRF_ATTEMPTCONNECT, false);
			SetFlags(WRF_CONNECTED, false);
			SetFlags(WRF_CHECKEDEXT, false);
			m_fAttemptConnectStart = 0;

			// Kill extension helper
			if (NULL != m_pExtension)
			{
				// Report extension destroyed
				for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
					(*itI)->OnExtensionUnplugged(this, m_pExtension);

				m_pExtension->Shutdown();
				SAFE_DELETE(m_pExtension);
			}

			// Report we failed to connect
			for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
				(*itI)->OnDisconnect(this, true);
			return;
		}
	}

	// Time for a new status update?
	if (true == CheckFlags(WRF_UPDATESTATUS) && fCurrTick >= m_fNextStatusUpdate)
	{
		RequestStatusUpdate();
		m_fNextStatusUpdate = fCurrTick + m_fStatusUpdateFreq;
	}
}

////////////////////////////////////////////////////
void CWR_WiiRemote::UpdateStatus(DataBuffer const& buffer)
{
	unsigned int nPrevFlags = (m_nFlags & WRF_STATUS_MASK) >> WRF_STATUS_SHIFT;
	unsigned int nPrevBattery = m_nBattery;

	// Check bits and set flags
	SetFlags(WRF_STATUS_EXPANSION, (buffer[WR_EXPANSION_STATUSBYTE]&WR_EXPANSION_CONTROLLER?true:false));
	SetFlags(WRF_STATUS_SPEAKER, (buffer[WR_EXPANSION_STATUSBYTE]&WR_EXPANSION_SPEAKER?true:false));
	SetFlags(WRF_STATUS_IR, (buffer[WR_EXPANSION_STATUSBYTE]&WR_EXPANSION_IR?true:false));
	SetFlags(WRF_STATUS_LED1, (buffer[WR_EXPANSION_STATUSBYTE]&WR_EXPANSION_LED1?true:false));
	SetFlags(WRF_STATUS_LED2, (buffer[WR_EXPANSION_STATUSBYTE]&WR_EXPANSION_LED2?true:false));
	SetFlags(WRF_STATUS_LED3, (buffer[WR_EXPANSION_STATUSBYTE]&WR_EXPANSION_LED3?true:false));
	SetFlags(WRF_STATUS_LED4, (buffer[WR_EXPANSION_STATUSBYTE]&WR_EXPANSION_LED4?true:false));

	// Check battery
	m_nBattery = (buffer[WR_EXPANSION_BATTERYBYTE]);

	// Check for differences
	unsigned int nCurrFlags = (m_nFlags & WRF_STATUS_MASK) >> WRF_STATUS_SHIFT;
	if (nPrevFlags != nCurrFlags || m_nBattery != nPrevBattery)
	{
		// Signal a change
		for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
			(*itI)->OnStatusUpdate(this, m_nFlags, m_nBattery);
	}

	// Extension status updated?
	if (false == CheckFlags(WRF_CHECKEDEXT) && true == CheckFlags(WRF_STATUS_EXPANSION))
	{
		// Initialize extension
		static WiiIOData InitData[1] = {0x00};
		m_pData->WriteData(WR_EXTENSION_REGISTERLOC, 1, InitData);

		// Reset report so that expansion data can start coming in
		SetReport(m_nReportMode, CheckFlags(WRF_CONTINUOUSREPORT));

		// Check its type
		m_pData->ReadData(WR_EXTENSION_TYPELOC, WR_EXTENSION_TYPESIZE);
	}
	else if (false == CheckFlags(WRF_STATUS_EXPANSION) && NULL != m_pExtension)
	{
		// Report extension destroyed
		for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
			(*itI)->OnExtensionUnplugged(this, m_pExtension);

		// Destroy the extension
		SAFE_DELETE(m_pExtension);
		SetFlags(WRF_CHECKEDEXT, false);
	}
}

////////////////////////////////////////////////////
void CWR_WiiRemote::OnDataRead(DataBuffer const& buffer)
{
	int nAddr = ((int)buffer[4]<<8)|buffer[5];

	// Was it extension type ret?
	if ((WR_EXTENSION_TYPELOC&0xFFFF) == nAddr && NULL == m_pExtension)
	{
		// Create the helper for it
		int nType = (buffer[6]<<8)|buffer[7];
		if (NULL != (m_pExtension = CreateExtensionHelper(nType)))
		{
			SetFlags(WRF_CHECKEDEXT, true);
			m_pExtension->Initialize(this);

			// Report extension created
			for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
				(*itI)->OnExtensionPluggedIn(this, m_pExtension);
		}
	}
}

////////////////////////////////////////////////////
void CWR_WiiRemote::SetFlags(unsigned int nFlags, bool bOn)
{
	if (true == bOn)
		m_nFlags = SET_BITS(nFlags,m_nFlags);
	else
		m_nFlags = CLEAR_BITS(nFlags,m_nFlags);
}

////////////////////////////////////////////////////
unsigned char CWR_WiiRemote::GetRumbleBit(void) const
{
	return (true == CheckFlags(WRF_RUMBLEON) ? WR_RUMBLE_ON : WR_RUMBLE_OFF);
}

////////////////////////////////////////////////////
bool CWR_WiiRemote::CheckFlags(unsigned int nFlags) const
{
	return CHECK_BITS(nFlags, m_nFlags);
}

////////////////////////////////////////////////////
void CWR_WiiRemote::SetConnectionTimeout(float fTimeout)
{
	m_fConnectionTimeout = fTimeout;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
void CWR_WiiRemote::WriteData(DataBuffer const& data)
{
	EnterCriticalSection(&_WROCS);
	_WriteQueue.push(data);
	LeaveCriticalSection(&_WROCS);
}

////////////////////////////////////////////////////
// WRO (writing) thread procedure
////////////////////////////////////////////////////
unsigned int __stdcall CWR_WiiRemote::WROThreadProc(void *pThis)
{
	CWR_WiiRemote *pRemote = (CWR_WiiRemote*)pThis;
	if (NULL == pRemote) 
	{
		_endthreadex(WR_WIIREMOTE_WRITETHREADFAIL);
		return WR_WIIREMOTE_WRITETHREADFAIL;
	}

	while (true)
	{
		// Get next thing to send
		EnterCriticalSection(&pRemote->_WROCS);
		if (true == pRemote->_WriteQueue.empty())
		{
			LeaveCriticalSection(&pRemote->_WROCS);

			// If we need to terminate, break out
			if (true == pRemote->_bWROThreadProcTerminate)
				break;

			// Continue on
			continue;
		}
		// Get it
		DataBuffer buffer = pRemote->_WriteQueue.front();
		LeaveCriticalSection(&pRemote->_WROCS);

		// Send it out
		DWORD dwWriteSize;
		if (TRUE == WriteFile(pRemote->m_hHandle, buffer, buffer, &dwWriteSize, &pRemote->m_hWROOverlap))
		{
			// Pop it if sent out correctly
			EnterCriticalSection(&pRemote->_WROCS);
			pRemote->_WriteQueue.pop();
			LeaveCriticalSection(&pRemote->_WROCS);
		}
		else
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				// Dump
				EnterCriticalSection(&pRemote->_WROCS);
				while (false == pRemote->_WriteQueue.empty()) pRemote->_WriteQueue.pop();
				LeaveCriticalSection(&pRemote->_WROCS);
			}
		}
	}

	_endthreadex(0);
	return 0;
}

////////////////////////////////////////////////////
// WRI (reading) thread procedure
////////////////////////////////////////////////////
unsigned int __stdcall CWR_WiiRemote::WRIThreadProc(void *pThis)
{
	CWR_WiiRemote *pRemote = (CWR_WiiRemote*)pThis;
	if (NULL == pRemote) 
	{
		_endthreadex(WR_WIIREMOTE_READTHREADFAIL);
		return WR_WIIREMOTE_READTHREADFAIL;
	}

	// Get overlapped object
	LPOVERLAPPED pOverlapped = &pRemote->m_hWROOverlap;

	while (false == pRemote->_bWRIThreadProcTerminate)
	{
		// Read data
		DataBuffer buffer;
		DWORD dwReadSize;
		if (TRUE == ReadFile(pRemote->m_hHandle, buffer, buffer, &dwReadSize, pOverlapped))
		{
			// Read the data into the read buffer
			EnterCriticalSection(&pRemote->_WRICS);
			pRemote->_ReadQueue.push(buffer);
			LeaveCriticalSection(&pRemote->_WRICS);
		}
		else
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				// Wait for event
				if (WAIT_TIMEOUT == WaitForSingleObject(pOverlapped->hEvent, 500))
					continue; // Check to make sure we aren't attempting to terminate

				// Async, wait for it to complete
				while (FALSE == GetOverlappedResult(pRemote->m_hHandle, pOverlapped, &dwReadSize, FALSE))
				{
					if (true == pRemote->_bWRIThreadProcTerminate)
						break;
				}

				// Read the data into the read buffer
				EnterCriticalSection(&pRemote->_WRICS);
				pRemote->_ReadQueue.push(buffer);
				LeaveCriticalSection(&pRemote->_WRICS);
			}
		}
	}

	_endthreadex(0);
	return 0;
}