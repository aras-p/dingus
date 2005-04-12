#ifndef __AUDIO_LISTENER_H
#define __AUDIO_LISTENER_H

#include "../math/Matrix4x4.h"

namespace dingus {


class SAudioListener {
public:
	SAudioListener()
		: velocity(0,0,0), rolloffFactor(1.0f), dopplerFactor(1.0f) { transform.identify(); }
	
	/// World space transform
	SMatrix4x4	transform;
	/// World space velocity
	SVector3	velocity;
	/// Rolloff factor
	float		rolloffFactor;
	/// Doppler factor
	float		dopplerFactor;
};


}; // namespace


#endif	  
