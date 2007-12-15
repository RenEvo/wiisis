////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// ThrowMotion.cpp
//
// Purpose: Motion on Wiimote
//	Used to make the player throw the item they
//	are looking at, or a grenade if hands are empty
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "Game.h"
#include "GameActions.h"
#include "Fists.h"

#include "BaseMotion.h"

class CThrowMotion : public CBaseMotion
{
public:
	// Motion params
	enum
	{
		STATE_HIT_YG = 0x01,
		STATE_HIT_ZG_N = 0x02,
		STATE_HIT_ZG_P = 0x04,
		STATE_END = 0x08,
		STATE_NOHIT_NEGY = 0x10,

		PASSED = (STATE_HIT_YG|STATE_HIT_ZG_N|STATE_HIT_ZG_P|STATE_END|STATE_NOHIT_NEGY),
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

		if (motion.vAccel.y > -1.5f)
			m_nMotionState |= STATE_NOHIT_NEGY;
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

		// Y acceleration
		if (motion.vAccel.y >= CHECK_PROFILE_FLOAT(ThrowSensitivity))
			m_nMotionState |= STATE_HIT_YG;

		// If Y acceleration ever goes below -1, kill Y accel and prevent it from
		//	being set again. This helps with punching gesture.
		if (motion.vAccel.y <= -1.5f)
			m_nMotionState &= ~STATE_NOHIT_NEGY;

		// Z acceleration (first neg, then pos)
		if (STATE_HIT_ZG_N != (m_nMotionState & STATE_HIT_ZG_N))
		{
			if (motion.vAccel.z <= -CHECK_PROFILE_FLOAT(ThrowSensitivity)*0.5f)
				m_nMotionState |= STATE_HIT_ZG_N;
		}
		else
		{
			if (motion.vAccel.z >= CHECK_PROFILE_FLOAT(ThrowSensitivity))
				m_nMotionState |= STATE_HIT_ZG_P;
		}
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

		// Y acceleration
		if (motion.vAccel.y >= CHECK_PROFILE_FLOAT(ThrowSensitivity))
			m_nMotionState |= STATE_HIT_YG;

		// If Y acceleration ever goes below -1, kill Y accel and prevent it from
		//	being set again. This helps with punching gesture.
		if (motion.vAccel.y <= -1.5f)
			m_nMotionState &= ~STATE_NOHIT_NEGY;

		// Z acceleration
		if (STATE_HIT_ZG_N == (m_nMotionState & STATE_HIT_ZG_N) &&
			motion.vAccel.z >= CHECK_PROFILE_FLOAT(ThrowSensitivity))
			m_nMotionState |= STATE_HIT_ZG_P;

		// Check ending pitch
		if (motion.fPitch >= DEG2RAD(-45.0f) && motion.fPitch <= DEG2RAD(45.0f) && 
			motion.fRoll > DEG2RAD(-45.0f) && motion.fRoll < DEG2RAD(45.0f))
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

		// Throw item in your hand if you have one
		COffHand* pOffHand = static_cast<COffHand*>(pPlayer->GetWeaponByClass(CItem::sOffHandClass));
		if (NULL != pOffHand)
		{
			int nOffHandState = pOffHand->GetOffHandState();
			if ((nOffHandState&(eOHS_HOLDING_OBJECT|eOHS_THROWING_OBJECT|eOHS_HOLDING_NPC|eOHS_THROWING_NPC)))
			{
				/*pOffHand->OnAction(pPlayer->GetEntityId(), rGameActions.attack1, eIS_Pressed, 1.0f);
				pOffHand->OnAction(pPlayer->GetEntityId(), rGameActions.attack1, eIS_Released, 1.0f);*/
				pOffHand->ThrowObject(eAAM_OnPress,nOffHandState&(eOHS_HOLDING_NPC|eOHS_THROWING_NPC)?true:false);
				pOffHand->ThrowObject(eAAM_OnRelease,nOffHandState&(eOHS_HOLDING_NPC|eOHS_THROWING_NPC)?true:false);
				g_pGame->GetWiiRemoteManager()->FreezeMovement();
				return;
			}

			// Throw a grenade instead
			pOffHand->OnAction(pPlayer->GetEntityId(), rGameActions.grenade, eIS_Pressed, 1.0f);
			pOffHand->OnAction(pPlayer->GetEntityId(), rGameActions.grenade, eIS_Released, 1.0f);
			g_pGame->GetWiiRemoteManager()->FreezeMovement();
		}
	}
};

REGISTER_MOTION(WIIMOTE, CThrowMotion, CThrowMotion::PASSED, 3.0f)