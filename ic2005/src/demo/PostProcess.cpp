#include "stdafx.h"
#include "PostProcess.h"
#include "DemoResources.h"


CPostProcess::CPostProcess( const char* ridTemp1, const char* ridTemp2 )
{
	mTempRids[0] = ridTemp1;
	mTempRids[1] = ridTemp2;

	const char* fx = "filterBloom";
	for( int i = 0; i < 2; ++i ) {
		mBloomPingPongs[i] = new CRenderableMesh( *RGET_MESH("billboard"), 0 );
		CEffectParams& ep = mBloomPingPongs[i]->getParams();
		ep.setEffect( *RGET_FX(fx) );
		ep.addVector4Ref( "vFixUV", mFixUVs4th );
		ep.addVector4Ref( "vTexelsX", mTexelOffsetsX );
		ep.addVector4Ref( "vTexelsY", mTexelOffsetsY );
	}
	mBloomPingPongs[0]->getParams().addTexture( "tBase", *RGET_S_TEX(ridTemp1) );
	mBloomPingPongs[1]->getParams().addTexture( "tBase", *RGET_S_TEX(ridTemp2) );
}

CPostProcess::~CPostProcess()
{
	delete mBloomPingPongs[0];
	delete mBloomPingPongs[1];
}

void CPostProcess::pingPongBlur( int passes, int startPass )
{
	CD3DDevice& dx = CD3DDevice::getInstance();

	CD3DSurface* pingPongS[2];
	pingPongS[0] = RGET_S_SURF(mTempRids[1]);
	pingPongS[1] = RGET_S_SURF(mTempRids[0]);

	// full-screen quad UV fixes
	D3DSURFACE_DESC sdesc;
	pingPongS[0]->getObject()->GetDesc( &sdesc );

	int swidth = sdesc.Width;
	int sheight = sdesc.Height;
	mFixUVs4th.set( 0.5f/swidth, 0.5f/sheight, 1.0f-2.0f/swidth, 1.0f-2.0f/sheight );

	// ping-pong blur passes
	dx.setZStencil( NULL );

	const SVector4 offsetX( 1, 1,-1,-1);
	const SVector4 offsetY( 1,-1,-1, 1);
	for( int i = 0; i < passes; ++i ) {
		int realPass = i + startPass;
		const float pixDist = realPass+0.5f;
		mTexelOffsetsX = offsetX * (mFixUVs4th.x*2 * pixDist);
		mTexelOffsetsY = offsetY * (mFixUVs4th.y*2 * pixDist);
		dx.setRenderTarget( pingPongS[realPass&1] );
		dx.sceneBegin();
		G_RENDERCTX->directBegin();
		G_RENDERCTX->directRender( *mBloomPingPongs[realPass&1] );
		G_RENDERCTX->directEnd();
		dx.sceneEnd();
	}
}

void CPostProcess::downsampleRT( IDirect3DSurface9& rt )
{
	CD3DDevice& dx = CD3DDevice::getInstance();
	dx.getDevice().StretchRect(
		&rt, NULL,
		RGET_S_SURF(mTempRids[0])->getObject(), NULL,
		dx.getCaps().getStretchFilter()
	);
}
