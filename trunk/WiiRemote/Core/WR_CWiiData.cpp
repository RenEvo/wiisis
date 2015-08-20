////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiData.cpp
//
// Purpose: Helper for the Wii Remote that manages
//	data transfers to/from flash memory on controller
//
// History:
//	- 11/3/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "WR_Implementation.h"
#include "WR_CWiiData.h"
#include "WR_CWiiRemote.h"

////////////////////////////////////////////////////
CWR_WiiData::CWR_WiiData(void)
{
	m_pRemote = NULL;
	m_bWasUpdated = false;
}

////////////////////////////////////////////////////
CWR_WiiData::~CWR_WiiData(void)
{
	Shutdown();
}

////////////////////////////////////////////////////
bool CWR_WiiData::Initialize(IWR_WiiRemote *pRemote)
{
	m_pRemote = (CWR_WiiRemote*)pRemote;
	if (NULL == m_pRemote) return false;

	return true;
}

////////////////////////////////////////////////////
void CWR_WiiData::Shutdown(void)
{

}

////////////////////////////////////////////////////
void CWR_WiiData::OnDataRead(DataBuffer const& buffer, int nOffset, float fCurrTick)
{
	assert(m_pRemote);

	m_bWasUpdated = true;
	
	// Get address
	int nAddr = ((int)buffer[nOffset+1]<<8)|buffer[nOffset+2];
	int nSize = ((buffer[nOffset+0]&0xF0)>>4) + 1;

	// Check the error bit
	int nError = buffer[nOffset+0]&0x0F;
	if (WR_DATAERROR_SUCCESS != nError)
	{
		// An error occured
		for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
			(*itI)->OnDataError(m_pRemote, this, nAddr, nError);
	}

	// Send back the data report
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		(*itI)->OnDataRead(m_pRemote, this, nAddr, nSize, (LPWiiIOData)&buffer[nOffset+3]);

	// Pop and call callback routine
	WiiIOCallBackMap::iterator itCallBack = m_CallBackmap.find(nAddr);
	if (m_CallBackmap.end() != itCallBack)
	{
		WiiIOCallBackEntry entry = itCallBack->second.front();
		itCallBack->second.pop_front();
		if (NULL != entry.first) entry.first(nAddr, nSize, (LPWiiIOData)&buffer[nOffset+3], entry.second);

		// Erase it
		m_CallBackmap.erase(itCallBack);
	}
}

////////////////////////////////////////////////////
void CWR_WiiData::OnPostUpdate(void)
{
	m_bWasUpdated = false;
}

////////////////////////////////////////////////////
void CWR_WiiData::AddListener(IWR_WiiDataListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		if (*itI == pListener)
			return;
	m_Listeners.push_back(pListener);
}

////////////////////////////////////////////////////
void CWR_WiiData::RemoveListener(IWR_WiiDataListener *pListener)
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
void CWR_WiiData::ReadData(int nAddr, int nSize, WiiIOCallBack pCallBack, WiiIOCallBackParam pParam)
{
	// Add entry to callback map
	m_CallBackmap[(nAddr&0xFFFF)].push_back(WiiIOCallBackEntry(pCallBack,pParam));

	int nSent = 0, nSendSize = 0, nSendAddr = 0;

	// Construct buffer and send it
	do
	{
		// Calculate send size
		nSendAddr = nAddr + nSent;
		nSendSize = MIN(nSize-nSent, 16);

		// Write buffer
		DataBuffer buffer;
		buffer[0] = WR_OUT_READDATA;
		buffer[1] = (nSendAddr&0xFF000000)>>24 | m_pRemote->GetRumbleBit();
		buffer[2] = (nSendAddr&0x00FF0000)>>16;
		buffer[3] = (nSendAddr&0x0000FF00)>>8;
		buffer[4] = (nSendAddr&0x000000FF);
		buffer[5] = (nSendSize&0xFF00)>>8;
		buffer[6] = (nSendSize&0x00FF);
		m_pRemote->WriteData(buffer);

		// Increase send size
		nSent += nSendSize;

		Sleep(100);
	}
	while (nSent < nSize);
}

////////////////////////////////////////////////////
void CWR_WiiData::WriteData(int nAddr, int nSize, const LPWiiIOData pData) const
{
	int nSent = 0, nSendSize = 0, nSendAddr = 0;

	// Construct buffer and send it
	int offset = 0;
	do
	{
		// Calculate send size
		nSendAddr = nAddr + nSent;
		nSendSize = MIN(nSize-nSent, 16);

		// Write buffer
		DataBuffer buffer;
		buffer[0] = WR_OUT_WRITEDATA;
		buffer[1] = (nSendAddr&0xFF000000)>>24 | m_pRemote->GetRumbleBit();
		buffer[2] = (nSendAddr&0x00FF0000)>>16;
		buffer[3] = (nSendAddr&0x0000FF00)>>8;
		buffer[4] = (nSendAddr&0x000000FF);
		buffer[5] = nSendSize;
		for (DWORD i = 6; i < (DWORD)buffer && offset < nSize; i++)
		{
			buffer[i] = pData[offset++];
		}

		m_pRemote->WriteData(buffer);

		// Increase send size
		nSent += nSendSize;
	}
	while (nSent < nSize);
}