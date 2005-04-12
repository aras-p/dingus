// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __EFFECTPARAMS_H
#define __EFFECTPARAMS_H

#include "../math/Vector3.h"
#include "../math/Vector4.h"
#include "../math/Matrix4x4.h"
#include "../kernel/Proxies.h"

namespace dingus {


/**
 *  Collection of parameters for an effect.
 *
 *  Parameter is a named typed value, influencing effect in some way.
 *  Examples are references to matrices, textures, float values, etc.
 *
 *  For efficiency, this calculates handles for all parameters for use with
 *  concrete effect object.
 */
class CEffectParams {
public:
	typedef D3DXHANDLE		TParamName;

public:
	CEffectParams();
	~CEffectParams();
	CEffectParams( const CEffectParams& rhs );
	const CEffectParams& operator=( const CEffectParams& rhs );

	void addInt( TParamName name, int i );
	void removeInt( TParamName name ) { removeParam1(name); }
	void addFloat( TParamName name, float f ) { addInt( name, *reinterpret_cast<int*>(&f) ); }
	void removeFloat( TParamName name ) { removeParam1(name); }
	void addBool( TParamName name, bool b ) { addInt( name, b ); }
	void removeBool( TParamName name ) { removeParam1(name); }
	
	void addVector3( TParamName name, const SVector3& v );
	void removeVector3( TParamName name ) { removeParam3(name); }
	void addVector4( TParamName name, const SVector4& v );
	void removeVector4( TParamName name ) { removeParam4(name); }
	void addMatrix4x4( TParamName name, const SMatrix4x4& m );
	void removeMatrix4x4( TParamName name ) { removeParam16(name); }

	// NOTE: can't pass by ref and take address - that takes address of temp. ref sometimes
	void addFloatRef( TParamName name, const float* f ) { addPtr( name, sizeof(float), f ); }
	// NOTE: can't pass by ref and take address - that takes address of temp. ref sometimes
	void addIntRef( TParamName name, const int* i ) { addPtr( name, sizeof(int), i ); }
	void addVector3Ref( TParamName name, const SVector3& v ) { addPtr( name, sizeof(SVector3), &v ); }
	void addVector4Ref( TParamName name, const SVector4& v ) { addPtr( name, sizeof(SVector4), &v ); }
	void addColor4Ref( TParamName name, const D3DXCOLOR& c ) { addPtr( name, sizeof(D3DXCOLOR), &c ); }
	void addMatrix4x4Ref( TParamName name, const SMatrix4x4& m ) { addPtr( name, sizeof(SMatrix4x4), &m ); }
	void removeRef( TParamName name ) { removeParamPtr(name); }

	void addTexture( TParamName name, const CD3DTexture& t ) { addProxy( name, t ); }
	void removeTexture( TParamName name ) { removeParamProxy(name); }
	void addCubeTexture( TParamName name, const CD3DCubeTexture& t ) { addProxy( name, t ); }
	void removeCubeTexture( TParamName name ) { removeParamProxy(name); }
	void addVolumeTexture( TParamName name, const CD3DVolumeTexture& t ) { addProxy( name, t ); }
	void removeVolumeTexture( TParamName name ) { removeParamProxy(name); }
	void addPixelShader( TParamName name, const CD3DPixelShader& t ) { addProxy( name, t ); }
	void removePixelShader( TParamName name ) { removeParamProxy(name); }
	void addVertexShader( TParamName name, const CD3DVertexShader& t ) { addProxy( name, t ); }
	void removeVertexShader( TParamName name ) { removeParamProxy(name); }

	void addPtr( TParamName name, int size, const void* ptr );

	void clear();

	/**
	 *  Sets effect object.
	 *  If the effect is different from the old one, then this recalculates
	 *  parameter handles for the given effect. Call this whenever the effect
	 *  object that these params will be applied to is changed.
	 *  @param force Force params recalculation.
	 */
	void setEffect( CD3DXEffect& fx, bool force = false );
	CD3DXEffect* getEffect() const { return mEffect; }
	bool isEffect() const { return (mEffect!=NULL); }
	/**
	 *  Applies params to previously set effect object.
	 */
	void applyToEffect();

private:
	void addProxy( TParamName name, const CBaseProxyClass& proxy );
	void removeParam1( TParamName name );
	void removeParam3( TParamName name );
	void removeParam4( TParamName name );
	void removeParam16( TParamName name );
	void removeParamPtr( TParamName name );
	void removeParamProxy( TParamName name );

private:
	struct SParamHandle {
		std::string name;
		TParamName	handle;
	};
	struct SParam1 : SParamHandle {
		union {
			BOOL	valB;
			int		valI;
			float	valF;
			const CBaseProxyClass*	proxy;
		};
	};
	struct SParam3 : SParamHandle {
		D3DXVECTOR3 valV;
	};
	struct SParam4 : SParamHandle {
		D3DXVECTOR4 valV;
	};
	struct SParam16 : SParamHandle {
		D3DXMATRIX valM;
	};
	struct SParamPtr : SParamHandle {
		int			size;
		const void*	ptr;
	};

	typedef fastvector< SParam1 >	TParam1Vector;
	typedef fastvector< SParam3 >	TParam3Vector;
	typedef fastvector< SParam4 >	TParam4Vector;
	typedef fastvector< SParam16 >	TParam16Vector;
	typedef fastvector< SParamPtr >	TParamPtrVector;
	typedef TParam1Vector			TParamProxyVector;

private:
	void initHandle( SParamHandle& par ) const;

private:
	CD3DXEffect*	mEffect;
	TParam1Vector	mParams1;
	TParam3Vector	mParams3;
	TParam4Vector	mParams4;
	TParam16Vector	mParams16;
	TParamPtrVector	mParamsPtr;
	TParamProxyVector	mParamsProxy;
};	


}; // namespace

#endif
