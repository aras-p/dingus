// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PHYSICS_CTX_H
#define __PHYSICS_CTX_H

#include "../math/ODEMarshal.h"
#include "PhysJointGroup.h"


namespace dingus {


// --------------------------------------------------------------------------


class CPhysContext : public boost::noncopyable {
public:
	CPhysContext();
	~CPhysContext();
	
	/**
	 *  If iterations are > 0, this does stepfast. Else does step.
	 */
	void	perform( float stepsize, int iterations );

	void	addContact( const dContact& c, TPhysBodyID body1, TPhysBodyID body2 );
	int		getContactCount() const;

	TPhysWorldID	getID() const;
	void	setGravity( const SVector3& g );
	void	getGravity( SVector3& g ) const;
	void	setERP( float erp );
	float	getERP() const;
	void	setCFM( float cfm );
	float	getCFM() const;

	void	setAutoDisable(
		bool autodisable, int steps, float time,
		float linearThr, float angularThr
	);
	
	void impulseToForce( float stepsize, const SVector3& imp, SVector3& force ) const;

private:
	TPhysWorldID		mID;
	CPhysJointGroup		mContacts;
	int					mContactCount;
};



// --------------------------------------------------------------------------


inline TPhysWorldID CPhysContext::getID() const { return mID; }

inline void	CPhysContext::setGravity( const SVector3& g )
{
	dWorldSetGravity(mID,g.x,g.y,g.z);
}
inline void	CPhysContext::getGravity( SVector3& g ) const {
	dVector3 gg;
	dWorldGetGravity(mID,gg);
	odemarshal::vec3FromVector3(gg,g);
}
	
inline void	CPhysContext::setERP( float erp )
{
	dWorldSetERP(mID, erp);
}
inline float CPhysContext::getERP() const
{
	return (float)dWorldGetERP(mID);
}
	
inline void	CPhysContext::setCFM( float cfm )
{
	dWorldSetCFM(mID, cfm);
}
inline float CPhysContext::getCFM() const
{
	return (float)dWorldGetCFM(mID);
}
	
inline int CPhysContext::getContactCount() const
{
	return mContactCount;
}
	
inline void	CPhysContext::impulseToForce( float stepsize, const SVector3& imp, SVector3& force ) const
{
	dVector3 f;
	dWorldImpulseToForce(mID,stepsize,imp.x,imp.y,imp.z,f);
	odemarshal::vec3FromVector3( f, force );
}

inline void CPhysContext::addContact( const dContact& c, TPhysBodyID body1, TPhysBodyID body2 )
{
	dJointID cj = dJointCreateContact( mID, mContacts, &c );
	dJointAttach( cj, body1, body2 );
	++mContactCount;
}

inline void CPhysContext::setAutoDisable(
		bool autodisable, int steps, float time,
		float linearThr, float angularThr )
{
	dWorldSetAutoDisableFlag( mID, autodisable );
	dWorldSetAutoDisableSteps( mID, steps );
	dWorldSetAutoDisableTime( mID, time );
	dWorldSetAutoDisableLinearThreshold( mID, linearThr );
	dWorldSetAutoDisableAngularThreshold( mID, angularThr );
}


}; // namespace

#endif
