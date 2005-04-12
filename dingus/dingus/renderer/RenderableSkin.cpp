// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "RenderableSkin.h"
#include "RenderContext.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"


using namespace dingus;

DEFINE_POOLED_ALLOC(dingus::CRenderableSkin,128,false);

CRenderableSkin::TVec3Vector CRenderableSkin::mPaletteSkinMatrices;



CRenderableSkin::CRenderableSkin( CSkinMesh& mesh, int group, const SVector3* skinMatrices, CEffectParams::TParamName matsParamName, CEffectParams::TParamName bonesParamName, const SVector3* origin, int priority )
:	CRenderable( origin, priority ),
	mMesh( &mesh ), mGroup( group ),
	mSkinMatrices( skinMatrices ),
	mMatsParamName( matsParamName )
{
	assert( group >= 0 && group < mesh.getMesh().getGroupCount() );
	assert( mesh.isCreated() );
	assert( skinMatrices );

	getParams().addInt( bonesParamName, mesh.getBonesPerVertex()-1 );

	int vec3sPerPal = mesh.getPaletteSize() * 4;
	if( mPaletteSkinMatrices.size() < vec3sPerPal )
		mPaletteSkinMatrices.resize( vec3sPerPal );
}


void CRenderableSkin::render( CRenderContext const& ctx )
{
	assert( mMesh );

	HRESULT hr;

	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();

	CMesh& mesh = mMesh->getMesh();

	device.setIndexBuffer( &mesh.getIB() );
	device.setVertexBuffer( 0, &mesh.getVB(), 0, mesh.getVertexStride() );
	device.setDeclaration( mesh.getVertexDecl() );

	// go through palettes and render each that is of needed group
	// TBD: suboptimal! should have a ID->palettes[] mapping
	int n = mMesh->getPaletteCount();
	for( int i = 0; i < n; ++i ) {
		const CSkinBonePalette& bpal = mMesh->getPalette(i);
		if( bpal.getGroupID() != mGroup )
			continue;
		
		// copy skin matrices into palette's space
		int palSize = bpal.getBoneCount(); // may be smaller than max. palette size
		SVector3* dst = &mPaletteSkinMatrices[0];
		for( int b = 0; b < palSize; ++b ) {
			int boneIdx = bpal.getBoneIdx( b );
			assert( boneIdx >= 0 && boneIdx < mMesh->getSkeleton().getBoneCount() );
			dst[0] = mSkinMatrices[boneIdx*4+0];
			dst[1] = mSkinMatrices[boneIdx*4+1];
			dst[2] = mSkinMatrices[boneIdx*4+2];
			dst[3] = mSkinMatrices[boneIdx*4+3];
			dst += 4;
		}
		
		// set palette on effect
		// TBD: kind of hack!
		getParams().getEffect()->getObject()->SetFloatArray( mMatsParamName, &mPaletteSkinMatrices[0].x, palSize*4*3 );
		getParams().getEffect()->commitParams();

		// render
		const CMesh::CGroup& group = mesh.getGroup( i );
		hr = dx.DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
			0, group.getFirstVertex(), group.getVertexCount(),
			group.getFirstPrim() * 3, group.getPrimCount() );
		stats.incVerticesRendered( group.getVertexCount() );
		stats.incPrimsRendered( group.getPrimCount() );
		if( FAILED( hr ) ) {
			THROW_DXERROR( hr, "failed to DIP" );
		}
		// stats
		stats.incDrawCalls();
	}
}
