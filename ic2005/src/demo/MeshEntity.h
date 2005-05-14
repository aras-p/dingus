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
	CMeshEntity( const std::string& descName );
	CMeshEntity( const std::string& descName, const std::string& meshName );
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
		TMeshVector& rmv = mRenderMeshes[renderMode];
		if( rmv.empty() )
			return NULL;
		return rmv[0];
	}

	const std::string& getDescName() const { return mDescName; }

private:
	void	init( const std::string& descName, const std::string& meshName );

private:
	typedef std::vector<CRenderableMesh*>	TMeshVector;

	CMesh*		mMesh;
	TMeshVector	mRenderMeshes[RMCOUNT];

	std::string	mDescName;
};


#endif
