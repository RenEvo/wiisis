////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// PickUpMotion.cpp
//
// Purpose: Motion on Wiimote
//	Used to make the player pick up the item they
//	are looking at
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "Game.h"
#include "GameActions.h"
#include "Fists.h"

#include "BaseMotion.h"

class CPickUpMotion : public CBaseMotion
{
public:
	// Motion params
	enum
	{
		STATE_BEGIN = 0x01,
		STATE_HIT_YG = 0x02,
		STATE_HIT_ZG = 0x04,
		STATE_END = 0x08,

		PASSED = (STATE_BEGIN|STATE_HIT_YG|STATE_HIT_ZG|STATE_END),
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

		// Pitch must be close to level
		if (motion.fPitch >= DEG2RAD(-30.0f) && motion.fPitch <= DEG2RAD(30.0f))
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

		// Must reach G force in Y and Z
		if (motion.vAccel.y >= CHECK_PROFILE_FLOAT(PickUpSensitivity))
			m_nMotionState |= STATE_HIT_YG;
		if (motion.vAccel.z >= CHECK_PROFILE_FLOAT(PickUpSensitivity))
			m_nMotionState |= STATE_HIT_ZG;
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

		// Must reach G force in Y and Z
		if (motion.vAccel.y >= CHECK_PROFILE_FLOAT(PickUpSensitivity))
			m_nMotionState |= STATE_HIT_YG;
		if (motion.vAccel.z >= CHECK_PROFILE_FLOAT(PickUpSensitivity))
			m_nMotionState |= STATE_HIT_ZG;

		// Must be above 45deg in pitch
		if (motion.fPitch >= DEG2RAD(45.0f))
			m_nMotionState |= STATE_END;
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

		// Pick the item in front of them up
		COffHand* pOffHand = static_cast<COffHand*>(pPlayer->GetWeaponByClass(CItem::sOffHandClass));
		if (NULL != pOffHand)
		{
			if (!(pOffHand->GetOffHandState()&(eOHS_HOLDING_OBJECT|eOHS_HOLDING_NPC|eOHS_THROWING_OBJECT|eOHS_THROWING_NPC)))
			{
				pOffHand->OnAction(pPlayer->GetEntityId(), rGameActions.use, eIS_Pressed, 1.0f);
				g_pGame->GetWiiRemoteManager()->FreezeMovement();
			}
		}
	}
};

REGISTER_MOTION(WIIMOTE, CPickUpMotion, CPickUpMotion::PASSED, 2.0f)