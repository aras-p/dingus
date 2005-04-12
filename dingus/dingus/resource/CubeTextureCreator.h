// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __CUBE_TEXTURE_CREATOR_H
#define __CUBE_TEXTURE_CREATOR_H

namespace dingus {


class ICubeTextureCreator : public CRefCounted {
public:
	typedef DingusSmartPtr<ICubeTextureCreator>	TSharedPtr;
public:
	virtual ~ICubeTextureCreator() = 0 { }
	virtual IDirect3DCubeTexture9* createTexture() = 0;
};


class CAbstractCubeTextureCreator : public ICubeTextureCreator {
public:
	CAbstractCubeTextureCreator( DWORD usage, D3DFORMAT format, D3DPOOL pool )
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



/// Creates fixed size cube texture
class CFixedCubeTextureCreator : public CAbstractCubeTextureCreator {
public:
	CFixedCubeTextureCreator( int size, int levels, DWORD usage, D3DFORMAT format, D3DPOOL pool )
		:	CAbstractCubeTextureCreator(usage,format,pool), mSize(size), mLevels(levels) { }

	virtual IDirect3DCubeTexture9* createTexture();

private:
	int			mSize, mLevels;
};


}; // namespace

#endif
