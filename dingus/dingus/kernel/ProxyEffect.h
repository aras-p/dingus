// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PROXY_EFFECT_H
#define __PROXY_EFFECT_H

#include "Proxies.h"

namespace dingus {


/**
 *  Proxy for ID3DXEffect with some addons.
 *
 *  Additionally has some indicators:
 *		- whether this effect requires strict back-to-front sorting.
 *		- required vertex shader version (0, 1.1, 2.0 or 3.0 currently).
 *	The indicators are read from effect technique annotations.
 */
class CD3DXEffect : public CBaseProxyClass {
public:
	CD3DXEffect( ID3DXEffect* object = 0 );

	/** Begins effect. @return Number of passes. */
	int		beginFx();
	/** Ends effect. */
	void	endFx();
	/** Begins pass. */
	void	beginPass( int p );
	/** Ends pass. */
	void	endPass();
	/** Commits parameters that are set after begin. */
	void commitParams();
	

	ID3DXEffect* getObject() const { return reinterpret_cast<ID3DXEffect*>(getPtr()); }
	void	setObject( ID3DXEffect* object );

	bool	isBackToFrontSorted() const { return mBackToFrontSorted; }
	//int		getVShaderVersion() const { return mVShaderVersion; }
	bool	isSoftwareVertexProcessed() const { return mSoftwareVertexProcessed; }

	bool	hasRestoringPass() const { return mHasRestoringPass; }

	/**
	 *  For sorting, computed from other params in ascending order.
	 *  Orders by back-to-front (non-sorted first, then sorted), then
	 *	by vshader version in ascending order.
	 */
	int		getSortValue() const { return mSortValue; }

private:
	void	init();
	bool	tryInit();
	bool	isValidTechnique( D3DXHANDLE tech );
	
private:
	int		mPassCount; // count or count-1, depending on presence of restoring pass
	int		mSortValue; // value computed from other params, for sorting
	bool	mBackToFrontSorted;
	bool	mSoftwareVertexProcessed;
	bool	mHasRestoringPass;
};


// We have API changes in D3DX effects from Summer 2004...
#undef USE_D3DX_SUMMER_2004
#if D3DX_VERSION >= 0x0902 && D3DX_SDK_VERSION >= 22
	#define USE_D3DX_SUMMER_2004
#endif



}; // namespace

#endif
