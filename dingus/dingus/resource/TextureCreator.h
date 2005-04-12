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
	DWORD		getUsage() const { return mUsage; }
	D3DFORMAT	getFormat() const { return mFormat; }
	D3DPOOL		getPool() const { return mPool; }
private:
	DWORD		mUsage;
	D3DFORMAT	mFormat;
	D3DPOOL		mPool;
};

/// Creates fixed size texture
class CFixedTextureCreator : public CAbstractTextureCreator {
public:
	CFixedTextureCreator( int width, int height, int levels, DWORD usage, D3DFORMAT format, D3DPOOL pool )
		:	CAbstractTextureCreator(usage,format,pool), mWidth(width), mHeight(height), mLevels(levels) { }

	virtual IDirect3DTexture9* createTexture();

protected:
	int	getWidth() const { return mWidth; }
	int getHeight() const { return mHeight; }
	int getLevels() const { return mLevels; }
	
private:
	int			mWidth, mHeight, mLevels;
};

/// Creates texture with seze proportional to backbuffer
class CScreenBasedTextureCreator : public CAbstractTextureCreator {
public:
	CScreenBasedTextureCreator( float widthFactor, float heightFactor, int levels, DWORD usage, D3DFORMAT format, D3DPOOL pool )
		: CAbstractTextureCreator(usage,format,pool), mWidthFactor(widthFactor), mHeightFactor(heightFactor), mLevels(levels) { }

	virtual IDirect3DTexture9* createTexture();

private:
	float		mWidthFactor;
	float		mHeightFactor;
	int			mLevels;
};

/// Creates pow-2 texture with seze proportional to backbuffer (lower pow-2)
class CScreenBasedPow2TextureCreator : public CAbstractTextureCreator {
public:
	CScreenBasedPow2TextureCreator( float widthFactor, float heightFactor, int levels, DWORD usage, D3DFORMAT format, D3DPOOL pool )
		: CAbstractTextureCreator(usage,format,pool), mWidthFactor(widthFactor), mHeightFactor(heightFactor), mLevels(levels) { }

	virtual IDirect3DTexture9* createTexture();

private:
	float		mWidthFactor;
	float		mHeightFactor;
	int			mLevels;
};


}; // namespace

#endif
