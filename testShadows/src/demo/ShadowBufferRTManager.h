#ifndef __SHADOW_BUFFER_RT_MANAGER_H
#define __SHADOW_BUFFER_RT_MANAGER_H

// --------------------------------------------------------------------------

const int kMaxShadowRTSize = 1024;
const int kMinShadowRTSize = 16;


// --------------------------------------------------------------------------


class ShadowBufferRTManager
{
public:
	ShadowBufferRTManager();

	void	BeginFrame();
	bool	RequestBuffer( int& size, CD3DTexture** outTex, CD3DSurface** outSurf );
	CD3DSurface*	FindDepthBuffer( int size );

	void	GetSizeStats( int size, int& outActive, int& outTotal ) const;

private:
	struct RenderTarget
	{
		RenderTarget( const char* id ) : resID(id) { }
		CResourceId		resID; // for both texture and surface
		CD3DTexture*	texture;
		CD3DSurface*	surface;
		bool			used;
	};
	typedef std::vector<RenderTarget>	RenderTargets;
	typedef std::map<int, RenderTargets> SizeToRenderTargetMap;

	SizeToRenderTargetMap	mSizeToRTs;
	
	struct DepthBuffer
	{
		DepthBuffer( const char* id ) : resID(id) { }
		CResourceId		resID; // for depth buffer surface
		CD3DSurface*	surface;
	};
	typedef std::map<int, DepthBuffer>	SizeToDepthBufferMap;
	
	SizeToDepthBufferMap	mSizeToDB;
};

extern ShadowBufferRTManager*	gShadowRTManager;


#endif
