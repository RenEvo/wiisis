////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// MeleeMotion.cpp
//
// Purpose: Motion on Wiimote
//	Used to make the player melee attack with their
//	current weapon
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "Game.h"
#include "GameActions.h"
#include "Item.h"

#include "BaseMotion.h"

class CMeleeMotion : public CBaseMotion
{
public:
	// Motion params
	enum
	{
		STATE_BEGIN = 0x01,
		STATE_HIT_XG = 0x02,
		STATE_HIT_ZG = 0x04,
		STATE_NOHIT_BADXG = 0x08,
		STATE_END_R = 0x10,

		PASSED = (STATE_BEGIN|STATE_HIT_XG|STATE_HIT_ZG|STATE_NOHIT_BADXG|STATE_END_R),
	};

	////////////////////////////////////////////////////
	// CanMelee
	////////////////////////////////////////////////////
	virtual bool CanMelee(void) const
	{
		CPlayer *pPlayer = GetPlayer();
		CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem();
		return (NULL != pCurrentItem && NULL != pCurrentItem->GetIWeapon());
	}

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
		if (!CanMelee()) return;

		// Pitch must be close to level
		if (motion.fPitch >= DEG2RAD(-30.0f) && motion.fPitch <= DEG2RAD(30.0f))
			m_nMotionState |= STATE_BEGIN;

		// If X accel passes 3.0, bad hit
		if (motion.vAccel.x < 3.0f)
			m_nMotionState |= STATE_NOHIT_BADXG;
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
		if (!CanMelee()) return;

		// Must reach G force in Y and Z
		if (motion.vAccel.x <= -CHECK_PROFILE_FLOAT(MeleeSensitivity))
			m_nMotionState |= STATE_HIT_XG;
		if (motion.vAccel.z <= -CHECK_PROFILE_FLOAT(PickUpSensitivity))
			m_nMotionState |= STATE_HIT_ZG;

		// If X accel passes 3.0, bad hit
		if (motion.vAccel.x >= 3.0f)
			m_nMotionState &= ~STATE_NOHIT_BADXG;
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
		if (!CanMelee()) return;

		// Must reach G force in Y and Z
		if (motion.vAccel.x <= -CHECK_PROFILE_FLOAT(MeleeSensitivity))
			m_nMotionState |= STATE_HIT_XG;
		if (motion.vAccel.z <= -CHECK_PROFILE_FLOAT(PickUpSensitivity))
			m_nMotionState |= STATE_HIT_ZG;

		// Check final resting roll to see if it is turned sideways
		if (motion.fRoll <= DEG2RAD(-45.0f) && motion.fRoll >= DEG2RAD(-135.0f))
			m_nMotionState |= STATE_END_R;

		// If X accel passes 3.0, bad hit
		if (motion.vAccel.x >= 3.0f)
			m_nMotionState &= ~STATE_NOHIT_BADXG;
	}

	////////////////////////////////////////////////////
	// Execute
	//
	// Purpose: Execute the behavior for this motion
	////////////////////////////////////////////////////
	virtual void Execute(void)
	{
		CBaseMotion::Execute();
		CPlayer *pPlayer = GetPlayer();
		const CGameActions &rGameActions = g_pGame->Actions();

		// Get current item, and do special if it is a weapon
		CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem();
		if (NULL != pCurrentItem && NULL != pCurrentItem->GetIWeapon())
		{
			pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.special, eIS_Pressed, 1.0f);
		}
	}
};

REGISTER_MOTION(WIIMOTE, CMeleeMotion, CMeleeMotion::PASSED, 2.5f)