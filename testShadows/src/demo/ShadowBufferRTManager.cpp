#include "stdafx.h"
#include "ShadowBufferRTManager.h"


ShadowBufferRTManager*	gShadowRTManager;


static int LowerPowerOf2( int x )
{
	int hibit = 0;
	while( x >> hibit != 0 )
		++hibit;
	x &= (1<<(hibit-1));
	return x;
}

static int MakeValidTextureSize( int x )
{
	x = LowerPowerOf2( x );
	if( x < kMinShadowRTSize )
		x = kMinShadowRTSize;
	else if( x > kMaxShadowRTSize )
		x = kMaxShadowRTSize;
	return x;
}


// --------------------------------------------------------------------------


ShadowBufferRTManager::ShadowBufferRTManager()
{
}

void ShadowBufferRTManager::BeginFrame()
{
	// mark all RTs as not used
	SizeToRenderTargetMap::iterator it, itEnd = mSizeToRTs.end();
	for( it = mSizeToRTs.begin(); it != itEnd; ++it )
	{
		RenderTargets& rts = it->second;
		for( int i = 0; i < rts.size(); ++i )
		{
			rts[i].used = false;
		}
	}
}

void ShadowBufferRTManager::GetSizeStats( int size, int& outActive, int& outTotal ) const
{
	size = MakeValidTextureSize( size );
	outActive = 0;
	outTotal = 0;
	SizeToRenderTargetMap::const_iterator it = mSizeToRTs.find( size );
	if( it == mSizeToRTs.end() )
		return;

	const RenderTargets& rts = it->second;
	outTotal = rts.size();
	for( int i = 0; i < outTotal; ++i )
	{
		if( rts[i].used )
			++outActive;
	}
}


bool ShadowBufferRTManager::RequestBuffer( int& size, CD3DTexture** outTex, CD3DSurface** outSurf )
{
	size = MakeValidTextureSize( size );

	// try to use non-used but earlier created buffer
	SizeToRenderTargetMap::iterator it = mSizeToRTs.find( size );
	if( it != mSizeToRTs.end() )
	{
		RenderTargets& rts = it->second;
		for( int i = 0; i < rts.size(); ++i )
		{
			if( !rts[i].used )
			{
				// found existing one
				*outTex = rts[i].texture;
				*outSurf = rts[i].surface;
				rts[i].used = true;
				return true;
			}
		}
	}

	// create new one
	static int idCounter = 0;
	char buf[100];
	sprintf( buf, "_shdRT-%i", idCounter++ );

	CConsole::getChannel("shadows") << "new shadow buffer: " << size << endl;

	RenderTarget rt( buf );
	rt.used = true;

	CSharedTextureBundle::getInstance().registerTexture( rt.resID,
		*new CFixedTextureCreator(size,size,1,D3DUSAGE_RENDERTARGET,D3DFMT_R32F,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( CSharedSurfaceBundle::getInstance(), rt.resID );

	rt.texture = RGET_S_TEX(rt.resID);
	rt.surface = RGET_S_SURF(rt.resID);

	if( it == mSizeToRTs.end() )
	{
		RenderTargets rts;
		rts.push_back( rt );
		mSizeToRTs.insert( std::make_pair(size, rts) );
	}
	else
	{
		it->second.push_back( rt );
	}

	*outTex = rt.texture;
	*outSurf = rt.surface;
	return true;
}

CD3DSurface* ShadowBufferRTManager::FindDepthBuffer( int size )
{
	size = MakeValidTextureSize( size );
	
	// try to find already created one
	SizeToDepthBufferMap::iterator it = mSizeToDB.find( size );
	if( it != mSizeToDB.end() )
	{
		return it->second.surface;
	}

	// create new depth buffer
	static int idCounter = 0;
	char buf[100];
	sprintf( buf, "_shdDB-%i", idCounter++ );

	CConsole::getChannel("shadows") << "new depth buffer: " << size << endl;

	DepthBuffer db( buf );
	CSharedSurfaceBundle::getInstance().registerSurface( db.resID,
		*new CFixedSurfaceCreator(size, size, true, D3DFMT_D24S8 ) );
	db.surface = RGET_S_SURF( db.resID );

	mSizeToDB.insert( std::make_pair(size, db) );


	return db.surface;
}

