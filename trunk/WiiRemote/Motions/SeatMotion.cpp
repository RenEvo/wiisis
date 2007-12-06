////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// SeatMotion.cpp
//
// Purpose: Motion on Wiimote
//	Used to switch seats while in a vehicle
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "Game.h"
#include "GameActions.h"
#include "IPlayerInput.h"
#include "IVehicleSystem.h"

#include "BaseMotion.h"

class CSeatNextMotion : public CBaseMotion
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
	// CanChangeSeat
	////////////////////////////////////////////////////
	virtual bool CanChangeSeat(void) const
	{
		CPlayer *pPlayer = GetPlayer();

		// Must be in a vehicle
		return (NULL != pPlayer->GetLinkedVehicle());
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
		if (!CanChangeSeat()) return;

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
		if (!CanChangeSeat()) return;
		const float fSensitivity = CHECK_PROFILE_FLOAT(Veh_SeatNextSensitivity);

		// If we hit good G, make sure we haven't already hit bad G
		if (motion.vAccel.x <= -fSensitivity && ((m_nMotionState & STATE_BADG) != STATE_BADG))
			m_nMotionState |= STATE_GOODG;

		// See if we hit bad G
		if (motion.vAccel.x >= fSensitivity)
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
		if (!CanChangeSeat()) return;
		const float fSensitivity = CHECK_PROFILE_FLOAT(Veh_SeatNextSensitivity);

		// If we hit good G, make sure we haven't already hit bad G
		if (motion.vAccel.x <= -fSensitivity && ((m_nMotionState & STATE_BADG) != STATE_BADG))
			m_nMotionState |= STATE_GOODG;

		// See if we hit bad G
		if (motion.vAccel.x >= fSensitivity)
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
		IVehicle *pVehicle = pPlayer->GetLinkedVehicle();
		if (NULL == pVehicle) return;

		// Change seat
		if (IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId()))
		{
			TVehicleSeatId nSeatId = pSeat->GetSeatId();
		
			// If last seat, select first one; othwerise, select next
			if (nSeatId == pVehicle->GetLastSeatId())
				nSeatId = 1;
			else
				nSeatId++;
			
			// Change to its seat
			switch (nSeatId)
			{
				case 1:
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat1, eIS_Pressed, 1.0f);
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat1, eIS_Released, 1.0f);
				break;
				case 2:
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat2, eIS_Pressed, 1.0f);
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat2, eIS_Released, 1.0f);
				break;
				case 3:
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat3, eIS_Pressed, 1.0f);
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat3, eIS_Released, 1.0f);
				break;
				case 4:
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat4, eIS_Pressed, 1.0f);
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat4, eIS_Released, 1.0f);
				break;
				case 5:
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat5, eIS_Pressed, 1.0f);
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat5, eIS_Released, 1.0f);
				break;
			}
		}
	}
};

class CSeatPrevMotion : public CBaseMotion
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
	// CanChangeSeat
	////////////////////////////////////////////////////
	virtual bool CanChangeSeat(void) const
	{
		CPlayer *pPlayer = GetPlayer();

		// Must be in a vehicle
		return (NULL != pPlayer->GetLinkedVehicle());
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
		if (!CanChangeSeat()) return;

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
		if (!CanChangeSeat()) return;
		const float fSensitivity = CHECK_PROFILE_FLOAT(Veh_SeatPrevSensitivity);

		// If we hit good G, make sure we haven't already hit bad G
		if (motion.vAccel.x >= fSensitivity && ((m_nMotionState & STATE_BADG) != STATE_BADG))
			m_nMotionState |= STATE_GOODG;

		// See if we hit bad G
		if (motion.vAccel.x <= -fSensitivity)
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
		if (!CanChangeSeat()) return;
		const float fSensitivity = CHECK_PROFILE_FLOAT(Veh_SeatPrevSensitivity);

		// If we hit good G, make sure we haven't already hit bad G
		if (motion.vAccel.x >= fSensitivity && ((m_nMotionState & STATE_BADG) != STATE_BADG))
			m_nMotionState |= STATE_GOODG;

		// See if we hit bad G
		if (motion.vAccel.x <= -fSensitivity)
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
		IVehicle *pVehicle = pPlayer->GetLinkedVehicle();
		if (NULL == pVehicle) return;

		// Change seat
		if (IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId()))
		{
			TVehicleSeatId nSeatId = pSeat->GetSeatId();
		
			// If first seat, select last one; otherwise, select previous
			if (nSeatId == 1)
				nSeatId = pVehicle->GetLastSeatId();
			else
				nSeatId--;
			
			// Change to its seat
			switch (nSeatId)
			{
				case 1:
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat1, eIS_Pressed, 1.0f);
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat1, eIS_Released, 1.0f);
				break;
				case 2:
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat2, eIS_Pressed, 1.0f);
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat2, eIS_Released, 1.0f);
				break;
				case 3:
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat3, eIS_Pressed, 1.0f);
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat3, eIS_Released, 1.0f);
				break;
				case 4:
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat4, eIS_Pressed, 1.0f);
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat4, eIS_Released, 1.0f);
				break;
				case 5:
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat5, eIS_Pressed, 1.0f);
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeseat5, eIS_Released, 1.0f);
				break;
			}
		}
	}
};

REGISTER_MOTION(WIIMOTE, CSeatNextMotion, CSeatNextMotion::PASSED, 0.50f)
REGISTER_MOTION(WIIMOTE, CSeatPrevMotion, CSeatPrevMotion::PASSED, 0.51f)