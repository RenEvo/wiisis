////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// CWiiRemoteProfile.cpp
//
// Purpose: Profile for the Wii Remote including
//	key bindings and sensitivity settings
//
// History:
//	- 11/15/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core\WR_Implementation.h"
#include "CWiiRemoteProfile.h"

#define PROFILE_NAME ("%USER%/wiiremote.xml")

CWiiRemoteProfile CWiiRemoteProfile::m_Instance;
CWiiRemoteProfile* g_WiiRemoteProfile = CWiiRemoteProfile::GetInstance();

////////////////////////////////////////////////////
CWiiRemoteProfile::CWiiRemoteProfile(void)
{
	// Create variables
	for (int i = 0; i < EPROFILEVARIABLES_COUNT; i++)
	{
		m_Variables[i] = new CWiiRemoteProfileVariable;
	}
}

////////////////////////////////////////////////////
CWiiRemoteProfile::~CWiiRemoteProfile(void)
{
	// Save configuration before exiting
	if (false == SaveCFG())
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to save Wiisis Remote Configuration");

	// Delete variables
	for (int i = 0; i < EPROFILEVARIABLES_COUNT; i++)
	{
		SAFE_DELETE(m_Variables[i]);
	}
}

////////////////////////////////////////////////////
CWiiRemoteProfile* CWiiRemoteProfile::GetInstance(void)
{
	return &m_Instance;
}

////////////////////////////////////////////////////
IWiiRemoteProfileVariable* CWiiRemoteProfile::GetVariable(char const* szName, char const* szGroup)
{
	for (int i = 0; i < EPROFILEVARIABLES_COUNT; i++)
	{
		if (stricmp(szName, m_Variables[i]->GetName()) == 0 &&
			stricmp(szGroup, m_Variables[i]->GetGroup()) == 0)
		{
			return m_Variables[i];
		}
	}
	return NULL;
}

////////////////////////////////////////////////////
IWiiRemoteProfileVariable const* CWiiRemoteProfile::GetVariable(char const* szName, char const* szGroup) const
{
	for (int i = 0; i < EPROFILEVARIABLES_COUNT; i++)
	{
		if (stricmp(szName, m_Variables[i]->GetName()) == 0 &&
			stricmp(szGroup, m_Variables[i]->GetGroup()) == 0)
		{
			return m_Variables[i];
		}
	}
	return NULL;
}

////////////////////////////////////////////////////
IWiiRemoteProfileVariable* CWiiRemoteProfile::GetVariable(int nIndex)
{
	if (nIndex < 0 || nIndex >= EPROFILEVARIABLES_COUNT)
		return NULL;
	return m_Variables[nIndex];
}

////////////////////////////////////////////////////
IWiiRemoteProfileVariable const* CWiiRemoteProfile::GetVariable(int nIndex) const
{
	if (nIndex < 0 || nIndex >= EPROFILEVARIABLES_COUNT)
		return NULL;
	return m_Variables[nIndex];
}

////////////////////////////////////////////////////
void CWiiRemoteProfile::SetToDefault(void)
{
	// Options
	m_Variables[RetainViewMode]->Initialize("RetainViewMode", "Options", true);
	m_Variables[HardLockView]->Initialize("HardLockView", "Options", true);
	m_Variables[UseIRSensor]->Initialize("UseIRSensor", "Options", false);
	m_Variables[ShowIRDot]->Initialize("ShowIRDot", "Options", true);
	m_Variables[IRDotOpacity]->Initialize("IRDotOpacity", "Options", 0.8f);
	m_Variables[IRDotOpacity]->SetRange(0.0f, 1.0f);
	m_Variables[MaxLockViewDist]->Initialize("MaxLockViewDist", "Options", 250.0f);
	m_Variables[MaxLockViewDist]->SetRange(1.0f);

	// Player controls
	m_Variables[JumpSensitivity]->Initialize("JumpSensitivity", "Player", 2.0f);
	m_Variables[JumpSensitivity]->SetRange(1.0f);
	m_Variables[CrouchTilt]->Initialize("CrouchTilt", "Player", 30.0f);
	m_Variables[CrouchTilt]->SetRange(0.0f, 90.0f);
	m_Variables[ProneTilt]->Initialize("ProneTilt", "Player", 60.0f);
	m_Variables[ProneTilt]->SetRange(0.0f, 90.0f);
	m_Variables[LeanTilt]->Initialize("LeanTilt", "Player", 45.0f);
	m_Variables[LeanTilt]->SetRange(0.0f, 90.0f);
	m_Variables[PunchSensitivity]->Initialize("PunchSensitivity", "Player", 2.0f);
	m_Variables[PunchSensitivity]->SetRange(1.0f);
	m_Variables[LookUpSensitivity]->Initialize("LookUpSensitivity", "Player", 0.01f);
	m_Variables[LookUpSensitivity]->SetRange(0.00001f, 1.0f);
	m_Variables[LookUpError]->Initialize("LookUpError", "Player", 1.5f);
	m_Variables[LookUpError]->SetRange(0.0f);
	m_Variables[LookUpMaxTilt]->Initialize("LookUpMaxTilt", "Player", 60.0f);
	m_Variables[LookUpMaxTilt]->SetRange(10.0f, 90.0f);
	m_Variables[TurnSensitivity]->Initialize("TurnSensitivity", "Player", 0.015f);
	m_Variables[TurnSensitivity]->SetRange(0.00001f);
	m_Variables[TurnTilt]->Initialize("TurnTilt", "Player", 10.0f);
	m_Variables[TurnTilt]->SetRange(0.0f, 90.0f);
	m_Variables[ZoomInSensitivity]->Initialize("ZoomInSensitivity", "Player", 1.5f);
	m_Variables[ZoomInSensitivity]->SetRange(1.0f);
	m_Variables[ZoomOutSensitivity]->Initialize("ZoomOutSensitivity", "Player", 1.5f);
	m_Variables[ZoomOutSensitivity]->SetRange(1.0f);
	m_Variables[HolsterItemSensitivity]->Initialize("HolsterItemSensitivity", "Player", 2.0f);
	m_Variables[HolsterItemSensitivity]->SetRange(2.0f);
	m_Variables[MeleeSensitivity]->Initialize("MeleeSensitivity", "Player", 2.0f);
	m_Variables[MeleeSensitivity]->SetRange(1.0f);
	m_Variables[PickUpSensitivity]->Initialize("PickUpSensitivity", "Player", 3.0f);
	m_Variables[PickUpSensitivity]->SetRange(1.0f);
	m_Variables[ThrowSensitivity]->Initialize("ThrowSensitivity", "Player", 3.0f);
	m_Variables[ThrowSensitivity]->SetRange(1.0f);
	m_Variables[DropSensitivity]->Initialize("DropSensitivity", "Player", 2.0f);
	m_Variables[DropSensitivity]->SetRange(1.0f);

	m_Variables[IRSensor_TurnSensitivity]->Initialize("IRSensor_TurnSensitivity", "Player", 0.1875f);
	m_Variables[IRSensor_TurnSensitivity]->SetRange(0.00001f);
	m_Variables[IRSensor_LookUpSensitivity]->Initialize("IRSensor_LookUpSensitivity", "Player", 0.1875f);
	m_Variables[IRSensor_LookUpSensitivity]->SetRange(0.00001f);

	m_Variables[SoftLock_LookUpSensitivity]->Initialize("SoftLock_LookUpSensitivity", "Player", 0.05f);
	m_Variables[SoftLock_LookUpSensitivity]->SetRange(0.00001f, 1.0f);
	m_Variables[SoftLock_TurnSensitivity]->Initialize("SoftLock_TurnSensitivity", "Player", 0.075f);
	m_Variables[SoftLock_TurnSensitivity]->SetRange(0.00001f);

	m_Variables[InverseLook]->Initialize("InverseLook", "Player", false);

	// Button mapping
	m_Variables[SprintButton]->Initialize("Button_Sprint", "Player", WIIBTN_C);
	m_Variables[NanoSuitMenuButton]->Initialize("Button_NanoSuitMenu", "Player", WIIBTN_Z);
	m_Variables[WeaponMenuButton]->Initialize("Button_WeaponMenu", "Player", WIIBTN_DPAD_UP);
	m_Variables[PrevWeaponButton]->Initialize("Button_PrevWeapon", "Player", WIIBTN_MINUS);
	m_Variables[NextWeaponButton]->Initialize("Button_NextWeapon", "Player", WIIBTN_PLUS);
	m_Variables[LockViewButton]->Initialize("Button_LockView", "Player", WIIBTN_A);
	m_Variables[FireButton]->Initialize("Button_Fire", "Player", WIIBTN_B);
	m_Variables[ReloadButton]->Initialize("Button_Reload", "Player", WIIBTN_HOME);
	m_Variables[BinocularsButton]->Initialize("Button_Binoculars", "Player", WIIBTN_DPAD_LEFT);
	m_Variables[NightVisionButton]->Initialize("Button_NightVision", "Player", WIIBTN_DPAD_RIGHT);
	m_Variables[ZoomInButton]->Initialize("Button_ZoomIn", "Player", WIIBTN_PLUS);
	m_Variables[ZoomOutButton]->Initialize("Button_ZoomOut", "Player", WIIBTN_MINUS);
	m_Variables[ToggleFiremodeButton]->Initialize("Button_ToggleFiremode", "Player", WIIBTN_1);
	m_Variables[ToggleGrenadeButton]->Initialize("Button_ToggleGrenade3", "Player", WIIBTN_2);
	m_Variables[UseButton]->Initialize("Button_Use", "Player", WIIBTN_DPAD_DOWN);

	// Vehicle sensitivity
	m_Variables[Veh_LookUpSensitivity]->Initialize("LookUpSensitivity", "Vehicle", 1.0f);
	m_Variables[Veh_LookUpSensitivity]->SetRange(0.001f, 100.0f);
	m_Variables[Veh_LookUpError]->Initialize("LookUpError", "Vehicle", 1.5f);
	m_Variables[Veh_LookUpError]->SetRange(0.0f);
	m_Variables[Veh_LookUpMaxTilt]->Initialize("LookUpMaxTilt", "Vehicle", 60.0f);
	m_Variables[Veh_LookUpMaxTilt]->SetRange(10.0f, 90.0f);
	m_Variables[Veh_TurnSensitivity]->Initialize("TurnSensitivity", "Vehicle", 5.0f);
	m_Variables[Veh_TurnSensitivity]->SetRange(1.0f);
	m_Variables[Veh_TurnTilt]->Initialize("TurnTilt", "Vehicle", 10.0f);
	m_Variables[Veh_TurnTilt]->SetRange(0.0f, 90.0f);
	m_Variables[Veh_SeatPrevSensitivity]->Initialize("SeatPrevSensitivity", "Vehicle", 1.5f);
	m_Variables[Veh_SeatPrevSensitivity]->SetRange(1.0f);
	m_Variables[Veh_SeatNextSensitivity]->Initialize("SeatNextSensitivity", "Vehicle", 1.5f);
	m_Variables[Veh_SeatNextSensitivity]->SetRange(1.0f);

	m_Variables[Veh_IRSensor_TurnSensitivity]->Initialize("IRSensor_TurnSensitivity", "Vehicle", 62.5f);
	m_Variables[Veh_IRSensor_TurnSensitivity]->SetRange(1.0f);
	m_Variables[Veh_IRSensor_LookUpSensitivity]->Initialize("IRSensor_LookUpSensitivity", "Vehicle", 62.5f);
	m_Variables[Veh_IRSensor_LookUpSensitivity]->SetRange(1.0f);

	m_Variables[Veh_UseStickSteer_Land]->Initialize("UseStickSteer_Land", "Vehicle", false);
	m_Variables[Veh_SteerTilt_Land]->Initialize("SteerTilt_Land", "Vehicle", 10.0f);
	m_Variables[Veh_SteerTilt_Land]->SetRange(0.0f, 90.0f);
	m_Variables[Veh_SteerSensitivity_Land]->Initialize("SteerSensitivity_Land", "Vehicle", 0.35f);
	m_Variables[Veh_SteerSensitivity_Land]->SetRange(0.001f);

	m_Variables[Veh_UseStickSteer_Sea]->Initialize("UseStickSteer_Sea", "Vehicle", false);
	m_Variables[Veh_SteerTilt_Sea]->Initialize("SteerTilt_Sea", "Vehicle", 10.0f);
	m_Variables[Veh_SteerTilt_Sea]->SetRange(0.0f, 90.0f);
	m_Variables[Veh_SteerSensitivity_Sea]->Initialize("SteerSensitivity_Sea", "Vehicle", 0.75f);
	m_Variables[Veh_SteerSensitivity_Sea]->SetRange(0.001f);

	m_Variables[Veh_InverseLook]->Initialize("InverseLook", "Vehicle", false);

	// Vehicle button mapping
	m_Variables[Veh_UseButton]->Initialize("Button_Use", "Vehicle", WIIBTN_DPAD_DOWN);
	m_Variables[Veh_HornButton]->Initialize("Button_Horn", "Vehicle", WIIBTN_DPAD_RIGHT);
	m_Variables[Veh_LightsButton]->Initialize("Button_Lights", "Vehicle", WIIBTN_2);
	m_Variables[Veh_PrevSeatButton]->Initialize("Button_PrevSeat", "Vehicle", WIIBTN_MINUS);
	m_Variables[Veh_NextSeatButton]->Initialize("Button_NextSeat", "Vehicle", WIIBTN_PLUS);
	m_Variables[Veh_BoostButton]->Initialize("Button_Boost", "Vehicle", WIIBTN_C);
	m_Variables[Veh_FireButton]->Initialize("Button_Fire", "Vehicle", WIIBTN_B);
	m_Variables[Veh_ReloadButton]->Initialize("Button_Reload", "Vehicle", WIIBTN_HOME);
	m_Variables[Veh_ViewButton]->Initialize("Button_View", "Vehicle", WIIBTN_1);

	// Helicopter sensitivity
	m_Variables[Heli_PitchSensitivity]->Initialize("PitchSensitivity", "Helicopter", 10.0f);
	m_Variables[Heli_PitchSensitivity]->SetRange(0.001f, 100.0f);
	m_Variables[Heli_PitchTilt]->Initialize("PitchTilt", "Helicopter", 10.0f);
	m_Variables[Heli_PitchTilt]->SetRange(0.0f, 90.0f);
	m_Variables[Heli_TurnSensitivity]->Initialize("TurnSensitivity", "Helicopter", 2.5f);
	m_Variables[Heli_TurnSensitivity]->SetRange(1.0f);
	m_Variables[Heli_TurnTilt]->Initialize("TurnTilt", "Helicopter", 10.0f);
	m_Variables[Heli_TurnTilt]->SetRange(0.0f, 90.0f); 
	m_Variables[Heli_IRSensor_PitchSensitivity]->Initialize("IRSensor_PitchSensitivity", "Helicopter", 30.0f);
	m_Variables[Heli_IRSensor_PitchSensitivity]->SetRange(1.0f);
	m_Variables[Heli_IRSensor_TurnSensitivity]->Initialize("IRSensor_TurnSensitivity", "Helicopter", 30.0f);
	m_Variables[Heli_IRSensor_TurnSensitivity]->SetRange(1.0f);
	m_Variables[Heli_RollSensitivity]->Initialize("RollSensitivity", "Helicopter", 2.0f);
	m_Variables[Heli_RollSensitivity]->SetRange(0.001f, 100.0f);
	m_Variables[Heli_RollTilt]->Initialize("RollTilt", "Helicopter", 10.0f);
	m_Variables[Heli_RollTilt]->SetRange(0.0f, 90.0f);
	m_Variables[Heli_SeatPrevSensitivity]->Initialize("SeatPrevSensitivity", "Helicopter", 1.5f);
	m_Variables[Heli_SeatPrevSensitivity]->SetRange(1.0f);
	m_Variables[Heli_SeatNextSensitivity]->Initialize("SeatNextSensitivity", "Helicopter", 1.5f);
	m_Variables[Heli_SeatNextSensitivity]->SetRange(1.0f);
	
	m_Variables[Heli_InverseLook]->Initialize("InverseLook", "Helicopter", false);
	m_Variables[Heli_UseStickRoll]->Initialize("UseStickRoll", "Helicopter", false);
	
	// Helicopter button mapping
	m_Variables[Heli_UseButton]->Initialize("Button_Use", "Helicopter", WIIBTN_DPAD_DOWN);
	m_Variables[Heli_BoostButton]->Initialize("Button_Boost", "Helicopter", WIIBTN_C);
	m_Variables[Heli_PrevSeatButton]->Initialize("Button_PrevSeat", "Helicopter", WIIBTN_MINUS);
	m_Variables[Heli_NextSeatButton]->Initialize("Button_NextSeat", "Helicopter", WIIBTN_PLUS);
	m_Variables[Heli_FireGunButton]->Initialize("Button_FireGun", "Helicopter", WIIBTN_B);
	m_Variables[Heli_FireRocketButton]->Initialize("Button_FireRocket", "Helicopter", WIIBTN_HOME);
	m_Variables[Heli_ViewButton]->Initialize("Button_View", "Helicopter", WIIBTN_1);

	// VTOL sensitivity
	m_Variables[VTOL_PitchSensitivity]->Initialize("PitchSensitivity", "VTOL", 10.0f);
	m_Variables[VTOL_PitchSensitivity]->SetRange(0.001f, 100.0f);
	m_Variables[VTOL_PitchTilt]->Initialize("PitchTilt", "VTOL", 10.0f);
	m_Variables[VTOL_PitchTilt]->SetRange(0.0f, 90.0f);
	m_Variables[VTOL_TurnSensitivity]->Initialize("TurnSensitivity", "VTOL", 2.5f);
	m_Variables[VTOL_TurnSensitivity]->SetRange(1.0f);
	m_Variables[VTOL_TurnTilt]->Initialize("TurnTilt", "VTOL", 10.0f);
	m_Variables[VTOL_TurnTilt]->SetRange(0.0f, 90.0f); 
	m_Variables[VTOL_IRSensor_PitchSensitivity]->Initialize("IRSensor_PitchSensitivity", "VTOL", 30.0f);
	m_Variables[VTOL_IRSensor_PitchSensitivity]->SetRange(1.0f);
	m_Variables[VTOL_IRSensor_TurnSensitivity]->Initialize("IRSensor_TurnSensitivity", "VTOL", 30.0f);
	m_Variables[VTOL_IRSensor_TurnSensitivity]->SetRange(1.0f);
	m_Variables[VTOL_VSensitivity]->Initialize("VSensitivity", "VTOL", 2.0f);
	m_Variables[VTOL_VSensitivity]->SetRange(0.001f, 100.0f);
	m_Variables[VTOL_VTilt]->Initialize("VTilt", "VTOL", 10.0f);
	m_Variables[VTOL_VTilt]->SetRange(0.0f, 90.0f);
	m_Variables[VTOL_HSensitivity]->Initialize("HSensitivity", "VTOL", 2.0f);
	m_Variables[VTOL_HSensitivity]->SetRange(0.001f, 100.0f);
	m_Variables[VTOL_HTilt]->Initialize("HTilt", "VTOL", 10.0f);
	m_Variables[VTOL_HTilt]->SetRange(0.0f, 90.0f); 
	m_Variables[VTOL_SeatPrevSensitivity]->Initialize("SeatPrevSensitivity", "VTOL", 1.5f);
	m_Variables[VTOL_SeatPrevSensitivity]->SetRange(1.0f);
	m_Variables[VTOL_SeatNextSensitivity]->Initialize("SeatNextSensitivity", "VTOL", 1.5f);
	m_Variables[VTOL_SeatNextSensitivity]->SetRange(1.0f);
	
	m_Variables[VTOL_InverseLook]->Initialize("InverseLook", "VTOL", false);
	m_Variables[VTOL_UseStickStrafe]->Initialize("UseStickStrafe", "VTOL", true);
	
	// VTOL button mapping
	m_Variables[VTOL_UseButton]->Initialize("Button_Use", "VTOL", WIIBTN_DPAD_DOWN);
	m_Variables[VTOL_BoostButton]->Initialize("Button_Boost", "VTOL", WIIBTN_C);
	m_Variables[VTOL_PrevSeatButton]->Initialize("Button_PrevSeat", "VTOL", WIIBTN_MINUS);
	m_Variables[VTOL_NextSeatButton]->Initialize("Button_NextSeat", "VTOL", WIIBTN_PLUS);
	m_Variables[VTOL_FireGunButton]->Initialize("Button_FireGun", "VTOL", WIIBTN_B);
	m_Variables[VTOL_FireRocketButton]->Initialize("Button_FireRocket", "VTOL", WIIBTN_HOME);
	m_Variables[VTOL_ViewButton]->Initialize("Button_View", "VTOL", WIIBTN_1);
}

////////////////////////////////////////////////////
int CWiiRemoteProfile::TranslateButton(EWiiButton nButton)
{
	// Switch
	switch (nButton)
	{
		case WIIBTN_DPAD_UP: return WR_BUTTON_DPAD_UP; break;
		case WIIBTN_DPAD_DOWN: return WR_BUTTON_DPAD_DOWN; break;
		case WIIBTN_DPAD_LEFT: return WR_BUTTON_DPAD_LEFT; break;
		case WIIBTN_DPAD_RIGHT: return WR_BUTTON_DPAD_RIGHT; break;
		case WIIBTN_A: return WR_BUTTON_A; break;
		case WIIBTN_B: return WR_BUTTON_B; break;
		case WIIBTN_1: return WR_BUTTON_ONE; break;
		case WIIBTN_2: return WR_BUTTON_TWO; break;
		case WIIBTN_HOME: return WR_BUTTON_HOME; break;
		case WIIBTN_MINUS: return WR_BUTTON_MINUS; break;
		case WIIBTN_PLUS: return WR_BUTTON_PLUS; break;
		case WIIBTN_C: return WR_NCBUTTON_C|NUNCHUK_BIT; break;
		case WIIBTN_Z: return WR_NCBUTTON_Z|NUNCHUK_BIT; break;
	}

	return 0;
}

////////////////////////////////////////////////////
EWiiButton CWiiRemoteProfile::TranslateStrButton(char const* szName)
{
	for (int i = 0; i < WIIBTN_MAX; i++)
	{
		if (strcmp(szWiiButtonNames[i], szName) == 0)
			return (EWiiButton)i;
	}
	return WIIBTN_NONE;
}

////////////////////////////////////////////////////
bool CWiiRemoteProfile::LoadCFG(void)
{
	// Open file for reading
	XmlNodeRef rootNode = gEnv->pSystem->LoadXmlFile(PROFILE_NAME);
	if (NULL == rootNode) return false;

	// Let each variable read from it
	for (int i = 0; i < EPROFILEVARIABLES_COUNT; i++)
		m_Variables[i]->ReadFromConfig(rootNode);

	return true;
}

////////////////////////////////////////////////////
bool CWiiRemoteProfile::SaveCFG(void) const
{
	// Open file for reading
	XmlNodeRef rootNode = gEnv->pSystem->LoadXmlFile(PROFILE_NAME);
	if (NULL == rootNode)
	{
		// Create a new file
		rootNode = gEnv->pSystem->CreateXmlNode("WiiRemote");
		if (NULL == rootNode)
			return false;
	}

	// Let each variable write to it
	for (int i = 0; i < EPROFILEVARIABLES_COUNT; i++)
		m_Variables[i]->WriteToConfig(rootNode);

	// Save it out
	char path[_MAX_PATH];
	const char* szAdjustedPath = gEnv->pCryPak->AdjustFileName(PROFILE_NAME,path,(ICryPak::FLAGS_NO_MASTER_FOLDER_MAPPING|ICryPak::FLAGS_FOR_WRITING));
	rootNode->saveToFile(szAdjustedPath);

	return true;
}

////////////////////////////////////////////////////
void CWiiRemoteProfile::RegisterConsoleCommands(struct IConsole *pConsole) const
{
	pConsole->AddCommand("wr_get", wr_get, 0, "Get the value of a variable from your Wii Remote profile\nSyntax: wr_get <group> <name>");
	pConsole->AddCommand("wr_set", wr_set, 0, "Set the value of a variable from your Wii Remote profile\nSyntax: wr_set <group> <name> <value>");
	pConsole->AddCommand("wr_reset", wr_reset, 0, "Reset your Wii Remote profile to the saved contents");
	pConsole->AddCommand("wr_resettodefault", wr_resettodefault, 0, "Reset your Wii Remote profile to the default values");
	pConsole->AddCommand("wr_save", wr_save, 0, "Save your Wii Remote profile to disk");
	pConsole->AddCommand("wr_list", wr_list, 0, "Print out all Wii Remote profile variables");
}

////////////////////////////////////////////////////
void CWiiRemoteProfile::UnregisterConsoleCommands(struct IConsole *pConsole) const
{
	pConsole->RemoveCommand("wr_get");
	pConsole->RemoveCommand("wr_set");
	pConsole->RemoveCommand("wr_reset");
	pConsole->RemoveCommand("wr_resettodefault");
	pConsole->RemoveCommand("wr_save");
	pConsole->RemoveCommand("wr_list");
}

////////////////////////////////////////////////////
void CWiiRemoteProfile::wr_get(IConsoleCmdArgs *pArgs)
{
	string szGroup = pArgs->GetArg(1);
	string szName = pArgs->GetArg(2);

	// Find variable and print it out
	IWiiRemoteProfileVariable *pVar = g_WiiRemoteProfile->GetVariable(szName.c_str(), szGroup.c_str());
	if (NULL != pVar)
	{
		// Switch type
		switch (pVar->GetType())
		{
			case WRP_VARIABLE_FLOAT:
			{
				CryLogAlways("wr_get [%s] %s = %f", pVar->GetGroup(), pVar->GetName(), pVar->GetValueFloat());
				return;
			}
			break;
			case WRP_VARIABLE_BUTTON:
			{
				CryLogAlways("wr_get [%s] %s = %s", pVar->GetGroup(), pVar->GetName(), szWiiButtonNames[pVar->GetValueButton()]);
				return;
			}
			break;
			case WRP_VARIABLE_BOOL:
			{
				CryLogAlways("wr_get [%s] %s = %d", pVar->GetGroup(), pVar->GetName(), pVar->GetValueBool()==false?0:1);
				return;
			}
			break;
		}
	}
	
	CryLogAlways("wr_get Bad variable group or name");
}

////////////////////////////////////////////////////
void CWiiRemoteProfile::wr_set(IConsoleCmdArgs *pArgs)
{
	string szGroup = pArgs->GetArg(1);
	string szName = pArgs->GetArg(2);
	string szValue = pArgs->GetArg(3);
	for (int i = 4; i < pArgs->GetArgCount(); i++)
		szValue += string(" ") + pArgs->GetArg(i);

	// Find variable and set it
	IWiiRemoteProfileVariable *pVar = g_WiiRemoteProfile->GetVariable(szName.c_str(), szGroup.c_str());
	if (NULL != pVar)
	{
		// Switch type
		switch (pVar->GetType())
		{
			case WRP_VARIABLE_FLOAT:
			{
				float fValue;
				sscanf(szValue.c_str(), "%f", &fValue);
				pVar->SetValue(fValue);
				CryLogAlways("wr_set [%s] %s = %f", pVar->GetGroup(), pVar->GetName(), pVar->GetValueFloat());
				return;
			}
			break;
			case WRP_VARIABLE_BUTTON:
			{
				pVar->SetValue(g_WiiRemoteProfile->TranslateStrButton(szValue.c_str()));
				CryLogAlways("wr_set [%s] %s = %s", pVar->GetGroup(), pVar->GetName(), szWiiButtonNames[pVar->GetValueButton()]);
				return;
			}
			break;
			case WRP_VARIABLE_BOOL:
			{
				int nValue;
				sscanf(szValue.c_str(), "%d", &nValue);
				pVar->SetValue(nValue==0?false:true);
				CryLogAlways("wr_set [%s] %s = %d", pVar->GetGroup(), pVar->GetName(), pVar->GetValueBool()==false?0:1);
				return;
			}
			break;
		}
	}

	CryLogAlways("wr_set Bad variable group or name");
}

////////////////////////////////////////////////////
void CWiiRemoteProfile::wr_reset(IConsoleCmdArgs *pArgs)
{
	// Attempt a load
	if (false == g_WiiRemoteProfile->LoadCFG())
		CryLogAlways("wr_reset Failed to load config");
	else
		CryLogAlways("wr_reset Success");
}

////////////////////////////////////////////////////
void CWiiRemoteProfile::wr_resettodefault(IConsoleCmdArgs *pArgs)
{
	g_WiiRemoteProfile->SetToDefault();
	CryLogAlways("wr_resettodefault Success");
}

////////////////////////////////////////////////////
void CWiiRemoteProfile::wr_save(IConsoleCmdArgs *pArgs)
{
	// Attempt a save
	if (false == g_WiiRemoteProfile->SaveCFG())
		CryLogAlways("wr_save Failed to save config");
	else
		CryLogAlways("wr_save Success");
}

////////////////////////////////////////////////////
void CWiiRemoteProfile::wr_list(IConsoleCmdArgs *pArgs)
{
	CryLogAlways("wr_list Variables:");
	CryLogAlways("[Group] Name");
	for (int i = 0; i < EPROFILEVARIABLES_COUNT; i++)
	{
		CryLogAlways("[%s] %s", g_WiiRemoteProfile->m_Variables[i]->GetGroup(),
			g_WiiRemoteProfile->m_Variables[i]->GetName());
	}
}

////////////////////////////////////////////////////
CWiiRemoteProfileVariable::CWiiRemoteProfileVariable(void)
{
	m_nType = WRP_VARIABLE_NOTYPE;
	m_fMin = FLT_MIN;
	m_fMax = FLT_MAX;
	m_nValue = WIIBTN_NONE;
}


////////////////////////////////////////////////////
CWiiRemoteProfileVariable::~CWiiRemoteProfileVariable(void)
{

}

////////////////////////////////////////////////////
void CWiiRemoteProfileVariable::Initialize(char const* szName, char const* szGroup, float value)
{
	m_szName = szName;
	m_szGroup = szGroup;
	m_nType = WRP_VARIABLE_FLOAT;
	m_fValue = value;
}

////////////////////////////////////////////////////
void CWiiRemoteProfileVariable::Initialize(char const* szName, char const* szGroup, EWiiButton value)
{
	m_szName = szName;
	m_szGroup = szGroup;
	m_nType = WRP_VARIABLE_BUTTON;
	m_nValue = value;
}

////////////////////////////////////////////////////
void CWiiRemoteProfileVariable::Initialize(char const* szName, char const* szGroup, bool value)
{
	m_szName = szName;
	m_szGroup = szGroup;
	m_nType = WRP_VARIABLE_BOOL;
	m_bValue = value;
}

////////////////////////////////////////////////////
void CWiiRemoteProfileVariable::SetRange(float min, float max)
{
	m_fMin = min;
	m_fMax = max;
}

////////////////////////////////////////////////////
char const* CWiiRemoteProfileVariable::GetName(void) const
{
	return m_szName.c_str();
}

////////////////////////////////////////////////////
char const* CWiiRemoteProfileVariable::GetGroup(void) const
{
	return m_szGroup.c_str();
}

////////////////////////////////////////////////////
int CWiiRemoteProfileVariable::GetType(void) const
{
	return m_nType;
}

////////////////////////////////////////////////////
float CWiiRemoteProfileVariable::GetValueFloat(void) const
{
	if (WRP_VARIABLE_FLOAT != m_nType) return 0;
	return m_fValue;
}

////////////////////////////////////////////////////
EWiiButton CWiiRemoteProfileVariable::GetValueButton(void) const
{
	if (WRP_VARIABLE_BUTTON != m_nType) return WIIBTN_NONE;
	return m_nValue;
}

////////////////////////////////////////////////////
bool CWiiRemoteProfileVariable::GetValueBool(void) const
{
	if (WRP_VARIABLE_BOOL != m_nType) return false;
	return m_bValue;
}

////////////////////////////////////////////////////
void CWiiRemoteProfileVariable::SetValue(float value)
{
	if (WRP_VARIABLE_FLOAT == m_nType)
		m_fValue = CLAMP(value,m_fMin,m_fMax);
}

////////////////////////////////////////////////////
void CWiiRemoteProfileVariable::SetValue(EWiiButton value)
{
	if (WRP_VARIABLE_BUTTON == m_nType)
		m_nValue = (EWiiButton)CLAMP(value,WIIBTN_NONE+1,WIIBTN_MAX-1);
}

////////////////////////////////////////////////////
void CWiiRemoteProfileVariable::SetValue(bool value)
{
	if (WRP_VARIABLE_BOOL == m_nType)
		m_bValue = value;
}

////////////////////////////////////////////////////
void CWiiRemoteProfileVariable::WriteToConfig(XmlNodeRef &pNode) const
{
	if (NULL == pNode) return;

	// Get group
	XmlNodeRef pGroup = NULL, pSearchGroup;
	int nGroupCount = pNode->getChildCount();
	for (int nGroup = 0; nGroup < nGroupCount; nGroup++)
	{
		pSearchGroup = pNode->getChild(nGroup);
		if (false == pSearchGroup->isTag("group")) continue;
		if (m_szGroup == pSearchGroup->getAttr("name"))
		{
			// Use it!
			pGroup = pSearchGroup;
			break;
		}
	}
	if (NULL == pGroup)
	{
		// Make a new one
		pGroup = pNode->newChild("group");
		pGroup->setAttr("name", m_szGroup.c_str());
	}

	// Get entry
	XmlNodeRef pEntry = NULL, pSearchEntry;
	int nEntryCount = pGroup->getChildCount();
	for (int nEntry = 0; nEntry < nEntryCount; nEntry++)
	{
		pSearchEntry = pGroup->getChild(nEntry);
		if (false == pSearchEntry->isTag("entry")) continue;
		if (m_szName == pSearchEntry->getAttr("name"))
		{
			// Use it!
			pEntry = pSearchEntry;
			break;
		}
	}
	if (NULL == pEntry)
	{
		// Make a new one
		pEntry = pGroup->newChild("entry");
		pEntry->setAttr("name", m_szName.c_str());
	}

	// Set value
	switch (m_nType)
	{
		case WRP_VARIABLE_FLOAT:
		{
			pEntry->setAttr("value", m_fValue);
		}
		break;
		case WRP_VARIABLE_BUTTON:
		{
			pEntry->setAttr("value", szWiiButtonNames[m_nValue]);
		}
		break;
		case WRP_VARIABLE_BOOL:
		{
			pEntry->setAttr("value", m_bValue);
		}
		break;
	}
}

////////////////////////////////////////////////////
void CWiiRemoteProfileVariable::ReadFromConfig(XmlNodeRef pNode)
{
	if (NULL == pNode) return;

	// Find group
	XmlNodeRef pGroup, pEntry;
	int nGroupCount = pNode->getChildCount();
	for (int nGroup = 0; nGroup < nGroupCount; nGroup++)
	{
		// Get group
		pGroup = pNode->getChild(nGroup);
		if (false == pGroup->isTag("group")) continue;

		// Check if this is the one
		if (m_szGroup == pGroup->getAttr("name"))
		{
			// Found group. Find entry.
			int nEntryCount = pGroup->getChildCount();
			for (int nEntry = 0; nEntry < nEntryCount; nEntry++)
			{
				// Get entry
				pEntry = pGroup->getChild(nEntry);
				if (false == pEntry->isTag("entry")) continue;

				// Check if this is the one
				if (m_szName == pEntry->getAttr("name"))
				{
					// Extract value based on type
					switch (m_nType)
					{
						case WRP_VARIABLE_FLOAT:
						{
							float fValue;
							pEntry->getAttr("value", fValue);
							SetValue(fValue);
						}
						break;
						case WRP_VARIABLE_BUTTON:
						{
							XmlString szButtonName;
							pEntry->getAttr("value", szButtonName);
							SetValue(g_WiiRemoteProfile->TranslateStrButton(szButtonName));
						}
						break;
						case WRP_VARIABLE_BOOL:
						{
							bool bValue;
							pEntry->getAttr("value", bValue);
							SetValue(bValue);
						}
						break;
					}

					break;
				}
			}

			break;
		}
	}
}