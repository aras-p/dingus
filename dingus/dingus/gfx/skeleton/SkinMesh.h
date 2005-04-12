// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SKIN_MESH_H
#define __SKIN_MESH_H

#include "../Mesh.h"
#include "SkeletonInfo.h"

namespace dingus {


// --------------------------------------------------------------------------

class CSkinBonePalette {
public:
	CSkinBonePalette() : mGroupID(-1) { };
	CSkinBonePalette( const CSkinBonePalette& r ) : mBoneIndices(r.mBoneIndices), mGroupID(r.mGroupID) { }
	void operator=( const CSkinBonePalette& r ) { mBoneIndices = r.mBoneIndices; mGroupID = r.mGroupID; }

    /// begin defining palette
    void beginPalette( int meshGroupID ) { mBoneIndices.reserve(32); mGroupID = meshGroupID; }
    /// set a bone index
    void setBone( int paletteIdx, int boneIdx ) {
		if( boneIdx < 0 )
			return;
		assert( mBoneIndices.size() == paletteIdx );
		mBoneIndices.push_back( boneIdx );
	}
    /// finish defining palette
    void endPalette() { };

	/// get mesh group index in a pre-partitioned mesh
	int getGroupID() const { return mGroupID; }

    /// get number of bones in palette
    int getBoneCount() const { return mBoneIndices.size(); }
    /// get bone index at given palette index
    int getBoneIdx( int paletteIdx ) const { return mBoneIndices[paletteIdx]; }

private:
	std::vector<int>	mBoneIndices;
	/// Mesh group index in a pre-partitioned mesh
	int		mGroupID;
};



// --------------------------------------------------------------------------

class CSkinMesh : public boost::noncopyable {
public:
	CSkinMesh();
	~CSkinMesh();

	void createResource( const CMesh& srcMesh, const CSkeletonInfo& skelInfo );
	void deleteResource();
	bool isCreated() const { return mMesh!=0 && mSkeleton!=0; }

	const CMesh& getMesh() const { return *mMesh; }
	CMesh& getMesh() { return *mMesh; }
	const CSkeletonInfo& getSkeleton() const { return *mSkeleton; }
	int getPaletteSize() const { return mPaletteSize; }
	int getBonesPerVertex() const { return mBonesPerVertex; }

	int getPaletteCount() const { return mPalettes.size(); }
	const CSkinBonePalette& getPalette( int i ) const { return mPalettes[i]; }
	
private:
	/// Partitioned mesh
	CMesh*	mMesh;
	/// Skeleton information
	const CSkeletonInfo*	mSkeleton;

	/// Bone palette size
	int		mPaletteSize;
	/// Maximum number of bones per vertex used
	int		mBonesPerVertex;
	/// Bone palettes
	std::vector<CSkinBonePalette>	mPalettes;
};


}; // namespace

#endif
