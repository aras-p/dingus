#ifndef __PHYSICS_H
#define __PHYSICS_H


#define PHYSICS_ODE
//#define PHYSICS_NOVODEX



#ifdef PHYSICS_ODE
#include <ode/ode.h>
typedef dBodyID TPhysObject;
typedef dGeomID	TPhysCollidable;
#endif


#ifdef PHYSICS_NOVODEX
#include <NxPhysics.h>
typedef NxActor*	TPhysObject;
typedef NxShape*	TPhysCollidable;
#endif



namespace physics {

	struct SStats {
		float	msPhys;
		float	msColl;
	};

	class CPhysObject : public boost::noncopyable {
	public:
		CPhysObject( const SMatrix4x4& matrix, const SVector3& size );
		~CPhysObject();
		void	update( SMatrix4x4& matrix );
	private:
		TPhysObject		mObject;
		TPhysCollidable	mCollidable;
	};

	void	initialize( float updDT, float grav, const SVector3& boundMin, const SVector3& boundMax );
	void	shutdown();
	void	update1();
	void	update2();

	const	SStats&		getStats();

	void	addPlane( const SMatrix4x4& matrix );

};


#endif
