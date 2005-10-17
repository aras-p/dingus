// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __TEXTURE_CREATOR_H
#define __TEXTURE_CREATOR_H

namespace dingus {


class ITextureCreator : public CRefCounted {
public:
	typedef DingusSmartPtr<ITextureCreator>	TSharedPtr;
public:
	virtual ~ITextureCreator() = 0 { }
	virtual IDirect3DTexture9* createTexture() = 0;
};


class CAbstractTextureCreator : public ITextureCreator {
public:
	CAbstractTextureCreator( DWORD usage, D3DFORMAT format, D3DPOOL pool )
		:	mUsage(usage), mFormat(format), mPool(pool) { }
protected:
	const DWORD		mUsage;
	const D3DFORMAT	mFormat;
	const D3DPOOL		mPool;
};

/// Creates fixed size texture
class CFixedTextureCreator : public CAbstractTextureCreator {
public:
	CFixedTextureCreator( int width, int height, int levels, DWORD usage, D3DFORMAT format, D3DPOOL pool )
		:	CAbstractTextureCreator(usage,format,pool), mWidth(width), mHeight(height), mLevels(levels) { }
	virtual IDirect3DTexture9* createTexture();
protected:
	const int mWidth, mHeight, mLevels;
};

/// Creates texture with size proportional to backbuffer
class CScreenBasedTextureCreator : public CAbstractTextureCreator {
public:
	CScreenBasedTextureCreator( float widthFactor, float heightFactor, int levels, DWORD usage, D3DFORMAT format, D3DPOOL pool )
		: CAbstractTextureCreator(usage,format,pool), mWidthFactor(widthFactor), mHeightFactor(heightFactor), mLevels(levels) { }
	virtual IDirect3DTexture9* createTexture();
protected:
	const float	mWidthFactor;
	const float	mHeightFactor;
	const int	mLevels;
};

/// Creates texture with size proportional to backbuffer, cropped to make divisible by some number.
class CScreenBasedDivTextureCreator : public CScreenBasedTextureCreator {
public:
	CScreenBasedDivTextureCreator( float widthFactor, float heightFactor, int bbDivisibleBy, int levels, DWORD usage, D3DFORMAT format, D3DPOOL pool )
		: CScreenBasedTextureCreator(widthFactor,heightFactor,levels,usage,format,pool), mBBDivisibleBy(bbDivisibleBy) { }
	virtual IDirect3DTexture9* createTexture();
protected:
	const int	mBBDivisibleBy;
};

/// Creates pow-2 texture with size proportional to backbuffer (lower pow-2)
class CScreenBasedPow2TextureCreator : public CScreenBasedTextureCreator {
public:
	CScreenBasedPow2TextureCreator( float widthFactor, float heightFactor, int levels, DWORD usage, D3DFORMAT format, D3DPOOL pool )
		: CScreenBasedTextureCreator(widthFactor,heightFactor,levels,usage,format,pool) { }
	virtual IDirect3DTexture9* createTexture();
};


}; // namespace

#endif
