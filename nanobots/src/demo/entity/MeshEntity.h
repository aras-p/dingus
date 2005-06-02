#ifndef __DEMO_MESH_ENTITY_H
#define __DEMO_MESH_ENTITY_H

#include "Entity.h"
#include "../DemoResources.h"
#include <dingus/renderer/RenderableMesh.h>


// --------------------------------------------------------------------------

/**
 *  Visible entity.
 *  Can optionally render itself in possible render modes.
 *  Can check frustum cull with mesh's AABB.
 */
class CMeshEntity : public CAbstractEntity {
public:
	typedef std::vector<CRenderableMesh*>	TMeshVector;
public:
	CMeshEntity( const std::string& name, int lodCount );
	virtual ~CMeshEntity();

	/// Gets AABB
	const CAABox& getAABB() const { return mMesh->getTotalAABB(); }

	/// Culls with current W matrix and given VP matrix. Returns true if outside frustum.
	bool	frustumCull( const SMatrix4x4& viewProj ) const {
		return getAABB().frustumCull( mWorldMat, viewProj );
	}
	/// Culls with current WVP matrix. Returns true if outside frustum.
	bool	frustumCull() const {
		return getAABB().frustumCull( mWVPMat );
	}
	/// Updates WVP, adds to render context if not culled.
	void	render( eRenderMode renderMode, int lod, bool direct/* = false*/ );

	int		getLodCount() const { return mLodCount; }

	TMeshVector* getRenderMeshes( eRenderMode renderMode, int lod ) {
		assert( lod >= 0 && lod < mLodCount );
		if( mRenderData[renderMode].renderMeshes == NULL )
			return NULL;
		return &mRenderData[renderMode].renderMeshes[lod];
	}

private:
	struct SRenderModeData {
		TMeshVector*	renderMeshes; // vector for each LOD
	};
	SRenderModeData	mRenderData[RMCOUNT];
	CMesh*			mMesh;
	int				mLodCount;
};


#endif
