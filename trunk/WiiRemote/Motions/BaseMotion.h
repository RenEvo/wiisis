////////////////////////////////////////////////////////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// BaseMotion.h
//
// Purpose: Base motion all others inherit from
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _BASEMOTION_H_
#define _BASEMOTION_H_

#include "..\Interfaces\IMotion.h"
#include "..\CWiiRemoteManager.h"
#include "..\CWiiRemoteProfile.h"
#include "Game.h"
#include "Player.h"

// Must go last!
#include "..\Core\WR_Implementation.h"

////////////////////////////////////////////////////
class CBaseMotion;
class CMotionAutoReg
{
public:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CMotionAutoReg(int nController,int nPassMask,float fPriority)
	{
		// Put in static list
		if (NULL == m_pLast) m_pFirst = this;
		else m_pLast->m_pNext = this;
		m_pLast = this;
		m_pNext = NULL;
		m_nController = nController;
		m_nPassMask = nPassMask;
		m_fPriority = fPriority;
	}

	////////////////////////////////////////////////////
	// Create
	//
	// Purpose: Called to create instance of registered
	//	motion object
	////////////////////////////////////////////////////
	virtual IMotion* Create(void) = 0;

	////////////////////////////////////////////////////
	int m_nController;
	int m_nPassMask;
	float m_fPriority;
	CMotionAutoReg *m_pNext;
	static CMotionAutoReg *m_pFirst;
	static CMotionAutoReg *m_pLast;
};
template <class T>
class CMotionAutoRegEntry : public CMotionAutoReg
{
public:
	CMotionAutoRegEntry(int nController,int nPassMask,float fPriority) : CMotionAutoReg(nController,nPassMask,fPriority) {}
	virtual IMotion* Create(void)
	{
		T* ret = new T();
		assert(ret);
		ret->SetPassMask(m_nPassMask);
		ret->SetPriority(m_fPriority);
		return ret;
	}
};

////////////////////////////////////////////////////
// Use this define to register a new motion control
// Ex. REGISTER_MOTION(NUNCHUK, CJumpMotion)
////////////////////////////////////////////////////
#define WIIMOTE 0
#define NUNCHUK 1
#define REGISTER_MOTION(Controller, MotionClass, PassMask, Priority) \
	CMotionAutoRegEntry<MotionClass> g_MotionReg##MotionClass(Controller,PassMask,Priority);

////////////////////////////////////////////////////
class CBaseMotion : public IMotion
{
protected:
	bool m_bFired;
	float m_fPriority;
	int m_nMotionState;

	// To be set by inherited class!
	int PASSED_MASK;

	////////////////////////////////////////////////////
	// GetPlayer
	//
	// Purpose: Get the client actor
	////////////////////////////////////////////////////
	CPlayer* GetPlayer(void) const;

public:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CBaseMotion(void);

	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CBaseMotion(void);

	////////////////////////////////////////////////////
	// IsFired
	//
	// Purpose: Returns TRUE if motion has fired
	////////////////////////////////////////////////////
	virtual bool IsFired(void) const;

	////////////////////////////////////////////////////
	// SetPassMask
	//
	// Purpose: Set the mask used for testing pass/fired
	////////////////////////////////////////////////////
	virtual void SetPassMask(int nMask);

	////////////////////////////////////////////////////
	// GetPriority
	//
	// Purpose: Return the motion's priority level
	////////////////////////////////////////////////////
	virtual float GetPriority(void) const;

	////////////////////////////////////////////////////
	// SetPriority
	//
	// Purpose: Set the motion's priority level
	//
	// In:	fPriority - Motion's priority, in case multiple
	//			motions succeed in the same gesture
	////////////////////////////////////////////////////
	virtual void SetPriority(float fPriority);

	////////////////////////////////////////////////////
	// OnBegin
	//
	// Purpose: Called when a gesture is starting
	//
	// In:	motion - Motion params
	////////////////////////////////////////////////////
	virtual void OnBegin(SMotionElement const& motion, int nState);

	////////////////////////////////////////////////////
	// OnUpdate
	//
	// Purpose: Called when a gesture is updated
	//
	// In:	motion - Motion params
	////////////////////////////////////////////////////
	virtual void OnUpdate(SMotionElement const& motion, int nState);

	////////////////////////////////////////////////////
	// OnEnd
	//
	// Purpose: Called when a gesture has ended
	//
	// In:	motion - Motion params
	////////////////////////////////////////////////////
	virtual void OnEnd(SMotionElement const& motion, int nState);

	////////////////////////////////////////////////////
	// Execute
	//
	// Purpose: Execute the behavior for this motion
	////////////////////////////////////////////////////
	virtual void Execute(void);
};

#endif //_BASEMOTION_H_