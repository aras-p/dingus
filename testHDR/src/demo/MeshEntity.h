#ifndef __DEMO_MESH_ENTITY_H
#define __DEMO_MESH_ENTITY_H

#include "Entity.h"
#include <dingus/gfx/Mesh.h>


// --------------------------------------------------------------------------

class CMeshEntity : public CAbstractEntity {
public:
	CMeshEntity( const std::string& meshName );

	const CAABox& getAABB() const { return mMesh->getTotalAABB(); }
	const CMesh& getMesh() const { return *mMesh; }
	CMesh& getMesh() { return *mMesh; }

	/// Culls with current W matrix and given VP matrix. Returns true if outside frustum.
	bool	frustumCull( const SMatrix4x4& viewProj ) const {
		return getAABB().frustumCull( mWorldMat, viewProj );
	}
	/// Culls with current WVP matrix. Returns true if outside frustum.
	bool	frustumCull() const {
		return getAABB().frustumCull( mWVPMat );
	}

private:
	CMesh*		mMesh;
};


#endif
