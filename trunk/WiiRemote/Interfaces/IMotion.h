////////////////////////////////////////////////////
// Wii Remote Game File
// Copyright (C), RenEvo Software & Designs, 2007
//
// IMotion.h
//
// Purpose: Interface object
//	Describes a base motion object that is updated
//	and executed when conditions are met
//
// History:
//	- 11/19/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _IMOTION_H_
#define _IMOTION_H_

#include <list>

struct SMotionElement;
struct IMotion
{
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~IMotion(void) {}

	////////////////////////////////////////////////////
	// IsFired
	//
	// Purpose: Returns TRUE if motion has fired
	////////////////////////////////////////////////////
	virtual bool IsFired(void) const = 0;

	////////////////////////////////////////////////////
	// SetPassMask
	//
	// Purpose: Set the mask used for testing pass/fired
	////////////////////////////////////////////////////
	virtual void SetPassMask(int nMask) = 0;

	////////////////////////////////////////////////////
	// GetPriority
	//
	// Purpose: Return the motion's priority level
	////////////////////////////////////////////////////
	virtual float GetPriority(void) const = 0;

	////////////////////////////////////////////////////
	// SetPriority
	//
	// Purpose: Set the motion's priority level
	//
	// In:	fPriority - Motion's priority, in case multiple
	//			motions succeed in the same gesture
	////////////////////////////////////////////////////
	virtual void SetPriority(float fPriority) = 0;

	////////////////////////////////////////////////////
	// OnBegin
	//
	// Purpose: Called when a gesture is starting
	//
	// In:	motion - Motion params
	//		nState - Current active state
	////////////////////////////////////////////////////
	virtual void OnBegin(SMotionElement const& motion, int nState) = 0;

	////////////////////////////////////////////////////
	// OnUpdate
	//
	// Purpose: Called when a gesture is updated
	//
	// In:	motion - Motion params
	//		nState - Current active state
	////////////////////////////////////////////////////
	virtual void OnUpdate(SMotionElement const& motion, int nState) = 0;

	////////////////////////////////////////////////////
	// OnEnd
	//
	// Purpose: Called when a gesture has ended
	//
	// In:	motion - Motion params
	//		nState - Current active state
	////////////////////////////////////////////////////
	virtual void OnEnd(SMotionElement const& motion, int nState) = 0;

	////////////////////////////////////////////////////
	// Execute
	//
	// Purpose: Execute the behavior for this motion
	////////////////////////////////////////////////////
	virtual void Execute(void) = 0;
};

// Motion map
typedef std::list<IMotion*> MotionList;

#endif //_IMOTION_H_