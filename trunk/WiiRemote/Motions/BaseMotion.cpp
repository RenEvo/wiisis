////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// BaseMotion.cpp
//
// Purpose: Base motion all others inherit from
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseMotion.h"

CMotionAutoReg* CMotionAutoReg::m_pFirst = NULL;
CMotionAutoReg* CMotionAutoReg::m_pLast = NULL;

////////////////////////////////////////////////////
CPlayer* CBaseMotion::GetPlayer(void) const
{
	CPlayer *p = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	assert(p);
	return p;
}

////////////////////////////////////////////////////
CBaseMotion::CBaseMotion(void)
{
	m_fPriority = 0.0f;
	m_bFired = false;
	m_nMotionState = 0;
	PASSED_MASK = 0;
}

////////////////////////////////////////////////////
CBaseMotion::~CBaseMotion(void)
{

}

////////////////////////////////////////////////////
bool CBaseMotion::IsFired(void) const
{
	return ((m_nMotionState & PASSED_MASK) == PASSED_MASK);
}

////////////////////////////////////////////////////
void CBaseMotion::SetPassMask(int nMask)
{
	PASSED_MASK = nMask;
}

////////////////////////////////////////////////////
float CBaseMotion::GetPriority(void) const
{
	return m_fPriority;
}

////////////////////////////////////////////////////
void CBaseMotion::SetPriority(float fPriority)
{
	m_fPriority = fPriority;
}

////////////////////////////////////////////////////
void CBaseMotion::OnBegin(SMotionElement const& motion, int nState)
{
	m_nMotionState = 0;
}

////////////////////////////////////////////////////
void CBaseMotion::OnUpdate(SMotionElement const& motion, int nState)
{

}

////////////////////////////////////////////////////
void CBaseMotion::OnEnd(SMotionElement const& motion, int nState)
{

}

////////////////////////////////////////////////////
void CBaseMotion::Execute(void)
{

}