// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SURFACE_CREATOR_H
#define __SURFACE_CREATOR_H

#include "../kernel/Proxies.h"

namespace dingus {


class ISurfaceCreator : public CRefCounted {
public:
	typedef DingusSmartPtr<ISurfaceCreator>	TSharedPtr;
public:
	virtual ~ISurfaceCreator() = 0 { }
	virtual IDirect3DSurface9* createSurface() = 0;
};


class CAbstractSurfaceCreator : public ISurfaceCreator {
public:
	CAbstractSurfaceCreator( bool depthStencil, D3DFORMAT format, bool useScreenMS = false, D3DMULTISAMPLE_TYPE msType = D3DMULTISAMPLE_NONE, DWORD msQuality = 0 )
		:	mDepthStencil(depthStencil), mFormat(format), mUseScreenMultiSample(useScreenMS), mMultiSampleType(msType), mMultiSampleQuality(msQuality) { }

protected:
	IDirect3DSurface9*	internalCreate( int width, int height ) const;

protected:
	bool		isDepthStencil() const { return mDepthStencil; }
	D3DFORMAT	getFormat() const { return mFormat; }
	bool		isUsingScreenMultiSample() const { return mUseScreenMultiSample; }
	D3DMULTISAMPLE_TYPE getMultiSampleType() const { return mMultiSampleType; }
	DWORD		getMultiSampleQuality() const { return mMultiSampleQuality; }
private:
	bool		mDepthStencil;
	D3DFORMAT	mFormat;
	bool		mUseScreenMultiSample;		// use backbuffer/zstencil MS or
	D3DMULTISAMPLE_TYPE mMultiSampleType;	// use these
	DWORD		mMultiSampleQuality;
};

/// Creates fixed size surface
class CFixedSurfaceCreator : public CAbstractSurfaceCreator {
public:
	CFixedSurfaceCreator( int width, int height, bool depthStencil, D3DFORMAT format, bool useScreenMS = false, D3DMULTISAMPLE_TYPE msType = D3DMULTISAMPLE_NONE, DWORD msQuality = 0 )
		:	CAbstractSurfaceCreator(depthStencil,format,useScreenMS,msType,msQuality), mWidth(width), mHeight(height) { }
	virtual IDirect3DSurface9* createSurface();
private:
	int			mWidth, mHeight;
};

/// Creates surface with size proportional to backbuffer
class CScreenBasedSurfaceCreator : public CAbstractSurfaceCreator {
public:
	CScreenBasedSurfaceCreator( float widthFactor, float heightFactor, bool depthStencil, D3DFORMAT format, bool useScreenMS = false, D3DMULTISAMPLE_TYPE msType = D3DMULTISAMPLE_NONE, DWORD msQuality = 0 )
		:	CAbstractSurfaceCreator(depthStencil,format,useScreenMS,msType,msQuality), mWidthFactor(widthFactor), mHeightFactor(heightFactor) { }
	virtual IDirect3DSurface9* createSurface();
private:
	float		mWidthFactor;
	float		mHeightFactor;
};

/// Creates pow-2 surface with seze proportional to backbuffer (lower pow-2)
class CScreenBasedPow2SurfaceCreator : public CAbstractSurfaceCreator {
public:
	CScreenBasedPow2SurfaceCreator( float widthFactor, float heightFactor, bool depthStencil, D3DFORMAT format, bool useScreenMS = false, D3DMULTISAMPLE_TYPE msType = D3DMULTISAMPLE_NONE, DWORD msQuality = 0 )
		:	CAbstractSurfaceCreator(depthStencil,format,useScreenMS,msType,msQuality), mWidthFactor(widthFactor), mHeightFactor(heightFactor) { }
	virtual IDirect3DSurface9* createSurface();
private:
	float		mWidthFactor;
	float		mHeightFactor;
};

/// Uses some surface level from existing texture
class CTextureLevelSurfaceCreator : public ISurfaceCreator {
public:
	CTextureLevelSurfaceCreator( CD3DTexture& texture, int level = 0 )
		:	mTexture(&texture), mLevel(level) { }
	virtual IDirect3DSurface9* createSurface();
private:
	CD3DTexture*	mTexture;
	int				mLevel;
};


}; // namespace

#endif
