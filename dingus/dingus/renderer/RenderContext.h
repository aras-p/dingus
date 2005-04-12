// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RENDER_CONTEXT_H
#define __RENDER_CONTEXT_H


#include "RenderCamera.h"
#include "EffectParams.h"

namespace dingus {

class CRenderable;



/**
 *  Renderer.
 */
class CRenderContext : public boost::noncopyable {
public:
	CRenderContext( CD3DXEffect& globalFx );

	/**
	 *  Apply global effect and it's parameters.
	 *  This MUST be  performed manually, it's not performed anywhere else
	 *  for performance reasons. Call this whenever there's a change in
	 *  global parameters.
	 */
	void applyGlobalEffect();

	//
	// "deferred" rendering

	/**
	 *  Attach renderable for later deferred rendering. Actual
	 *  rendering/sorting will be performed in perform(), after that
	 *  renderables will be detached.
	 */
	void attach( CRenderable& r ) { mRenderables.push_back(&r); };
	/**
	 *  Perform optimized rendering of all attached renderables and detach
	 *  them all.
	 */
	void perform();

	//
	// "direct" rendering

	void directBegin();
	void directRender( CRenderable& r );
	void directEnd();
	int directSetFX( CD3DXEffect& fx ); // returns pass count-1
	void directFXPassBegin( int pass );
	void directFXPassEnd();

	
	const CRenderCamera& getCamera() const { return mRenderCamera; };
	CRenderCamera& getCamera() { return mRenderCamera; };
	const CEffectParams& getGlobalParams() const { return mGlobalParams; };
	CEffectParams& getGlobalParams() { return mGlobalParams; };

private:
	void	sortRenderables();
	void	renderRenderables();

private:
	typedef std::vector<CRenderable*>	TRenderableVector;
private:
	TRenderableVector	mRenderables;
	CRenderCamera		mRenderCamera;

	CEffectParams		mGlobalParams;
	CD3DXEffect*		mGlobalEffect;

	// direct rendering part
	bool			mInsideDirect;
	CD3DXEffect*	mDirectCurrFX;
	int				mDirectCurrPasses;
};


}; // namespace

#endif
