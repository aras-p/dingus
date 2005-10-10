#ifndef __STREAM_IMPOSTORS_RENDERER_H
#define __STREAM_IMPOSTORS_RENDERER_H

#include <dingus/renderer/RenderableBuffer.h>


class CStreamImpostorsRenderer : public boost::noncopyable {
public:
	CStreamImpostorsRenderer( CD3DIndexBuffer& ib );
	~CStreamImpostorsRenderer();

	void	beginEntities() {
		mEntities.clear();
	}
	void	addEntity( const SMatrix4x4& matrix, float alpha, int type ) {
		if( alpha <= 1.0f/255.0f )
			return;
		mEntities.push_back( SEntity() );
		mEntities.back().matrix = &matrix;
		mEntities.back().alpha = alpha;
		mEntities.back().type = type;
	}
	void	endEntities() {
	}

	void	render();

private:
	CRenderableIndexedBuffer*	mRenderable;
	CD3DIndexBuffer*	mIB;
	CD3DVertexDecl*		mVDecl;

	struct SEntity {
		const SMatrix4x4* matrix;
		float		alpha;
		float		type;
	};
	std::vector<SEntity>	mEntities;
};


#endif
