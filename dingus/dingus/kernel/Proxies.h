// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PROXIES_H
#define __PROXIES_H

namespace dingus {


class CBaseProxyClass {
public:
	bool	isNull() const { return (mPtr==0); }
	void*	getPtr() const { return mPtr; }

protected:
	CBaseProxyClass( void* u = 0 ) : mPtr(u) { }
	void	setPtr( void* u ) { mPtr = u; }

private:
	void*	mPtr;
};



template<typename T>
class CProxyClass : public CBaseProxyClass {
public:
	CProxyClass( T* object = 0 ) : CBaseProxyClass(object) { }

	T*		getObject() const { return reinterpret_cast<T*>(getPtr()); }
	void	setObject( T* object ) { setPtr(object); }
};

typedef CProxyClass<IDirect3DTexture9>		CD3DTexture;
typedef CProxyClass<IDirect3DCubeTexture9>	CD3DCubeTexture;
typedef CProxyClass<IDirect3DVolumeTexture9>CD3DVolumeTexture;
typedef CProxyClass<IDirect3DSurface9>		CD3DSurface;

typedef CProxyClass<IDirect3DIndexBuffer9>	CD3DIndexBuffer;
typedef CProxyClass<IDirect3DVertexBuffer9>	CD3DVertexBuffer;

typedef CProxyClass<IDirect3DPixelShader9>	CD3DPixelShader;
typedef CProxyClass<IDirect3DVertexShader9>	CD3DVertexShader;

typedef CProxyClass<IDirect3DQuery9>				CD3DQuery;
typedef CProxyClass<IDirect3DStateBlock9>			CD3DStateBlock;
typedef CProxyClass<IDirect3DVertexDeclaration9>	CD3DVertexDecl;

typedef CProxyClass<ID3DXMesh>		CD3DXMesh;

// effect isn't just a proxy - it adds some stuff. will be included later
//typedef CProxyClass<ID3DXEffect>	CD3DXEffect;


}; // namespace


#include "ProxyEffect.h"

#endif
