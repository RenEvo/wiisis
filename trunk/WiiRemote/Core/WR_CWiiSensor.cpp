////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CWiiSensor.cpp
//
// Purpose: Helper for the Wii Remote that manages
//	IR sensor readings/updating
//
// History:
//	- 11/23/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "WR_Implementation.h"
#include "WR_CWiiSensor.h"
#include "WR_CWiiRemote.h"

////////////////////////////////////////////////////
CWR_WiiSensor::CWR_WiiSensor(void)
{
	m_pRemote = NULL;
	m_bWasUpdated = false;
	m_bEnabled = false;
	for (int i = 0; i < 4; i++)
	{
		m_SensorDots[i].bTargeted = false;
		m_SensorDots[i].nRawX = 0x3FF;
		m_SensorDots[i].nRawY = 0x3FF;
	}

	m_bOnScreen = false;
	m_fLastOnScreenTime = 0.0f;
	m_fX = 0.0f;
	m_fY = 0.0f;
}

////////////////////////////////////////////////////
CWR_WiiSensor::~CWR_WiiSensor(void)
{

}

////////////////////////////////////////////////////
bool CWR_WiiSensor::Initialize(IWR_WiiRemote *pRemote)
{
	m_pRemote = (CWR_WiiRemote*)pRemote;
	if (NULL == m_pRemote) return false;

	// Reset data
	m_bEnabled = false;
	for (int i = 0; i < 4; i++)
	{
		m_SensorDots[i].bTargeted = false;
		m_SensorDots[i].nRawX = 0x3FF;
		m_SensorDots[i].nRawY = 0x3FF;
	}

	m_bOnScreen = false;
	m_fLastOnScreenTime = 0.0f;
	m_fX = 0.0f;
	m_fY = 0.0f;

	return true;
}

////////////////////////////////////////////////////
void CWR_WiiSensor::Shutdown(void)
{
	m_pRemote = NULL;
}

////////////////////////////////////////////////////
void CWR_WiiSensor::OnSensorUpdate(struct DataBuffer const& buffer, int nOffset, float fCurrTick)
{
	m_SensorDots[0].bTargeted = false;
	m_SensorDots[1].bTargeted = false;
	m_SensorDots[2].bTargeted = false;
	m_SensorDots[3].bTargeted = false;

	// Parse each dot in the data
	if (buffer[nOffset+0] != 0xFF || buffer[nOffset+1] != 0xFF)
	{
		m_SensorDots[0].bTargeted = true;
		m_SensorDots[0].nRawX = buffer[nOffset+0] | (((buffer[nOffset+2]>>4)&0x3) << 8);
		m_SensorDots[0].nRawY = buffer[nOffset+1] | (((buffer[nOffset+2]>>6)&0x3) << 8);
	}
	if (buffer[nOffset+3] != 0xFF || buffer[nOffset+4] != 0xFF)
	{
		m_SensorDots[1].bTargeted = true;
		m_SensorDots[1].nRawX = buffer[nOffset+3] | (((buffer[nOffset+2]>>0)&0x3) << 8);
		m_SensorDots[1].nRawY = buffer[nOffset+4] | (((buffer[nOffset+2]>>2)&0x3) << 8);
	}
	/*if (buffer[nOffset+5] != 0xFF || buffer[nOffset+6] != 0xFF)
	{
		m_SensorDots[2].bTargeted = true;
		m_SensorDots[2].nRawX = buffer[nOffset+5] | (((buffer[nOffset+7]>>4)&0x3) << 8);
		m_SensorDots[2].nRawY = buffer[nOffset+6] | (((buffer[nOffset+7]>>6)&0x3) << 8);
	}
	if (buffer[nOffset+8] != 0xFF || buffer[nOffset+9] != 0xFF)
	{
		m_SensorDots[3].bTargeted = true;
		m_SensorDots[3].nRawX = buffer[nOffset+8] | (((buffer[nOffset+7]>>0)&0x3) << 8);
		m_SensorDots[3].nRawY = buffer[nOffset+9] | (((buffer[nOffset+7]>>2)&0x3) << 8);
	}*/
}

////////////////////////////////////////////////////
void CWR_WiiSensor::OnPostUpdate(void)
{
	if (m_SensorDots[0].bTargeted || m_SensorDots[1].bTargeted || 
		m_SensorDots[2].bTargeted || m_SensorDots[3].bTargeted)
	{
		// Determine cursor point based on raw data
		m_fX = 1.0f - float(m_SensorDots[0].nRawX)/WR_WIISENSOR_MAX_X;
		m_fY = float(m_SensorDots[0].nRawY)/WR_WIISENSOR_MAX_Y;
		for (int i = 1; i < 4; i++)
		{
			// Convert to camera zone
			if (m_SensorDots[i].nRawX != 0x3FF)
				m_fX = (m_fX + 1.0f - float(m_SensorDots[i].nRawX)/WR_WIISENSOR_MAX_X) * 0.5f;
			if (m_SensorDots[i].nRawY != 0x3FF)
				m_fY = (m_fY + float(m_SensorDots[i].nRawY)/WR_WIISENSOR_MAX_Y) * 0.5f;
		}

		// Remember data
		bool bFirstOn = !m_bOnScreen;
		m_bOnScreen = true;

		// Did we just enter the screen?
		if (true == bFirstOn)
		{
			for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
				(*itI)->OnEnterScreen(m_pRemote, this, m_fX, m_fY);
		}
		else
		{
			// Send update report
			for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
				(*itI)->OnCursorUpdate(m_pRemote, this, m_fX, m_fY);
		}

		m_fLastOnScreenTime = g_pWR->pTimer->GetCurrTime();
	}
	else
	{
		// Reset outter point data
		m_SensorDots[2].nRawX = m_SensorDots[2].nRawY = 0x3FF;
		m_SensorDots[3].nRawX = m_SensorDots[3].nRawY = 0x3FF;

		// Did we just leave the screen
		if (true == m_bOnScreen)
		{
			for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
				(*itI)->OnLeaveScreen(m_pRemote, this, m_fX, m_fY);
		}

		// Reset data last so we can use it on leave
		m_bOnScreen = false;
		m_fX = 0.0f;
		m_fY = 0.0f;
	}
}

////////////////////////////////////////////////////
void CWR_WiiSensor::AddListener(IWR_WiiSensorListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		if (*itI == pListener)
			return;
	m_Listeners.push_back(pListener);
}

////////////////////////////////////////////////////
void CWR_WiiSensor::RemoveListener(IWR_WiiSensorListener *pListener)
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
bool CWR_WiiSensor::IsEnabled(void) const
{
	return m_bEnabled;
}

////////////////////////////////////////////////////
void CWR_WiiSensor::OnSetReport(int nMode)
{
	// Write to both registries first
	DataBuffer buffer;
	buffer[1] = 0x04 | m_pRemote->GetRumbleBit();
	buffer[0] = WR_OUT_IR_RUMBLE; m_pRemote->WriteData(buffer);
	buffer[0] = WR_OUT_IR2; m_pRemote->WriteData(buffer);

	// Prepare IR sensor reading
	static WiiIOData IR_MASTER[] = {0x08};
	m_pRemote->GetDataHelper()->WriteData(WR_SENSORREG_MASTER, sizeof(IR_MASTER), IR_MASTER);

	// Send sensitivity info
	static WiiIOData IR_SENSITIVITY_1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0xC0};
	static WiiIOData IR_SENSITIVITY_2[] = {0x40, 0x00};
	m_pRemote->GetDataHelper()->WriteData(WR_SENSORREG_SENSITIVITY_1, sizeof(IR_SENSITIVITY_1), IR_SENSITIVITY_1);
	m_pRemote->GetDataHelper()->WriteData(WR_SENSORREG_SENSITIVITY_2, sizeof(IR_SENSITIVITY_2), IR_SENSITIVITY_2);

	// Set mode
	static WiiIOData IR_MODE[] = {0x01};
	static WiiIOData IR_MASTER_FIN[] = {0x08};
	m_pRemote->GetDataHelper()->WriteData(WR_SENSORREG_MODE, sizeof(IR_MODE), IR_MODE);
	//m_pRemote->GetDataHelper()->WriteData(WR_SENSORREG_MASTER, sizeof(IR_MASTER_FIN), IR_MASTER_FIN);

	m_bEnabled = true;
	m_fLastOnScreenTime = g_pWR->pTimer->GetCurrTime();
}