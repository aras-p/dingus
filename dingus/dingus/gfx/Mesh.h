// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __GFX_MESH_H
#define __GFX_MESH_H

#include "VertexFormat.h"
#include "../math/Vector3.h"
#include "../math/AABox.h"
#include "../kernel/Proxies.h"


namespace dingus {


class CMesh : public boost::noncopyable {
public:
	enum eBufferType { BUF_STATIC = 0, BUF_SYSTEMMEM };

public:
	/**
	 *  A subset of a mesh. Mesh can have multiple subsets, eg. when portions
	 *  of it have different materials. Contains AABB of the subset.
	 */
	class CGroup {
	public:
		CGroup( int firstVert, int vertCount, int firstPrim, int primCount )
			: mFirstVertex(firstVert), mVertexCount(vertCount), mFirstPrim(firstPrim), mPrimCount(primCount), mAABB() { }
		int getFirstVertex() const { return mFirstVertex; }
		int getVertexCount() const { return mVertexCount; }
		int getFirstPrim() const { return mFirstPrim; }
		int getPrimCount() const { return mPrimCount; }
		const CAABox& getAABB() const { return mAABB; }
		CAABox& getAABB() { return mAABB; }
	private:
		int		mFirstVertex,	mVertexCount;
		int		mFirstPrim,		mPrimCount;
		CAABox	mAABB;
	};
	typedef std::vector<CGroup>	TGroupVector;

public:
	CMesh();
	~CMesh();

	void	createResource( int vertCount, int idxCount,
		const CVertexFormat& vertFormat, int indexStride, CD3DVertexDecl& vertDecl, eBufferType bufferType );
	void	deleteResource();
	bool	isCreated() const { return !mVB.isNull() && !mIB.isNull(); }

	int		getVertexCount() const { return mVertexCount; }
	int		getIndexCount() const { return mIndexCount; }
	
	const CVertexFormat& getVertexFormat() const { return mVertexFormat; }
	int		getVertexStride() const { return mVertexStride; }
	int		getIndexStride() const { return mIndexStride; }
	CD3DVertexDecl& getVertexDecl() const { return *mVertexDecl; }

	const CD3DVertexBuffer&	getVB() const { return mVB; }
	CD3DVertexBuffer&	getVB() { return mVB; }
	const CD3DIndexBuffer&	getIB() const { return mIB; }
	CD3DIndexBuffer&	getIB() { return mIB; }
	const void*	lockVBRead() const;
	void	unlockVBRead() const;
	void*	lockVBWrite();
	void	unlockVBWrite();
	const void*	lockIBRead() const;
	void	unlockIBRead() const;
	void*	lockIBWrite();
	void	unlockIBWrite();

	/// Creates D3DX mesh from this mesh. May be useful for tools.
	ID3DXMesh*	createD3DXMesh() const;

	/**
	 *  Get triangle vertex indices.
	 *  Index buffer must be locked for reading!
	 */
	void	getTriIndices( const void* lockedIB, int tri, int& idx0, int& idx1, int& idx2 ) const;
	
	void	computeAABBs();
	const CAABox& getTotalAABB() const { return mTotalAABB; }
	CAABox& getTotalAABB() { return mTotalAABB; }

	int		getGroupCount() const { return (int)mGroups.size(); }
	const CGroup& getGroup( int i ) const { assert(i>=0&&i<getGroupCount()); return mGroups[i]; }
	CGroup& getGroup( int i ) { assert(i>=0&&i<getGroupCount()); return mGroups[i]; }
	void	reserveGroups( int count ) { mGroups.reserve(count); }
	void	addGroup( const CGroup& g ) { mGroups.push_back(g); }

private:
	// buffers type
	eBufferType		mBufferType;
	// vertex/index count
	int				mVertexCount;
	int				mIndexCount;
	// formats/strides
	CVertexFormat	mVertexFormat;
	int				mVertexStride; // in bytes, depends on format
	int				mIndexStride;
	CD3DVertexDecl*	mVertexDecl;
	// VB/IB
	CD3DVertexBuffer	mVB;
	CD3DIndexBuffer		mIB;
	// groups
	TGroupVector	mGroups;
	// total AABB
	CAABox			mTotalAABB;

	// for assertions
	enum eLock { NOLOCK, LOCK_READ, LOCK_WRITE };
	mutable eLock	mVBLock;
	mutable eLock	mIBLock;
};


}; // namespace


#endif
