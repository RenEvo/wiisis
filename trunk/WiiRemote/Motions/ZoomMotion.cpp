////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// ZoomMotion.cpp
//
// Purpose: Motion on Wiimote
//	Used to zoom in/out in binoculars, etc
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "Game.h"
#include "GameActions.h"
#include "Item.h"
#include "Binocular.h"
#include "IVehicleSystem.h"

#include "BaseMotion.h"

class CZoomInMotion : public CBaseMotion
{
public:
	// Motion params
	enum
	{
		STATE_BEGIN = 0x01,
		STATE_GOODG = 0x02,
		STATE_BADG = 0x04,
		STATE_END = 0x08,

		PASSED = (STATE_BEGIN|STATE_GOODG|STATE_END),
	};

	////////////////////////////////////////////////////
	// CanZoom
	////////////////////////////////////////////////////
	virtual bool CanZoom(void) const
	{
		CPlayer *pPlayer = GetPlayer();

		// Zoom in on current item
		if (CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem())
		{
			CBinocular *pBinoculars = static_cast<CBinocular*>(GetPlayer()->GetItemByClass(CItem::sBinocularsClass));
			if (NULL != pBinoculars && pCurrentItem->GetEntityId() == pBinoculars->GetEntityId())
				return true;
			if (NULL != pCurrentItem->GetIWeapon())
				return true;
		}

		// Zoom in if in a vehicle and the gunner
		if (IVehicle *pVehicle = pPlayer->GetLinkedVehicle())
		{
			if (IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId()))
			{
				if (pSeat->IsGunner()) return true;
			}
		}

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
		if (nState == STATE_NANOSUITMENU || nState == STATE_WEAPONMENU) return;
		if (!CanZoom()) return;

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
		if (nState == STATE_NANOSUITMENU || nState == STATE_WEAPONMENU) return;
		if (!CanZoom()) return;
		const float fSensitivity = CHECK_PROFILE_FLOAT(ZoomInSensitivity);

		// If we hit good G, make sure we haven't already hit bad G
		if (motion.vAccel.y <= -fSensitivity && ((m_nMotionState & STATE_BADG) != STATE_BADG))
			m_nMotionState |= STATE_GOODG;

		// See if we hit bad G
		if (motion.vAccel.y >= fSensitivity)
			m_nMotionState |= STATE_BADG;
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
		if (nState == STATE_NANOSUITMENU || nState == STATE_WEAPONMENU) return;
		if (!CanZoom()) return;
		const float fSensitivity = CHECK_PROFILE_FLOAT(ZoomInSensitivity);

		// If we hit good G, make sure we haven't already hit bad G
		if (motion.vAccel.y <= -fSensitivity && ((m_nMotionState & STATE_BADG) != STATE_BADG))
			m_nMotionState |= STATE_GOODG;

		// See if we hit bad G
		if (motion.vAccel.y >= fSensitivity)
			m_nMotionState |= STATE_BADG;

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
		const CGameActions &rGameActions = g_pGame->Actions();
		CPlayer *pPlayer = GetPlayer();

		// Zoom in on current item
		CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem();
		if (NULL != pCurrentItem)
		{
			CBinocular *pBinoculars = static_cast<CBinocular*>(GetPlayer()->GetItemByClass(CItem::sBinocularsClass));
			if (NULL != pBinoculars && pCurrentItem->GetEntityId() == pBinoculars->GetEntityId())
			{
				// Let binoculars handle action
				pCurrentItem->OnAction(pPlayer->GetEntityId(), rGameActions.zoom_in, eIS_Pressed, 1.0f);
			}
			else if (NULL != pCurrentItem->GetIWeapon())
			{
				// Must be a weapon
				// If not zoomed, enter zoom mode; otherwise, zoom in
				if (false == pCurrentItem->GetIWeapon()->IsZoomed())
					pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.zoom, eIS_Pressed, 1.0f);
				else
					pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.zoom_in, eIS_Pressed, 1.0f);
			}
		}

		// Zoom in with vehicle then
		if (IVehicle *pVehicle = pPlayer->GetLinkedVehicle())
		{
			EntityId nWeaponId = pVehicle->GetCurrentWeaponId(pPlayer->GetEntityId());
			if (CItem *pWeaponItem = (CItem*)gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(nWeaponId))
			{
				if (NULL != pWeaponItem->GetIWeapon())
				{
					// Must be a weapon
					// If not zoomed, enter zoom mode; otherwise, zoom in
					if (false == pWeaponItem->GetIWeapon()->IsZoomed())
						pWeaponItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.zoom, eIS_Pressed, 1.0f);
					else
						pWeaponItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.zoom_in, eIS_Pressed, 1.0f);
				}
			}
		}
	}
};

class CZoomOutMotion : public CBaseMotion
{
public:
	// Motion params
	enum
	{
		STATE_BEGIN = 0x01,
		STATE_GOODG = 0x02,
		STATE_BADG = 0x04,
		STATE_END = 0x08,

		PASSED = (STATE_BEGIN|STATE_GOODG|STATE_END),
	};

	////////////////////////////////////////////////////
	// CanZoom
	////////////////////////////////////////////////////
	virtual bool CanZoom(void) const
	{
		CPlayer *pPlayer = GetPlayer();

		// Zoom in on current item
		if (CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem())
		{
			CBinocular *pBinoculars = static_cast<CBinocular*>(GetPlayer()->GetItemByClass(CItem::sBinocularsClass));
			if (NULL != pBinoculars && pCurrentItem->GetEntityId() == pBinoculars->GetEntityId())
				return true;
			if (NULL != pCurrentItem->GetIWeapon())
				return true;
		}

		// Zoom in if in a vehicle and the gunner
		if (IVehicle *pVehicle = pPlayer->GetLinkedVehicle())
		{
			if (IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId()))
			{
				if (pSeat->IsGunner()) return true;
			}
		}

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
		if (nState == STATE_NANOSUITMENU || nState == STATE_WEAPONMENU) return;
		if (!CanZoom()) return;

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
		if (nState == STATE_NANOSUITMENU || nState == STATE_WEAPONMENU) return;
		if (!CanZoom()) return;
		const float fSensitivity = CHECK_PROFILE_FLOAT(ZoomOutSensitivity);

		// If we hit good G, make sure we haven't already hit bad G
		if (motion.vAccel.y >= fSensitivity && ((m_nMotionState & STATE_BADG) != STATE_BADG))
			m_nMotionState |= STATE_GOODG;

		// See if we hit bad G
		if (motion.vAccel.y <= -fSensitivity)
			m_nMotionState |= STATE_BADG;
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
		if (nState == STATE_NANOSUITMENU || nState == STATE_WEAPONMENU) return;
		if (!CanZoom()) return;
		const float fSensitivity = CHECK_PROFILE_FLOAT(ZoomOutSensitivity);

		// If we hit good G, make sure we haven't already hit bad G
		if (motion.vAccel.y >= fSensitivity && ((m_nMotionState & STATE_BADG) != STATE_BADG))
			m_nMotionState |= STATE_GOODG;

		// See if we hit bad G
		if (motion.vAccel.y <= -fSensitivity)
			m_nMotionState |= STATE_BADG;

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
		const CGameActions &rGameActions = g_pGame->Actions();
		CPlayer *pPlayer = GetPlayer();

		// Zoom out on current item
		CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem();
		if (NULL != pCurrentItem)
		{
			CBinocular *pBinoculars = static_cast<CBinocular*>(GetPlayer()->GetItemByClass(CItem::sBinocularsClass));
			if (NULL != pBinoculars && pCurrentItem->GetEntityId() == pBinoculars->GetEntityId())
			{
				// Let binoculars handle action
				pCurrentItem->OnAction(pPlayer->GetEntityId(), rGameActions.zoom_out, eIS_Pressed, 1.0f);
			}
			else if (NULL != pCurrentItem->GetIWeapon())
			{
				// Must be a weapon
				// Zoom out. If all the way zoomed out, exit zoom mode
				if (true == pCurrentItem->GetIWeapon()->IsZoomed())
				{
					if (IZoomMode *pZoomMode = pCurrentItem->GetIWeapon()->GetZoomMode(pCurrentItem->GetIWeapon()->GetCurrentZoomMode()))
					{
						if (1 == pZoomMode->GetCurrentStep())
							pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.zoom, eIS_Pressed, 1.0f);
						else
							pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.zoom_out, eIS_Pressed, 1.0f);
					}
					else
						pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.zoom_out, eIS_Pressed, 1.0f);
				}
			}
		}

		// Zoom out with vehicle then
		if (IVehicle *pVehicle = pPlayer->GetLinkedVehicle())
		{
			EntityId nWeaponId = pVehicle->GetCurrentWeaponId(pPlayer->GetEntityId());
			if (CItem *pWeaponItem = (CItem*)gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(nWeaponId))
			{
				if (NULL != pWeaponItem->GetIWeapon())
				{
					// Must be a weapon
					// Zoom out. If all the way zoomed out, exit zoom mode
					if (true == pWeaponItem->GetIWeapon()->IsZoomed())
					{
						if (IZoomMode *pZoomMode = pWeaponItem->GetIWeapon()->GetZoomMode(pWeaponItem->GetIWeapon()->GetCurrentZoomMode()))
						{
							if (1 == pZoomMode->GetCurrentStep())
								pWeaponItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.zoom, eIS_Pressed, 1.0f);
							else
								pWeaponItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.zoom_out, eIS_Pressed, 1.0f);
						}
						else
							pWeaponItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.zoom_out, eIS_Pressed, 1.0f);
					}
				}
			}
		}
	}
};

REGISTER_MOTION(WIIMOTE, CZoomInMotion, CZoomInMotion::PASSED, 0.40f)
REGISTER_MOTION(WIIMOTE, CZoomOutMotion, CZoomOutMotion::PASSED, 0.41f)