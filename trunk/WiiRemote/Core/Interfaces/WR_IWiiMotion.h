////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_IWiiMotion.h
//
// Purpose: Interface object
//	Describes a helper for the Wii Remote that manages
//	motion detection
//
// History:
//	- 11/3/07 : File created - KAK
////////////////////////////////////////////////////

#ifndef _WR_IWIIMOTION_H_
#define _WR_IWIIMOTION_H_

// 1G Gravity point epsilon (for detecting motion)
#define WR_MOTION_1GEPSILON (0.2f)
#define WR_MOTION_GESTUREEPSILON (0.1f)

// WR_WIIMOTION_LSBOFFSETS
//	Offsets into buffer where LSB data is stored
enum WR_WIIMOTION_LSBOFFSETS
{
	WR_LSBOFFSET_X = 0,
	WR_LSBOFFSET_Y = 1,
	WR_LSBOFFSET_Z = 1,
};

// WR_WIIMOTION_LSBMASKS
//	Masks to get LSB data out
enum WR_WIIMOTION_LSBMASKS
{
	WR_LSBMASK_X = 0x40,
	WR_LSBMASK_X_SHIFT = 6,
	WR_LSBMASK_Y = 0x20,
	WR_LSBMASK_Y_SHIFT = 5,
	WR_LSBMASK_Z = 0x40,
	WR_LSBMASK_Z_SHIFT = 6,
};

// Vector helper
template <typename T> struct SMOTIONVECT
{
	T x,y,z;
	SMOTIONVECT(void) : x(0),y(0),z(0) {}
	SMOTIONVECT(T _x, T _y, T _z) : x(_x),y(_y),z(_z) {}
	SMOTIONVECT(SMOTIONVECT<T> const& v) : x(v.x),y(v.y),z(v.z) {}
	void Set(T _x = 0, T _y = 0, T _z = 0) { x=_x; y=_y; z=_z; }

#define MATH_OPERATOR(op) \
	SMOTIONVECT<T> operator op(SMOTIONVECT<T> const& v) \
	{ \
		SMOTIONVECT<T> temp; temp.x = x op v.x; temp.y = y op v.y; temp.z = z op v.z; return temp; \
	} \
	void operator op##=(SMOTIONVECT<T> const& v) \
	{ \
	x op##= v.x; y op##= v.y; z op##= v.z; \
	} \
	SMOTIONVECT<T> operator op(T const& v) \
	{ \
		SMOTIONVECT<T> temp; temp.x = x op v; temp.y = y op v; temp.z = z op v; return temp; \
	} \
	void operator op##=(T const& v) \
	{ \
	x op##= v; y op##= v; z op##= v; \
	}

	// Operator overloads
	bool operator ==(SMOTIONVECT<T> const& v)
	{
		return (x==v.x&&y==v.y&&z==v.z);
	}
	bool operator !=(SMOTIONVECT<T> const& v)
	{
		return !(*this == v);
	}
	MATH_OPERATOR(+);
	MATH_OPERATOR(-);
	MATH_OPERATOR(*);
	MATH_OPERATOR(/);

#undef MATH_OPERATOR
};
typedef SMOTIONVECT<float> SMotionVec3F;
typedef SMOTIONVECT<unsigned char> SMotionVec3;

// Motion element
struct SMotionElement
{
	int nLifetime;
	float fGForce;
	float fPitch, fRoll;
	SMotionVec3F vAccel, vDir;
};

// Motion queue
typedef std::queue<SMotionElement> WiiMotionQueue;

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Motion Listener
struct IWR_WiiRemote;
struct IWR_WiiMotion;
struct IWR_WiiMotionListener
{
	////////////////////////////////////////////////////
	// OnSingleMotion
	//
	// Purpose: Called when the controller has moved
	//
	// In:	pRemote - Controller object
	//		pMotion - Motion helper
	//		motion - Motion element
	////////////////////////////////////////////////////
	virtual void OnSingleMotion(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion) = 0;

	////////////////////////////////////////////////////
	// OnMotionStart
	//
	// Purpose: Called when a motion has started
	//
	// In:	pRemote - Controller object
	//		pMotion - Motion helper
	//		motion - Motion element
	////////////////////////////////////////////////////
	virtual void OnMotionStart(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion) = 0;

	////////////////////////////////////////////////////
	// OnMotionUpdate
	//
	// Purpose: Called when a motion has been updated
	//
	// In:	pRemote - Controller object
	//		pMotion - Motion helper
	//		motion - Motion element
	////////////////////////////////////////////////////
	virtual void OnMotionUpdate(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion) = 0;

	////////////////////////////////////////////////////
	// OnMotionEnd
	//
	// Purpose: Called when a motion has ended
	//
	// In:	pRemote - Controller object
	//		pMotion - Motion helper
	//		motion - Motion element
	////////////////////////////////////////////////////
	virtual void OnMotionEnd(IWR_WiiRemote *pRemote, IWR_WiiMotion *pMotion, SMotionElement const& motion) = 0;
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Motion state flags
enum WR_WIIMOTION_FLAGS
{
	WMF_ISCALIBRATED = 0x01,		// Set if motion has been calibrated
	WMF_ACTIVEMOTION = 0x02,		// Set if motion is active based on min size
};

struct IWR_WiiMotion
{
	////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////
	virtual ~IWR_WiiMotion(void) { }

	////////////////////////////////////////////////////
	// AddListener
	//
	// Purpose: Add a listener
	//
	// In:	pListener - Listener to add
	////////////////////////////////////////////////////
	virtual void AddListener(IWR_WiiMotionListener *pListener) = 0;

	////////////////////////////////////////////////////
	// RemoveListener
	//
	// Purpose: Remove a listener
	//
	// In:	pListener - Listener to remove
	////////////////////////////////////////////////////
	virtual void RemoveListener(IWR_WiiMotionListener *pListener) = 0;

	////////////////////////////////////////////////////
	// Calibrate
	//
	// Purpose: Call to calibrate the motion
	////////////////////////////////////////////////////
	virtual void Calibrate(void) = 0;

	////////////////////////////////////////////////////
	// IsCalibrated
	//
	// Purpose: Returns TRUE if the remote has been
	//	calibrated yet
	////////////////////////////////////////////////////
	virtual bool IsCalibrated(void) const = 0;

	////////////////////////////////////////////////////
	// SetMotionSize
	//
	// Purpose: Set the minimum size (consecutive updates)
	//	for a motion to be reported
	//
	// In:	nSize - Size (in number of consecutive updates)
	////////////////////////////////////////////////////
	virtual void SetMotionSize(int nSize) = 0;

	////////////////////////////////////////////////////
	// GetMotionSize
	//
	// Purpose: Get the minimum size (consecutive updates)
	//	for a motion to be reported
	////////////////////////////////////////////////////
	virtual int GetMotionSize(void) const = 0;

	////////////////////////////////////////////////////
	// IsMotionActive
	//
	// Purpose: Returns TRUE if a motion is active
	////////////////////////////////////////////////////
	virtual bool IsMotionActive(void) const = 0;

	////////////////////////////////////////////////////
	// GetMotionLifetime
	//
	// Returns the current motion's lifetime or 0 if no
	//	motion is active right now
	////////////////////////////////////////////////////
	virtual int GetMotionLifetime(void) const = 0;

	////////////////////////////////////////////////////
	// GetAcceleration
	//
	// Purpose: Get the acceleration vector on the remote
	//	as it was on the last update
	//
	// Out: v - Acceleration vector
	////////////////////////////////////////////////////
	virtual void GetAcceleration(SMotionVec3F &v) const = 0;

	////////////////////////////////////////////////////
	// GetAccelerationXYZ
	//
	// Purpose: Get a component of the acceleration vector
	////////////////////////////////////////////////////
	virtual float GetAccelerationX(void) const = 0;
	virtual float GetAccelerationY(void) const = 0;
	virtual float GetAccelerationZ(void) const = 0;

	////////////////////////////////////////////////////
	// GetDirection
	//
	// Purpose: Get the direction vector on the remote
	//	as it was on the last update
	//
	// Out: v - Direction vector
	////////////////////////////////////////////////////
	virtual void GetDirection(SMotionVec3F &v) const = 0;

	////////////////////////////////////////////////////
	// GetDirectionXYZ
	//
	// Purpose: Get a component of the direction vector
	////////////////////////////////////////////////////
	virtual float GetDirectionX(void) const = 0;
	virtual float GetDirectionY(void) const = 0;
	virtual float GetDirectionZ(void) const = 0;

	////////////////////////////////////////////////////
	// GetPitch
	//
	// Purpose: Get pitch component of rotation
	////////////////////////////////////////////////////
	virtual float GetPitch(void) const = 0;

	////////////////////////////////////////////////////
	// GetRoll
	//
	// Purpose: Get roll component of rotation
	////////////////////////////////////////////////////
	virtual float GetRoll(void) const = 0;

protected:
	////////////////////////////////////////////////////
	// Initialize
	//
	// Purpose: Initialize to prepare for motion status
	//	reports
	//
	// In:	pRemote - Owning remote controller
	//
	// Returns TRUE on success, FALSE on error
	////////////////////////////////////////////////////
	virtual bool Initialize(IWR_WiiRemote *pRemote) = 0;

	////////////////////////////////////////////////////
	// Shutdown
	//
	// Purpose: Clean up
	////////////////////////////////////////////////////
	virtual void Shutdown(void) = 0;

	////////////////////////////////////////////////////
	// OnMotionUpdate
	//
	// Purpose: Call when motion is updated on the remote
	//
	// In:	buffer - Recv buffer containing motion
	//			status
	//		nOffset - Offset into buffer where data is
	//		fCurrTick - Current tick time
	////////////////////////////////////////////////////
	virtual void OnMotionUpdate(DataBuffer const& buffer, int nOffset, float fCurrTick) = 0;

	////////////////////////////////////////////////////
	// OnPostUpdate
	//
	// Purpose: Finalize update
	//
	// Note: Should be called at end of Update irregardless
	//	if OnMotionUpdate is called
	////////////////////////////////////////////////////
	virtual void OnPostUpdate(void) = 0;
};

#endif //_WR_IWIIMOTION_H_