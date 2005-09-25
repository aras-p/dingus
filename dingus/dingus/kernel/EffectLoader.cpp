// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "EffectLoader.h"
#include "ProxyEffect.h"
#include "D3DDevice.h"
#include "../console/Console.h"
#include "../utils/Errors.h"

using namespace dingus;

// --------------------------------------------------------------------------

enum eFxStateType {
	FXST_RENDERSTATE,
	FXST_TSS,
	FXST_NPATCH,
	FXST_SAMPLER,
	FXST_VERTEXSHADER,
	FXST_PIXELSHADER,
};

struct SFxState {
	eFxStateType	type;
	const char*		name;
	DWORD			code;
};

static SFxState FX_STATES[] = {
	{ FXST_VERTEXSHADER, "VertexShader", 0 },
	{ FXST_PIXELSHADER, "PixelShader", 0 },
	{ FXST_NPATCH, "PatchSegments", 0 },
	{ FXST_RENDERSTATE, "ZENABLE",				   D3DRS_ZENABLE				 },
	{ FXST_RENDERSTATE, "FILLMODE", 			   D3DRS_FILLMODE				 },
	{ FXST_RENDERSTATE, "SHADEMODE",			   D3DRS_SHADEMODE				 },
	{ FXST_RENDERSTATE, "ZWRITEENABLE", 		   D3DRS_ZWRITEENABLE			 },
	{ FXST_RENDERSTATE, "ALPHATESTENABLE",		   D3DRS_ALPHATESTENABLE		 },
	{ FXST_RENDERSTATE, "LASTPIXEL",			   D3DRS_LASTPIXEL				 },
	{ FXST_RENDERSTATE, "SRCBLEND", 			   D3DRS_SRCBLEND				 },
	{ FXST_RENDERSTATE, "DESTBLEND",			   D3DRS_DESTBLEND				 },
	{ FXST_RENDERSTATE, "CULLMODE", 			   D3DRS_CULLMODE				 },
	{ FXST_RENDERSTATE, "ZFUNC",				   D3DRS_ZFUNC					 },
	{ FXST_RENDERSTATE, "ALPHAREF", 			   D3DRS_ALPHAREF				 },
	{ FXST_RENDERSTATE, "ALPHAFUNC",			   D3DRS_ALPHAFUNC				 },
	{ FXST_RENDERSTATE, "DITHERENABLE", 		   D3DRS_DITHERENABLE			 },
	{ FXST_RENDERSTATE, "ALPHABLENDENABLE", 	   D3DRS_ALPHABLENDENABLE		 },
	{ FXST_RENDERSTATE, "FOGENABLE",			   D3DRS_FOGENABLE				 },
	{ FXST_RENDERSTATE, "SPECULARENABLE",		   D3DRS_SPECULARENABLE 		 },
	{ FXST_RENDERSTATE, "FOGCOLOR", 			   D3DRS_FOGCOLOR				 },
	{ FXST_RENDERSTATE, "FOGTABLEMODE", 		   D3DRS_FOGTABLEMODE			 },
	{ FXST_RENDERSTATE, "FOGSTART", 			   D3DRS_FOGSTART				 },
	{ FXST_RENDERSTATE, "FOGEND",				   D3DRS_FOGEND 				 },
	{ FXST_RENDERSTATE, "FOGDENSITY",			   D3DRS_FOGDENSITY 			 },
	{ FXST_RENDERSTATE, "RANGEFOGENABLE",		   D3DRS_RANGEFOGENABLE 		 },
	{ FXST_RENDERSTATE, "STENCILENABLE",		   D3DRS_STENCILENABLE			 },
	{ FXST_RENDERSTATE, "STENCILFAIL",			   D3DRS_STENCILFAIL			 },
	{ FXST_RENDERSTATE, "STENCILZFAIL", 		   D3DRS_STENCILZFAIL			 },
	{ FXST_RENDERSTATE, "STENCILPASS",			   D3DRS_STENCILPASS			 },
	{ FXST_RENDERSTATE, "STENCILFUNC",			   D3DRS_STENCILFUNC			 },
	{ FXST_RENDERSTATE, "STENCILREF",			   D3DRS_STENCILREF 			 },
	{ FXST_RENDERSTATE, "STENCILMASK",			   D3DRS_STENCILMASK			 },
	{ FXST_RENDERSTATE, "STENCILWRITEMASK", 	   D3DRS_STENCILWRITEMASK		 },
	{ FXST_RENDERSTATE, "TEXTUREFACTOR",		   D3DRS_TEXTUREFACTOR			 },
	{ FXST_RENDERSTATE, "WRAP0",				   D3DRS_WRAP0					 },
	{ FXST_RENDERSTATE, "WRAP1",				   D3DRS_WRAP1					 },
	{ FXST_RENDERSTATE, "WRAP2",				   D3DRS_WRAP2					 },
	{ FXST_RENDERSTATE, "WRAP3",				   D3DRS_WRAP3					 },
	{ FXST_RENDERSTATE, "WRAP4",				   D3DRS_WRAP4					 },
	{ FXST_RENDERSTATE, "WRAP5",				   D3DRS_WRAP5					 },
	{ FXST_RENDERSTATE, "WRAP6",				   D3DRS_WRAP6					 },
	{ FXST_RENDERSTATE, "WRAP7",				   D3DRS_WRAP7					 },
	{ FXST_RENDERSTATE, "CLIPPING", 			   D3DRS_CLIPPING				 },
	{ FXST_RENDERSTATE, "LIGHTING", 			   D3DRS_LIGHTING				 },
	{ FXST_RENDERSTATE, "AMBIENT",				   D3DRS_AMBIENT				 },
	{ FXST_RENDERSTATE, "FOGVERTEXMODE",		   D3DRS_FOGVERTEXMODE			 },
	{ FXST_RENDERSTATE, "COLORVERTEX",			   D3DRS_COLORVERTEX			 },
	{ FXST_RENDERSTATE, "LOCALVIEWER",			   D3DRS_LOCALVIEWER			 },
	{ FXST_RENDERSTATE, "NORMALIZENORMALS", 	   D3DRS_NORMALIZENORMALS		 },
	{ FXST_RENDERSTATE, "DIFFUSEMATERIALSOURCE",   D3DRS_DIFFUSEMATERIALSOURCE	 },
	{ FXST_RENDERSTATE, "SPECULARMATERIALSOURCE",  D3DRS_SPECULARMATERIALSOURCE  },
	{ FXST_RENDERSTATE, "AMBIENTMATERIALSOURCE",   D3DRS_AMBIENTMATERIALSOURCE	 },
	{ FXST_RENDERSTATE, "EMISSIVEMATERIALSOURCE",  D3DRS_EMISSIVEMATERIALSOURCE  },
	{ FXST_RENDERSTATE, "VERTEXBLEND",			   D3DRS_VERTEXBLEND			 },
	{ FXST_RENDERSTATE, "CLIPPLANEENABLE",		   D3DRS_CLIPPLANEENABLE		 },
	{ FXST_RENDERSTATE, "POINTSIZE",			   D3DRS_POINTSIZE				 },
	{ FXST_RENDERSTATE, "POINTSIZE_MIN",		   D3DRS_POINTSIZE_MIN			 },
	{ FXST_RENDERSTATE, "POINTSPRITEENABLE",	   D3DRS_POINTSPRITEENABLE		 },
	{ FXST_RENDERSTATE, "POINTSCALEENABLE", 	   D3DRS_POINTSCALEENABLE		 },
	{ FXST_RENDERSTATE, "POINTSCALE_A", 		   D3DRS_POINTSCALE_A			 },
	{ FXST_RENDERSTATE, "POINTSCALE_B", 		   D3DRS_POINTSCALE_B			 },
	{ FXST_RENDERSTATE, "POINTSCALE_C", 		   D3DRS_POINTSCALE_C			 },
	{ FXST_RENDERSTATE, "MULTISAMPLEANTIALIAS",    D3DRS_MULTISAMPLEANTIALIAS	 },
	{ FXST_RENDERSTATE, "MULTISAMPLEMASK",		   D3DRS_MULTISAMPLEMASK		 },
	{ FXST_RENDERSTATE, "PATCHEDGESTYLE",		   D3DRS_PATCHEDGESTYLE 		 },
	{ FXST_RENDERSTATE, "DEBUGMONITORTOKEN",	   D3DRS_DEBUGMONITORTOKEN		 },
	{ FXST_RENDERSTATE, "POINTSIZE_MAX",		   D3DRS_POINTSIZE_MAX			 },
	{ FXST_RENDERSTATE, "INDEXEDVERTEXBLENDENABLE",D3DRS_INDEXEDVERTEXBLENDENABLE},
	{ FXST_RENDERSTATE, "COLORWRITEENABLE", 	   D3DRS_COLORWRITEENABLE		 },
	{ FXST_RENDERSTATE, "TWEENFACTOR",			   D3DRS_TWEENFACTOR			 },
	{ FXST_RENDERSTATE, "BLENDOP",				   D3DRS_BLENDOP				 },
	{ FXST_RENDERSTATE, "POSITIONDEGREE",		   D3DRS_POSITIONDEGREE 		 },
	{ FXST_RENDERSTATE, "NORMALDEGREE", 		   D3DRS_NORMALDEGREE			 },
	{ FXST_RENDERSTATE, "SCISSORTESTENABLE",	   D3DRS_SCISSORTESTENABLE		 },
	{ FXST_RENDERSTATE, "SLOPESCALEDEPTHBIAS",	   D3DRS_SLOPESCALEDEPTHBIAS	 },
	{ FXST_RENDERSTATE, "ANTIALIASEDLINEENABLE",   D3DRS_ANTIALIASEDLINEENABLE	 },
	{ FXST_RENDERSTATE, "MINTESSELLATIONLEVEL",    D3DRS_MINTESSELLATIONLEVEL	 },
	{ FXST_RENDERSTATE, "MAXTESSELLATIONLEVEL",    D3DRS_MAXTESSELLATIONLEVEL	 },
	{ FXST_RENDERSTATE, "TWOSIDEDSTENCILMODE",	   D3DRS_TWOSIDEDSTENCILMODE	 },
	{ FXST_RENDERSTATE, "CCW_STENCILFAIL",		   D3DRS_CCW_STENCILFAIL		 },
	{ FXST_RENDERSTATE, "CCW_STENCILZFAIL", 	   D3DRS_CCW_STENCILZFAIL		 },
	{ FXST_RENDERSTATE, "CCW_STENCILPASS",		   D3DRS_CCW_STENCILPASS		 },
	{ FXST_RENDERSTATE, "CCW_STENCILFUNC",		   D3DRS_CCW_STENCILFUNC		 },
	{ FXST_RENDERSTATE, "COLORWRITEENABLE1",	   D3DRS_COLORWRITEENABLE1		 },
	{ FXST_RENDERSTATE, "COLORWRITEENABLE2",	   D3DRS_COLORWRITEENABLE2		 },
	{ FXST_RENDERSTATE, "COLORWRITEENABLE3",	   D3DRS_COLORWRITEENABLE3		 },
	{ FXST_RENDERSTATE, "BLENDFACTOR",			   D3DRS_BLENDFACTOR			 },
	{ FXST_RENDERSTATE, "SRGBWRITEENABLE",		   D3DRS_SRGBWRITEENABLE		 },
	{ FXST_RENDERSTATE, "DEPTHBIAS",			   D3DRS_DEPTHBIAS				 },
	{ FXST_RENDERSTATE, "WRAP8",				   D3DRS_WRAP8					 },
	{ FXST_RENDERSTATE, "WRAP9",				   D3DRS_WRAP9					 },
	{ FXST_RENDERSTATE, "WRAP10",				   D3DRS_WRAP10 				 },
	{ FXST_RENDERSTATE, "WRAP11",				   D3DRS_WRAP11 				 },
	{ FXST_RENDERSTATE, "WRAP12",				   D3DRS_WRAP12 				 },
	{ FXST_RENDERSTATE, "WRAP13",				   D3DRS_WRAP13 				 },
	{ FXST_RENDERSTATE, "WRAP14",				   D3DRS_WRAP14 				 },
	{ FXST_RENDERSTATE, "WRAP15",				   D3DRS_WRAP15 				 },
	{ FXST_RENDERSTATE, "SEPARATEALPHABLENDENABLE",D3DRS_SEPARATEALPHABLENDENABLE},
	{ FXST_RENDERSTATE, "SRCBLENDALPHA",		   D3DRS_SRCBLENDALPHA			 },
	{ FXST_RENDERSTATE, "DESTBLENDALPHA",		   D3DRS_DESTBLENDALPHA 		 },
	{ FXST_RENDERSTATE, "BLENDOPALPHA", 		   D3DRS_BLENDOPALPHA			 },
	{ FXST_TSS, "COLOROP",        D3DTSS_COLOROP       },
	{ FXST_TSS, "COLORARG1",      D3DTSS_COLORARG1     },
	{ FXST_TSS, "COLORARG2",      D3DTSS_COLORARG2     },
	{ FXST_TSS, "ALPHAOP",        D3DTSS_ALPHAOP       },
	{ FXST_TSS, "ALPHAARG1",      D3DTSS_ALPHAARG1     },
	{ FXST_TSS, "ALPHAARG2",      D3DTSS_ALPHAARG2     },
	{ FXST_TSS, "BUMPENVMAT00",   D3DTSS_BUMPENVMAT00  },
	{ FXST_TSS, "BUMPENVMAT01",   D3DTSS_BUMPENVMAT01  },
	{ FXST_TSS, "BUMPENVMAT10",   D3DTSS_BUMPENVMAT10  },
	{ FXST_TSS, "BUMPENVMAT11",   D3DTSS_BUMPENVMAT11  },
	{ FXST_TSS, "TEXCOORDINDEX",  D3DTSS_TEXCOORDINDEX },
	{ FXST_TSS, "BUMPENVLSCALE",  D3DTSS_BUMPENVLSCALE },
	{ FXST_TSS, "BUMPENVLOFFSET", D3DTSS_BUMPENVLOFFSET},
	{ FXST_TSS, "TEXTURETRANSFORMFLAGS", D3DTSS_TEXTURETRANSFORMFLAGS},
	{ FXST_TSS, "COLORARG0",      D3DTSS_COLORARG0     },
	{ FXST_TSS, "ALPHAARG0",      D3DTSS_ALPHAARG0     },
	{ FXST_TSS, "RESULTARG",      D3DTSS_RESULTARG     },
	{ FXST_TSS, "CONSTANT",       D3DTSS_CONSTANT      },
	{ FXST_SAMPLER, "ADDRESSU",      D3DSAMP_ADDRESSU     },
	{ FXST_SAMPLER, "ADDRESSV",      D3DSAMP_ADDRESSV     },
	{ FXST_SAMPLER, "ADDRESSW",      D3DSAMP_ADDRESSW     },
	{ FXST_SAMPLER, "BORDERCOLOR",   D3DSAMP_BORDERCOLOR  },
	{ FXST_SAMPLER, "MAGFILTER",     D3DSAMP_MAGFILTER    },
	{ FXST_SAMPLER, "MINFILTER",     D3DSAMP_MINFILTER    },
	{ FXST_SAMPLER, "MIPFILTER",     D3DSAMP_MIPFILTER    },
	{ FXST_SAMPLER, "MIPMAPLODBIAS", D3DSAMP_MIPMAPLODBIAS},
	{ FXST_SAMPLER, "MAXMIPLEVEL",   D3DSAMP_MAXMIPLEVEL  },
	{ FXST_SAMPLER, "MAXANISOTROPY", D3DSAMP_MAXANISOTROPY},
	{ FXST_SAMPLER, "SRGBTEXTURE",   D3DSAMP_SRGBTEXTURE  },
	{ FXST_SAMPLER, "ELEMENTINDEX",  D3DSAMP_ELEMENTINDEX },
	{ FXST_SAMPLER, "DMAPOFFSET",    D3DSAMP_DMAPOFFSET   },
};

static int FX_STATES_SIZE = sizeof(FX_STATES) / sizeof(FX_STATES[0]);



// --------------------------------------------------------------------------


class CEffectStateInspector : public ID3DXEffectStateManager {
public:
	struct SState {
		SState( int pass_, int index_, int stage_, DWORD value_ )
			: pass(pass_), index(index_), stage(stage_), value(value_)
		{
			assert( index >= 0 && index < FX_STATES_SIZE );
		}

		int pass;		///< Effect pass
		int index;		///< Index into FX_STATES
		int stage;		///< For TSS, sampler etc.
		DWORD value;	///< Value
	};
public:
	CEffectStateInspector()
	{
		beginEffect();
	}
	
	void	beginEffect()
	{
		mCurrentPass = 0;
		mStates.clear();
		mStates.reserve( 32 );
	}

	void	beginPass()
	{
		++mCurrentPass;
	}

	void	debugPrintFx( const char* fileName )
	{
		FILE* f = fopen( fileName, "wt" );
		fprintf( f, "pass P0 {\n" );
		int n = mStates.size();
		for( int i = 0; i < n; ++i ) {
			// new pass?
			if( i > 0 && mStates[i].pass != mStates[i-1].pass ) {
				fprintf( f, "}\n" );
				fprintf( f, "pass P%i {\n", mStates[i].pass );
			}
			// dump states
			fprintf( f, "\t%s[%i] = %i\n",
				FX_STATES[mStates[i].index].name,
				mStates[i].stage,
				mStates[i].value
			);
		}
		fprintf( f, "}\n" );
	}

	// ------------------------------------------
	
	// ID3DXEffectStateManager
    STDMETHOD(SetTransform)( D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(SetMaterial)( const D3DMATERIAL9* material )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(SetLight)( DWORD index, const D3DLIGHT9* light )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(LightEnable)( DWORD index, BOOL enable )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(SetRenderState)( D3DRENDERSTATETYPE state, DWORD value )
	{
		int index = findState( FXST_RENDERSTATE, state );
		mStates.push_back( SState( mCurrentPass, index, 0, value ) );
		return S_OK;
	}

    STDMETHOD(SetTexture)( DWORD stage, IDirect3DBaseTexture9* texture )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(SetTextureStageState)( DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value )
	{
		int index = findState( FXST_TSS, type );
		mStates.push_back( SState( mCurrentPass, index, stage, value ) );
		return S_OK;
	}

    STDMETHOD(SetSamplerState)( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value )
	{
		int index = findState( FXST_SAMPLER, type );
		mStates.push_back( SState( mCurrentPass, index, sampler, value ) );
		return S_OK;
	}

    STDMETHOD(SetNPatchMode)( float numSegments )
	{
		int index = findState( FXST_NPATCH, 0 );
		mStates.push_back( SState( mCurrentPass, index, 0, 0 ) );
		return S_OK;
	}

    STDMETHOD(SetFVF)( DWORD fvf )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(SetVertexShader)( IDirect3DVertexShader9* shader )
	{
		int index = findState( FXST_VERTEXSHADER, 0 );
		mStates.push_back( SState( mCurrentPass, index, 0, 0 ) );
		return S_OK;
	}

    STDMETHOD(SetVertexShaderConstantF)( UINT registerIdx, const float* data, UINT registerCount )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT registerIdx, const int *data, UINT registerCount )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT registerIdx, const BOOL *data, UINT registerCount )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(SetPixelShader)( IDirect3DPixelShader9* shader )
	{
		int index = findState( FXST_PIXELSHADER, 0 );
		mStates.push_back( SState( mCurrentPass, index, 0, 0 ) );
		return S_OK;
	}

    STDMETHOD(SetPixelShaderConstantF)( UINT registerIdx, const float *data, UINT registerCount )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(SetPixelShaderConstantI)( UINT registerIdx, const int *data, UINT registerCount )
	{
		// don't inspect
		return S_OK;
	}

    STDMETHOD(SetPixelShaderConstantB)( UINT registerIdx, const BOOL *data, UINT registerCount )
	{
		// don't inspect
		return S_OK;
	}

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
		return 1;
	}
    STDMETHOD_(ULONG, Release)() {
		return 1;
	}

private:
	int		findState( eFxStateType type, int code ) const
	{
		for( int i = 0; i < FX_STATES_SIZE; ++i ) {
			if( FX_STATES[i].type == type && FX_STATES[i].code == code )
				return i;
		}
		ASSERT_FAIL_MSG( "Supplied effect state not found" );
		return -1;
	}

private:
	/// Inspected states
	std::vector<SState> mStates;
	/// Current pass
	int	mCurrentPass;
};


// --------------------------------------------------------------------------

bool dingus::loadD3DXEffect(
	const std::string& id, const std::string& fileName,
	CD3DXEffect& dest, std::string& errorMsgs,
	ID3DXEffectPool* pool, ID3DXEffectStateManager* stateManager,
	const D3DXMACRO* macros, bool optimizeShaders, CConsoleChannel& console )
{
	assert( dest.getObject() == NULL );
	
	ID3DXEffect* fx = NULL;
	ID3DXBuffer* errors = NULL;

	errorMsgs = "";

	assert( pool );
	HRESULT hres = D3DXCreateEffectFromFile(
		&CD3DDevice::getInstance().getDevice(),
		fileName.c_str(),
		macros,
		NULL, // TBD ==> includes
		optimizeShaders ? 0 : D3DXSHADER_SKIPOPTIMIZATION,
		pool,
		&fx,
		&errors );
	if( errors && errors->GetBufferSize() > 1 ) {
		std::string msg = "messages compiling effect '" + fileName + "': ";
		errorMsgs = (const char*)errors->GetBufferPointer();
		msg += errorMsgs;
		CConsole::CON_ERROR.write( msg );
	}

	if( FAILED( hres ) ) {
		return false;
	}
	assert( fx );

	console.write( "fx loaded '" + id + "'" );

	if( errors )
		errors->Release();


	dest.setObject( fx );

	// TBD TEST
	/*
	static CEffectStateInspector hackInspector;

	fx->SetStateManager( &hackInspector );
	hackInspector.beginEffect();
	int passes = dest.beginFx();
	for( int i = 0; i < passes; ++i ) {
		hackInspector.beginPass();
		dest.beginPass( i );
		dest.endPass();
		hackInspector.debugPrintFx( (fileName+".dbg.fx").c_str() );
	}
	dest.endFx();
	*/

	// set state manager
	if( stateManager )
		fx->SetStateManager( stateManager );

	return true;
}
