// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RENDER_SKIN_H
#define __RENDER_SKIN_H

#include "Renderable.h"
#include "../gfx/skeleton/SkinMesh.h"
#include "../utils/MemoryPool.h"


namespace dingus {


class CRenderableSkin : public CRenderable {
public:
	CRenderableSkin( CSkinMesh& mesh, int group, const SVector3* skinMatrices,
		CEffectParams::TParamName matsParamName = "mSkin",
		CEffectParams::TParamName bonesParamName = "iBones",
		const SVector3* origin = 0, int priority = 0 );

	const CSkinMesh& getMesh() const { return *mMesh; }
	void setGroup( int g ) { mGroup = g; }
	int getGroup() const { return mGroup; }

	virtual void render( const CRenderContext& ctx );
	
	virtual const CD3DVertexBuffer*	getUsedVB() const { return &mMesh->getMesh().getVB(); }
	virtual const CD3DIndexBuffer*	getUsedIB() const { return &mMesh->getMesh().getIB(); }
	
private:
	DECLARE_POOLED_ALLOC(dingus::CRenderableSkin);
private:
	typedef std::vector<SVector3>	TVec3Vector;

	CSkinMesh*	mMesh;
	int			mGroup;

	/// Name of skin matrices param in effect.
	CEffectParams::TParamName	mMatsParamName; // TBD - handle would be faster!
	
	/// Pointer to 4x3 skin matrices. Each matrix is four SVector3's (X,Y,Z,O).
	const SVector3*	mSkinMatrices;

	/// Static space for single palette's skin matrices (4 vec3s per matrix).
	static TVec3Vector	mPaletteSkinMatrices;
};


}; // namespace


#endif
