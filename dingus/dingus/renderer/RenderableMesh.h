// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RENDER_MESH_H
#define __RENDER_MESH_H

#include "Renderable.h"
#include "../gfx/Mesh.h"
#include "../utils/MemoryPool.h"


namespace dingus {


class CRenderableMesh : public CRenderable {
public:
	enum { ALL_GROUPS = -1 };
public:
	CRenderableMesh( CMesh& mesh, int group, const SVector3* origin = 0, int priority = 0 );

	const CMesh& getMesh() const { return *mMesh; }
	CMesh& getMesh() { return *mMesh; }
	void setGroup( int g ) { mGroup = g; }
	int getGroup() const { return mGroup; }

	virtual void render( const CRenderContext& ctx );
	
	virtual const CD3DVertexBuffer*	getUsedVB() const { return &mMesh->getVB(); }
	virtual const CD3DIndexBuffer*	getUsedIB() const { return &mMesh->getIB(); }
	
private:
	DECLARE_POOLED_ALLOC(dingus::CRenderableMesh);
private:
	CMesh*	mMesh;
	int		mGroup;
};


}; // namespace


#endif
