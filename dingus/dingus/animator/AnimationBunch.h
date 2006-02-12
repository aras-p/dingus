// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __ANIMATION_BUNCH_H
#define __ANIMATION_BUNCH_H

#include "Animation.h"
#include "../math/Vector3.h"
#include "../math/Vector4.h"
#include "../math/Quaternion.h"


namespace dingus {

/**
 *  Named animation bunch.
 *
 *  Holds named, typed (eg. vector3, quat) animations. Permits animation
 *  lookup by name.
 *
 *  Also holds animations curve descriptors (names and parent indices). Hence
 *  all animations in particular bunch must have the same logical structure
 *  (that is, same internal hierarchy and curve names).
 */
class CAnimationBunch : public boost::noncopyable {
public:
	typedef IAnimation<SVector3>	TVector3Animation;
	typedef IAnimation<SVector4>	TVector4Animation;
	typedef IAnimation<SQuaternion>	TQuatAnimation;
	typedef IAnimation<float>		TFloatAnimation;

public:
	CAnimationBunch( const std::string& name ) : mName(name) { };
	~CAnimationBunch();

	const std::string& getName() const { return mName; }

	/// Adds named Vector3 animation.
	void addVector3Anim( const std::string& name, TVector3Animation& anim );
	/// Adds named Vector4 animation.
	void addVector4Anim( const std::string& name, TVector4Animation& anim );
	/// Adds named Quaternion animation.
	void addQuatAnim( const std::string& name, TQuatAnimation& anim );
	/// Adds named Float animation.
	void addFloatAnim( const std::string& name, TFloatAnimation& anim );

	/// Finds Vector3 animation by name, or returns NULL.
	TVector3Animation* findVector3Anim( const std::string& name ) const;
	/// Finds Vector4 animation by name, or returns NULL.
	TVector4Animation* findVector4Anim( const std::string& name ) const;
	/// Finds Quaternion animation by name, or returns NULL.
	TQuatAnimation* findQuatAnim( const std::string& name ) const;
	/// Finds Float animation by name, or returns NULL.
	TFloatAnimation* findFloatAnim( const std::string& name ) const;

	//
	// curve descriptors

	/**
	 *  Adds curve information - name, parent index
	 *
	 *  The curve's parent information must already be added. Names must be
	 *  unique. After adding all curves, call endCurves() to compute children
	 *  counts.
	 *
	 *  @param name Curve name.
	 *  @param parentIndex Index of curve's parent, or -1 if root.
	 */
	void	addCurveDesc( const std::string& name, int parentIndex );
	/// Finish adding curves, compute children counts.
	void	endCurves();
	/// Get curve count.
	int		getCurveCount() const;
	/// Get curve's name by index.
	const std::string& getCurveName( int curveIdx ) const;
	/// Gets curve's parent index, or -1 if curve is root.
	int		getCurveParent( int curveIdx ) const;
	/// Gets curve's children count.
	int		getCurveChildrenCount( int curveIdx ) const;
	/// Gets curve's index by it's name, or -1 if no such curve exists.
	int		getCurveIndexByName( const std::string& name ) const;

private:
	typedef std::map<std::string, TVector3Animation*>	TVector3AnimMap;
	typedef std::map<std::string, TVector4Animation*>	TVector4AnimMap;
	typedef std::map<std::string, TQuatAnimation*>		TQuatAnimMap;
	typedef std::map<std::string, TFloatAnimation*>		TFloatAnimMap;

	struct SCurveDesc {
		SCurveDesc( const std::string& name, int parentIdx )
			: mName(name), mParentIndex(parentIdx), mChildrenCount(0) { }
		std::string mName;
		int			mParentIndex;
		int			mChildrenCount;
	};
	typedef std::vector<SCurveDesc>		TCurveDescVector;

private:
	TVector3AnimMap		mVector3Anims;
	TVector4AnimMap		mVector4Anims;
	TQuatAnimMap		mQuatAnims;
	TFloatAnimMap		mFloatAnims;

	TCurveDescVector	mCurveDescs;

	std::string			mName;
};


// --------------------------------------------------------------------------

inline void CAnimationBunch::addVector3Anim( const std::string& name, TVector3Animation& anim )
{
	bool unique = mVector3Anims.insert( std::make_pair( name, &anim ) ).second;
	assert( unique );
}
inline void CAnimationBunch::addVector4Anim( const std::string& name, TVector4Animation& anim )
{
	bool unique = mVector4Anims.insert( std::make_pair( name, &anim ) ).second;
	assert( unique );
}
inline void CAnimationBunch::addQuatAnim( const std::string& name, TQuatAnimation& anim )
{
	bool unique = mQuatAnims.insert( std::make_pair( name, &anim ) ).second;
	assert( unique );
}
inline void CAnimationBunch::addFloatAnim( const std::string& name, TFloatAnimation& anim )
{
	bool unique = mFloatAnims.insert( std::make_pair( name, &anim ) ).second;
	assert( unique );
}

inline CAnimationBunch::TVector3Animation* CAnimationBunch::findVector3Anim( const std::string& name ) const
{
	TVector3AnimMap::const_iterator it = mVector3Anims.find( name );
	return (it!=mVector3Anims.end()) ? it->second : 0;
}
inline CAnimationBunch::TVector4Animation* CAnimationBunch::findVector4Anim( const std::string& name ) const
{
	TVector4AnimMap::const_iterator it = mVector4Anims.find( name );
	return (it!=mVector4Anims.end()) ? it->second : 0;
}
inline CAnimationBunch::TQuatAnimation* CAnimationBunch::findQuatAnim( const std::string& name ) const
{
	TQuatAnimMap::const_iterator it = mQuatAnims.find( name );
	return (it!=mQuatAnims.end()) ? it->second : 0;
}
inline CAnimationBunch::TFloatAnimation* CAnimationBunch::findFloatAnim( const std::string& name ) const
{
	TFloatAnimMap::const_iterator it = mFloatAnims.find( name );
	return (it!=mFloatAnims.end()) ? it->second : 0;
}

inline void CAnimationBunch::addCurveDesc( const std::string& name, int parentIndex )
{
	assert( parentIndex == -1 || (parentIndex >= 0 && parentIndex < mCurveDescs.size() ) );
	assert( getCurveIndexByName(name) == -1 );
	mCurveDescs.push_back( SCurveDesc( name, parentIndex ) );
}
inline int CAnimationBunch::getCurveCount() const
{
	return (int)mCurveDescs.size();
}
inline const std::string& CAnimationBunch::getCurveName( int curveIdx ) const
{
	assert( curveIdx >= 0 && curveIdx < mCurveDescs.size() );
	return mCurveDescs[curveIdx].mName;
}
inline int CAnimationBunch::getCurveParent( int curveIdx ) const
{
	assert( curveIdx >= 0 && curveIdx < mCurveDescs.size() );
	return mCurveDescs[curveIdx].mParentIndex;
}
inline int CAnimationBunch::getCurveChildrenCount( int curveIdx ) const
{
	assert( curveIdx >= 0 && curveIdx < mCurveDescs.size() );
	return mCurveDescs[curveIdx].mChildrenCount;
}
inline int CAnimationBunch::getCurveIndexByName( const std::string& name ) const
{
	int n = (int)mCurveDescs.size();
	for( int i = 0; i < n; ++i ) {
		if( mCurveDescs[i].mName == name )
			return i;
	}
	return -1;
}

}; // namespace

#endif
