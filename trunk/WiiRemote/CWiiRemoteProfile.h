////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// CWiiRemoteProfile.h
//
// Purpose: Profile for the Wii Remote including
//	key bindings and sensitivity settings
//
// History:
//	- 11/15/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _CWIIREMOTEPROFILE_H_
#define _CWIIREMOTEPROFILE_H_

#include "Interfaces\IWiiRemoteProfile.h"

// IR sensor deadzones
#define WII_IRSENSOR_DEADZONE_LEFT	(0.45f)
#define WII_IRSENSOR_DEADZONE_RIGHT	(0.55f)
#define WII_IRSENSOR_DEADZONE_UP	(0.45f)
#define WII_IRSENSOR_DEADZONE_DOWN	(0.55f)

// Variable IDs
enum EPROFILEVARIABLES
{
	RetainViewMode = 0,		// TRUE if view should retain focus
	HardLockView,			// TRUE if view should be hard locked when retained (cannot be moved around)
	UseIRSensor,			// TRUE to use IR sensor for viewing
	ShowIRDot,				// TRUE to show the IR Dot on the HUD
	IRDotOpacity,			// Value between 0 and 1 for opacity of IR Dot on the HUD
	MaxLockViewDist,		// Maximum distance entity can be to be locked on
	EnableRumble,			// TRUE if rumble is allowed

	JumpSensitivity,		// How many Gs must be applied for jump to occur
	CrouchTilt,				// Degree remote must be tilted down to toggle crouch
	ProneTilt,				// Degree remote must be tilted down to toggle prone
	LeanTilt,				// Degree remote must be tilted sideways to toggle leaning
	PunchSensitivity,		// How many Gs must be applied for punching to occur
	LookUpSensitivity,		// Ratio applied to degree of tilt for looking up/down
	LookUpMaxTilt,			// Degree remote must be tilted to be looking straight up/down
	LookUpError,			// Error in difference that must be reached before pitch is updated
	TurnSensitivity,		// Ratio applied to degree of tilt for turning
	TurnTilt,				// Degree remote must be tilted sideways to toggle turning
	ZoomInSensitivity,		// How many Gs must be applied for zoom in to occur
	ZoomOutSensitivity,		// How many Gs must be applied for zoom out to occur
	HolsterItemSensitivity,	// How many Gs must be applied after a long gesture to put away current item
	MeleeSensitivity,		// How many Gs must be applied for weapon melee to trigger (in X and Z axis, +)
	PickUpSensitivity,		// How many Gs must be applied for pick up gesture to trigger (in Y and Z axis, +)
	ThrowSensitivity,		// How many Gs must be applied for throw gesture to trigger (in Y and Z axis, +)
	DropSensitivity,		// How many Gs must be applied for drop gesture to trigger (in X and Z axis, -)

	// Used in IR sensor mode
	IRSensor_TurnSensitivity,
	IRSensor_LookUpSensitivity,

	// Used in soft lock retain view mode
	SoftLock_LookUpSensitivity,
	SoftLock_TurnSensitivity,
	
	InverseLook,			// TRUE if tilting up the remote causes you to look down

	// Buttons
	SprintButton,			// Button to use to sprint
	NanoSuitMenuButton,		// Button to open up nano suit menu
	WeaponMenuButton,		// Button to open up weapon menu
	PrevWeaponButton,
	NextWeaponButton,
	LockViewButton,			// Button to lock camera when held
	FireButton,				// Button to attack with current item
	ReloadButton,			// Button to reload current item
	BinocularsButton,		// Button to toggle binoculars
	NightVisionButton,		// Button to toggle night vision
	ZoomInButton,
	ZoomOutButton,
	ToggleFiremodeButton,	// Button to toggle the fire mode on the weapon
	ToggleGrenadeButton,	// Button to toggle the grenade type
	UseButton,				// Button to toggle "use"

	// Vehicle stuff
	Veh_LookUpSensitivity,	// Ratio applied to degree of tilt for looking up/down
	Veh_LookUpMaxTilt,		// Degree remote must be tilted to be looking straight up/down
	Veh_LookUpError,		// Error in difference that must be reached before pitch is updated
	Veh_TurnSensitivity,	// Ratio applied to degree of tilt for turning
	Veh_TurnTilt,
	Veh_IRSensor_TurnSensitivity,
	Veh_IRSensor_LookUpSensitivity,
	Veh_SeatPrevSensitivity,	//How many Gs must be applied for previous seat motion to occur
	Veh_SeatNextSensitivity,	//How many Gs must be applied for next seat motion to occur

	Veh_UseStickSteer_Land,	// TRUE if the analog stick should be used to steer on land
	Veh_SteerTilt_Land,
	Veh_SteerSensitivity_Land,

	Veh_UseStickSteer_Sea,	// TRUE if the analog stick should be used to steer on sea
	Veh_SteerTilt_Sea,
	Veh_SteerSensitivity_Sea,

	Veh_InverseLook,		// TRUE if tilting up the remote causes you to look down
	
	Veh_UseButton,			// Button to toggle "use"
	Veh_HornButton,			// Button to toggle horn
	Veh_LightsButton,		// Button to toggle lights
	Veh_PrevSeatButton,		// Button to move to previous seat
	Veh_NextSeatButton,		// Button to move to next seat
	Veh_BoostButton,		// Button to toggle boost
	Veh_FireButton,			// Button to fire vehicle's weapon
	Veh_ReloadButton,		// Button to reload vehicle's weapon
	Veh_ViewButton,			// Button to toggle between first and third person view

	// Helicopter controls
	Heli_PitchSensitivity,
	Heli_PitchTilt,
	Heli_TurnSensitivity,
	Heli_TurnTilt,
	Heli_IRSensor_PitchSensitivity,
	Heli_IRSensor_TurnSensitivity,
	Heli_RollSensitivity,
	Heli_RollTilt,
	Heli_SeatPrevSensitivity,
	Heli_SeatNextSensitivity,

	Heli_InverseLook,
	Heli_UseStickRoll,
	
	Heli_UseButton,
	Heli_BoostButton,
	Heli_PrevSeatButton,
	Heli_NextSeatButton,
	Heli_FireGunButton,
	Heli_FireRocketButton,
	Heli_ViewButton,

	// VTOL controls
	VTOL_PitchSensitivity,	
	VTOL_PitchTilt,
	VTOL_TurnSensitivity,
	VTOL_TurnTilt,
	VTOL_IRSensor_PitchSensitivity,
	VTOL_IRSensor_TurnSensitivity,
	VTOL_VSensitivity,
	VTOL_VTilt,
	VTOL_HSensitivity,
	VTOL_HTilt,
	VTOL_SeatPrevSensitivity,
	VTOL_SeatNextSensitivity,

	VTOL_InverseLook,
	VTOL_UseStickStrafe,

	VTOL_UseButton,
	VTOL_BoostButton,
	VTOL_PrevSeatButton,
	VTOL_NextSeatButton,
	VTOL_FireGunButton,
	VTOL_FireRocketButton,
	VTOL_ViewButton,

	// MAX COUNT
	EPROFILEVARIABLES_COUNT,
};

////////////////////////////////////////////////////
class CWiiRemoteProfileVariable : public IWiiRemoteProfileVariable
{
protected:
	int m_nType;	// Variable type
	string m_szName;
	string m_szGroup;

	float m_fMin, m_fMax;

	// Value
	union
	{
		EWiiButton m_nValue;
		float m_fValue;
		bool m_bValue;
	};

public:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWiiRemoteProfileVariable(void);
private:
	CWiiRemoteProfileVariable(CWiiRemoteProfileVariable const&) {}
	CWiiRemoteProfileVariable& operator =(CWiiRemoteProfileVariable const&) {return *this;}

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWiiRemoteProfileVariable(void);

	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize the variable
	//
	// In:	szName - Name of the variable
	//		szGroup - Group name of the variable
	//		value - Initial value of the variable
	////////////////////////////////////////////////////
	virtual void Initialize(char const* szName, char const* szGroup, float value);
	virtual void Initialize(char const* szName, char const* szGroup, EWiiButton value);
	virtual void Initialize(char const* szName, char const* szGroup, bool value);

	////////////////////////////////////////////////////
	// SetRange
	//
	// Purpose: Set the range of the variable (for floats)
	//
	// In:	max - Maximum value
	//		min - Minimum value
	////////////////////////////////////////////////////
	virtual void SetRange(float min = FLT_MIN, float max = FLT_MAX);

	////////////////////////////////////////////////////
	// GetName
	//
	// Purpose: Get the name of the variable
	////////////////////////////////////////////////////
	virtual char const* GetName(void) const;

	////////////////////////////////////////////////////
	// GetGroup
	//
	// Purpose: Get the group name of the variable
	////////////////////////////////////////////////////
	virtual char const* GetGroup(void) const;

	////////////////////////////////////////////////////
	// GetType
	//
	// Purpose: Get the type of the variable
	//
	// Note: See WRP_VARIABLE_TYPE
	////////////////////////////////////////////////////
	virtual int GetType(void) const;

	////////////////////////////////////////////////////
	// GetValue
	//
	// Purpose: Get the value of the variable
	////////////////////////////////////////////////////
	virtual float GetValueFloat(void) const;
	virtual EWiiButton GetValueButton(void) const;
	virtual bool GetValueBool(void) const;

	////////////////////////////////////////////////////
	// SetValue
	//
	// Purpose: Set the value of the variable
	//
	// In:	value - Value of the variable
	////////////////////////////////////////////////////
	virtual void SetValue(float value);
	virtual void SetValue(EWiiButton value);
	virtual void SetValue(bool value);

	////////////////////////////////////////////////////
	// WriteToConfig
	//
	// Purpose: Write the variable to the config
	//
	// In:	pNode - Config file node
	////////////////////////////////////////////////////
	virtual void WriteToConfig(XmlNodeRef &pNode) const;

	////////////////////////////////////////////////////
	// ReadFromConfig
	//
	// Purpose: Read the variable to the config
	//
	// In:	pNode - Config file node
	////////////////////////////////////////////////////
	virtual void ReadFromConfig(XmlNodeRef pNode);
};

////////////////////////////////////////////////////
class CWiiRemoteProfile : public IWiiRemoteProfile
{
protected:
	// Variables
	IWiiRemoteProfileVariable* m_Variables[EPROFILEVARIABLES_COUNT];

private:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWiiRemoteProfile(void);
	CWiiRemoteProfile(CWiiRemoteProfile const&) {}
	CWiiRemoteProfile& operator =(CWiiRemoteProfile const&) {return *this;}

	// Single instance
	static CWiiRemoteProfile m_Instance;

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWiiRemoteProfile(void);

	////////////////////////////////////////////////////
	// GetInstance
	//
	// Purpose: Return single instance
	////////////////////////////////////////////////////
	static CWiiRemoteProfile* GetInstance(void);

	////////////////////////////////////////////////////
	// GetVariable
	//
	// Purpose: Get a variable item
	//
	// In:	szName - Name of the variable
	//		szGroup - Group name of the variable
	//		nIndex - Index ID of variable
	////////////////////////////////////////////////////
	virtual IWiiRemoteProfileVariable* GetVariable(char const* szName, char const* szGroup);
	virtual IWiiRemoteProfileVariable const* GetVariable(char const* szName, char const* szGroup) const;
	virtual IWiiRemoteProfileVariable* GetVariable(int nIndex);
	virtual IWiiRemoteProfileVariable const* GetVariable(int nIndex) const;

	////////////////////////////////////////////////////
	// SetToDefault
	//
	// Purpose: Set the profile values to default
	////////////////////////////////////////////////////
	virtual void SetToDefault(void);

	////////////////////////////////////////////////////
	// TranslateButton
	//
	// Purpose: Translate the given WIIBTN value to the
	//	corresponding value used in WRLib
	//
	// In:	nButton - WIIBTN value
	//
	// Returns translated value
	////////////////////////////////////////////////////
	int TranslateButton(EWiiButton nButton);

	////////////////////////////////////////////////////
	// TranslateStrButton
	//
	// Purpose: Translate the given button name to the
	//	corresponding WIIBTN value
	//
	// In:	szName - Name of button
	//
	// Returns translated value
	////////////////////////////////////////////////////
	EWiiButton TranslateStrButton(char const* szName);

	////////////////////////////////////////////////////
	// LoadCFG
	//
	// Purpose: Load the CFG file for the active user
	//
	// Returns TRUE if it was loaded
	////////////////////////////////////////////////////
	virtual bool LoadCFG(void);

	////////////////////////////////////////////////////
	// SaveCFG
	//
	// Purpose: Save the CFG file for the active user
	//
	// Returns TRUE if it was saved
	////////////////////////////////////////////////////
	virtual bool SaveCFG(void) const;

	////////////////////////////////////////////////////
	// RegisterConsoleCommands
	//
	// Purpose: Register console commands
	//
	// In:	pConsole - Console object
	////////////////////////////////////////////////////
	virtual void RegisterConsoleCommands(struct IConsole *pConsole) const;

	////////////////////////////////////////////////////
	// UnregisterConsoleCommands
	//
	// Purpose: Unregister console commands
	//
	// In:	pConsole - Console object
	////////////////////////////////////////////////////
	virtual void UnregisterConsoleCommands(struct IConsole *pConsole) const;

protected:
	static void wr_get(IConsoleCmdArgs *pArgs);
	static void wr_set(IConsoleCmdArgs *pArgs);
	static void wr_reset(IConsoleCmdArgs *pArgs);
	static void wr_resettodefault(IConsoleCmdArgs *pArgs);
	static void wr_save(IConsoleCmdArgs *pArgs);
	static void wr_list(IConsoleCmdArgs *pArgs);
};

extern CWiiRemoteProfile* g_WiiRemoteProfile;

////////////////////////////////////////////////////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// CHECK_PROFILE_FLOAT
//
// Purpose: Check the value of a profile option
//
// In:	option - Option to check i.e. "JumpSensitivity"
////////////////////////////////////////////////////
#define CHECK_PROFILE_FLOAT(option) g_WiiRemoteProfile->GetVariable(option)->GetValueFloat()

////////////////////////////////////////////////////
// CHECK_PROFILE_BOOL
//
// Purpose: Check the value of a profile option
//
// In:	option - Option to check i.e. "UseIRSensor"
////////////////////////////////////////////////////
#define CHECK_PROFILE_BOOL(option) g_WiiRemoteProfile->GetVariable(option)->GetValueBool()

////////////////////////////////////////////////////
// CHECK_PROFILE_BUTTON
//
// Purpose: Check and translate the button mapping
//	for the given option to the right WRLib value
//
// In:	option - Option to check i.e. "Sprint" for
//	"SprintButton"
////////////////////////////////////////////////////
#define CHECK_PROFILE_BUTTON(option) g_WiiRemoteProfile->TranslateButton(g_WiiRemoteProfile->GetVariable(option##Button)->GetValueButton())


#endif //_CWIIREMOTEPROFILE_H_