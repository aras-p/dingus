// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SKELETON_INFO_H
#define __SKELETON_INFO_H

#include "../../math/Matrix4x4.h"


namespace dingus {
	

/**
 *  A boned skeleton information.
 *
 *  This is shared object that contains hierarchy setup, bone names and pose
 *  matrices.
 *
 *  Bones must be laid out in a depth-first traversal order (so that from any
 *  bone all it's children are right after him).
 */
class CSkeletonInfo : public boost::noncopyable {
public:
	/// Single bone information (inv pose matrix, name, parent index).
	class CBoneInfo {
	public:
		CBoneInfo( const SMatrix4x4& invPose, const SMatrix4x4& localPose, const std::string& name, int parentIdx );

		const SMatrix4x4& getInvPoseMatrix() const { return mInvPoseMatrix; }
		const SMatrix4x4& getLocalPoseMatrix() const { return mLocalPoseMatrix; }
		const std::string& getName() const { return mName; }
		int		getParentIndex() const { return mParentIndex;  }
		bool	hasParent() const { return mParentIndex >= 0; }
	private:
		/// Inverse of bone's world matrix, when in skin pose.
		SMatrix4x4	mInvPoseMatrix;
		/// Local matrix, when in skin pose
		SMatrix4x4	mLocalPoseMatrix;
		std::string mName;
		int			mParentIndex;
	};
public:
	typedef std::map<std::string,int>	TNameIndexMap;
	typedef std::vector<CBoneInfo>	TBoneInfoVector;

public:
	/**
	 *  Initialize fixed-size skeleton info. Note that the bones info won't be
	 *  initialized, just created.
	 */
	CSkeletonInfo( int boneCount );

	void addBoneInfo( int parentIdx, const SMatrix4x4& invPoseMatrix, const std::string& name );

	int getBoneCount() const { return mBones.size(); }
	const CBoneInfo& getBone( int idx ) const { assert(idx>=0 && idx<mBones.size()); return mBones[idx]; }

	/// @return -1 if not found
	int getBoneIndex( const std::string& name ) const;

private:
	TBoneInfoVector	mBones;
	TNameIndexMap	mNameMap;
};


	
}; // namespace

#endif
