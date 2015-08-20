//////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CTimer.cpp
//
// Purpose: High-resolution timer
//
// History:
//	- 11/3/07 : File created - KAK
//////////////////////////////////////////////////////

#include "stdafx.h"
#include "WR_Implementation.h"
#include "WR_CTimer.h"

REGISTER_WR_MODULE(CWR_Timer, WIIREMOTE);

//////////////////////////////////////////////////////
CWR_Timer::CWR_Timer(void)
{
	m_fDT = 0.0f;
	m_fLifeTime = 0.0f;
}

//////////////////////////////////////////////////////
CWR_Timer::~CWR_Timer(void)
{
	Shutdown();
}

//////////////////////////////////////////////////////
int CWR_Timer::Initialize(void)
{
	QueryPerformanceCounter(&m_nStartTimer);
	m_nLastUpdate = m_nStartTimer; // Last update is the start!
	m_fDT = 0.0f;
	m_fLifeTime = 0.0f;

	return WR_WIITIMER_OK;
}

//////////////////////////////////////////////////////
void CWR_Timer::Shutdown(void)
{

}

//////////////////////////////////////////////////////
void CWR_Timer::Update(void)
{
	LARGE_INTEGER nUpdate;
	QueryPerformanceCounter(&nUpdate);

	// Calculate FDT
	LARGE_INTEGER nFreq;
	QueryPerformanceFrequency(&nFreq);
	m_fDT = (float)(nUpdate.QuadPart-m_nLastUpdate.QuadPart)/(float)nFreq.QuadPart;
	m_fLifeTime += m_fDT;

	// Remark the last update
	m_nLastUpdate = nUpdate;
}

//////////////////////////////////////////////////////
float CWR_Timer::GetCurrTime(void) const
{
	// Return time elapsed from start to last update
	LARGE_INTEGER nFreq;
	QueryPerformanceFrequency(&nFreq);
	return (float)(m_nLastUpdate.QuadPart-m_nStartTimer.QuadPart)/(float)nFreq.QuadPart;
}

//////////////////////////////////////////////////////
float CWR_Timer::GetPreciseTime(void) const
{
	// Get current time
	LARGE_INTEGER nCurrTime;
	QueryPerformanceCounter(&nCurrTime);

	// Return time elapsed from start to now
	LARGE_INTEGER nFreq;
	QueryPerformanceFrequency(&nFreq);
	return (float)(nCurrTime.QuadPart-m_nStartTimer.QuadPart)/(float)nFreq.QuadPart;
}

//////////////////////////////////////////////////////
float CWR_Timer::GetDeltaTime(void) const
{
	return m_fDT;
}

//////////////////////////////////////////////////////
float CWR_Timer::GetLifeTime(void) const
{
	return m_fLifeTime;
}