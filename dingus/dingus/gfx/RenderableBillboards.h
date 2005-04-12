// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RENDER_BILLBOARDS_H
#define __RENDER_BILLBOARDS_H

#include "../kernel/Proxies.h"
#include "../renderer/Renderable.h"
#include "../utils/Pool.h"
#include "geometry/VBChunk.h"
#include "geometry/VBManagerSource.h"


namespace dingus {



struct SVertexXyzDiffuseTex1;
struct SMatrix4x4;



struct SBillboard {
	friend class CRenderableBillboards;
public:
	void	setWholeTexture() { tu1=tv1=0; tu2=tv2=1; }
	void	discard() { discarded = true; }
public:
	float			x1, y1, x2, y2;
	float			tu1, tv1, tu2, tv2;
	D3DCOLOR		color;
private:
	CD3DTexture*	texture;
	bool			discarded;
};


/**
 *  Billboard renderer.
 *
 *  Renders billboards (aka sprites, or screen-aligned textured
 *  quads).
 */
class CRenderableBillboards : public CRenderable, public IRenderListener {
public:
	CRenderableBillboards( CD3DIndexBuffer& ib, CEffectParams::TParamName texParamName );
	virtual ~CRenderableBillboards();
	
	SBillboard&	addBill( CD3DTexture& texture );

	// IRenderListener
	virtual void beforeRender( CRenderable& r, CRenderContext& ctx );
	virtual void afterRender( CRenderable& r, CRenderContext& ctx );

	virtual void render( const CRenderContext& ctx );

	virtual const CD3DVertexBuffer*	getUsedVB() const { return CDynamicVBManager::getInstance().getBuffer(); };
	virtual const CD3DIndexBuffer*	getUsedIB() const { return mIB; };

private:
	typedef SVertexXyzDiffuseTex1			TVertex;
	typedef CPool<SBillboard>				TSlot;
	typedef std::map< CD3DTexture*, TSlot > TTextureSlotMap;
	typedef std::vector< std::pair<CD3DTexture*, CVBChunk::TSharedPtr> > TTextureChunkVector;

private:
	TSlot& getSlot( CD3DTexture& tex );

private:
	CD3DIndexBuffer*	mIB;
	CVBManagerSource	mVBSource;
	CEffectParams::TParamName	mTexParamName;
	TTextureSlotMap		mSlots;
	TTextureChunkVector	mChunks;
};


}; // namespace


#endif
