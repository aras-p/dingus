#ifndef __ENTITY_INFO_RENDERER_H
#define __ENTITY_INFO_RENDERER_H

#include <dingus/renderer/RenderableBuffer.h>


class CEntityInfoRenderer : public boost::noncopyable {
public:
	CEntityInfoRenderer( CD3DIndexBuffer& ib );
	~CEntityInfoRenderer();

	void	beginInfos() {
		mBars.clear();
		mBrackets.clear();
	}
	void	addBar( const SVector3& pos, float dy, D3DCOLOR color, float fill, float size = 1.0f ) {
		mBars.push_back( SBar() );
		SBar& b = mBars.back();
		b.pos = pos;
		b.dy = dy;
		b.color = color;
		b.fill = fill;
		b.size = size;
	}
	void	addBracket( const SVector3& pos, float dy, D3DCOLOR color, float size = 2.0f ) {
		mBrackets.push_back( SBracket() );
		SBracket& b = mBrackets.back();
		b.pos = pos;
		b.dy = dy;
		b.color = color;
		b.size = size;
	}
	void	endInfos() {
	}

	void	render();

private:
	CRenderableIndexedBuffer*	mRenderable;
	CD3DIndexBuffer*	mIB;
	CD3DVertexDecl*		mVDecl;

	// health bars
	struct SBar {
		SVector3	pos;
		float		dy;
		D3DCOLOR	color;
		float		fill;
		float		size;
	};
	std::vector<SBar>	mBars;

	// selection brackets
	struct SBracket {
		SVector3	pos;
		float		dy;
		D3DCOLOR	color;
		float		size;
	};
	std::vector<SBracket>	mBrackets;
};


#endif
