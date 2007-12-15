////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// CWiiRemoteManager.h
//
// Purpose: Manager to handle the Wii Remote
//
// History:
//	- 11/15/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _CWIIREMOTEMANAGER_H_
#define _CWIIREMOTEMANAGER_H_

#include "Interfaces\IWiiRemoteManager.h"
#include "Interfaces\IMotion.h"
#include "IActionMapManager.h"

#define WRERROR_SHOWMESSAGE_RATE (10.0f)
#define WRERROR_SHOWMESSAGE_LENGTH (5.0f)
#define WRERROR_BATTERY_MARKER (0.1f) // Show battery low warning at 10%

// Battery icon dimensions
#define BatteryWidth 32.f
#define BatteryHeight 64.f
#define BatteryLevelX 4.f
#define BatteryLevelY 5.f
#define BatteryLevelWidth 25.f
#define BatteryLevelHeight 45.f

// Error levels
enum EWiiRemoteErrorLevel
{
	WIIREMOTE_ERROR_NOCONNECTION = 0,
	WIIREMOTE_ERROR_NOEXTENSION,
	WIIREMOTE_ERROR_DISCONNECTED,
	WIIREMOTE_ERROR_LOWBATTERY,

	WIIREMOTE_ERROR_MAX,
};

// Error objects
struct SWiiRemoteError
{
	bool bOn;
	int nTextureID;
	float fTextureWidth, fTextureHeight;
	string szMessage;

	SWiiRemoteError(void) : bOn(false), nTextureID(0), fTextureWidth(0), fTextureHeight(0) {}
};

// States
enum EStates
{
	STATE_PLAYER = 0,
	STATE_NANOSUITMENU,
	STATE_BINOCULARS,
	STATE_WEAPONMENU,
	STATE_LANDVEHICLE,
	STATE_SEAVEHICLE,
	STATE_HELIVEHICLE,
	STATE_VTOLVEHICLE,
};

class CWiiRemoteProfile;
class CWiiRemoteManager : public IWiiRemoteManager, public IActionListener
{
	friend struct SWiiRemoteListener;
	friend struct SWiiInputListener;

protected:
	bool m_bMasterEnabled;
	class CGame *m_pGame;
	class CWR_GlobalInstance *m_pWR;
	struct IWR_WiiRemote *m_pRemote;

	// Error message
	SWiiRemoteError m_Errors[WIIREMOTE_ERROR_MAX];
	float m_fLastErrorDisplay;

	// Battery texture info
	int m_nBatteryTexture;
	int m_nBatteryLevelTexture;
	float m_fBatteryLevel;

	// IR Dot texture info
	int m_nIRDotTexture;
	float m_fIRDotWidth, m_fIRDotHeight;

	// Movement status
	bool m_bSprint;
	bool m_bLockView;
	bool m_bForceLockView;
	bool m_bLockedNPC;
	IRenderNode *m_pLockedEntity;
	Vec3 m_vLockedEntityOffset;

	// Used for double force lock check
	float m_fLastLockRelease;
	float m_fFreezeMovement;

	// Motion objects
	MotionList m_NunchukMotions;
	MotionList m_WiimoteMotions;

public:
	// IActionListener
	virtual void OnAction(const ActionId& action, int activationMode, float value);

	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWiiRemoteManager(class CGame *pGame);
private:
	CWiiRemoteManager(CWiiRemoteManager const&) {}
	CWiiRemoteManager& operator =(CWiiRemoteManager const&) {return *this;}

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWiiRemoteManager(void);

	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Find and prepare the Wii Remote
	////////////////////////////////////////////////////
	virtual void Initialize(void);

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Clean up
	////////////////////////////////////////////////////
	virtual void Shutdown(void);

	////////////////////////////////////////////////////
	// Update
	//
	// Purpose: Update the Wii Remote
	//
	// In:	bHaveFocus - TRUE if window has focus
	//		nUpdateFlags - Update flags
	////////////////////////////////////////////////////
	virtual void Update(bool bHaveFocus, int nUpdateFlags);

	////////////////////////////////////////////////////
	// UpdateHUD
	//
	// Purpose: Update the HUD elements for the remote
	//
	// In:	pHUD - HUD object
	////////////////////////////////////////////////////
	virtual void UpdateHUD(class CHUD *pHUD);

	////////////////////////////////////////////////////
	// EditorResetGame
	//
	// Purpose: Call when Editor game resets
	//
	// In:	bStart - TRUE if Editor game has started
	////////////////////////////////////////////////////
	virtual void EditorResetGame(bool bStart);

	////////////////////////////////////////////////////
	// SetMasterEnabled
	//
	// Purpose: Set if Wii Remote is utilized
	//
	// In:	bOn - TRUE to use the remote, FALSE to turn
	//		it off
	////////////////////////////////////////////////////
	virtual void SetMasterEnabled(bool bOn);

	////////////////////////////////////////////////////
	// IsMasterEnabled
	//
	// Purpose: Returns TRUE if Remote is enabled
	////////////////////////////////////////////////////
	virtual bool IsMasterEnabled(void) const;

	////////////////////////////////////////////////////
	// GetLockedEntity
	//
	// Purpose: Returns locked entity
	//
	// Out:	vOffset - Locked offset
	////////////////////////////////////////////////////
	virtual IRenderNode* GetLockedEntity(Vec3 &vOffset) const;

	////////////////////////////////////////////////////
	// FreezeMovement
	//
	// Purpose: Call to freeze movement for a bit
	//
	// Note: Used by gestures like throw to help prevent
	//	dizzy spells
	////////////////////////////////////////////////////
	virtual void FreezeMovement(void);

	////////////////////////////////////////////////////
	// IsMovementFrozen
	//
	// Purpose: Returns TRUE if movement is frozen
	////////////////////////////////////////////////////
	virtual bool IsMovementFrozen(void) const;

	////////////////////////////////////////////////////
	// SetErrorLevel
	//
	// Purpose: Set the error level to be displayed
	//
	// In:	nLevel - Error message level
	////////////////////////////////////////////////////
	virtual void SetErrorLevel(int nLevel);

	////////////////////////////////////////////////////
	// ClearErrorLevel
	//
	// Purpose: Clear error
	//
	// In:	nLevel - Level to clear or -1 for all levels
	////////////////////////////////////////////////////
	virtual void ClearErrorLevel(int nLevel = -1);

	////////////////////////////////////////////////////
	// SetRemote
	//
	// Purpose: Set the Wii Remote
	//
	// In:	pRemote - Wii Remote to use
	////////////////////////////////////////////////////
	virtual void SetRemote(struct IWR_WiiRemote *pRemote);

	////////////////////////////////////////////////////
	// GetRemote
	//
	// Purpose: Return the Wii Remote in use
	////////////////////////////////////////////////////
	virtual struct IWR_WiiRemote* GetRemote(void) const;

protected:
	////////////////////////////////////////////////////
	// UpdatePlayerMovement
	//
	// Purpose: Update player movement based on remote
	//	instance
	////////////////////////////////////////////////////
	virtual void UpdatePlayerMovement(void);

	////////////////////////////////////////////////////
	// IsButtonDown
	//
	// Purpose: Determine if bound button is down
	//
	// In:	nButton - Button to check
	////////////////////////////////////////////////////
	virtual bool IsButtonDown(int nButton) const;
};

#endif //_CWIIREMOTEMANAGER_H_