// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __FX_STATE_MANAGER_H
#define __FX_STATE_MANAGER_H

namespace dingus {

class CEffectStateManager : public ID3DXEffectStateManager/*, public CRefCounted*/ {
public:
	CEffectStateManager();
	void reset();
	
	// ID3DXEffectStateManager
    STDMETHOD(SetTransform)( D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix );
    STDMETHOD(SetMaterial)( const D3DMATERIAL9* material );
    STDMETHOD(SetLight)( DWORD index, const D3DLIGHT9* light );
    STDMETHOD(LightEnable)( DWORD index, BOOL enable );
    STDMETHOD(SetRenderState)( D3DRENDERSTATETYPE state, DWORD value );
    STDMETHOD(SetTexture)( DWORD stage, IDirect3DBaseTexture9* texture );
    STDMETHOD(SetTextureStageState)( DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value );
    STDMETHOD(SetSamplerState)( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value );
    STDMETHOD(SetNPatchMode)( float numSegments );
    STDMETHOD(SetFVF)( DWORD fvf );
    STDMETHOD(SetVertexShader)( IDirect3DVertexShader9* shader );
    STDMETHOD(SetVertexShaderConstantF)( UINT registerIdx, const float* data, UINT registerCount );
    STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT registerIdx, const int *data, UINT registerCount );
    STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT registerIdx, const BOOL *data, UINT registerCount );
    STDMETHOD(SetPixelShader)( IDirect3DPixelShader9* shader );
    STDMETHOD(SetPixelShaderConstantF)( UINT registerIdx, const float *data, UINT registerCount );
    STDMETHOD(SetPixelShaderConstantI)( UINT registerIdx, const int *data, UINT registerCount );
    STDMETHOD(SetPixelShaderConstantB)( UINT registerIdx, const BOOL *data, UINT registerCount );

    // IUnknown
    STDMETHOD(QueryInterface)( REFIID iid, LPVOID *ppv ) {
		if( iid == IID_ID3DXEffectStateManager ) {
			*ppv = this;
			AddRef();
			return NOERROR;
		}
		return ResultFromScode(E_NOINTERFACE);
	}
    STDMETHOD_(ULONG, AddRef)() {
		//return incRef();
		return 1;
	}
    STDMETHOD_(ULONG, Release)() {
		//return decRef();
		return 1;
	}

private:
	enum { NUM_LIGHTS_CACHED = 8 };
	enum { NUM_RENDERSTATES_CACHED = 256 };
	enum { NUM_TEXTURES_CACHED = 4 };
	enum { NUM_TEXSTATES_CACHED = 32 };
	enum { NUM_SAMPLERS_CACHED = 8 };
	enum { NUM_SAMPSTATES_CACHED = 16 };

	BOOL	mLightEnabled[NUM_LIGHTS_CACHED];
	
	DWORD	mRenderStates[NUM_RENDERSTATES_CACHED];
	BOOL	mGotRenderStates[NUM_RENDERSTATES_CACHED];

	IDirect3DBaseTexture9* mTextures[NUM_TEXTURES_CACHED];
	DWORD	mTextureStates[NUM_TEXTURES_CACHED][NUM_TEXSTATES_CACHED];
	BOOL	mGotTextureStates[NUM_TEXTURES_CACHED][NUM_TEXSTATES_CACHED];
	DWORD	mSamplerStates[NUM_SAMPLERS_CACHED][NUM_SAMPSTATES_CACHED];
	BOOL	mGotSamplerStates[NUM_SAMPLERS_CACHED][NUM_SAMPSTATES_CACHED];

	IDirect3DVertexShader9* mVShader;
	IDirect3DPixelShader9* mPShader;
};

}; // namespace

#endif
