#ifndef __DEMO_MESH_ENTITY_H
#define __DEMO_MESH_ENTITY_H

#include "Entity.h"
#include "DemoResources.h"
#include <dingus/renderer/RenderableMesh.h>


// --------------------------------------------------------------------------

/**
 *  Visible entity.
 *  Can optionally render itself in possible render modes.
 *  Can check frustum cull with mesh's AABB.
 */
class CMeshEntity : public CAbstractEntity {
public:
	CMeshEntity( const std::string& name );
	virtual ~CMeshEntity();

	/// Gets AABB
	const CAABox& getAABB() const { return mMesh->getTotalAABB(); }
	/// Gets mesh
	const CMesh& getMesh() const { return *mMesh; }

	/// Culls with current W matrix and given VP matrix. Returns true if outside frustum.
	bool	frustumCull( const SMatrix4x4& viewProj ) const {
		return getAABB().frustumCull( mWorldMat, viewProj );
	}
	/// Culls with current WVP matrix. Returns true if outside frustum.
	bool	frustumCull() const {
		return getAABB().frustumCull( mWVPMat );
	}
	/// Updates WVP, adds to render context if not culled.
	void	render( eRenderMode renderMode, bool direct = false );

	CRenderableMesh* getRenderMesh( eRenderMode renderMode ) {
		return mRenderMeshes[renderMode][0];
	}

private:
	typedef std::vector<CRenderableMesh*>	TMeshVector;

	CMesh*		mMesh;
	TMeshVector	mRenderMeshes[RMCOUNT];
};


#endif
