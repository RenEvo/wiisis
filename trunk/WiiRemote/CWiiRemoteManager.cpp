////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// CWiiRemoteManager.cpp
//
// Purpose: Manager to handle the Wii Remote
//
// History:
//	- 11/15/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "VehicleMovementHelicopter.h"
#include "VehicleMovementVTOL.h"
#include "GameRules.h"
#include "GameActions.h"
#include "Core\WR_Implementation.h"
#include "CWiiRemoteManager.h"
#include "CWiiRemoteProfile.h"
#include "Motions\BaseMotion.h"

// Crysis files
#include "Game.h"
#include "IPlayerInput.h"
#include "IVehicleSystem.h"
#include "HUD\Hud.h"
#include "HUD\HudCrosshair.h"
#include "IUIDraw.h"
#include "Fists.h"
#include "Binocular.h"

////////////////////////////////////////////////////
// GetPlayer
//
// Purpose: Get the client player
////////////////////////////////////////////////////
CPlayer* GetPlayer(void)
{
	return static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
}

////////////////////////////////////////////////////
// UsingIRSensor
//
// Purpose: Returns if IR sensor is enabled
////////////////////////////////////////////////////
bool UsingIRSensor(void)
{
	if (g_pGame)
		if (CHUD *pHUD = g_pGame->GetHUD())
			if (CHUDCrosshair *pCH = pHUD->GetCrosshair())
				return pCH->IRIsEnabled();
	return false;
}

////////////////////////////////////////////////////
// AddPlayerVehicleMovementAction
//
// Purpose: Add a vehicle movement action request
//	to the player's linked vehicle
////////////////////////////////////////////////////
void AddPlayerVehicleMovementAction(CPlayer *pPlayer, SVehicleMovementAction const& action)
{
	if (pPlayer)
		if (IVehicle *pVehicle = pPlayer->GetLinkedVehicle())
			if (IVehicleMovement *pMovement = pVehicle->GetMovement())
				pMovement->RequestActions(action);
}

////////////////////////////////////////////////////
// Error Listener
////////////////////////////////////////////////////
struct SWiiRemoteErrorListener : public IWR_ErrorListener
{
	CWiiRemoteManager *pManager;

	virtual void OnError(int nModule, int nError, char const* szErrorMsg)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "[WiiRemoteManager] An error has occured: Module=%d Error=%d \'%s\'",
			nModule, nError, szErrorMsg);
	}
} g_WiiRemoteErrorListener;

////////////////////////////////////////////////////
// Controller Listener
////////////////////////////////////////////////////
struct SWiiRemoteListener : public IWR_WiiRemoteListener
{
	CWiiRemoteManager *pManager;

	virtual void OnConnect(IWR_WiiRemote *pRemote);
	virtual void OnDisconnect(IWR_WiiRemote *pRemote, bool bAbnormal);
	virtual void OnConnecting(IWR_WiiRemote *pRemote);
	virtual void OnReportChanged(IWR_WiiRemote *pRemote, int nReport, bool bContinuous);
	virtual void OnStatusUpdate(IWR_WiiRemote *pRemote, int nStatus, int nBattery);
	virtual void OnExtensionPluggedIn(IWR_WiiRemote *premote, IWR_WiiExtension *pExtension);
	virtual void OnExtensionUnplugged(IWR_WiiRemote *premote, IWR_WiiExtension *pExtension);
} g_WiiRemoteListener;

////////////////////////////////////////////////////
// HID Listener
////////////////////////////////////////////////////
struct SWiiRemoteHIDListener : public IWR_HIDControllerListener
{
	CWiiRemoteManager *pManager;

	virtual void OnFoundRemoteDevice(char const* szDevice, HANDLE hHandle)
	{
		CryLogAlways("[WiiRemoteManager] Found Remote HID entry. Initializing the remote...");
		GetWiiRemoteSystem()->pHIDController->InitializeRemote(WII_REMOTE_P1);
	}
	virtual void OnRemoteInitialized(IWR_WiiRemote* pRemote, RemoteID nID)
	{
		CryLogAlways("[WiiRemoteManager] Wii Remote Initialized!");
		pRemote->AddListener(&g_WiiRemoteListener);
		pRemote->SetConnectionTimeout(5.0f);

		pManager->SetRemote(pRemote);

		// No extension at first
		pManager->SetErrorLevel(WIIREMOTE_ERROR_NOEXTENSION);
	}
} g_WiiRemoteHIDListener;

////////////////////////////////////////////////////
// Input Listener
////////////////////////////////////////////////////
struct SWiiInputListener : public IWR_WiiExtensionListener, public IWR_WiiButtonsListener,
							public IWR_WiiMotionListener, public IWR_WiiSensorListener
{
	CWiiRemoteManager *pManager;
	bool bInGesture;
	int nState;
	int nEndLifetime, nExtensionEndLifetime;

	SWiiInputListener(void)
	{
		bInGesture = false;
		nState = STATE_PLAYER;
		nEndLifetime = nExtensionEndLifetime = 0;
	}

	// IWR_WiiButtonsListener
	virtual void OnButton(IWR_WiiRemote *pRemote, IWR_WiiButtons *pButtons, unsigned int nButton, int nStatus, bool bDown);
	virtual void OnAction(IWR_WiiRemote *pRemote, IWR_WiiButtons *pButtons, char const* szAction, ActionID nActionID, int nStatus, bool bDown);

	// IWR_WiiMotionListener
	virtual void OnSingleMotion(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion);
	virtual void OnMotionStart(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion);
	virtual void OnMotionUpdate(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion);
	virtual void OnMotionEnd(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion);

	// IWR_WiiExtensionListener
	virtual void OnExtensionButton(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, unsigned int nButton, int nStatus, bool bDown);
	virtual void OnExtensionAction(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, char const* szAction, ActionID nActionID, int nStatus, bool bDown);
	virtual void OnExtensionSingleMotion(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion);
	virtual void OnExtensionMotionStart(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion);
	virtual void OnExtensionMotionUpdate(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion);
	virtual void OnExtensionMotionEnd(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion);
	virtual void OnExtensionAnalogUpdate(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, int nStickID, float fX, float fY);

	// IWR_WiiSensorListener
	virtual void OnEnterScreen(IWR_WiiRemote *pRemote, IWR_WiiSensor *pSensor, float fX, float fY);
	virtual void OnLeaveScreen(IWR_WiiRemote *pRemote, IWR_WiiSensor *pSensor, float fX, float fY);
	virtual void OnCursorUpdate(IWR_WiiRemote *pRemote, IWR_WiiSensor *pSensor, float fX, float fY);

	virtual void OnCommonButton(IWR_WiiRemote *pRemote, unsigned int nButton, int nStatus, bool bDown, bool bExtension);
} g_WiiInputListener;

////////////////////////////////////////////////////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
CWiiRemoteManager::CWiiRemoteManager(CGame *pGame)
{
	m_bMasterEnabled = true;
	m_pGame = pGame;
	m_pWR = GetWiiRemoteSystem();
	m_fLastErrorDisplay = 0.0f;
	m_pRemote = NULL;

	m_nBatteryTexture = m_nBatteryLevelTexture = m_nIRDotTexture = 0;
	m_fBatteryLevel = 1.0f;
	m_fIRDotWidth = m_fIRDotHeight = 0.0f;
	m_fRumbleKillTime = 0.0f;
	m_nLastPlayerHealth = 0.0f;

	g_WiiRemoteErrorListener.pManager = this;
	g_WiiRemoteListener.pManager = this;
	g_WiiRemoteHIDListener.pManager = this;
	g_WiiInputListener.pManager = this;

	m_bSprint = false;
	m_bLockView = false;
	m_bLockedNPC = false;
	m_bForceLockView = false;
	m_pLockedEntity = 0;
	m_fLastLockRelease = 0;
	m_fFreezeMovement = 0;
	m_vLockedEntityOffset.Set(0,0,0);
}

////////////////////////////////////////////////////
CWiiRemoteManager::~CWiiRemoteManager(void)
{
	Shutdown();
}

////////////////////////////////////////////////////
void CWiiRemoteManager::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions& actions = g_pGame->Actions();
	if (action == actions.wr_master && activationMode == eAAM_OnPress)
	{
		// Disable it
		SetMasterEnabled(false);
	}
}

////////////////////////////////////////////////////
void CWiiRemoteManager::Initialize(void)
{
	// Setup error levels
	m_Errors[WIIREMOTE_ERROR_NOCONNECTION].szMessage = "Connecting to Wii Remote...";
	m_Errors[WIIREMOTE_ERROR_NOEXTENSION].szMessage = "Plug in Wii Nunchuk";
	m_Errors[WIIREMOTE_ERROR_DISCONNECTED].szMessage = "Wii Remote Disconnected";
	m_Errors[WIIREMOTE_ERROR_LOWBATTERY].szMessage = "Low Battery";

	// Load images
	if (IUIDraw *pUI = m_pGame->GetIGameFramework()->GetIUIDraw())
	{
		// Disconnected
		m_Errors[WIIREMOTE_ERROR_DISCONNECTED].nTextureID = pUI->CreateTexture("Textures\\Gui\\HUD\\WiiRemoteIndicators\\Disconnected.dds");
		pUI->GetTextureSize(m_Errors[WIIREMOTE_ERROR_DISCONNECTED].nTextureID, m_Errors[WIIREMOTE_ERROR_DISCONNECTED].fTextureWidth,
			m_Errors[WIIREMOTE_ERROR_DISCONNECTED].fTextureHeight);

		// NoConnection
		m_Errors[WIIREMOTE_ERROR_NOCONNECTION].nTextureID = pUI->CreateTexture("Textures\\Gui\\HUD\\WiiRemoteIndicators\\NoConnection.dds");
		pUI->GetTextureSize(m_Errors[WIIREMOTE_ERROR_NOCONNECTION].nTextureID, m_Errors[WIIREMOTE_ERROR_NOCONNECTION].fTextureWidth,
			m_Errors[WIIREMOTE_ERROR_NOCONNECTION].fTextureHeight);

		// NoExtension
		m_Errors[WIIREMOTE_ERROR_NOEXTENSION].nTextureID = pUI->CreateTexture("Textures\\Gui\\HUD\\WiiRemoteIndicators\\NoExtension.dds");
		pUI->GetTextureSize(m_Errors[WIIREMOTE_ERROR_NOEXTENSION].nTextureID, m_Errors[WIIREMOTE_ERROR_NOEXTENSION].fTextureWidth,
			m_Errors[WIIREMOTE_ERROR_NOEXTENSION].fTextureHeight);

		// Battery
		m_nBatteryTexture = pUI->CreateTexture("Textures\\Gui\\HUD\\WiiRemoteIndicators\\Battery.dds");
		m_nBatteryLevelTexture = pUI->CreateTexture("Textures\\Gui\\HUD\\WiiRemoteIndicators\\BatteryLevel.dds");

		// IR Dot
		m_nIRDotTexture = pUI->CreateTexture("Textures\\Gui\\HUD\\IRDot.dds");
		pUI->GetTextureSize(m_nIRDotTexture, m_fIRDotWidth, m_fIRDotHeight);
	}

	// Create profile
	g_WiiRemoteProfile->SetToDefault();
	if (false == g_WiiRemoteProfile->LoadCFG())
	{
		CryLogAlways("[WiiRemoteManager] Missing config file, creating new one...");
		if (false == g_WiiRemoteProfile->SaveCFG())
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to save Wiisis Remote Configuration");
	}

	// Add error listener
	m_pWR->AddErrorListener(&g_WiiRemoteErrorListener);
	m_pWR->pHIDController->GetRemoteCount();
	m_pWR->pHIDController->AddListener(&g_WiiRemoteHIDListener);

	// Initialie the core objets
	CryLogAlways("[WiiRemoteManager] Initializing Core files...");
	if (false == m_pWR->Initialize())
	{
		CryError("[WiiRemoteManager] Failed to initialize core files!");
		return;
	}
	// Find a remote
	CryLogAlways("[WiiRemoteManager] Pooling for Wii Remote devices...");
	m_pWR->pHIDController->ClearFoundRemotes();
	m_pWR->pHIDController->PoolRemoteDevices();

	// Create auto-registered motions
	CMotionAutoReg *p = CMotionAutoReg::m_pFirst;
	while (NULL != p)
	{
		if (p->m_nController == WIIMOTE)
		{
			m_WiimoteMotions.push_back(p->Create());
		}
		else if (p->m_nController == NUNCHUK)
		{
			m_NunchukMotions.push_back(p->Create());
		}
		p = p->m_pNext;
	}
}

////////////////////////////////////////////////////
void CWiiRemoteManager::Shutdown(void)
{
	EditorResetGame(false);

	// Cleanup
	CryLogAlways("[WiiRemoteManager] Shutting down Core files...");
	m_pWR->pHIDController->RemoveListener(&g_WiiRemoteHIDListener);
	m_pWR->Shutdown();
	m_pWR->RemoveErrorListener(&g_WiiRemoteErrorListener);

	// Delete motion objects
	for (MotionList::iterator itMotion = m_WiimoteMotions.begin(); itMotion != m_WiimoteMotions.end(); itMotion++)
		SAFE_DELETE(*itMotion);
	for (MotionList::iterator itMotion = m_NunchukMotions.begin(); itMotion != m_NunchukMotions.end(); itMotion++)
		SAFE_DELETE(*itMotion);
	m_WiimoteMotions.clear();
	m_NunchukMotions.clear();
}

////////////////////////////////////////////////////
void CWiiRemoteManager::Update(bool bHaveFocus, int nUpdateFlags)
{
	// Update the core
	m_pWR->Update();
	float fCurrTime = m_pWR->pTimer->GetCurrTime();

	// Don't continue if master disabled
	if (false == IsMasterEnabled()) return;

	// If we don't have a remote, add an error message underneath
	if (NULL == m_pRemote || false == m_pRemote->IsConnected())
	{
		SetErrorLevel(WIIREMOTE_ERROR_DISCONNECTED);
	}

	// Check if error message needs to be updated
	for (int i = 0; i < WIIREMOTE_ERROR_MAX; i++)
	{
		if (true == m_Errors[i].bOn && false == m_Errors[i].szMessage.empty())
		{
			// Check if message needs to be redisplayed
			if (fCurrTime-m_fLastErrorDisplay >= WRERROR_SHOWMESSAGE_RATE)
			{
				m_fLastErrorDisplay = fCurrTime;
				if (NULL != m_pGame->GetHUD())
					m_pGame->GetHUD()->DisplayBigOverlayFlashMessage(m_Errors[i].szMessage.c_str(), WRERROR_SHOWMESSAGE_LENGTH);
			}
			m_pRemote->SetLEDs(WR_LED_NONE);
			m_pRemote->SetRumble(false);
			return;
		}
	}

	CPlayer *pPlayer = GetPlayer();
	if (NULL == pPlayer) return;

	// Check which state we are in
	int nNewState = -1;
	int nOpenedMenu = (m_pGame->GetHUD() ? m_pGame->GetHUD()->GetOpenedMenu() : CHUD::MENU_NONE);
	CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem();
	if (nOpenedMenu == CHUD::MENU_NANOSUIT)
	{
		// In nano suit menu
		nNewState = STATE_NANOSUITMENU;
	}
	else if (nOpenedMenu == CHUD::MENU_WEAPON)
	{
		// In weapon menu
		nNewState = STATE_WEAPONMENU;
	}
	else
	{
		// Check if they have binoculars up
		if (NULL != pCurrentItem)
		{
			CBinocular *pBinoculars = static_cast<CBinocular*>(pPlayer->GetItemByClass(CItem::sBinocularsClass));
			if (NULL != pBinoculars && pCurrentItem->GetEntityId() == pBinoculars->GetEntityId())
				nNewState = STATE_BINOCULARS;
		}
		if (nNewState == -1)
		{
			// Check if in vehicle
			if (pPlayer->GetLinkedVehicle())
			{
				// Check type
				IVehicleMovement *pMovement = pPlayer->GetLinkedVehicle()->GetMovement();
				switch (pMovement->GetMovementType())
				{
					case IVehicleMovement::eVMT_Land:
						nNewState = STATE_LANDVEHICLE;
					break;

					case IVehicleMovement::eVMT_Sea:
					case IVehicleMovement::eVMT_Amphibious:
						nNewState = STATE_SEAVEHICLE;
					break;

					case IVehicleMovement::eVMT_Air:
					{
						// Test if it is VTOL or not
						CVehicleMovementHelicopter *pHeli = (CVehicleMovementHelicopter*)(pMovement);
						nNewState = (false==pHeli->IsVTOL()?STATE_HELIVEHICLE:STATE_VTOLVEHICLE);
					}
					break;

					case IVehicleMovement::eVMT_Other:
					default:
					{
						// Kill movement controls
						SetMasterEnabled(false);
						return;
					};
					break;
				}
			}
			else
			{
				// No state yet found, so must just be a player
				nNewState = STATE_PLAYER;
			}
		}
	}
	g_WiiInputListener.nState = nNewState;

	if (true == bHaveFocus)
	{
		// Notify crosshair of IR enabled status
		if (CHUD *pHud = m_pGame->GetHUD())
		{
			if (CHUDCrosshair *pCH = pHud->GetCrosshair())
			{
				pCH->IREnable(CHECK_PROFILE_BOOL(UseIRSensor));
			}
		}

		// Update player movement
		UpdatePlayerMovement();

		// Set LEDs to match current weapon's ammo percentage
		if (NULL == pCurrentItem || NULL == pCurrentItem->GetIWeapon())
		{
			m_pRemote->SetLEDs(WR_LED_ALL);
		}
		else
		{
			int nCurrFiremode = pCurrentItem->GetIWeapon()->GetCurrentFireMode();
			if (IFireMode *pFireMode = pCurrentItem->GetIWeapon()->GetFireMode(nCurrFiremode))
			{
				int nCapacity = pFireMode->GetClipSize();
				int nCurrent = pFireMode->GetAmmoCount();
				float fPerc = (float)nCurrent / (float)nCapacity;
				int nLEDs = WR_LED_ALL;
				if (fPerc <= 0.75f)	nLEDs &= ~WR_LED_P4;
				if (fPerc <= 0.5f)	nLEDs &= ~WR_LED_P3;
				if (fPerc <= 0.25f)	nLEDs &= ~WR_LED_P2;
				if (fPerc <= 0.0f)	nLEDs &= ~WR_LED_P1;
				m_pRemote->SetLEDs(nLEDs);
			}
			else
				m_pRemote->SetLEDs(WR_LED_ALL);
		}

		// Check player's health to see if we should rumble
		int32 nHealth = pPlayer->GetHealth();
		if (nHealth < m_nLastPlayerHealth)
			Rumble(RUMBLE_DAMAGE_TIMER);
		m_nLastPlayerHealth = nHealth;

		// Check rumble kill time
		if (true == m_pRemote->IsRumbleOn() && (CHECK_PROFILE_BOOL(EnableRumble) == false || m_fRumbleKillTime <= m_pWR->pTimer->GetCurrTime()))
		{
			m_fRumbleKillTime = 0.0f;
			m_pRemote->SetRumble(false);
		}
	}
	else
	{
		m_pRemote->SetLEDs(WR_LED_NONE);
		m_pRemote->SetRumble(false);
	}
}

////////////////////////////////////////////////////
void CWiiRemoteManager::OnWeaponShoot(unsigned int nShooterId)
{
	CPlayer *pPlayer = GetPlayer();
	if (NULL == pPlayer || pPlayer->GetEntityId() != nShooterId) return;
	Rumble(RUMBLE_FIRE_WEAPON_TIMER);
}

////////////////////////////////////////////////////
void CWiiRemoteManager::UpdateHUD(CHUD *pHUD)
{
	// Must be enabled
	if (false == m_bMasterEnabled) return;

	if (IUIDraw *pUI = m_pGame->GetIGameFramework()->GetIUIDraw())
	{
		// Draw marker for where IR is pointing
		if (true == CHECK_PROFILE_BOOL(ShowIRDot))
		{
			CHUDCrosshair *pH = pHUD->GetCrosshair();
			if (NULL != pH && true == pH->IRIsEnabled())
			{
				Vec3 vPos = pH->IRGetCursorPos();
				vPos.x *= 800.f;
				vPos.y *= 600.f;
				pUI->DrawImageCentered(m_nIRDotTexture, vPos.x/*-m_fIRDotWidth*0.5f*/, vPos.y/*-m_fIRDotHeight*0.5f*/,
					m_fIRDotWidth, m_fIRDotHeight, 0, 1, 1, 1, CHECK_PROFILE_FLOAT(IRDotOpacity));
			}
		}

		// Display error icon
		for (int i = 0; i < WIIREMOTE_ERROR_MAX; i++)
		{
			if (true == m_Errors[i].bOn && 0 != m_Errors[i].nTextureID)
			{
				pUI->DrawImageCentered(m_Errors[i].nTextureID, 5.0f+m_Errors[i].fTextureWidth*0.5f,
					5.0f+m_Errors[i].fTextureHeight*0.5f, m_Errors[i].fTextureWidth, m_Errors[i].fTextureHeight,
					0, 1, 1, 1, 1);
				return;
			}
		}

		// No error icons, so display battery
		// First, draw battery container
		pUI->DrawImageCentered(m_nBatteryTexture, 5.0f+BatteryWidth*0.5f, 5.0f+BatteryHeight*0.5f, 
			BatteryWidth, BatteryHeight, 0, 1, 1, 1, 0.8f);

		// Calculate Y offset and new height based on fill percent
		const float fY = 5.0f + BatteryLevelY + (-BatteryLevelHeight*m_fBatteryLevel) + BatteryLevelHeight;
		const float fHeight = (BatteryLevelHeight + 5.0f + BatteryLevelY) - fY;

		// Determine color based on percent and draw
		float r = 0.f; 
		float g = 1.f; 
		float b = 0.f;
		if (m_fBatteryLevel <= 0.25f) { r = 1.f; g = 0.f; b = 0.f; }
		else if (m_fBatteryLevel <= 0.5f) { r = 1.f; g = 1.f; b = 0.f; }
		pUI->DrawImage(m_nBatteryLevelTexture, 5.0f+BatteryLevelX, fY, BatteryLevelWidth, fHeight, 0, r, g, b, 0.8f);
	}
}

////////////////////////////////////////////////////
void CWiiRemoteManager::EditorResetGame(bool bStart)
{
	if (false == bStart)
	{
		// If we have the remote, turn its LEDs off
		if (IWR_WiiRemote *pRemote = m_pWR->pHIDController->GetRemote(WII_REMOTE_P1))
		{
			pRemote->SetLEDs(WR_LED_NONE);
		}
		SetMasterEnabled(false);
	}
	else
	{
		if (IWR_WiiRemote *pRemote = m_pWR->pHIDController->GetRemote(WII_REMOTE_P1))
		{
			// Attempt a reconnect
			pRemote->Reconnect();
		}
		SetMasterEnabled(true);
	}
}

////////////////////////////////////////////////////
void CWiiRemoteManager::SetMasterEnabled(bool bOn)
{
	if (NULL == m_pRemote) return;

	m_bMasterEnabled = bOn;
	if (CPlayer *pPlayer = GetPlayer())
		if (IGameObject *pGameObject = pPlayer->GetGameObject())
		{
			if (false == bOn)
			{
				if (m_pGame && m_pGame->GetHUD()) m_pGame->GetHUD()->FadeOutBigOverlayFlashMessage();
				m_fLastErrorDisplay = 0.0f;

				// Hack to fix the lean
				if (pPlayer->GetMovementController())
				{
					CMovementRequest mr;
					mr.SetLean(0.0f);
					pPlayer->GetMovementController()->RequestMovement(mr);
				}

				// Turn remote's lights OFF
				m_pRemote->SetLEDs(WR_LED_NONE);
				m_pRemote->SetRumble(false);

				// Set action listener to the player's input controller
				pGameObject->ReleaseActions(this);
				if (pPlayer->GetPlayerInput()) pGameObject->CaptureActions(pPlayer->GetPlayerInput()->GetActionListener());
			}
			else
			{
				// Turn remote's lights ON
				m_pRemote->SetLEDs(WR_LED_ALL);
				m_pRemote->SetRumble(false);

				// Set action listening to us
				if (pPlayer->GetPlayerInput()) pGameObject->ReleaseActions(pPlayer->GetPlayerInput()->GetActionListener());
				pGameObject->CaptureActions(this);
			}
		}

	// Notify crosshair
	if (m_pGame)
		if (CHUD *pHud = m_pGame->GetHUD())
		{
			if (CHUDCrosshair *pCH = pHud->GetCrosshair())
			{
				if (false == bOn || false == CHECK_PROFILE_BOOL(UseIRSensor))
					pCH->IREnable(false);
				else
					pCH->IREnable(true);
			}
		}
}

////////////////////////////////////////////////////
bool CWiiRemoteManager::IsMasterEnabled(void) const
{
	return m_bMasterEnabled;
}

////////////////////////////////////////////////////
void CWiiRemoteManager::FreezeMovement(void)
{
	m_fFreezeMovement = m_pWR->pTimer->GetCurrTime();
}

////////////////////////////////////////////////////
bool CWiiRemoteManager::IsMovementFrozen(void) const
{
	return (NULL == m_pWR || m_pWR->pTimer->GetCurrTime() - m_fFreezeMovement <= 0.5f);
}

////////////////////////////////////////////////////
void CWiiRemoteManager::SetErrorLevel(int nLevel)
{
	if (nLevel < 0 || nLevel >= WIIREMOTE_ERROR_MAX) return;
	m_Errors[nLevel].bOn = true;
}

////////////////////////////////////////////////////
void CWiiRemoteManager::ClearErrorLevel(int nLevel)
{
	for (int i = 0; i < WIIREMOTE_ERROR_MAX; i++)
	{
		if (nLevel == -1 || nLevel == i)
			m_Errors[i].bOn = false;
	}

	// If all are cleared, fade message now
	for (int i = 0; i < WIIREMOTE_ERROR_MAX; i++)
	{
		if (true == m_Errors[i].bOn) return;
	}
	if (NULL != m_pGame->GetHUD())
		m_pGame->GetHUD()->FadeOutBigOverlayFlashMessage();
	m_fLastErrorDisplay = 0.0f;
}

////////////////////////////////////////////////////
void CWiiRemoteManager::SetRemote(IWR_WiiRemote *pRemote)
{
	m_pRemote = pRemote;
}

////////////////////////////////////////////////////
IWR_WiiRemote* CWiiRemoteManager::GetRemote(void) const
{
	return m_pRemote;
}

////////////////////////////////////////////////////
void CWiiRemoteManager::Rumble(float fDuration)
{
	if (NULL == m_pRemote || NULL == m_pWR || false == CHECK_PROFILE_BOOL(EnableRumble)) return;
	m_fRumbleKillTime = m_pWR->pTimer->GetCurrTime() + fDuration;
	m_pRemote->SetRumble(true);
}

////////////////////////////////////////////////////
void SWiiRemoteListener::OnConnect(IWR_WiiRemote *pRemote)
{
	CryLogAlways("[WiiRemoteManager] Wii Remote Connection Established!");

	// Set update interval
	pRemote->SetStatusUpdate(0.1f);
	pRemote->SetReport(WR_REPORT_ALL, true);

	// Setup helpers
	pRemote->GetButtonHelper()->EnableBufferedInput(true);
	pRemote->GetMotionHelper()->Calibrate();
	pRemote->GetMotionHelper()->SetMotionSize(10);

	// Turn on all LEDs
	pRemote->SetLEDs(WR_LED_ALL);

	// Listeners
	pRemote->GetButtonHelper()->AddListener(&g_WiiInputListener);
	pRemote->GetMotionHelper()->AddListener(&g_WiiInputListener);
	pRemote->GetSensorHelper()->AddListener(&g_WiiInputListener);

	pManager->ClearErrorLevel(WIIREMOTE_ERROR_NOCONNECTION);
	pManager->ClearErrorLevel(WIIREMOTE_ERROR_DISCONNECTED);
}

////////////////////////////////////////////////////
void SWiiRemoteListener::OnDisconnect(IWR_WiiRemote *pRemote, bool bAbnormal)
{
	CryLogAlways("[WiiRemoteManager] Wii Remote disconected %s", (bAbnormal?"abormally":"normally"));
	if (true == bAbnormal)
	{
		// Attempt to reconnect
		CryLogAlways("[WiiRemoteManager] Attempting to reconnect...");
		pManager->SetErrorLevel(WIIREMOTE_ERROR_DISCONNECTED);
		pRemote->Reconnect();
	}
}

////////////////////////////////////////////////////
void SWiiRemoteListener::OnConnecting(IWR_WiiRemote *pRemote)
{
	CryLogAlways("[WiiRemoteManager] Attempting Wii Remote connection...");

	// Set error message
	pManager->SetErrorLevel(WIIREMOTE_ERROR_NOCONNECTION);
}

////////////////////////////////////////////////////
void SWiiRemoteListener::OnReportChanged(IWR_WiiRemote *pRemote, int nReport, bool bContinuous)
{

}

////////////////////////////////////////////////////
void SWiiRemoteListener::OnStatusUpdate(IWR_WiiRemote *pRemote, int nStatus, int nBattery)
{
	// Update manager's battery level
	pManager->m_fBatteryLevel = CLAMP(((float)nBattery * (1.0f/100.0f)), 0.0f, 1.0f);
	if (pManager->m_fBatteryLevel <= WRERROR_BATTERY_MARKER)
		pManager->SetErrorLevel(WIIREMOTE_ERROR_LOWBATTERY);
	else
		pManager->ClearErrorLevel(WIIREMOTE_ERROR_LOWBATTERY);
}

////////////////////////////////////////////////////
void SWiiRemoteListener::OnExtensionPluggedIn(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension)
{
	if (pExtension->GetType() != CWR_WiiNunchuk::TYPE) return;
	pManager->ClearErrorLevel(WIIREMOTE_ERROR_NOEXTENSION);

	CWR_WiiNunchuk *pNunchuk = (CWR_WiiNunchuk*)pExtension;
	pNunchuk->EnableBufferedInput(true);
	pNunchuk->Calibrate();

	// Add listener
	pNunchuk->AddListener(&g_WiiInputListener);
}

////////////////////////////////////////////////////
void SWiiRemoteListener::OnExtensionUnplugged(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension)
{
	pManager->SetErrorLevel(WIIREMOTE_ERROR_NOEXTENSION);

	// Remove listener
	pExtension->RemoveListener(&g_WiiInputListener);
}

////////////////////////////////////////////////////
void SWiiInputListener::OnButton(IWR_WiiRemote *pRemote, IWR_WiiButtons *pButtons, unsigned int nButton, int nStatus, bool bDown)
{
	OnCommonButton(pRemote, nButton, nStatus, bDown, false);
}

////////////////////////////////////////////////////
void SWiiInputListener::OnExtensionButton(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, unsigned int nButton, int nStatus, bool bDown)
{
	OnCommonButton(pRemote, nButton|NUNCHUK_BIT, nStatus, bDown, true);
}

////////////////////////////////////////////////////
void SWiiInputListener::OnCommonButton(IWR_WiiRemote *pRemote, unsigned int nButton, int nStatus, bool bDown, bool bExtension)
{
	if (NULL == pManager || false == pManager->IsMasterEnabled()) return;

	CMovementRequest mr;
	CPlayer *pPlayer = GetPlayer();
	if (NULL == pPlayer) return;
	const CGameActions &rGameActions = pManager->m_pGame->Actions();

	// Can we enter a menu?
	bool bCanEnterMenu = true;
	bool bInLockRetain = (true == pManager->m_bLockView && true == CHECK_PROFILE_BOOL(RetainViewMode));
	if (true == pManager->m_bForceLockView || true == pManager->IsMovementFrozen() || 
		(true == bInLockRetain && NULL != pManager->m_pLockedEntity))
	{
		// Only if Z button is not held down
		if (CWR_WiiNunchuk *pN = (CWR_WiiNunchuk*)pRemote->GetExtensionHelper())
			bCanEnterMenu = !(pN->IsButtonDown(WR_NCBUTTON_Z));
	}

	// Nanosuit menu
	if (nState != STATE_WEAPONMENU)
	{
		if (true == bCanEnterMenu && CHECK_PROFILE_BUTTON(NanoSuitMenu) == nButton && NULL != pManager->m_pGame->GetHUD())
		{
			if (WR_BUTTONSTATUS_PUSHED == nStatus)
			{
				pManager->m_pGame->GetHUD()->OnAction(rGameActions.hud_suit_menu, eIS_Pressed, 1);
			}
			else if (WR_BUTTONSTATUS_RELEASED == nStatus)
			{
				pManager->m_pGame->GetHUD()->OnAction(rGameActions.hud_suit_menu, eIS_Released, 1);
			}
		}

		// Nanomenu buttons
		if (nState == STATE_NANOSUITMENU)
		{
			if (false == bExtension && nButton == WR_BUTTON_B && nStatus == WR_BUTTONSTATUS_RELEASED)
				pManager->m_pGame->GetHUD()->OnAction(rGameActions.hud_suit_menu, eIS_Released, 1);
		}
	}

	// Weapon menu
	if (nState != STATE_NANOSUITMENU)
	{
		if (true == bCanEnterMenu && CHECK_PROFILE_BUTTON(WeaponMenu) == nButton && NULL != pManager->m_pGame->GetHUD())
		{
			if (WR_BUTTONSTATUS_PUSHED == nStatus)
			{
				pManager->m_pGame->GetHUD()->OnAction(rGameActions.hud_weapon_mod, eIS_Pressed, 1);
			}
			else if (WR_BUTTONSTATUS_RELEASED == nStatus)
			{
				pManager->m_pGame->GetHUD()->OnAction(rGameActions.hud_weapon_mod, eIS_Released, 1);
			}
		}

		// Weapon menu buttons
		if (nState == STATE_WEAPONMENU)
		{
			if (nButton == WR_BUTTON_B && nStatus == WR_BUTTONSTATUS_RELEASED)
				pManager->m_pGame->GetHUD()->OnAction(rGameActions.hud_weapon_mod, eIS_Released, 1);
			else if ((false == bExtension && nButton == WR_BUTTON_A) || (true == bExtension && (nButton&~NUNCHUK_BIT) == WR_NCBUTTON_Z))
			{
				if (IHardwareMouse *pMouse = gEnv->pSystem->GetIHardwareMouse())
				{
					if (nStatus == WR_BUTTONSTATUS_PUSHED)
					{
						// Send hardware left mouse button press
						pMouse->Event(pManager->m_pGame->GetHUD()->m_nLastMouseEvent_X, 
							pManager->m_pGame->GetHUD()->m_nLastMouseEvent_Y, HARDWAREMOUSEEVENT_LBUTTONDOWN);
					}
					else if (nStatus == WR_BUTTONSTATUS_RELEASED)
					{
						// Send hardware left mouse button press
						pMouse->Event(pManager->m_pGame->GetHUD()->m_nLastMouseEvent_X, 
							pManager->m_pGame->GetHUD()->m_nLastMouseEvent_Y, HARDWAREMOUSEEVENT_LBUTTONUP);
					}
				}
			}
		}
	}

	// Lock view check
	if (CHECK_PROFILE_BUTTON(LockView) == nButton)
	{
		pManager->m_bLockView = bDown;
		if (false == bDown)
		{
			pManager->m_pLockedEntity = NULL; // Lost locked entity
			pManager->m_bLockedNPC = false;
		}

		// Check if force lock
		if (true == bDown)
		{
			if (pManager->m_pWR->pTimer->GetCurrTime() - pManager->m_fLastLockRelease <= 0.25f)
				pManager->m_bForceLockView = true;
		}
		else
		{
			// If the button is up, mark when
			if (false == pManager->m_bForceLockView)
				pManager->m_fLastLockRelease = pManager->m_pWR->pTimer->GetCurrTime();
			else
				pManager->m_bForceLockView = false;
		}
	}

	// Player controls
	if (nState == STATE_PLAYER || nState == STATE_BINOCULARS)
	{
		// Sprinting
		if (CHECK_PROFILE_BUTTON(Sprint) == nButton)
		{
			if (WR_BUTTONSTATUS_PUSHED == nStatus)
				pManager->m_bSprint = true;
			else if (WR_BUTTONSTATUS_RELEASED == nStatus)
				pManager->m_bSprint = false;
			pPlayer->GetPlayerInput()->SetForceSprint(pManager->m_bSprint);
		}

		// Use key
		if (CHECK_PROFILE_BUTTON(Use) == nButton)
		{
			if (WR_BUTTONSTATUS_PUSHED == nStatus) pPlayer->GetPlayerInput()->OnAction(rGameActions.use, eIS_Pressed, 1.0f);
			if (WR_BUTTONSTATUS_RELEASED == nStatus) pPlayer->GetPlayerInput()->OnAction(rGameActions.use, eIS_Released, 1.0f);
		}

		// Toggle grenade type
		if (CHECK_PROFILE_BUTTON(ToggleGrenade) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus)
		{
			pPlayer->GetPlayerInput()->OnAction(rGameActions.handgrenade, eIS_Pressed, 1.0f);
		}

		// Toggle night vision
		if (CHECK_PROFILE_BUTTON(NightVision) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus &&
			NULL != pManager->m_pGame->GetHUD())
		{
			pManager->m_pGame->GetHUD()->OnAction(rGameActions.hud_night_vision, eIS_Pressed, 1.0f);
		}

		// Picked-up item
		COffHand* pOffHand = static_cast<COffHand*>(pPlayer->GetWeaponByClass(CItem::sOffHandClass));
		if (NULL != pOffHand)
		{
			if ((pOffHand->GetOffHandState()&(eOHS_HOLDING_OBJECT|eOHS_HOLDING_NPC|eOHS_THROWING_OBJECT|eOHS_THROWING_NPC)))
			{
				// Throw on fire
				if (CHECK_PROFILE_BUTTON(Fire) == nButton)
				{
					if (nStatus == WR_BUTTONSTATUS_PUSHED)
						pOffHand->OnAction(pPlayer->GetEntityId(), rGameActions.attack1, eIS_Pressed, 1.0f);
					else if (nStatus == WR_BUTTONSTATUS_RELEASED)	
						pOffHand->OnAction(pPlayer->GetEntityId(), rGameActions.attack1, eIS_Released, 1.0f);
				}
			}
		}
		
		CItem *pCurrentItem = (CItem*)pPlayer->GetCurrentItem();
		if (NULL != pCurrentItem)
		{
			// Item controls
			if (CHECK_PROFILE_BUTTON(Binoculars) == nButton && nStatus == WR_BUTTONSTATUS_PUSHED)
			{
				// Select/deselect binoculars
				CBinocular *pBinoculars = static_cast<CBinocular*>(pPlayer->GetItemByClass(CItem::sBinocularsClass));
				if (NULL != pBinoculars)
				{
					if (pBinoculars->GetEntityId() == pCurrentItem->GetEntityId())
						pPlayer->SelectLastItem(false);
					else
						pPlayer->SelectItem(pBinoculars->GetEntityId(), true);
				}
			}

			// Toggle zooming
			if (nState == STATE_BINOCULARS)
			{
				if (CHECK_PROFILE_BUTTON(ZoomIn) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus)
					pCurrentItem->OnAction(pPlayer->GetEntityId(), rGameActions.zoom_in, eIS_Pressed, 1.0f);
				if (CHECK_PROFILE_BUTTON(ZoomOut) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus)
					pCurrentItem->OnAction(pPlayer->GetEntityId(), rGameActions.zoom_out, eIS_Pressed, 1.0f);
				if (CHECK_PROFILE_BUTTON(Fire) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus)
					pPlayer->SelectLastItem(false); // Deselect them on fire as well
			}

			// Weapon controls
			if (NULL != pCurrentItem->GetIWeapon())
			{
				// Fire
				if (CHECK_PROFILE_BUTTON(Fire) == nButton)
				{
					if (nStatus == WR_BUTTONSTATUS_PUSHED)
						pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.attack1, eIS_Pressed, 1.0f);
					else if (nStatus == WR_BUTTONSTATUS_RELEASED)	
						pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.attack1, eIS_Released, 1.0f);
				}

				// Reload
				if (CHECK_PROFILE_BUTTON(Reload) == nButton && nStatus == WR_BUTTONSTATUS_PUSHED)
				{
					pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.reload, eIS_Pressed, 1.0f);
				}

				// Toggle firemode
				if (CHECK_PROFILE_BUTTON(ToggleFiremode) == nButton && nStatus == WR_BUTTONSTATUS_PUSHED)
				{
					pCurrentItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.firemode, eIS_Pressed, 1.0f);
				}
			}
		}
		
		// Toggle weapon
		if (nState == STATE_PLAYER)
		{
			if (CHECK_PROFILE_BUTTON(PrevWeapon) == nButton)
			{
				if (WR_BUTTONSTATUS_PUSHED == nStatus)
					pPlayer->SelectNextItem(-1, true, 0);
			}
			if (CHECK_PROFILE_BUTTON(NextWeapon) == nButton)
			{
				if (WR_BUTTONSTATUS_PUSHED == nStatus)
					pPlayer->SelectNextItem(1, true, 0);
			}
		}
	}
	else if (nState == STATE_LANDVEHICLE || nState == STATE_SEAVEHICLE)
	{
		// Get vehicle
		IVehicle *pVehicle = pPlayer->GetLinkedVehicle();
		if (NULL != pVehicle)
		{
			// Use key
			if (CHECK_PROFILE_BUTTON(Veh_Use) == nButton)
			{
				if (WR_BUTTONSTATUS_PUSHED == nStatus) pPlayer->GetPlayerInput()->OnAction(rGameActions.use, eIS_Pressed, 1.0f);
				if (WR_BUTTONSTATUS_RELEASED == nStatus) pPlayer->GetPlayerInput()->OnAction(rGameActions.use, eIS_Released, 1.0f);
			}

			// Change seat
			if (IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId()))
			{
				TVehicleSeatId nSeatId = pSeat->GetSeatId();
				TVehicleSeatId nPrevId = nSeatId;
				if (CHECK_PROFILE_BUTTON(Veh_PrevSeat) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus)
				{
					// If first seat, select last one; otherwise, select previous
					if (nSeatId == 1)
						nSeatId = pVehicle->GetLastSeatId();
					else
						nSeatId--;
				}
				else if (CHECK_PROFILE_BUTTON(Veh_NextSeat) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus)
				{
					// If last seat, select first one; othwerise, select next
					if (nSeatId == pVehicle->GetLastSeatId())
						nSeatId = 1;
					else
						nSeatId++;
				}
				if (nSeatId != nPrevId)
				{
					// Change to its seat
					IVehicleSeat *pPrevSeat = pVehicle->GetSeatById(nPrevId);
					IVehicleSeat *pNextSeat = pVehicle->GetSeatById(nSeatId);
					if (pPrevSeat && pNextSeat)
					{
						pPrevSeat->Exit(false, true);
						pNextSeat->Enter(pPlayer->GetEntityId(), false);
					}
				}
			}

			// Beep dat horn dawg
			if (CHECK_PROFILE_BUTTON(Veh_Horn) == nButton)
			{
				if (WR_BUTTONSTATUS_PUSHED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_horn, eIS_Pressed, 1.0f);
				else if (WR_BUTTONSTATUS_RELEASED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_horn, eIS_Released, 1.0f);
			}

			// Lights
			if (CHECK_PROFILE_BUTTON(Veh_Lights) == nButton)
			{
				if (WR_BUTTONSTATUS_PUSHED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_lights, eIS_Pressed, 1.0f);
				else if (WR_BUTTONSTATUS_RELEASED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_lights, eIS_Released, 1.0f);
			}

			// Boost
			if (CHECK_PROFILE_BUTTON(Veh_Boost) == nButton)
			{
				if (WR_BUTTONSTATUS_PUSHED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_boost, eIS_Pressed, 1.0f);
				else if (WR_BUTTONSTATUS_RELEASED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_boost, eIS_Released, 1.0f);
			}

			// Change view
			if (CHECK_PROFILE_BUTTON(Veh_View) == nButton)
			{
				if (WR_BUTTONSTATUS_RELEASED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeview, eIS_Released, 1.0f);
			}

			// Handle vehicle weapon
			EntityId nWeaponId = pVehicle->GetCurrentWeaponId(pPlayer->GetEntityId());
			if (CItem *pWeaponItem = (CItem*)gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(nWeaponId))
			{
				if (NULL != pWeaponItem->GetIWeapon())
				{
					// Fire
					if (CHECK_PROFILE_BUTTON(Veh_Fire) == nButton)
					{
						if (nStatus == WR_BUTTONSTATUS_PUSHED)
							pWeaponItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.attack1, eIS_Pressed, 1.0f);
						else if (nStatus == WR_BUTTONSTATUS_RELEASED)	
							pWeaponItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.attack1, eIS_Released, 1.0f);
					}

					// Reload
					if (CHECK_PROFILE_BUTTON(Veh_Reload) == nButton && nStatus == WR_BUTTONSTATUS_PUSHED)
					{
						pWeaponItem->GetIWeapon()->OnAction(pPlayer->GetEntityId(), rGameActions.reload, eIS_Pressed, 1.0f);
					}
				}
			}
		}
	}
	else if (nState == STATE_VTOLVEHICLE)
	{
		// Get vehicle
		IVehicle *pVehicle = pPlayer->GetLinkedVehicle();
		if (NULL != pVehicle)
		{
			// Use key
			if (CHECK_PROFILE_BUTTON(VTOL_Use) == nButton)
			{
				if (WR_BUTTONSTATUS_PUSHED == nStatus) pPlayer->GetPlayerInput()->OnAction(rGameActions.use, eIS_Pressed, 1.0f);
				if (WR_BUTTONSTATUS_RELEASED == nStatus) pPlayer->GetPlayerInput()->OnAction(rGameActions.use, eIS_Released, 1.0f);
			}

			// Change seat
			if (IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId()))
			{
				TVehicleSeatId nSeatId = pSeat->GetSeatId();
				TVehicleSeatId nPrevId = nSeatId;
				if (CHECK_PROFILE_BUTTON(VTOL_PrevSeat) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus)
				{
					// If first seat, select last one; otherwise, select previous
					if (nSeatId == 1)
						nSeatId = pVehicle->GetLastSeatId();
					else
						nSeatId--;
				}
				else if (CHECK_PROFILE_BUTTON(VTOL_NextSeat) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus)
				{
					// If last seat, select first one; othwerise, select next
					if (nSeatId == pVehicle->GetLastSeatId())
						nSeatId = 1;
					else
						nSeatId++;
				}
				if (nSeatId != nPrevId)
				{
					// Change to its seat
					IVehicleSeat *pPrevSeat = pVehicle->GetSeatById(nPrevId);
					IVehicleSeat *pNextSeat = pVehicle->GetSeatById(nSeatId);
					if (pPrevSeat && pNextSeat)
					{
						pPrevSeat->Exit(false, true);
						pNextSeat->Enter(pPlayer->GetEntityId(), false);
					}
				}
			}

			// Boost
			if (CHECK_PROFILE_BUTTON(VTOL_Boost) == nButton)
			{
				if (WR_BUTTONSTATUS_PUSHED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_boost, eIS_Pressed, 1.0f);
				else if (WR_BUTTONSTATUS_RELEASED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_boost, eIS_Released, 1.0f);
			}

			// Change view
			if (CHECK_PROFILE_BUTTON(VTOL_View) == nButton)
			{
				if (WR_BUTTONSTATUS_RELEASED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeview, eIS_Released, 1.0f);
			}

			// Handle vehicle weapon
			if (CHECK_PROFILE_BUTTON(VTOL_FireGun) == nButton)
			{
				if (nStatus == WR_BUTTONSTATUS_PUSHED)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.attack1, eIS_Pressed, 1.0f);
				else if (nStatus == WR_BUTTONSTATUS_RELEASED)	
					pPlayer->GetPlayerInput()->OnAction(rGameActions.attack1, eIS_Released, 1.0f);
			}
			if (CHECK_PROFILE_BUTTON(VTOL_FireRocket) == nButton)
			{
				if (nStatus == WR_BUTTONSTATUS_PUSHED)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_attack2, eIS_Pressed, 1.0f);
				else if (nStatus == WR_BUTTONSTATUS_RELEASED)	
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_attack2, eIS_Released, 1.0f);
			}
		}
	}
	else if (nState == STATE_HELIVEHICLE)
	{
		// Get vehicle
		IVehicle *pVehicle = pPlayer->GetLinkedVehicle();
		if (NULL != pVehicle)
		{
			// Use key
			if (CHECK_PROFILE_BUTTON(Heli_Use) == nButton)
			{
				if (WR_BUTTONSTATUS_PUSHED == nStatus) pPlayer->GetPlayerInput()->OnAction(rGameActions.use, eIS_Pressed, 1.0f);
				if (WR_BUTTONSTATUS_RELEASED == nStatus) pPlayer->GetPlayerInput()->OnAction(rGameActions.use, eIS_Released, 1.0f);
			}

			// Change seat
			if (IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId()))
			{
				TVehicleSeatId nSeatId = pSeat->GetSeatId();
				TVehicleSeatId nPrevId = nSeatId;
				if (CHECK_PROFILE_BUTTON(Heli_PrevSeat) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus)
				{
					// If first seat, select last one; otherwise, select previous
					if (nSeatId == 1)
						nSeatId = pVehicle->GetLastSeatId();
					else
						nSeatId--;
				}
				else if (CHECK_PROFILE_BUTTON(Heli_NextSeat) == nButton && WR_BUTTONSTATUS_PUSHED == nStatus)
				{
					// If last seat, select first one; othwerise, select next
					if (nSeatId == pVehicle->GetLastSeatId())
						nSeatId = 1;
					else
						nSeatId++;
				}
				if (nSeatId != nPrevId)
				{
					// Change to its seat
					IVehicleSeat *pPrevSeat = pVehicle->GetSeatById(nPrevId);
					IVehicleSeat *pNextSeat = pVehicle->GetSeatById(nSeatId);
					if (pPrevSeat && pNextSeat)
					{
						pPrevSeat->Exit(false, true);
						pNextSeat->Enter(pPlayer->GetEntityId(), false);
					}
				}
			}

			// Boost
			if (CHECK_PROFILE_BUTTON(Heli_Boost) == nButton)
			{
				if (WR_BUTTONSTATUS_PUSHED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_boost, eIS_Pressed, 1.0f);
				else if (WR_BUTTONSTATUS_RELEASED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_boost, eIS_Released, 1.0f);
			}

			// Change view
			if (CHECK_PROFILE_BUTTON(Heli_View) == nButton)
			{
				if (WR_BUTTONSTATUS_RELEASED == nStatus)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_changeview, eIS_Released, 1.0f);
			}

			// Handle vehicle weapon
			if (CHECK_PROFILE_BUTTON(Heli_FireGun) == nButton)
			{
				if (nStatus == WR_BUTTONSTATUS_PUSHED)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.attack1, eIS_Pressed, 1.0f);
				else if (nStatus == WR_BUTTONSTATUS_RELEASED)	
					pPlayer->GetPlayerInput()->OnAction(rGameActions.attack1, eIS_Released, 1.0f);
			}
			if (CHECK_PROFILE_BUTTON(Heli_FireRocket) == nButton)
			{
				if (nStatus == WR_BUTTONSTATUS_PUSHED)
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_attack2, eIS_Pressed, 1.0f);
				else if (nStatus == WR_BUTTONSTATUS_RELEASED)	
					pPlayer->GetPlayerInput()->OnAction(rGameActions.v_attack2, eIS_Released, 1.0f);
			}
		}
	}

	pPlayer->GetMovementController()->RequestMovement(mr);
}

////////////////////////////////////////////////////
void SWiiInputListener::OnAction(IWR_WiiRemote *pRemote, IWR_WiiButtons *pButtons, char const* szAction, ActionID nActionID, int nStatus, bool bDown)
{

}

////////////////////////////////////////////////////
void SWiiInputListener::OnExtensionAction(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, char const* szAction, ActionID nActionID, int nStatus, bool bDown)
{

}

////////////////////////////////////////////////////
void SWiiInputListener::OnSingleMotion(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion)
{
	if (NULL == pManager || false == pManager->IsMasterEnabled()) return;

	if (motion.nLifetime == nEndLifetime) return;

	CPlayer *pPlayer = GetPlayer();
	if (NULL == pPlayer) return;
	const CGameActions &rGameActions = pManager->m_pGame->Actions();

	// Player controls
	if (false == pManager->m_bForceLockView && false == pManager->IsMovementFrozen())
	{
		if (nState == STATE_PLAYER || nState == STATE_BINOCULARS)
		{
			if (false == UsingIRSensor())
			{
				// Ignore if a motion is alive
				bool bContinue = true;
				if (true == bInGesture)
				{
					// Have we dropped out?
					if (pRemote->GetMotionHelper()->GetMotionLifetime() == 1)
						bInGesture = false;
					bContinue = false;
				}
				else if (pRemote->GetMotionHelper()->GetMotionLifetime() > 1)
				{
					// In a gesture
					bInGesture = true;
					bContinue = false;
				}
				if (true == bContinue)
				{
					CMovementRequest mr;
					bool bInLockRetain = (true == pManager->m_bLockView && true == CHECK_PROFILE_BOOL(RetainViewMode));
					if ((false == pManager->m_bLockView) || (true == bInLockRetain && NULL == pManager->m_pLockedEntity))
					{
						// Look up/down
						SMovementState info;
						pPlayer->GetMovementController()->GetMovementState(info);
						Vec3 vForward = info.aimDirection;
						Vec3 vPlanarForward(vForward.x,vForward.y,0.0f);
						const float fPitch = acosf(vForward.Dot(vPlanarForward)) * (vForward.z < 0.0f ? -1.0f : 1.0f);
						const float fMotionPitch = DEG2RAD(180.0f) * (motion.fPitch / DEG2RAD(CHECK_PROFILE_FLOAT(LookUpMaxTilt)));
						const float fDelta = (fMotionPitch * (true == CHECK_PROFILE_BOOL(InverseLook) ? -1.0f : 1.0f)) - fPitch;
						float fLookUp = 0.0f;
						if (RAD2DEG(fabs(fDelta)) >= (CHECK_PROFILE_FLOAT(LookUpError) * (180.0f/CHECK_PROFILE_FLOAT(LookUpMaxTilt))))
						{
							// Change in angle is larger than the error scaled by our max tilt, so apply
							fLookUp = fDelta * CHECK_PROFILE_FLOAT(LookUpSensitivity);
						}

						// Turn
						float fTurn = 0.0f;
						const float fTurnMin = CHECK_PROFILE_FLOAT(TurnTilt);
						if (fabs(motion.fRoll) >= DEG2RAD(fTurnMin))
						{
							fTurn = -motion.fRoll*CHECK_PROFILE_FLOAT(TurnSensitivity);
						}
						
						// Request rotation
						mr.AddDeltaRotation(Ang3(fLookUp,0.0f,fTurn));
					}
					else if (true == bInLockRetain && NULL != pManager->m_pLockedEntity && false == CHECK_PROFILE_BOOL(HardLockView))
					{
						// Get move speeds based on sensitivity and dead zones
						float fMovePitch = 0.0f, fMoveRoll = 0.0f;
						SMovementState info;
						pPlayer->GetMovementController()->GetMovementState(info);
						Vec3 vForward = info.aimDirection;
						Vec3 vPlanarForward(vForward.x,vForward.y,0.0f);
						const float fPitch = acosf(vForward.Dot(vPlanarForward)) * (vForward.z < 0.0f ? -1.0f : 1.0f);
						const float fMotionPitch = DEG2RAD(180.0f) * (motion.fPitch / DEG2RAD(CHECK_PROFILE_FLOAT(LookUpMaxTilt)));
						const float fDelta = (fMotionPitch * (true == CHECK_PROFILE_BOOL(InverseLook) ? -1.0f : 1.0f)) - fPitch;
						float fLookUp = 0.0f;
						if (RAD2DEG(fabs(fDelta)) >= (CHECK_PROFILE_FLOAT(LookUpError) * (180.0f/CHECK_PROFILE_FLOAT(LookUpMaxTilt))))
						{
							fMovePitch = fDelta * CHECK_PROFILE_FLOAT(SoftLock_LookUpSensitivity);
						}
						const float fTurnMin = CHECK_PROFILE_FLOAT(TurnTilt);
						if (fabs(motion.fRoll) >= DEG2RAD(fTurnMin))
						{
							fMoveRoll = -motion.fRoll*CHECK_PROFILE_FLOAT(SoftLock_TurnSensitivity);
						}

						// Move around
						Matrix34 mPlayerMat = pPlayer->GetEntity()->GetLocalTM();
						Vec3 const& vRight = mPlayerMat.GetColumn0();
						Vec3 const& vUp = mPlayerMat.GetColumn2();
						pManager->m_vLockedEntityOffset += vRight*fMoveRoll;
						pManager->m_vLockedEntityOffset += vUp*fMovePitch;

						// Clamp to its AABB (in local space)
						AABB aabb = pManager->m_pLockedEntity->GetBBox();
						Vec3 vPos = aabb.GetCenter()+Vec3(0.f,0.f,(aabb.max.z-aabb.min.z)*0.25f);
						aabb.max -= vPos;
						aabb.min -= vPos;
						if (pManager->m_vLockedEntityOffset.x < aabb.min.x) pManager->m_vLockedEntityOffset.x = aabb.min.x;
						if (pManager->m_vLockedEntityOffset.x > aabb.max.x) pManager->m_vLockedEntityOffset.x = aabb.max.x;
						if (pManager->m_vLockedEntityOffset.y < aabb.min.y) pManager->m_vLockedEntityOffset.y = aabb.min.y;
						if (pManager->m_vLockedEntityOffset.y > aabb.max.y) pManager->m_vLockedEntityOffset.y = aabb.max.y;
						if (pManager->m_vLockedEntityOffset.z < aabb.min.z) pManager->m_vLockedEntityOffset.z = aabb.min.z;
						if (pManager->m_vLockedEntityOffset.z > aabb.max.z) pManager->m_vLockedEntityOffset.z = aabb.max.z;
					}
		
					pPlayer->GetMovementController()->RequestMovement(mr);
				}
			}
		}
		else if (nState == STATE_LANDVEHICLE || nState == STATE_SEAVEHICLE)
		{
			// Get vehicle
			IVehicle* pVehicle = NULL;
			IPlayerInput *pPlayerInput = NULL;
			if (pVehicle = pPlayer->GetLinkedVehicle())
			{
				// Send it through actions
				pPlayerInput = pPlayer->GetPlayerInput();
			}
			if (NULL != pPlayerInput)
			{
				bool bInLockRetain = (true == pManager->m_bLockView && true == CHECK_PROFILE_BOOL(RetainViewMode));
				if ((false == pManager->m_bLockView) || (true == bInLockRetain && NULL == pManager->m_pLockedEntity))
				{
					// Ignore if using IR sensor
					if (false == UsingIRSensor())
					{
						// Look up/down
						SMovementState info;
						pPlayer->GetMovementController()->GetMovementState(info);
						Vec3 vForward = info.aimDirection;
						Vec3 vPlanarForward(vForward.x,vForward.y,0.0f);
						const float fPitch = acosf(vForward.Dot(vPlanarForward)) * (vForward.z < 0.0f ? -1.0f : 1.0f);
						const float fMotionPitch = DEG2RAD(180.0f) * (motion.fPitch / DEG2RAD(CHECK_PROFILE_FLOAT(Veh_LookUpMaxTilt)));
						const float fDelta = (fMotionPitch * (true == CHECK_PROFILE_BOOL(Veh_InverseLook) ? -1.0f : 1.0f)) - fPitch;
						float fLookUp = 0.0f;
						if (RAD2DEG(fabs(fDelta)) >= (CHECK_PROFILE_FLOAT(Veh_LookUpError) * (180.0f/CHECK_PROFILE_FLOAT(Veh_LookUpMaxTilt))))
						{
							// Change in angle is larger than the error scaled by our max tilt, so apply
							fLookUp = -fDelta * CHECK_PROFILE_FLOAT(Veh_LookUpSensitivity);
						}

						// Turn
						float fTurn = 0.0f;
						const float fTurnMin = CHECK_PROFILE_FLOAT(Veh_TurnTilt);
						if (fabs(motion.fRoll) >= DEG2RAD(fTurnMin))
						{
							fTurn = motion.fRoll*CHECK_PROFILE_FLOAT(Veh_TurnSensitivity);
						}

						// Scale if in third person view or if gunner
						IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId());
						if (pSeat)
						{
							IVehicleView *pView = pSeat->GetView(pSeat->GetCurrentView());
							if (pView && pView->IsThirdPerson())
							{
								fLookUp *= 400.0f*gEnv->pTimer->GetFrameTime();
								fTurn *= 200.0f*gEnv->pTimer->GetFrameTime();
							}
							else if (pSeat->IsGunner() && !pSeat->IsDriver())
							{
								fLookUp *= 400.0f*gEnv->pTimer->GetFrameTime();
								fTurn *= 200.0f*gEnv->pTimer->GetFrameTime();
							}
						}

						// Look around
						pPlayerInput->OnAction(rGameActions.v_rotatepitch, eAAM_Always, fLookUp);
						pPlayerInput->OnAction(rGameActions.v_rotateyaw, eAAM_Always, fTurn);
					}
				}
				else if (true == bInLockRetain && NULL != pManager->m_pLockedEntity && false == CHECK_PROFILE_BOOL(HardLockView))
				{
					// Get move speeds based on sensitivity and dead zones
					float fMovePitch = 0.0f, fMoveRoll = 0.0f;
					SMovementState info;
					pPlayer->GetMovementController()->GetMovementState(info);
					Vec3 vForward = info.aimDirection;
					Vec3 vPlanarForward(vForward.x,vForward.y,0.0f);
					const float fPitch = acosf(vForward.Dot(vPlanarForward)) * (vForward.z < 0.0f ? -1.0f : 1.0f);
					const float fMotionPitch = DEG2RAD(180.0f) * (motion.fPitch / DEG2RAD(CHECK_PROFILE_FLOAT(Veh_LookUpMaxTilt)));
					const float fDelta = (fMotionPitch * (true == CHECK_PROFILE_BOOL(Veh_InverseLook) ? -1.0f : 1.0f)) - fPitch;
					float fLookUp = 0.0f;
					if (RAD2DEG(fabs(fDelta)) >= (CHECK_PROFILE_FLOAT(Veh_LookUpError) * (180.0f/CHECK_PROFILE_FLOAT(Veh_LookUpMaxTilt))))
					{
						fMovePitch = fDelta * CHECK_PROFILE_FLOAT(SoftLock_LookUpSensitivity);
					}
					const float fTurnMin = CHECK_PROFILE_FLOAT(Veh_TurnTilt);
					if (fabs(motion.fRoll) >= DEG2RAD(fTurnMin))
					{
						fMoveRoll = -motion.fRoll*CHECK_PROFILE_FLOAT(SoftLock_TurnSensitivity);
					}

					// Scale if in third person view or if gunner
					IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId());
					if (pSeat)
					{
						IVehicleView *pView = pSeat->GetView(pSeat->GetCurrentView());
						if (pView && pView->IsThirdPerson())
						{
							fMovePitch *= 400.0f*gEnv->pTimer->GetFrameTime();
							fMoveRoll *= 200.0f*gEnv->pTimer->GetFrameTime();
						}
						else if (pSeat->IsGunner() && !pSeat->IsDriver())
						{
							fMovePitch *= 400.0f*gEnv->pTimer->GetFrameTime();
							fMoveRoll *= 200.0f*gEnv->pTimer->GetFrameTime();
						}
					}

					// Move around
					Matrix34 mPlayerMat = pPlayer->GetEntity()->GetLocalTM();
					Vec3 const& vRight = mPlayerMat.GetColumn0();
					Vec3 const& vUp = mPlayerMat.GetColumn2();
					pManager->m_vLockedEntityOffset += vRight*fMoveRoll;
					pManager->m_vLockedEntityOffset += vUp*fMovePitch;

					// Clamp to its AABB (in local space)
					AABB aabb = pManager->m_pLockedEntity->GetBBox();
					Vec3 vPos = aabb.GetCenter()+Vec3(0.f,0.f,(aabb.max.z-aabb.min.z)*0.25f);
					aabb.max -= vPos;
					aabb.min -= vPos;
					if (pManager->m_vLockedEntityOffset.x < aabb.min.x) pManager->m_vLockedEntityOffset.x = aabb.min.x;
					if (pManager->m_vLockedEntityOffset.x > aabb.max.x) pManager->m_vLockedEntityOffset.x = aabb.max.x;
					if (pManager->m_vLockedEntityOffset.y < aabb.min.y) pManager->m_vLockedEntityOffset.y = aabb.min.y;
					if (pManager->m_vLockedEntityOffset.y > aabb.max.y) pManager->m_vLockedEntityOffset.y = aabb.max.y;
					if (pManager->m_vLockedEntityOffset.z < aabb.min.z) pManager->m_vLockedEntityOffset.z = aabb.min.z;
					if (pManager->m_vLockedEntityOffset.z > aabb.max.z) pManager->m_vLockedEntityOffset.z = aabb.max.z;
				}
			}
		}
		else if (nState == STATE_VTOLVEHICLE)
		{
			// Get vehicle
			IVehicle* pVehicle = NULL;
			IPlayerInput *pPlayerInput = NULL;
			if (pVehicle = pPlayer->GetLinkedVehicle())
			{
				// Send it through actions
				pPlayerInput = pPlayer->GetPlayerInput();
			}
			if (NULL != pPlayerInput)
			{
				bool bInLockRetain = (true == pManager->m_bLockView && true == CHECK_PROFILE_BOOL(RetainViewMode));
				if ((false == pManager->m_bLockView) || (true == bInLockRetain && NULL == pManager->m_pLockedEntity))
				{
					// Ignore if using IR sensor
					if (false == UsingIRSensor())
					{
						// Pitch update
						float fPitch = 0.0f;
						const float fPitchMin = CHECK_PROFILE_FLOAT(VTOL_PitchTilt);
						if (fabs(motion.fPitch) >= DEG2RAD(fPitchMin))
						{
							fPitch = -motion.fPitch*(CHECK_PROFILE_BOOL(VTOL_InverseLook)?-1.0f:1.0f)*CHECK_PROFILE_FLOAT(VTOL_PitchSensitivity);
						}

						// Turn update
						float fTurn = 0.0f;
						const float fTurnMin = CHECK_PROFILE_FLOAT(VTOL_TurnTilt);
						if (fabs(motion.fRoll) >= DEG2RAD(fTurnMin))
						{
							fTurn = motion.fRoll*CHECK_PROFILE_FLOAT(VTOL_TurnSensitivity);
						}

						// Scale if in third person view or if gunner
						bool bIsGunner = false;
						IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId());
						if (pSeat)
						{
							bIsGunner = (pSeat->IsGunner() && !pSeat->IsDriver());

							IVehicleView *pView = pSeat->GetView(pSeat->GetCurrentView());
							if (pView && pView->IsThirdPerson())
							{
								fPitch *= 400.0f*gEnv->pTimer->GetFrameTime();
								fTurn *= 200.0f*gEnv->pTimer->GetFrameTime();
							}
							else if (true == bIsGunner)
							{
								fPitch *= 400.0f*gEnv->pTimer->GetFrameTime();
								fTurn *= 200.0f*gEnv->pTimer->GetFrameTime();
							}
						}

						// Look around
						if (false == bIsGunner)
						{
							CVehicleMovementVTOL *pVM = (CVehicleMovementVTOL*)pPlayer->GetLinkedVehicle()->GetMovement();
							pVM->OnAction(eVAI_RotatePitch, eAAM_Always, fPitch);
							pVM->OnAction(eVAI_RotateYaw, eAAM_Always, fTurn);
						}
						else
						{
							pPlayerInput->OnAction(rGameActions.v_rotatepitch, eAAM_Always, fPitch);
							pPlayerInput->OnAction(rGameActions.v_rotateyaw, eAAM_Always, fTurn);
						}
					}
				}
				else if (true == bInLockRetain && NULL != pManager->m_pLockedEntity && false == CHECK_PROFILE_BOOL(HardLockView))
				{
					// Get move speeds based on sensitivity and dead zones
					float fMovePitch = 0.0f, fMoveRoll = 0.0f;
					if (fabs(motion.fPitch) >= DEG2RAD(CHECK_PROFILE_FLOAT(VTOL_PitchTilt)))
					{
						fMovePitch = -motion.fPitch*CHECK_PROFILE_FLOAT(SoftLock_LookUpSensitivity);
					}
					if (fabs(motion.fRoll) >= DEG2RAD(CHECK_PROFILE_FLOAT(VTOL_TurnTilt)))
					{
						fMoveRoll = -motion.fRoll*CHECK_PROFILE_FLOAT(SoftLock_TurnSensitivity);
					}

					// Scale if in third person view or if gunner
					IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId());
					if (pSeat)
					{
						IVehicleView *pView = pSeat->GetView(pSeat->GetCurrentView());
						if (pView && pView->IsThirdPerson())
						{
							fMovePitch *= 400.0f*gEnv->pTimer->GetFrameTime();
							fMoveRoll *= 200.0f*gEnv->pTimer->GetFrameTime();
						}
						else if (pSeat->IsGunner() && !pSeat->IsDriver())
						{
							fMovePitch *= 400.0f*gEnv->pTimer->GetFrameTime();
							fMoveRoll *= 200.0f*gEnv->pTimer->GetFrameTime();
						}
					}

					// Move around
					Matrix34 mPlayerMat = pPlayer->GetEntity()->GetLocalTM();
					Vec3 const& vRight = mPlayerMat.GetColumn0();
					Vec3 const& vUp = mPlayerMat.GetColumn2();
					pManager->m_vLockedEntityOffset += vRight*fMoveRoll;
					pManager->m_vLockedEntityOffset += vUp*fMovePitch;

					// Clamp to its AABB (in local space)
					AABB aabb = pManager->m_pLockedEntity->GetBBox();
					Vec3 vPos = aabb.GetCenter()+Vec3(0.f,0.f,(aabb.max.z-aabb.min.z)*0.25f);
					aabb.max -= vPos;
					aabb.min -= vPos;
					if (pManager->m_vLockedEntityOffset.x < aabb.min.x) pManager->m_vLockedEntityOffset.x = aabb.min.x;
					if (pManager->m_vLockedEntityOffset.x > aabb.max.x) pManager->m_vLockedEntityOffset.x = aabb.max.x;
					if (pManager->m_vLockedEntityOffset.y < aabb.min.y) pManager->m_vLockedEntityOffset.y = aabb.min.y;
					if (pManager->m_vLockedEntityOffset.y > aabb.max.y) pManager->m_vLockedEntityOffset.y = aabb.max.y;
					if (pManager->m_vLockedEntityOffset.z < aabb.min.z) pManager->m_vLockedEntityOffset.z = aabb.min.z;
					if (pManager->m_vLockedEntityOffset.z > aabb.max.z) pManager->m_vLockedEntityOffset.z = aabb.max.z;
				}
			}
		}
		else if (nState == STATE_HELIVEHICLE)
		{
			// Get vehicle
			IVehicle* pVehicle = NULL;
			IPlayerInput *pPlayerInput = NULL;
			if (pVehicle = pPlayer->GetLinkedVehicle())
			{
				// Send it through actions
				pPlayerInput = pPlayer->GetPlayerInput();
			}
			if (NULL != pPlayerInput)
			{
				bool bInLockRetain = (true == pManager->m_bLockView && true == CHECK_PROFILE_BOOL(RetainViewMode));
				if ((false == pManager->m_bLockView) || (true == bInLockRetain && NULL == pManager->m_pLockedEntity))
				{
					// Ignore if using IR sensor
					if (false == UsingIRSensor())
					{
						// Pitch update
						float fPitch = 0.0f;
						const float fPitchMin = CHECK_PROFILE_FLOAT(Heli_PitchTilt);
						if (fabs(motion.fPitch) >= DEG2RAD(fPitchMin))
						{
							fPitch = -motion.fPitch*(CHECK_PROFILE_BOOL(Heli_InverseLook)?-1.0f:1.0f)*CHECK_PROFILE_FLOAT(Heli_PitchSensitivity);
						}

						// Turn update
						float fTurn = 0.0f;
						const float fTurnMin = CHECK_PROFILE_FLOAT(Heli_TurnTilt);
						if (fabs(motion.fRoll) >= DEG2RAD(fTurnMin))
						{
							fTurn = motion.fRoll*CHECK_PROFILE_FLOAT(Heli_TurnSensitivity);
						}

						// Scale if in third person view or if gunner
						bool bIsGunner = false;
						IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId());
						if (pSeat)
						{
							bIsGunner = (pSeat->IsGunner() && !pSeat->IsDriver());

							IVehicleView *pView = pSeat->GetView(pSeat->GetCurrentView());
							if (pView && pView->IsThirdPerson())
							{
								fPitch *= 400.0f*gEnv->pTimer->GetFrameTime();
								fTurn *= 200.0f*gEnv->pTimer->GetFrameTime();
							}
							else if (true == bIsGunner)
							{
								fPitch *= 400.0f*gEnv->pTimer->GetFrameTime();
								fTurn *= 200.0f*gEnv->pTimer->GetFrameTime();
							}
						}

						// Look around
						if (false == bIsGunner)
						{
							CVehicleMovementHelicopter *pVM = (CVehicleMovementHelicopter*)pPlayer->GetLinkedVehicle()->GetMovement();
							pVM->OnAction(eVAI_RotatePitch, eAAM_Always, fPitch);
							pVM->OnAction(eVAI_RotateYaw, eAAM_Always, fTurn);
						}
						else
						{
							pPlayerInput->OnAction(rGameActions.v_rotatepitch, eAAM_Always, fPitch);
							pPlayerInput->OnAction(rGameActions.v_rotateyaw, eAAM_Always, fTurn);
						}
					}
				}
				else if (true == bInLockRetain && NULL != pManager->m_pLockedEntity && false == CHECK_PROFILE_BOOL(HardLockView))
				{
					// Get move speeds based on sensitivity and dead zones
					float fMovePitch = 0.0f, fMoveRoll = 0.0f;
					if (fabs(motion.fPitch) >= DEG2RAD(CHECK_PROFILE_FLOAT(Heli_PitchTilt)))
					{
						fMovePitch = -motion.fPitch*CHECK_PROFILE_FLOAT(SoftLock_LookUpSensitivity);
					}
					if (fabs(motion.fRoll) >= DEG2RAD(CHECK_PROFILE_FLOAT(Heli_TurnTilt)))
					{
						fMoveRoll = -motion.fRoll*CHECK_PROFILE_FLOAT(SoftLock_TurnSensitivity);
					}

					// Scale if in third person view or if gunner
					IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId());
					if (pSeat)
					{
						IVehicleView *pView = pSeat->GetView(pSeat->GetCurrentView());
						if (pView && pView->IsThirdPerson())
						{
							fMovePitch *= 400.0f*gEnv->pTimer->GetFrameTime();
							fMoveRoll *= 200.0f*gEnv->pTimer->GetFrameTime();
						}
						else if (pSeat->IsGunner() && !pSeat->IsDriver())
						{
							fMovePitch *= 400.0f*gEnv->pTimer->GetFrameTime();
							fMoveRoll *= 200.0f*gEnv->pTimer->GetFrameTime();
						}
					}

					// Move around
					Matrix34 mPlayerMat = pPlayer->GetEntity()->GetLocalTM();
					Vec3 const& vRight = mPlayerMat.GetColumn0();
					Vec3 const& vUp = mPlayerMat.GetColumn2();
					pManager->m_vLockedEntityOffset += vRight*fMoveRoll;
					pManager->m_vLockedEntityOffset += vUp*fMovePitch;

					// Clamp to its AABB (in local space)
					AABB aabb = pManager->m_pLockedEntity->GetBBox();
					Vec3 vPos = aabb.GetCenter()+Vec3(0.f,0.f,(aabb.max.z-aabb.min.z)*0.25f);
					aabb.max -= vPos;
					aabb.min -= vPos;
					if (pManager->m_vLockedEntityOffset.x < aabb.min.x) pManager->m_vLockedEntityOffset.x = aabb.min.x;
					if (pManager->m_vLockedEntityOffset.x > aabb.max.x) pManager->m_vLockedEntityOffset.x = aabb.max.x;
					if (pManager->m_vLockedEntityOffset.y < aabb.min.y) pManager->m_vLockedEntityOffset.y = aabb.min.y;
					if (pManager->m_vLockedEntityOffset.y > aabb.max.y) pManager->m_vLockedEntityOffset.y = aabb.max.y;
					if (pManager->m_vLockedEntityOffset.z < aabb.min.z) pManager->m_vLockedEntityOffset.z = aabb.min.z;
					if (pManager->m_vLockedEntityOffset.z > aabb.max.z) pManager->m_vLockedEntityOffset.z = aabb.max.z;
				}
			}
		}
	}
}

////////////////////////////////////////////////////
void SWiiInputListener::OnMotionStart(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion)
{
	if (NULL == pManager || false == pManager->IsMasterEnabled()) return;

	nEndLifetime = 0;
	for (MotionList::iterator itMotion = pManager->m_WiimoteMotions.begin(); 
		itMotion != pManager->m_WiimoteMotions.end(); itMotion++)
	{
		(*itMotion)->OnBegin(motion, nState);
	}
}

////////////////////////////////////////////////////
void SWiiInputListener::OnMotionUpdate(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion)
{
	if (NULL == pManager || false == pManager->IsMasterEnabled()) return;

	for (MotionList::iterator itMotion = pManager->m_WiimoteMotions.begin(); 
		itMotion != pManager->m_WiimoteMotions.end(); itMotion++)
	{
		(*itMotion)->OnUpdate(motion, nState);
	}
}

////////////////////////////////////////////////////
void SWiiInputListener::OnMotionEnd(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion)
{
	// Find which one fired here
	IMotion *pFiredMotion = NULL;
	IMotion *pMotionI = NULL;
	for (MotionList::iterator itMotion = pManager->m_WiimoteMotions.begin(); 
		itMotion != pManager->m_WiimoteMotions.end(); itMotion++)
	{
		pMotionI = *itMotion;
		pMotionI->OnEnd(motion, nState);
		if (pMotionI->IsFired() && (NULL == pFiredMotion || pMotionI->GetPriority() > pFiredMotion->GetPriority()))
			pFiredMotion = pMotionI;
	}
	
	if (NULL == pManager || false == pManager->IsMasterEnabled()) return;

	// Execute fired one
	if (NULL != pFiredMotion)
	{
		pFiredMotion->Execute();
		nEndLifetime = motion.nLifetime;
	}
}

////////////////////////////////////////////////////
void SWiiInputListener::OnExtensionSingleMotion(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion)
{
	if (NULL == pManager || false == pManager->IsMasterEnabled()) return;

	if (motion.nLifetime == nExtensionEndLifetime) return;

	CPlayer *pPlayer = GetPlayer();
	if (NULL == pPlayer) return;

	// Player controls
	if (nState == STATE_PLAYER || nState == STATE_BINOCULARS)
	{
		// Don't continue if we are jumping
		SActorStats *pStats = pPlayer->GetActorStats();
		if (pStats->inAir <= 0.0f || true == pStats->inZeroG)
		{
			CMovementRequest mr;

			// Check pitch to adjust the stance
			if (motion.fPitch > DEG2RAD(-CHECK_PROFILE_FLOAT(CrouchTilt)))
			{
				mr.SetStance(STANCE_STAND);
				pPlayer->GetPlayerInput()->SetForceStance(STANCE_NULL);
			}
			else if (motion.fPitch > DEG2RAD(-CHECK_PROFILE_FLOAT(ProneTilt)))
			{
				mr.SetStance(STANCE_CROUCH);
				pPlayer->GetPlayerInput()->SetForceStance(STANCE_CROUCH);
			}
			else
			{
				mr.SetStance(STANCE_PRONE);
				pPlayer->GetPlayerInput()->SetForceStance(STANCE_PRONE);
			}

			// Use roll as lean if not in dead zone
			float fLean = 0.0f;
			const float fMin = CHECK_PROFILE_FLOAT(LeanTilt);
			if (motion.fRoll >= DEG2RAD(fMin))
			{
				fLean = ((motion.fRoll-DEG2RAD(fMin))/(1.0f-DEG2RAD(fMin)));
			}
			else if (motion.fRoll <= DEG2RAD(-fMin))
			{
				fLean = ((motion.fRoll-DEG2RAD(-fMin))/(-(-1.0f-DEG2RAD(-fMin))));
			}
			mr.SetLean(NEGSATURATE(fLean));
	
			pPlayer->GetMovementController()->RequestMovement(mr);
		}
	}
}

////////////////////////////////////////////////////
void SWiiInputListener::OnExtensionMotionStart(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion)
{
	if (NULL == pManager || false == pManager->IsMasterEnabled()) return;

	nExtensionEndLifetime = 0;
	for (MotionList::iterator itMotion = pManager->m_NunchukMotions.begin(); 
		itMotion != pManager->m_NunchukMotions.end(); itMotion++)
	{
		(*itMotion)->OnBegin(motion, nState);
	}
}

////////////////////////////////////////////////////
void SWiiInputListener::OnExtensionMotionUpdate(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion)
{
	if (NULL == pManager || false == pManager->IsMasterEnabled()) return;

	for (MotionList::iterator itMotion = pManager->m_NunchukMotions.begin(); 
		itMotion != pManager->m_NunchukMotions.end(); itMotion++)
	{
		(*itMotion)->OnUpdate(motion, nState);
	}
}

////////////////////////////////////////////////////
void SWiiInputListener::OnExtensionMotionEnd(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, SMotionElement const& motion)
{
	// Find which one fired here
	IMotion *pFiredMotion = NULL;
	IMotion *pMotionI = NULL;
	for (MotionList::iterator itMotion = pManager->m_NunchukMotions.begin(); 
		itMotion != pManager->m_NunchukMotions.end(); itMotion++)
	{
		pMotionI = *itMotion;
		pMotionI->OnEnd(motion, nState);
		if (pMotionI->IsFired() && (NULL == pFiredMotion || pMotionI->GetPriority() > pFiredMotion->GetPriority()))
			pFiredMotion = pMotionI;
	}
	
	if (NULL == pManager || false == pManager->IsMasterEnabled()) return;

	// Execute fired one
	if (NULL != pFiredMotion)
	{
		pFiredMotion->Execute();
		nExtensionEndLifetime = motion.nLifetime;
	}
}

////////////////////////////////////////////////////
void SWiiInputListener::OnExtensionAnalogUpdate(IWR_WiiRemote *pRemote, IWR_WiiExtension *pExtension, int nStickID, float fX, float fY)
{
	
}

////////////////////////////////////////////////////
void CWiiRemoteManager::UpdatePlayerMovement(void)
{
	if (false == IsMasterEnabled()) return;

	CPlayer *pPlayer = GetPlayer();
	if (NULL == pPlayer || NULL == m_pRemote || NULL == m_pRemote->GetExtensionHelper()) return;
	CWR_WiiNunchuk *pNunchuk = (CWR_WiiNunchuk*)m_pRemote->GetExtensionHelper();
	const CGameActions &rGameActions = m_pGame->Actions();

	// Analog stick positions
	float fX = pNunchuk->GetAnalogX();
	float fY = pNunchuk->GetAnalogY();

	// Player controls
	if (g_WiiInputListener.nState == STATE_PLAYER || g_WiiInputListener.nState == STATE_BINOCULARS)
	{
		CMovementRequest mr;

		// Apply dead zone
		if (fY >= 0.05f || fY <= -0.05f)
		{
			mr.AddDeltaMovement(Vec3(0,fY,0));
		}
		if (fX >= 0.05f || fX <= -0.05f)
		{
			mr.AddDeltaMovement(Vec3(fX,0,0));
		}
	
		pPlayer->GetMovementController()->RequestMovement(mr);
	}
	else if (g_WiiInputListener.nState == STATE_LANDVEHICLE)
	{
		SVehicleMovementAction action;

		// Apply dead zone
		if (fY >= 0.05f || fY <= -0.05f)
		{
			action.power = fY;
		}
		else
		{
			action.brake = true;
		}

		// Steer if stick is enabled
		if (true == CHECK_PROFILE_BOOL(Veh_UseStickSteer_Land))
		{
			if (fX >= 0.05f || fX <= -0.05f)
			{
				action.rotateYaw = fX;
			}
		}
		else
		{
			// Use roll
			float fRoll = ((CWR_WiiNunchuk*)m_pRemote->GetExtensionHelper())->GetRoll();
			const float fMin = CHECK_PROFILE_FLOAT(Veh_SteerTilt_Land);
			if (fabs(fRoll) >= DEG2RAD(fMin))
			{
				action.rotateYaw = CLAMP(fRoll*CHECK_PROFILE_FLOAT(Veh_SteerSensitivity_Land),-1.0f,1.0f);
			}
		}

		AddPlayerVehicleMovementAction(pPlayer, action);
	}
	else if (g_WiiInputListener.nState == STATE_SEAVEHICLE)
	{
		SVehicleMovementAction action;

		// Apply dead zone
		if (fY >= 0.05f || fY <= -0.05f)
		{
			action.power = fY;
		}
		else
		{
			action.brake = true;
		}

		// Steer if stick is enabled
		if (true == CHECK_PROFILE_BOOL(Veh_UseStickSteer_Sea))
		{
			if (fX >= 0.05f || fX <= -0.05f)
			{
				action.rotateYaw = fX;
			}
		}
		else
		{
			// Use roll
			float fRoll = ((CWR_WiiNunchuk*)m_pRemote->GetExtensionHelper())->GetRoll();
			const float fMin = CHECK_PROFILE_FLOAT(Veh_SteerTilt_Sea);
			if (fabs(fRoll) >= DEG2RAD(fMin))
			{
				action.rotateYaw = CLAMP(fRoll*CHECK_PROFILE_FLOAT(Veh_SteerSensitivity_Sea),-1.0f,1.0f);
			}
		}

		AddPlayerVehicleMovementAction(pPlayer, action);
	}
	else if (g_WiiInputListener.nState == STATE_VTOLVEHICLE)
	{
		CVehicleMovementVTOL *pVM = (CVehicleMovementVTOL*)pPlayer->GetLinkedVehicle()->GetMovement();
		bool bUseStickStrafe = CHECK_PROFILE_BOOL(VTOL_UseStickStrafe);

		// Forward/backwards movement
		if (fY >= 0.05f)
		{
			pVM->OnAction(eVAI_MoveBack, eAAM_OnRelease, 0.0f);
			pVM->OnAction(eVAI_MoveForward, eAAM_OnPress, fY);
		}
		else if (fY <= -0.05f)
		{
			pVM->OnAction(eVAI_MoveForward, eAAM_OnRelease, 0.0f);
			pVM->OnAction(eVAI_MoveBack, eAAM_OnPress, -fY);
		}
		else
		{
			pVM->OnAction(eVAI_MoveForward, eAAM_OnRelease, 0.0f);
			pVM->OnAction(eVAI_MoveBack, eAAM_OnRelease, 0.0f);
		}

		// Strafeing
		if (true == bUseStickStrafe)
		{
			if (fX >= 0.05f)
			{
				pVM->OnAction(eVAI_StrafeLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_StrafeRight, eAAM_OnPress, fX);
			}
			else if (fX <= -0.05f)
			{
				pVM->OnAction(eVAI_StrafeRight, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_StrafeLeft, eAAM_OnPress, -fX);
			}
			else
			{
				pVM->OnAction(eVAI_StrafeLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_StrafeRight, eAAM_OnRelease, 0.0f);
			}
		}
		else
		{
			float fRoll = ((CWR_WiiNunchuk*)m_pRemote->GetExtensionHelper())->GetRoll();
			const float fDZ = CHECK_PROFILE_FLOAT(VTOL_HTilt);
			if (fRoll >= DEG2RAD(fDZ))
			{
				float fSpeed = CLAMP((fRoll*(1.0f/DEG2RAD(90.0f))) * CHECK_PROFILE_FLOAT(VTOL_HSensitivity),0.0f,1.0f);
				pVM->OnAction(eVAI_StrafeLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_StrafeRight, eAAM_OnPress, fSpeed);
			}
			else if (fRoll <= DEG2RAD(fDZ))
			{
				float fSpeed = CLAMP((-fRoll*(1.0f/DEG2RAD(90.0f))) * CHECK_PROFILE_FLOAT(VTOL_HSensitivity),0.0f,1.0f);
				pVM->OnAction(eVAI_StrafeRight, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_StrafeLeft, eAAM_OnPress, fSpeed);
			}
			else
			{
				pVM->OnAction(eVAI_StrafeLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_StrafeRight, eAAM_OnRelease, 0.0f);
			}
		}

		// Rolling
		if (false == bUseStickStrafe)
		{
			if (fX >= 0.05f)
			{
				pVM->OnAction(eVAI_RollLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollRight, eAAM_OnPress, fX);
			}
			else if (fX <= -0.05f)
			{
				pVM->OnAction(eVAI_RollRight, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollLeft, eAAM_OnPress, -fX);
			}
			else
			{
				pVM->OnAction(eVAI_RollLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollRight, eAAM_OnRelease, 0.0f);
			}
		}
		else
		{
			float fRoll = ((CWR_WiiNunchuk*)m_pRemote->GetExtensionHelper())->GetRoll();
			const float fDZ = CHECK_PROFILE_FLOAT(VTOL_HTilt);
			if (fRoll >= DEG2RAD(fDZ))
			{
				float fSpeed = CLAMP((fRoll*(1.0f/DEG2RAD(90.0f))) * CHECK_PROFILE_FLOAT(VTOL_HSensitivity),0.0f,1.0f);
				pVM->OnAction(eVAI_RollLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollRight, eAAM_OnPress, fSpeed);
			}
			else if (fRoll <= DEG2RAD(fDZ))
			{
				float fSpeed = CLAMP((-fRoll*(1.0f/DEG2RAD(90.0f))) * CHECK_PROFILE_FLOAT(VTOL_HSensitivity),0.0f,1.0f);
				pVM->OnAction(eVAI_RollRight, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollLeft, eAAM_OnPress, fSpeed);
			}
			else
			{
				pVM->OnAction(eVAI_RollLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollRight, eAAM_OnRelease, 0.0f);
			}
		}

		// Check nunchuk pitch for vertical movement
		float fPitch = ((CWR_WiiNunchuk*)m_pRemote->GetExtensionHelper())->GetPitch();
		const float fDZ = CHECK_PROFILE_FLOAT(VTOL_VTilt);
		if (fPitch >= DEG2RAD(fDZ))
		{
			float fSpeed = CLAMP((fPitch*(1.0f/DEG2RAD(90.0f))) * CHECK_PROFILE_FLOAT(VTOL_VSensitivity),0.0f,1.0f);
			pVM->OnAction(eVAI_MoveDown, eAAM_OnRelease, 0.0f);
			pVM->OnAction(eVAI_MoveUp, eAAM_OnPress, fSpeed);
		}
		else if (fPitch <= DEG2RAD(-fDZ))
		{
			float fSpeed = CLAMP((-fPitch*(1.0f/DEG2RAD(90.0f))) * CHECK_PROFILE_FLOAT(VTOL_VSensitivity),0.0f,1.0f);
			pVM->OnAction(eVAI_MoveUp, eAAM_OnRelease, 0.0f);
			pVM->OnAction(eVAI_MoveDown, eAAM_OnPress, fSpeed);
		}
		else
		{
			pVM->OnAction(eVAI_MoveUp, eAAM_OnRelease, 0.0f);
			pVM->OnAction(eVAI_MoveDown, eAAM_OnRelease, 0.0f);
		}
	}
	else if (g_WiiInputListener.nState == STATE_HELIVEHICLE)
	{
		CVehicleMovementHelicopter *pVM = (CVehicleMovementHelicopter*)pPlayer->GetLinkedVehicle()->GetMovement();

		// Forward/backwards movement
		if (fY >= 0.05f)
		{
			pVM->OnAction(eVAI_MoveDown, eAAM_OnRelease, 0.0f);
			pVM->OnAction(eVAI_MoveUp, eAAM_OnPress, fY);
		}
		else if (fY <= -0.05f)
		{
			pVM->OnAction(eVAI_MoveUp, eAAM_OnRelease, 0.0f);
			pVM->OnAction(eVAI_MoveDown, eAAM_OnPress, -fY);
		}
		else
		{
			pVM->OnAction(eVAI_MoveUp, eAAM_OnRelease, 0.0f);
			pVM->OnAction(eVAI_MoveDown, eAAM_OnRelease, 0.0f);
		}

		// Rolling
		if (true == CHECK_PROFILE_BOOL(Heli_UseStickRoll))
		{
			if (fX >= 0.05f)
			{
				pVM->OnAction(eVAI_RollLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollRight, eAAM_OnPress, fX);
			}
			else if (fX <= -0.05f)
			{
				pVM->OnAction(eVAI_RollRight, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollLeft, eAAM_OnPress, -fX);
			}
			else
			{
				pVM->OnAction(eVAI_RollLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollRight, eAAM_OnRelease, 0.0f);
			}
		}
		else
		{
			float fRoll = ((CWR_WiiNunchuk*)m_pRemote->GetExtensionHelper())->GetRoll();
			const float fDZ = CHECK_PROFILE_FLOAT(Heli_RollTilt);
			if (fRoll >= DEG2RAD(fDZ))
			{
				float fSpeed = CLAMP((fRoll*(1.0f/DEG2RAD(90.0f))) * CHECK_PROFILE_FLOAT(Heli_RollSensitivity),0.0f,1.0f);
				pVM->OnAction(eVAI_RollLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollRight, eAAM_OnPress, fSpeed);
			}
			else if (fRoll <= DEG2RAD(fDZ))
			{
				float fSpeed = CLAMP((-fRoll*(1.0f/DEG2RAD(90.0f))) * CHECK_PROFILE_FLOAT(Heli_RollSensitivity),0.0f,1.0f);
				pVM->OnAction(eVAI_RollRight, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollLeft, eAAM_OnPress, fSpeed);
			}
			else
			{
				pVM->OnAction(eVAI_RollLeft, eAAM_OnRelease, 0.0f);
				pVM->OnAction(eVAI_RollRight, eAAM_OnRelease, 0.0f);
			}
		}
	}
	else if (CHUD *pHUD = m_pGame->GetHUD())
	{
		if (g_WiiInputListener.nState == STATE_NANOSUITMENU)
		{
			// Send in hud mouse actions
			if (fX >= 0.05f || fX <= -0.05f) pHUD->OnAction(rGameActions.hud_mousex, eIS_Pressed, fX*100.0f);
			if (fY >= 0.05f || fY <= -0.05f) pHUD->OnAction(rGameActions.hud_mousey, eIS_Pressed, fY*-100.0f);
		}
		else if (g_WiiInputListener.nState == STATE_WEAPONMENU && false == UsingIRSensor())
		{
			// Send in hardware mouse movements, HUD will pick up on it
			float fMX, fMY;
			gEnv->pSystem->GetIHardwareMouse()->GetHardwareMouseClientPosition(&fMX, &fMY);
			if (fX >= 0.05f || fX <= -0.05f) fMX += fX*50.0f;
			if (fY >= 0.05f || fY <= -0.05f) fMY += fY*-50.0f;
			gEnv->pSystem->GetIHardwareMouse()->SetHardwareMouseClientPosition(fMX, fMY);
		}
	}

	// If view is locked, update retain view mode
	if (true == m_bLockView && true == CHECK_PROFILE_BOOL(RetainViewMode))
	{
		CCamera PlayerCam = gEnv->pSystem->GetViewCamera();
		Matrix34 PlayerMat = PlayerCam.GetMatrix();
		IMovementController *pMC = pPlayer->GetMovementController();

		// Find entity to lock on to
		if ((NULL == m_pLockedEntity || false == m_bLockedNPC) && NULL != pMC && false == m_bForceLockView)
		{
			// See if we are looking at one now
			SMovementState info; pMC->GetMovementState(info);
			Vec3 vPlayerForward = info.fireDirection * CHECK_PROFILE_FLOAT(MaxLockViewDist);
			Vec3 vPlayerPos(PlayerCam.GetPosition());
			ray_hit actorHit, objectHit;
			IPhysicalEntity *pSkip[3] = {pPlayer->GetEntity()->GetPhysics(), NULL, NULL};
			if (pPlayer->GetLinkedVehicle())
				pSkip[1] = pPlayer->GetLinkedVehicle()->GetEntity()->GetPhysics();
			COffHand* pOffHand = static_cast<COffHand*>(pPlayer->GetWeaponByClass(CItem::sOffHandClass));
			if (NULL != pOffHand && pOffHand->GetOffHandState()&(eOHS_HOLDING_OBJECT|eOHS_HOLDING_NPC|eOHS_THROWING_OBJECT|eOHS_THROWING_NPC))
			{
				// Try to ignore the object picked up as well
				if (IEntity *pEnt = gEnv->pEntitySystem->GetEntity(pOffHand->GetHeldEntityId()))
					pSkip[2] = pEnt->GetPhysics();
			}

			// Perform a trace on actors and on objects
			int nActorHit = gEnv->pPhysicalWorld->RayWorldIntersection(vPlayerPos, vPlayerForward, (ent_independent), 
				(rwi_stop_at_pierceable|rwi_colltype_any|rwi_ignore_back_faces), &actorHit, 1, pSkip, 3);
			int nObjectHit = 0;
			if (NULL == m_pLockedEntity)
				nObjectHit = gEnv->pPhysicalWorld->RayWorldIntersection(vPlayerPos, vPlayerForward, (ent_static|ent_rigid|ent_sleeping_rigid|ent_living), 
					(rwi_stop_at_pierceable|rwi_colltype_any|rwi_ignore_back_faces), &objectHit, 1, pSkip, 3);

			// Try to take the actor over the object
			ray_hit *pHit = ((nActorHit && actorHit.pCollider) ? &actorHit : ((nObjectHit && objectHit.pCollider) ? &objectHit : NULL));
			if (NULL != pHit)
			{
				if (IEntity *pEnt = (IEntity*)pHit->pCollider->GetForeignData(PHYS_FOREIGN_ID_ENTITY))
				{
					if (IEntityRenderProxy *pRenderProxy = (IEntityRenderProxy*)pEnt->GetProxy(ENTITY_PROXY_RENDER))
						m_pLockedEntity = pRenderProxy->GetRenderNode();
				}
				else if (IRenderNode *pRndNode = (IRenderNode*)pHit->pCollider->GetForeignData(PHYS_FOREIGN_ID_STATIC))
				{
					m_pLockedEntity = pRndNode;
				}
				if (NULL != m_pLockedEntity)
				{
					// Set offset point to maintain hit point when using only remote
					//if (false == UsingIRSensor())
					{
						AABB aabb = m_pLockedEntity->GetBBox();
						Vec3 vPos = aabb.GetCenter()+Vec3(0.f,0.f,(aabb.max.z-aabb.min.z)*0.25f);
						if (pHit == &objectHit)
						{
							// Use offset from hit point to position
							m_bLockedNPC = false;
							m_vLockedEntityOffset = pHit->pt - vPos;
						}
						else
						{
							// Use 0-based offset (to retain center position)
							m_bLockedNPC = true;
							m_vLockedEntityOffset.Set(0,0,0);
						}
					}
				}
			}
		}
		if (NULL != m_pLockedEntity)
		{
			// If it has been picked up by us, drop focus on it
			COffHand* pOffHand = static_cast<COffHand*>(pPlayer->GetWeaponByClass(CItem::sOffHandClass));
			if (NULL != pOffHand /*&& pOffHand->GetOffHandState()&(eOHS_HOLDING_OBJECT|eOHS_HOLDING_NPC|eOHS_THROWING_OBJECT|eOHS_THROWING_NPC)*/)
			{
				if (IEntity *pHeldEntity = gEnv->pEntitySystem->GetEntity(pOffHand->GetHeldEntityId()))
					if (IEntityRenderProxy *pRenderProxy = (IEntityRenderProxy*)pHeldEntity->GetProxy(ENTITY_PROXY_RENDER))
						if (m_pLockedEntity == pRenderProxy->GetRenderNode())
						{
							// Same. Drop it
							m_pLockedEntity = NULL;
							m_bLockedNPC = false;

							// Enter force lock to retain lock view
							m_bForceLockView = true;
						}
			}
		}
		if (NULL != m_pLockedEntity && NULL == pPlayer->GetLinkedVehicle())
		{
			AABB aabb = m_pLockedEntity->GetBBox();
			Vec3 vPos = aabb.GetCenter()+Vec3(0.f,0.f,(aabb.max.z-aabb.min.z)*0.25f);

			// Look at its point plus move offset
			Vec3 vLookAtPt = vPos + m_vLockedEntityOffset;
			Vec3 vForward = vLookAtPt-PlayerCam.GetPosition(); vForward.Normalize();
			Vec3 vRight = vForward.Cross(Vec3(0,0,1));
			PlayerMat.SetFromVectors(vRight, vForward, Vec3(0,0,0), pPlayer->GetEntity()->GetPos());
			PlayerMat.OrthonormalizeFast();
			pPlayer->GetEntity()->SetLocalTM(PlayerMat);
		}
	}
}

////////////////////////////////////////////////////
IRenderNode* CWiiRemoteManager::GetLockedEntity(Vec3 &vOffset) const
{
	vOffset = m_vLockedEntityOffset;
	return m_pLockedEntity;
}

////////////////////////////////////////////////////
bool CWiiRemoteManager::IsButtonDown(int nButton) const
{
	if (nButton == WIIBTN_C || nButton == WIIBTN_Z)
	{
		CWR_WiiNunchuk *pNunchuk = (CWR_WiiNunchuk*)m_pRemote->GetExtensionHelper();
		return (pNunchuk?pNunchuk->IsButtonDown(g_WiiRemoteProfile->TranslateButton((EWiiButton)nButton)):false);
	}
	return m_pRemote->GetButtonHelper()->IsButtonDown(g_WiiRemoteProfile->TranslateButton((EWiiButton)nButton));
}

////////////////////////////////////////////////////
void SWiiInputListener::OnEnterScreen(IWR_WiiRemote *pRemote, IWR_WiiSensor *pSensor, float fX, float fY)
{
	CHUD *pHUD;
	CHUDCrosshair *pCH;
	if (NULL == pManager->m_pGame || NULL == (pHUD = pManager->m_pGame->GetHUD()) || NULL == (pCH = pHUD->GetCrosshair())) return;
	
	pCH->IRSetOnScreen(true);
}

////////////////////////////////////////////////////
void SWiiInputListener::OnLeaveScreen(IWR_WiiRemote *pRemote, IWR_WiiSensor *pSensor, float fX, float fY)
{
	CHUD *pHUD;
	CHUDCrosshair *pCH;
	if (NULL == pManager->m_pGame || NULL == (pHUD = pManager->m_pGame->GetHUD()) || NULL == (pCH = pHUD->GetCrosshair())) return;
	
	pCH->IRSetOnScreen(false);
}

////////////////////////////////////////////////////
void SWiiInputListener::OnCursorUpdate(IWR_WiiRemote *pRemote, IWR_WiiSensor *pSensor, float fX, float fY)
{
	CHUD *pHUD;
	CHUDCrosshair *pCH;
	if (NULL == pManager->m_pGame || NULL == (pHUD = pManager->m_pGame->GetHUD()) || NULL == (pCH = pHUD->GetCrosshair())) return;
	const CGameActions &rGameActions = pManager->m_pGame->Actions();

	// Ignore if disabled
	if (false == pCH->IRIsEnabled()) return;

	// Ignore if a motion is alive
	if (true == bInGesture)
	{
		// Have we dropped out?
		if (pRemote->GetMotionHelper()->GetMotionLifetime() == 0)
			bInGesture = false;
		return;
	}
	else if (pRemote->GetMotionHelper()->GetMotionLifetime() > 0)
	{
		// In a gesture
		bInGesture = true;
		return;
	}

	CPlayer *pPlayer = GetPlayer();
	if (NULL == pPlayer) return;
	
	if (nState == STATE_PLAYER || nState == STATE_BINOCULARS || nState == STATE_LANDVEHICLE || nState == STATE_SEAVEHICLE ||
		nState == STATE_VTOLVEHICLE || nState == STATE_HELIVEHICLE)
	{
		bool bInLockRetain = (true == pManager->m_bLockView && true == CHECK_PROFILE_BOOL(RetainViewMode));
		if (true == pManager->m_bForceLockView || true == pManager->IsMovementFrozen() || 
			(true == bInLockRetain && NULL != pManager->m_pLockedEntity))
		{
			// If Z button is held down, force it to the center
			if (CWR_WiiNunchuk *pN = (CWR_WiiNunchuk*)pRemote->GetExtensionHelper())
			{
				if (true == pN->IsButtonDown(WR_NCBUTTON_Z))
				{
					pCH->IRSetCursorPos(0.5f,0.5f);
					return;
				}
			}
		}

		// Set the cursor position
		pCH->IRSetCursorPos(fX,fY);

		if (false == pManager->m_bForceLockView && false == pManager->IsMovementFrozen())
		{
			bool bInLockRetain = (true == pManager->m_bLockView && true == CHECK_PROFILE_BOOL(RetainViewMode));
			if ((false == pManager->m_bLockView) || (true == bInLockRetain && NULL == pManager->m_pLockedEntity))
			{
				float fTurn = 0.0f, fLookUp = 0.0f;

				float fTurnSensitivity = CHECK_PROFILE_FLOAT(IRSensor_TurnSensitivity);
				float fLookUpSensitivity = CHECK_PROFILE_FLOAT(IRSensor_LookUpSensitivity);
				bool bInverseLook = CHECK_PROFILE_BOOL(InverseLook);
				if (nState == STATE_LANDVEHICLE || nState == STATE_SEAVEHICLE)
				{
					fTurnSensitivity = CHECK_PROFILE_FLOAT(Veh_IRSensor_TurnSensitivity);
					fLookUpSensitivity = CHECK_PROFILE_FLOAT(Veh_IRSensor_LookUpSensitivity);
					bInverseLook = CHECK_PROFILE_BOOL(Veh_InverseLook);
				}
				else if (nState == STATE_VTOLVEHICLE)
				{
					fTurnSensitivity = CHECK_PROFILE_FLOAT(VTOL_IRSensor_TurnSensitivity);
					fLookUpSensitivity = CHECK_PROFILE_FLOAT(VTOL_IRSensor_PitchSensitivity);
					bInverseLook = CHECK_PROFILE_BOOL(VTOL_InverseLook);
				}
				else if (nState == STATE_HELIVEHICLE)
				{
					fTurnSensitivity = CHECK_PROFILE_FLOAT(Heli_IRSensor_TurnSensitivity);
					fLookUpSensitivity = CHECK_PROFILE_FLOAT(Heli_IRSensor_PitchSensitivity);
					bInverseLook = CHECK_PROFILE_BOOL(Heli_InverseLook);
				}

				// Check to see if we need to turn
				if (fX < WII_IRSENSOR_DEADZONE_LEFT)
				{
					float fRatio = (WII_IRSENSOR_DEADZONE_LEFT-fX)/(WII_IRSENSOR_DEADZONE_LEFT);
					fTurn = fRatio*fTurnSensitivity;
				}
				else if (fX > WII_IRSENSOR_DEADZONE_RIGHT)
				{
					float fRatio = (fX-WII_IRSENSOR_DEADZONE_RIGHT)/(1.0f-WII_IRSENSOR_DEADZONE_RIGHT);
					fTurn = -fRatio*fTurnSensitivity;
				}

				// Get angle for look based on cursor point from origin, applied by deadzone
				if (nState != STATE_VTOLVEHICLE && nState != STATE_HELIVEHICLE)
				{
					float fMotionPitch = 0.0f;
					static bool bWasInZone = false;
					if (fY < WII_IRSENSOR_DEADZONE_UP)
					{
						bWasInZone = true;
						float fRatio = (WII_IRSENSOR_DEADZONE_UP-fY)/(WII_IRSENSOR_DEADZONE_UP);
						fMotionPitch = 90.0f * fRatio;
					}
					else if (fY > WII_IRSENSOR_DEADZONE_DOWN)
					{
						bWasInZone = true;
						float fRatio = (fY-WII_IRSENSOR_DEADZONE_DOWN)/(1.0f-WII_IRSENSOR_DEADZONE_DOWN);
						fMotionPitch = -90.0f * fRatio;
					}
					
					// Look up/down
					if (fMotionPitch || bWasInZone)
					{
						// Calculat player pitch
						SMovementState info;
						pPlayer->GetMovementController()->GetMovementState(info);
						Vec3 vForward = info.aimDirection;
						Vec3 vPlanarForward(vForward.x,vForward.y,0.0f);
						const float fPitch = acosf(vForward.Dot(vPlanarForward)) * (vForward.z < 0.0f ? -1.0f : 1.0f);

						// If the pitch is close to home, we're done
						if (RAD2DEG(fPitch) >= -CHECK_PROFILE_FLOAT(LookUpError) &&
							RAD2DEG(fPitch) <= CHECK_PROFILE_FLOAT(LookUpError))
						{
							bWasInZone = false;
						}
						else
						{
							// Calculate delta and move towards it at sensitivity speed
							const float fDelta = DEG2RAD(fMotionPitch) * (bInverseLook == true ? -1.0f : 1.0f) - fPitch;
							if (RAD2DEG(fabs(fDelta)) >= CHECK_PROFILE_FLOAT(LookUpError))
							{
								fLookUp = fDelta * fLookUpSensitivity;
							}
						}
					}
				}
				else
				{
					// Check to see if we need to adjust pitch
					if (fY < WII_IRSENSOR_DEADZONE_UP)
					{
						float fRatio = (WII_IRSENSOR_DEADZONE_UP-fY)/(WII_IRSENSOR_DEADZONE_UP);
						fLookUp = fRatio*fLookUpSensitivity;
					}
					else if (fY > WII_IRSENSOR_DEADZONE_DOWN)
					{
						float fRatio = (fY-WII_IRSENSOR_DEADZONE_DOWN)/(1.0f-WII_IRSENSOR_DEADZONE_DOWN);
						fLookUp = -fRatio*fLookUpSensitivity;
					}
				}
				
				// Request rotation
				if (nState == STATE_PLAYER || nState == STATE_BINOCULARS)
				{
					CMovementRequest mr;
					mr.AddDeltaRotation(Ang3(fLookUp,0.0f,fTurn));
					pPlayer->GetMovementController()->RequestMovement(mr);
				}
				else if (nState == STATE_LANDVEHICLE || nState == STATE_SEAVEHICLE)
				{
					// Vehicle - Alter accordingly
					// Get vehicle
					IVehicle* pVehicle = NULL;
					IPlayerInput *pPlayerInput = NULL;
					if (pVehicle = pPlayer->GetLinkedVehicle())
					{
						// Send it through actions
						pPlayerInput = pPlayer->GetPlayerInput();
					}
					if (NULL != pPlayerInput)
					{
						// Scale if in third person view or if gunner
						IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId());
						if (pSeat)
						{
							IVehicleView *pView = pSeat->GetView(pSeat->GetCurrentView());
							if (pView && pView->IsThirdPerson())
							{
								fLookUp *= 32.0f*gEnv->pTimer->GetFrameTime();
								fTurn *= 16.0f*gEnv->pTimer->GetFrameTime();
							}
							else if (pSeat->IsGunner() && !pSeat->IsDriver())
							{
								fLookUp *= 32.0f*gEnv->pTimer->GetFrameTime();
								fTurn *= 16.0f*gEnv->pTimer->GetFrameTime();
							}
						}

						// Look around
						pPlayerInput->OnAction(rGameActions.v_rotatepitch, eAAM_Always, -fLookUp);
						pPlayerInput->OnAction(rGameActions.v_rotateyaw, eAAM_Always, -fTurn);
					}
				}
				else
				{
					IVehicle* pVehicle = NULL;
					IPlayerInput *pPlayerInput = NULL;
					if (pVehicle = pPlayer->GetLinkedVehicle())
					{
						// Send it through actions
						pPlayerInput = pPlayer->GetPlayerInput();
					}
					if (NULL != pPlayerInput)
					{
						// Scale if in third person view or if gunner
						bool bIsGunner = false;
						IVehicleSeat *pSeat = pVehicle->GetSeatForPassenger(pPlayer->GetEntityId());
						if (pSeat)
						{
							bIsGunner = (pSeat->IsGunner() && !pSeat->IsDriver());

							IVehicleView *pView = pSeat->GetView(pSeat->GetCurrentView());
							if (pView && pView->IsThirdPerson())
							{
								fLookUp *= 32.0f*gEnv->pTimer->GetFrameTime();
								fTurn *= 16.0f*gEnv->pTimer->GetFrameTime();
							}
							else if (true == bIsGunner)
							{
								fLookUp *= 32.0f*gEnv->pTimer->GetFrameTime();
								fTurn *= 32.0f*gEnv->pTimer->GetFrameTime();
							}
						}

						// Apply through movement controller
						if (false == bIsGunner)
						{
							CVehicleMovementHelicopter *pVM = (CVehicleMovementHelicopter*)pVehicle->GetMovement();
							pVM->OnAction(eVAI_RotatePitch, eAAM_Always, -fLookUp);
							pVM->OnAction(eVAI_RotateYaw, eAAM_Always, -fTurn);
						}
						else
						{
							pPlayerInput->OnAction(rGameActions.v_rotatepitch, eAAM_Always, -fLookUp);
							pPlayerInput->OnAction(rGameActions.v_rotateyaw, eAAM_Always, -fTurn);
						}
					}
				}
			}
		}
	}
	else if (CHUD *pHUD = pManager->m_pGame->GetHUD())
	{
		if (nState == STATE_WEAPONMENU && true == UsingIRSensor())
		{
			// Send in hardware mouse movements, HUD will pick up on it
			int x0,y0,width,height;
			float aspectRatio;
			pHUD->GetWeaponMenu()->GetFlashPlayer()->GetViewport(x0,y0,width,height,aspectRatio);
			float fCFX = (float)width*fX;
			float fCFY = (float)height*fY;
			gEnv->pSystem->GetIHardwareMouse()->SetHardwareMouseClientPosition(x0+fCFX,y0+fCFY);
		}
	}
}