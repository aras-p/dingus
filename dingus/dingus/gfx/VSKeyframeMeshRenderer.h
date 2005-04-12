// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VS_KEYFRAME_MESH_RENDERER_H
#define __VS_KEYFRAME_MESH_RENDERER_H

#include "../kernel/Proxies.h"
#include "../renderer/RenderableBuffer.h"
#include "Mesh.h"

namespace dingus {



class CVSKeyframe2MeshRenderer {
public:
	CVSKeyframe2MeshRenderer( CD3DVertexDecl& vDecl );
	
	void setKeyframe( CRenderableIndexedBuffer& renderable, CMesh& mesh0, CMesh& mesh1, float t );
	const float* getBlendParam() const { return &mBlendParam; }

private:
	CD3DVertexDecl*		mVertexDecl;
	float				mBlendParam;
};


}; // namespace


#endif
