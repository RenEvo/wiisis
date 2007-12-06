////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_Implementation.h
//
// Purpose: Main implementation of the Wii Remote
//			Core API
//
// History:
//	- 11/1/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_IMPLEMENTATION_H_
#define _WR_IMPLEMENTATION_H_

// Wii Remote Vendor and Product IDs
#define WR_VENDORID		(0x057e)
#define WR_PRODUCTID	(0x0306)

// Success code used by all modules
#define WR_ERROR_SUCCESS (0)
#define WR_SUCCESS(s) (WR_ERROR_SUCCESS == (s))
#define WR_FAIL(s) (WR_ERROR_SUCCESS != (s))

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <crtdbg.h>
#include <cmath>
#include <string>
#include <list>
#include <queue>
#include <map>

// Core files
#include "Interfaces\WR_ITimer.h"
#include "Interfaces\WR_IHIDController.h"
#include "Interfaces\WR_IWiiRemote.h"
#include "Interfaces\WR_IWiiButtons.h"
#include "Interfaces\WR_IWiiMotion.h"
#include "Interfaces\WR_IWiiData.h"
#include "Interfaces\WR_IWiiExtension.h"
#include "Interfaces\WR_IWiiSensor.h"

// Extension files
#include "WR_CWiiNunchuk.h"

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Helper macros

// assert - Debug assertion
#ifndef assert
	#define assert _ASSERT
#endif //assert

// MIN - Get number in minimum range
#ifndef MIN
	#define MIN(num,min) (((num)<(min))?(num):(min))
#endif //MIN

// MAX - Get number in maximum range
#ifndef MAX
	#define MAX(num,max) (((num)<(max))?(max):(num))
#endif //MAX

// CLAMP - Get number within inclusive range
#ifndef CLAMP
	#define CLAMP(num,min,max) (((num)<(min))?(min):(((num)>(max))?(max):(num)))
#endif //CLAMP

// NEGSATURATE - Get number within inclusive range from [-1,1]
#ifndef NEGSATURATE
	#define NEGSATURATE(num) CLAMP(num,-1.0f,1.0f)
#endif //NEGSATURATE

// CHECK_BITS - Check if bits are on
#ifndef CHECK_BITS
	#define CHECK_BITS(bit, mask) ((bit)==((mask)&(bit)))
#endif //CHECK_BITS

// SET_BITS - Turn bits on
#ifndef SET_BITS
	#define SET_BITS(bit, mask) ((mask)|(bit))
#endif //SET_BITS

// CLEAR_BITS - Turn bits off
#ifndef CLEAR_BITS
	#define CLEAR_BITS(bit, mask) ((mask)&~(bit))
#endif //CLEAR_BITS

// PI - Math is fun!
#ifndef PI
	#define PI (3.14159265f)
#endif //PI

// RADTODEG - Convert number from radians to degrees
#ifndef RADTODEG
	#define RADTODEG(num) ((num)*(180.0f/PI))
#endif //RADTODEG

// DEGTORAD - Convert number from degrees to radians
#ifndef DEGTORAD
	#define DEGTORAD(num) ((num)*(PI/180.0f))
#endif //DEGTORAD

// SAFEDELETE - Safely free memory back to heap
#ifndef SAFE_DELETE
	#define SAFE_DELETE(p) \
		if (NULL != (p)) \
		{ \
			delete (p); \
			(p) = NULL; \
		}
#endif //SAFE_DELETE

// SAFEDELETE - Safely free blocked memory back to heap
#ifndef SAFE_DELETE_ARRAY
	#define SAFE_DELETE_ARRAY(p) \
		if (NULL != (p)) \
		{ \
			delete [] (p); \
			(p) = NULL; \
		}
#endif //SAFE_DELETE_ARRAY

// SAFE_RELEASE - Safely Release() an instance
#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(p) \
		if (NULL != (p)) \
		{ \
			(p)->Release(); \
			(p) = NULL; \
		}
#endif //SAFE_RELEASE

// IS_POW2 - Determine if number is a power of 2
#ifndef IS_POW2
	#define IS_POW2(num) (0 == ((num) & ((num)-1)))
#endif // IS_POW2

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Module IDs, used for error checking
enum eModule
{
	WR_NONE,
	WR_HIDCONTROLLER,
	WR_WIIREMOTE,
	WR_WIITIMER,
};
static char const* szModules[] =
{
	"None",
	"HIDController",
	"WiiRemote",
	"WiiTimer",
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Error listener
struct IWR_ErrorListener
{
	////////////////////////////////////////////////////
	// OnError
	//
	// Purpose: Called when an error occurs
	//
	// In:	nModule - Module ID
	//		nError - Error ID
	//		szErrorMsg - Error message
	////////////////////////////////////////////////////
	virtual void OnError(int nModule, int nError, char const* szErrorMsg) = 0;
};

// Global instance
class CWR_GlobalInstance
{
private:
	////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////
	CWR_GlobalInstance(void);
	CWR_GlobalInstance(CWR_GlobalInstance const&) {}
	CWR_GlobalInstance& operator =(CWR_GlobalInstance const&) {return *this;}

	// Single instance
	static CWR_GlobalInstance m_Instance;

	// Error status
	struct SErrorDef
	{
		int nModule;
		int nError;
		char const** szErrorMsgs;
		std::string szErrorMsg;
	} m_ErrorDef;

	// Listeners
	typedef std::list<IWR_ErrorListener*> Listeners;
	Listeners m_ErrorListeners;

public:
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~CWR_GlobalInstance(void);

	////////////////////////////////////////////////////
	// GetInstance
	//
	// Purpose: Return single instance
	////////////////////////////////////////////////////
	static CWR_GlobalInstance* GetInstance(void);

	////////////////////////////////////////////////////
	// GetLastError
	//
	// Purpose: Returns the last error that occured
	//
	// Out: nModule - Module that signaled the erorr
	//		nError - Error that occured
	////////////////////////////////////////////////////
	virtual void GetLastError(int &nModule, int &nError);

	////////////////////////////////////////////////////
	// GetLastErrorString
	//
	// Purpose: Returns the last error string
	//
	// In:	nSize - Size of string buffer
	//
	// Out:	szError - Error message
	//
	// Returns actual length of error message or 0 if
	//	no error occured
	////////////////////////////////////////////////////
	virtual int GetLastErrorString(char *szError, int nSize = MAX_PATH);

	////////////////////////////////////////////////////
	// RaiseError
	//
	// Purpose: Raise an error
	//
	// In:	nModule - Module ID
	//		nError - Error code
	//		szErrorMsgs - Optional pointer to array
	//			of error message strings that correspond
	//			to error
	////////////////////////////////////////////////////
	virtual void RaiseError(int nModule, int nError, char const** szErrorMsgs = NULL);

	////////////////////////////////////////////////////
	// RaiseManualError
	//
	// Purpose: Raise a manual error
	//
	// In:	szError - Error message
	//		nError - Error number
	////////////////////////////////////////////////////
	virtual void RaiseManualError(char const* szError, int nError = -1);

	////////////////////////////////////////////////////
	// AddErrorListener
	//
	// Purpose: Add an error listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddErrorListener(IWR_ErrorListener *pListener);

	////////////////////////////////////////////////////
	// RemoveErrorListener
	//
	// Purpose: Remove an error listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveErrorListener(IWR_ErrorListener *pListener);

	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize the core objects
	//
	// Returns TRUE on success, FALSE on error
	////////////////////////////////////////////////////
	virtual bool Initialize(void);

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Destroy the core objects
	////////////////////////////////////////////////////
	virtual void Shutdown(void);

	////////////////////////////////////////////////////
	// Update
	//
	// Purpose: Update the core objects
	////////////////////////////////////////////////////
	virtual void Update(void);

public:
	// Core files
	IWR_HIDController *pHIDController;
	IWR_Timer *pTimer;
};

#define SETUP_WR_MODULE() \
	static int _nWRMODID; \
	static char const** _szWRMODERRORS;

#define REGISTER_WR_MODULE(class, module) \
	int class::_nWRMODID = WR_ ## module; \
	char const** class::_szWRMODERRORS = WR_ ## module ## _ERRORSTR;

#define WR_RAISEERROR(error) \
	{ \
		g_pWR->RaiseError(_nWRMODID, error, _szWRMODERRORS); \
		return error; \
	}
#define WR_RAISEERROR_NORET(error) g_pWR->RaiseError(_nWRMODID, error, _szWRMODERRORS);

////////////////////////////////////////////////////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// GetWiiRemoteSystem
//
// Purpose: Returns the Wii Remote System implementation
//	object
////////////////////////////////////////////////////
CWR_GlobalInstance* GetWiiRemoteSystem(void);

extern CWR_GlobalInstance *g_pWR;


////////////////////////////////////////////////////
////////////////////////////////////////////////////

#endif //_WR_IMPLEMENTATION_H_