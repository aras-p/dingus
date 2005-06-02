#ifndef __MINIMAP_RENDERER_H
#define __MINIMAP_RENDERER_H

#include <dingus/renderer/RenderableBuffer.h>


class CMinimapRenderer : public boost::noncopyable {
public:
	CMinimapRenderer( CD3DIndexBuffer& ib );
	~CMinimapRenderer();

	void	beginEntities() {
		mEntities.clear();
	}
	void	addEntity( const SVector3& pos, D3DCOLOR color, float size = 1.0f ) {
		mEntities.push_back( SEntity() );
		mEntities.back().pos = pos;
		mEntities.back().color = color;
		mEntities.back().size = size;
	}
	void	endEntities() {
	}

	void	render();

private:
	CRenderableIndexedBuffer*	mRenderable;
	CD3DIndexBuffer*	mIB;
	CD3DVertexDecl*		mVDecl;

	struct SEntity {
		SVector3	pos;
		D3DCOLOR	color;
		float		size;
	};
	std::vector<SEntity>	mEntities;
};


#endif
