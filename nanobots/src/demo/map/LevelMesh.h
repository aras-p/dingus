#ifndef __LEVEL_MESH_H
#define __LEVEL_MESH_H

#include <dingus/gfx/Vertices.h>
#include <dingus/renderer/RenderableMesh.h>
#include "../DemoResources.h"
#include "Collider.h"

class CGameMap;
class CSubdivMesh;
class CTriangleMesh;
class CGameMapMeshData;
class CRenderableLevelMesh;
class CGameMapMeshCreator;

// --------------------------------------------------------------------------


class CLevelMesh : public boost::noncopyable {
public:
	typedef SVertexXyzDiffuse	TVertex;
	enum eMeshMode { FULL=0, NOTOP, MESHMODECOUNT };
public:
	CLevelMesh( const CGameMap& gameMap );
	~CLevelMesh();

	void	updateDetailLevel( int level );

	void	render( eRenderMode renderMode, eMeshMode meshMode );
	void	renderMinimap();

	const CCollisionMesh&	getColMesh() const { return *mColMesh; }

	/**
	 *  Given a sphere, adjusts it's position so that it doesn't collide
	 *  with the level mesh.
	 */
	void	fitSphere( SVector3& pos, float radius ) const;

	/**
	 *  Given a sphere, returns true if it collides.
	 */
	bool	collideSphere( SVector3& pos, float radius ) const;

	/**
	 *  Given a sphere, returns triangles it collides with.
	 */
	void	collideSphere( SVector3& pos, float radius, std::vector<int>& tris ) const;

private:
	void	initSubdivMesh( CSubdivMesh& mesh );

	/// @return true if loaded from cache
	bool	loadCachedData();
	void	saveDataToCache();
	/// Computes data from scratch
	void	computeData();

private:
	const CGameMap*		mGameMap;
	CTriangleMesh*		mMeshTri[GFX_DETAIL_LEVELS];
	CGameMapMeshData*		mMeshData[GFX_DETAIL_LEVELS][MESHMODECOUNT];
	CRenderableLevelMesh*	mRenderable[MESHMODECOUNT][RMCOUNT];
	CGameMapMeshCreator*	mMeshCreator[MESHMODECOUNT];

	bool	mLoadedFromCache;

	CRenderableMesh*	mRenderableMinimap;

	mutable CCollisionSphere		mColSphere;
	mutable CMeshSphereCollider2	mMeshSphereCollider;
	mutable CCollisionResult		mColResult;
	CCollisionMesh*					mColMesh;
};


#endif
