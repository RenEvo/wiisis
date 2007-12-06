////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// PunchMotion.cpp
//
// Purpose: Motion on Wiimote
//	Used to make the player punch with their fists
//	or with the item they are holding
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "Game.h"
#include "GameActions.h"
#include "Fists.h"

#include "BaseMotion.h"

class CPunchMotion : public CBaseMotion
{
public:
	// Motion params
	enum
	{
		STATE_BEGIN = 0x01,
		STATE_HITG = 0x02,
		STATE_HITNG = 0x04,
		STATE_END = 0x08,

		PASSED = (STATE_BEGIN|STATE_HITG|STATE_HITNG|STATE_END),
	};

	////////////////////////////////////////////////////
	// CanPunch
	////////////////////////////////////////////////////
	virtual bool CanPunch(void) const
	{
		CPlayer *pPlayer = GetPlayer();

		// Check if we are holding on to something
		COffHand* pOffHand = static_cast<COffHand*>(pPlayer->GetWeaponByClass(CItem::sOffHandClass));
		if (NULL != pOffHand)
		{
			if (pOffHand->GetOffHandState()&(eOHS_HOLDING_OBJECT|eOHS_HOLDING_NPC))
				return false;
		}

		// Check if fists are out
		CFists *pFists = static_cast<CFists*>(pPlayer->GetItemByClass(CItem::sFistsClass));
		CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem();
		if (NULL != pCurrentItem && NULL != pFists && pFists->GetEntityId() == pCurrentItem->GetEntityId())
			return true;

		// No good
		return false;
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
		if (nState != STATE_PLAYER) return;
		if (!CanPunch()) return;

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
		if (nState != STATE_PLAYER) return;
		if (!CanPunch()) return;

		// Must reach G force in Z
		if (STATE_HITNG != (m_nMotionState & STATE_HITNG))
		{
			if (motion.vAccel.y <= -CHECK_PROFILE_FLOAT(PunchSensitivity))
				m_nMotionState |= STATE_HITNG;
		}
		else if (motion.vAccel.y >= CHECK_PROFILE_FLOAT(PunchSensitivity))
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
		if (nState != STATE_PLAYER) return;
		if (!CanPunch()) return;

		// Must reach G force in Z
		if (STATE_HITNG == (m_nMotionState & STATE_HITNG) && motion.vAccel.y >= CHECK_PROFILE_FLOAT(PunchSensitivity))
			m_nMotionState |= STATE_HITG;

		// Check roll
		if (motion.fRoll > DEG2RAD(-45.0f) && motion.fRoll < DEG2RAD(45.0f))
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

		// If they are holding on to an item, melee with it
		COffHand* pOffHand = static_cast<COffHand*>(pPlayer->GetWeaponByClass(CItem::sOffHandClass));
		if (NULL != pOffHand)
		{
			if (pOffHand->GetOffHandState()&(eOHS_HOLDING_OBJECT|eOHS_HOLDING_NPC))
			{
				pOffHand->OnAction(pPlayer->GetEntityId(), rGameActions.special, eIS_Pressed, 1.0f);
				return;
			}
		}

		// Check if fists are out
		CFists *pFists = static_cast<CFists*>(pPlayer->GetItemByClass(CItem::sFistsClass));
		CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem();
		if (NULL != pCurrentItem && NULL != pFists && pFists->GetEntityId() == pCurrentItem->GetEntityId())
			pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.attack1, eIS_Pressed, 1.0f);
	}
};

REGISTER_MOTION(WIIMOTE, CPunchMotion, CPunchMotion::PASSED, 1.0f)