////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// IWiiRemoteProfile.h
//
// Purpose: Interface object
//	Describes a profile for the Wii Remote including
//	key bindings and sensitivity settings
//
// History:
//	- 11/29/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _IWIIREMOTEPROFILE_H_
#define _IWIIREMOTEPROFILE_H_

// Wii Button mappings
#define NUNCHUK_BIT (0xF0)
enum EWiiButton
{
	WIIBTN_NONE = 0,

	WIIBTN_DPAD_UP,
	WIIBTN_DPAD_DOWN,
	WIIBTN_DPAD_LEFT,
	WIIBTN_DPAD_RIGHT,
	WIIBTN_A,
	WIIBTN_B,
	WIIBTN_1,
	WIIBTN_2,
	WIIBTN_HOME,
	WIIBTN_MINUS,
	WIIBTN_PLUS,
	WIIBTN_C,
	WIIBTN_Z,

	WIIBTN_MAX
};
static char const* szWiiButtonNames[WIIBTN_MAX] =
{
	"None", "DPad Up", "DPad Down", "DPad Left", "DPad Right", "A", "B",
	"1", "2", "Home", "Minus", "Plus", "C", "Z",
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// WRP_VARIABLE_TYPE
//	Variable type IDs
enum WRP_VARIABLE_TYPE
{
	WRP_VARIABLE_NOTYPE = 0,
	WRP_VARIABLE_FLOAT,
	WRP_VARIABLE_BUTTON,
	WRP_VARIABLE_BOOL,
};

struct IWiiRemoteProfileVariable
{
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~IWiiRemoteProfileVariable(void) { }

	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize the variable
	//
	// In:	szName - Name of the variable
	//		szGroup - Group name of the variable
	//		value - Initial value of the variable
	////////////////////////////////////////////////////
	virtual void Initialize(char const* szName, char const* szGroup, float value) = 0;
	virtual void Initialize(char const* szName, char const* szGroup, EWiiButton value) = 0;
	virtual void Initialize(char const* szName, char const* szGroup, bool value) = 0;

	////////////////////////////////////////////////////
	// SetRange
	//
	// Purpose: Set the range of the variable (for floats)
	//
	// In:	max - Maximum value
	//		min - Minimum value
	////////////////////////////////////////////////////
	virtual void SetRange(float min = FLT_MIN, float max = FLT_MAX) = 0;

	////////////////////////////////////////////////////
	// GetName
	//
	// Purpose: Get the name of the variable
	////////////////////////////////////////////////////
	virtual char const* GetName(void) const = 0;

	////////////////////////////////////////////////////
	// GetGroup
	//
	// Purpose: Get the group name of the variable
	////////////////////////////////////////////////////
	virtual char const* GetGroup(void) const = 0;

	////////////////////////////////////////////////////
	// GetType
	//
	// Purpose: Get the type of the variable
	//
	// Note: See WRP_VARIABLE_TYPE
	////////////////////////////////////////////////////
	virtual int GetType(void) const = 0;

	////////////////////////////////////////////////////
	// GetValue
	//
	// Purpose: Get the value of the variable
	////////////////////////////////////////////////////
	virtual float GetValueFloat(void) const = 0;
	virtual EWiiButton GetValueButton(void) const = 0;
	virtual bool GetValueBool(void) const = 0;

	////////////////////////////////////////////////////
	// SetValue
	//
	// Purpose: Set the value of the variable
	//
	// In:	value - Value of the variable
	////////////////////////////////////////////////////
	virtual void SetValue(float value) = 0;
	virtual void SetValue(EWiiButton value) = 0;
	virtual void SetValue(bool value) = 0;

	////////////////////////////////////////////////////
	// WriteToConfig
	//
	// Purpose: Write the variable to the config
	//
	// In:	pNode - Config file node
	////////////////////////////////////////////////////
	virtual void WriteToConfig(XmlNodeRef &pNode) const = 0;

	////////////////////////////////////////////////////
	// ReadFromConfig
	//
	// Purpose: Read the variable to the config
	//
	// In:	pNode - Config file node
	////////////////////////////////////////////////////
	virtual void ReadFromConfig(XmlNodeRef pNode) = 0;
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

struct IWiiRemoteProfile
{
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~IWiiRemoteProfile(void) { }

	////////////////////////////////////////////////////
	// GetVariable
	//
	// Purpose: Get a variable item
	//
	// In:	szName - Name of the variable
	//		szGroup - Group name of the variable
	//		nIndex - Index ID of variable
	////////////////////////////////////////////////////
	virtual IWiiRemoteProfileVariable* GetVariable(char const* szName, char const* szGroup) = 0;
	virtual IWiiRemoteProfileVariable const* GetVariable(char const* szName, char const* szGroup) const = 0;
	virtual IWiiRemoteProfileVariable* GetVariable(int nIndex) = 0;
	virtual IWiiRemoteProfileVariable const* GetVariable(int nIndex) const = 0;

	////////////////////////////////////////////////////
	// SetToDefault
	//
	// Purpose: Set the profile values to default
	////////////////////////////////////////////////////
	virtual void SetToDefault(void) = 0;

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
	virtual int TranslateButton(EWiiButton nButton) = 0;

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
	virtual EWiiButton TranslateStrButton(char const* szName) = 0;

	////////////////////////////////////////////////////
	// LoadCFG
	//
	// Purpose: Load the CFG file for the active user
	//
	// Returns TRUE if it was loaded
	////////////////////////////////////////////////////
	virtual bool LoadCFG(void) = 0;

	////////////////////////////////////////////////////
	// SaveCFG
	//
	// Purpose: Save the CFG file for the active user
	//
	// Returns TRUE if it was saved
	////////////////////////////////////////////////////
	virtual bool SaveCFG(void) const = 0;

	////////////////////////////////////////////////////
	// RegisterConsoleCommands
	//
	// Purpose: Register console commands
	//
	// In:	pConsole - Console object
	////////////////////////////////////////////////////
	virtual void RegisterConsoleCommands(struct IConsole *pConsole) const = 0;

	////////////////////////////////////////////////////
	// UnregisterConsoleCommands
	//
	// Purpose: Unregister console commands
	//
	// In:	pConsole - Console object
	////////////////////////////////////////////////////
	virtual void UnregisterConsoleCommands(struct IConsole *pConsole) const = 0;
};

#endif //_IWIIREMOTEPROFILE_H_