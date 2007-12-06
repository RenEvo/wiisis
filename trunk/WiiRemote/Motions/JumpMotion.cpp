////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// JumpMotion.cpp
//
// Purpose: Motion on Nunchuk
//	Used to make the player jump in the air, like he
//	just doesn't care!
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "Game.h"
#include "BaseMotion.h"

class CJumpMotion : public CBaseMotion
{
public:
	// Motion params
	enum
	{
		STATE_BEGIN = 0x01,
		STATE_HITG = 0x02,

		PASSED = (STATE_BEGIN|STATE_HITG),
	};

	////////////////////////////////////////////////////
	// OnBegin
	//
	// Purpose: Called when a gesture is starting
	//
	// In:	motion - Motion params
	////////////////////////////////////////////////////
	virtual void OnBegin(SMotionElement const& motion, int nState)
	{
		CBaseMotion::OnBegin(motion, nState);
		if (nState != STATE_PLAYER && nState != STATE_BINOCULARS) return;

		// Pitch must be between crouch tilt
		float fCrouchTilt = CHECK_PROFILE_FLOAT(CrouchTilt);
		if (motion.fPitch >= DEG2RAD(-fCrouchTilt) && motion.fPitch <= DEG2RAD(fCrouchTilt))
			m_nMotionState |= STATE_BEGIN;
	}

	////////////////////////////////////////////////////
	// OnUpdate
	//
	// Purpose: Called when a gesture is updated
	//
	// In:	motion - Motion params
	////////////////////////////////////////////////////
	virtual void OnUpdate(SMotionElement const& motion, int nState)
	{
		CBaseMotion::OnUpdate(motion, nState);
		if (nState != STATE_PLAYER && nState != STATE_BINOCULARS) return;

		// Must reach G force in Z
		if (motion.vAccel.z >= CHECK_PROFILE_FLOAT(JumpSensitivity))
			m_nMotionState |= STATE_HITG;
	}

	////////////////////////////////////////////////////
	// OnEnd
	//
	// Purpose: Called when a gesture has ended
	//
	// In:	motion - Motion params
	////////////////////////////////////////////////////
	virtual void OnEnd(SMotionElement const& motion, int nState)
	{
		CBaseMotion::OnEnd(motion, nState);
		if (nState != STATE_PLAYER && nState != STATE_BINOCULARS) return;

		// Must reach G force in Z
		if (motion.vAccel.z >= CHECK_PROFILE_FLOAT(JumpSensitivity))
			m_nMotionState |= STATE_HITG;
	}

	////////////////////////////////////////////////////
	// Execute
	//
	// Purpose: Execute the behavior for this motion
	////////////////////////////////////////////////////
	virtual void Execute(void)
	{
		CBaseMotion::Execute();

		// Make player jump
		CMovementRequest mr;
		mr.SetJump();
		mr.SetStance(STANCE_STAND);
		mr.SetLean(0.0f);
		GetPlayer()->GetMovementController()->RequestMovement(mr);
	}
};

REGISTER_MOTION(NUNCHUK, CJumpMotion, CJumpMotion::PASSED, 1.0f)