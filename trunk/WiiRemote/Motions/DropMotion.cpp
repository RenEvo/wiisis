////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// PickUpMotion.cpp
//
// Purpose: Motion on Wiimote
//	Used to make the player drop the item they are
//	holding or their weapon if hands are empty
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "Game.h"
#include "GameActions.h"
#include "OffHand.h"
#include "PlayerInput.h"

#include "BaseMotion.h"

class CDropMotion : public CBaseMotion
{
public:
	// Motion params
	enum
	{
		STATE_BEGIN = 0x01,
		STATE_HIT_XG = 0x02,
		STATE_HIT_ZG = 0x04,
		STATE_END = 0x08,

		PASSED = (STATE_BEGIN|STATE_HIT_XG|STATE_HIT_ZG|STATE_END),
	};

	////////////////////////////////////////////////////
	// CanDrop
	////////////////////////////////////////////////////
	virtual bool CanDrop(void)
	{
		CPlayer *pPlayer = GetPlayer();

		// Check if we are holding on to something
		COffHand* pOffHand = static_cast<COffHand*>(pPlayer->GetWeaponByClass(CItem::sOffHandClass));
		if (NULL != pOffHand)
		{
			if (pOffHand->GetOffHandState()&(eOHS_HOLDING_OBJECT|eOHS_HOLDING_NPC))
				return true;
		}

		// Check if we have an item out
		CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem();
		if (NULL != pCurrentItem && true == pCurrentItem->CanDrop()) return true;

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
		if (nState != STATE_PLAYER && nState != STATE_BINOCULARS) return;
		if (!CanDrop()) return;

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
		if (!CanDrop()) return;

		// Check G forces
		if (motion.vAccel.x <= -CHECK_PROFILE_FLOAT(DropSensitivity))
			m_nMotionState |= STATE_HIT_XG;
		if (motion.vAccel.z <= -CHECK_PROFILE_FLOAT(DropSensitivity))
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
		if (!CanDrop()) return;

		// Check G forces
		if (motion.vAccel.x <= -CHECK_PROFILE_FLOAT(DropSensitivity))
			m_nMotionState |= STATE_HIT_XG;
		if (motion.vAccel.z <= -CHECK_PROFILE_FLOAT(DropSensitivity))
			m_nMotionState |= STATE_HIT_ZG;

		// Check final roll
		if (motion.fRoll <= DEG2RAD(-45.0f) || motion.fRoll >= DEG2RAD(45.0f))
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

		// Throw item in hand first
		COffHand* pOffHand = static_cast<COffHand*>(pPlayer->GetWeaponByClass(CItem::sOffHandClass));
		if (NULL != pOffHand)
		{
			if (pOffHand->GetOffHandState()&(eOHS_HOLDING_OBJECT|eOHS_HOLDING_NPC))
			{
				pOffHand->OnAction(pPlayer->GetEntityId(), rGameActions.zoom, eIS_Pressed, 1.0f);
				pOffHand->OnAction(pPlayer->GetEntityId(), rGameActions.zoom, eIS_Released, 1.0f);
				g_pGame->GetWiiRemoteManager()->FreezeMovement();
				return;
			}
		}

		// Otherwise, get current item and drop it
		if (CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem())
		{
			pPlayer->GetPlayerInput()->OnAction(rGameActions.drop, eIS_Pressed, 1.0f);
			pPlayer->GetPlayerInput()->OnAction(rGameActions.drop, eIS_Released, 1.0f);
			g_pGame->GetWiiRemoteManager()->FreezeMovement();
		}
	}
};

REGISTER_MOTION(WIIMOTE, CDropMotion, CDropMotion::PASSED, 0.1f)