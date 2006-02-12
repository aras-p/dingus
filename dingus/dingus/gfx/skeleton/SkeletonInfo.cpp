// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "SkeletonInfo.h"

using namespace dingus;

CSkeletonInfo::CBoneInfo::CBoneInfo( const SMatrix4x4& invPose, const SMatrix4x4& localPose, const std::string& name, int parentIdx )
:	mInvPoseMatrix(invPose), mLocalPoseMatrix(localPose), mName(name), mParentIndex(parentIdx)
{
}

CSkeletonInfo::CSkeletonInfo( int boneCount )
{
	assert( boneCount > 0 );
	mBones.reserve( boneCount );
}

void CSkeletonInfo::addBoneInfo( int parentIdx, const SMatrix4x4& invPoseMatrix, const std::string& name )
{
	assert( mBones.size() < mBones.capacity() );
	assert( parentIdx >= 0 && parentIdx < getBoneCount() || parentIdx == -1 );

	// bone.world = inv( bone.invpose )
	// parent.world = inv( parent.invpose )
	// bone.world = bone.local * parent.world
	// bone.local = bone.world * inv(parent.world) = inv(bone.invpose) * parent.invpose
	SMatrix4x4 localPose;
	if( parentIdx >= 0 ) {
		D3DXMatrixInverse( &localPose, NULL, &invPoseMatrix );
		localPose *= mBones[parentIdx].getInvPoseMatrix();
	} else {
		localPose.identify();
	}

	CBoneInfo b( invPoseMatrix, localPose, name, parentIdx );
	mBones.push_back( b );

	std::pair<TNameIndexMap::iterator,bool> res = mNameMap.insert( std::make_pair( name, (int)mBones.size()-1 ) );
	assert( res.second == true );
}

int CSkeletonInfo::getBoneIndex( const std::string& name ) const
{
	TNameIndexMap::const_iterator it = mNameMap.find( name );
	return (it == mNameMap.end()) ? -1 : it->second;
}
