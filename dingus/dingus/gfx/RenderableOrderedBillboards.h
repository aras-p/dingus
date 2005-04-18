// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RENDER_ORDERED_BILLBOARDS_H
#define __RENDER_ORDERED_BILLBOARDS_H

#include "../kernel/Proxies.h"
#include "../renderer/Renderable.h"
#include "geometry/BufferChunk.h"
#include "geometry/VBManagerSource.h"


namespace dingus {



struct SVertexXyzDiffuseTex1;
struct SMatrix4x4;


struct SOBillboard {
public:
	void	setWholeTexture() { tu1=tv1=0; tu2=tv2=1; }
public:
	float			x1, y1, x2, y2;
	float			tu1, tv1, tu2, tv2;
	D3DCOLOR		color;
	CD3DTexture*	texture;
};


/**
 *  Billboard renderer with order-preserving.
 *
 *  Renders billboards (aka sprites, or screen-aligned textured
 *  quads). Preserves billboard submitting order - this may be not very
 *  efficient if billboards use different textures. Use CRenderableBillboards
 *  if order doesn't matter.
 */
class CRenderableOrderedBillboards : public CRenderable, public IRenderListener {
public:
	CRenderableOrderedBillboards( CD3DIndexBuffer& ib, CEffectParams::TParamName texParamName );
	virtual ~CRenderableOrderedBillboards();
	
	SOBillboard& addBill() { mBills.push_back(SOBillboard()); return mBills.back(); };
	void clear() { mBills.clear(); }

	// IRenderListener
	virtual void beforeRender( CRenderable& r, CRenderContext& ctx );
	virtual void afterRender( CRenderable& r, CRenderContext& ctx );

	virtual void render( const CRenderContext& ctx );

	virtual const CD3DVertexBuffer*	getUsedVB() const { return CDynamicVBManager::getInstance().getBuffer(); };
	virtual const CD3DIndexBuffer*	getUsedIB() const { return mIB; };

private:
	typedef SVertexXyzDiffuseTex1		TVertex;
	typedef std::vector<SOBillboard>	TBillVector;

private:
	CD3DIndexBuffer*	mIB;
	CVBManagerSource	mVBSource;
	CEffectParams::TParamName	mTexParamName;
	TBillVector mBills;
	TVBChunk::TSharedPtr mChunk;
};


}; // namespace


#endif
