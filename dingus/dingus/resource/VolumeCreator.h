// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VOLUME_TEXTURE_CREATOR_H
#define __VOLUME_TEXTURE_CREATOR_H

namespace dingus {


class IVolumeCreator : public CRefCounted {
public:
	typedef DingusSmartPtr<IVolumeCreator>	TSharedPtr;
public:
	virtual ~IVolumeCreator() = 0 { }
	virtual IDirect3DVolumeTexture9* createTexture() = 0;
};


class CAbstractVolumeCreator : public IVolumeCreator {
public:
	CAbstractVolumeCreator( DWORD usage, D3DFORMAT format, D3DPOOL pool )
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
class CFixedVolumeCreator : public CAbstractVolumeCreator {
public:
	CFixedVolumeCreator( int width, int height, int depth, int levels, DWORD usage, D3DFORMAT format, D3DPOOL pool )
		:	CAbstractVolumeCreator(usage,format,pool), mWidth(width), mHeight(height), mDepth(depth), mLevels(levels) { }

	virtual IDirect3DVolumeTexture9* createTexture();

private:
	int			mWidth, mHeight, mDepth, mLevels;
};


}; // namespace

#endif
