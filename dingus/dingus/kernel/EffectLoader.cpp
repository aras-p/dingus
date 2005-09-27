// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "EffectLoader.h"
#include "ProxyEffect.h"
#include "D3DDevice.h"
#include "../console/Console.h"
#include "../utils/Errors.h"

#include "../lua/LuaSingleton.h"
#include "../lua/LuaHelper.h"
#include "../lua/LuaIterator.h"


using namespace dingus;

// --------------------------------------------------------------------------

namespace {

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



SFxState FX_STATES[] = {
	{ FXST_VERTEXSHADER, "VertexShader", 0 },
	{ FXST_PIXELSHADER, "PixelShader", 0 },
	{ FXST_NPATCH, "PatchSegments", 0 },
	{ FXST_RENDERSTATE, "ZEnable",				   D3DRS_ZENABLE				 },
	{ FXST_RENDERSTATE, "FillMode", 			   D3DRS_FILLMODE				 },
	{ FXST_RENDERSTATE, "ShadeMode",			   D3DRS_SHADEMODE				 },
	{ FXST_RENDERSTATE, "ZWriteEnable", 		   D3DRS_ZWRITEENABLE			 },
	{ FXST_RENDERSTATE, "AlphaTestEnable",		   D3DRS_ALPHATESTENABLE		 },
	{ FXST_RENDERSTATE, "LastPixel",			   D3DRS_LASTPIXEL				 },
	{ FXST_RENDERSTATE, "SrcBlend", 			   D3DRS_SRCBLEND				 },
	{ FXST_RENDERSTATE, "DestBlend",			   D3DRS_DESTBLEND				 },
	{ FXST_RENDERSTATE, "CullMode", 			   D3DRS_CULLMODE				 },
	{ FXST_RENDERSTATE, "ZFunc",				   D3DRS_ZFUNC					 },
	{ FXST_RENDERSTATE, "AlphaRef", 			   D3DRS_ALPHAREF				 },
	{ FXST_RENDERSTATE, "AlphaFunc",			   D3DRS_ALPHAFUNC				 },
	{ FXST_RENDERSTATE, "DitherEnable", 		   D3DRS_DITHERENABLE			 },
	{ FXST_RENDERSTATE, "AlphaBlendEnable", 	   D3DRS_ALPHABLENDENABLE		 },
	{ FXST_RENDERSTATE, "FogEnable",			   D3DRS_FOGENABLE				 },
	{ FXST_RENDERSTATE, "SpecularEnable",		   D3DRS_SPECULARENABLE 		 },
	{ FXST_RENDERSTATE, "FogColor", 			   D3DRS_FOGCOLOR				 },
	{ FXST_RENDERSTATE, "FogTableMode", 		   D3DRS_FOGTABLEMODE			 },
	{ FXST_RENDERSTATE, "FogStart", 			   D3DRS_FOGSTART				 },
	{ FXST_RENDERSTATE, "FogEnd",				   D3DRS_FOGEND 				 },
	{ FXST_RENDERSTATE, "FogDensity",			   D3DRS_FOGDENSITY 			 },
	{ FXST_RENDERSTATE, "RangeFogEnable",		   D3DRS_RANGEFOGENABLE 		 },
	{ FXST_RENDERSTATE, "StencilEnable",		   D3DRS_STENCILENABLE			 },
	{ FXST_RENDERSTATE, "StencilFail",			   D3DRS_STENCILFAIL			 },
	{ FXST_RENDERSTATE, "StencilZFail", 		   D3DRS_STENCILZFAIL			 },
	{ FXST_RENDERSTATE, "StencilPass",			   D3DRS_STENCILPASS			 },
	{ FXST_RENDERSTATE, "StencilFunc",			   D3DRS_STENCILFUNC			 },
	{ FXST_RENDERSTATE, "StencilRef",			   D3DRS_STENCILREF 			 },
	{ FXST_RENDERSTATE, "StencilMask",			   D3DRS_STENCILMASK			 },
	{ FXST_RENDERSTATE, "StencilWriteMask", 	   D3DRS_STENCILWRITEMASK		 },
	{ FXST_RENDERSTATE, "TextureFactor",		   D3DRS_TEXTUREFACTOR			 },
	{ FXST_RENDERSTATE, "Wrap0",				   D3DRS_WRAP0					 },
	{ FXST_RENDERSTATE, "Wrap1",				   D3DRS_WRAP1					 },
	{ FXST_RENDERSTATE, "Wrap2",				   D3DRS_WRAP2					 },
	{ FXST_RENDERSTATE, "Wrap3",				   D3DRS_WRAP3					 },
	{ FXST_RENDERSTATE, "Wrap4",				   D3DRS_WRAP4					 },
	{ FXST_RENDERSTATE, "Wrap5",				   D3DRS_WRAP5					 },
	{ FXST_RENDERSTATE, "Wrap6",				   D3DRS_WRAP6					 },
	{ FXST_RENDERSTATE, "Wrap7",				   D3DRS_WRAP7					 },
	{ FXST_RENDERSTATE, "Clipping", 			   D3DRS_CLIPPING				 },
	{ FXST_RENDERSTATE, "Lighting", 			   D3DRS_LIGHTING				 },
	{ FXST_RENDERSTATE, "Ambient",				   D3DRS_AMBIENT				 },
	{ FXST_RENDERSTATE, "FogVertexMode",		   D3DRS_FOGVERTEXMODE			 },
	{ FXST_RENDERSTATE, "ColorVertex",			   D3DRS_COLORVERTEX			 },
	{ FXST_RENDERSTATE, "LocalViewer",			   D3DRS_LOCALVIEWER			 },
	{ FXST_RENDERSTATE, "NormalizeNormals", 	   D3DRS_NORMALIZENORMALS		 },
	{ FXST_RENDERSTATE, "DiffuseMaterialSource",   D3DRS_DIFFUSEMATERIALSOURCE	 },
	{ FXST_RENDERSTATE, "SpecularMaterialSource",  D3DRS_SPECULARMATERIALSOURCE  },
	{ FXST_RENDERSTATE, "AmbientMaterialSource",   D3DRS_AMBIENTMATERIALSOURCE	 },
	{ FXST_RENDERSTATE, "EmissiveMaterialSource",  D3DRS_EMISSIVEMATERIALSOURCE  },
	{ FXST_RENDERSTATE, "VertexBlend",			   D3DRS_VERTEXBLEND			 },
	{ FXST_RENDERSTATE, "ClipPlaneEnable",		   D3DRS_CLIPPLANEENABLE		 },
	{ FXST_RENDERSTATE, "PointSize",			   D3DRS_POINTSIZE				 },
	{ FXST_RENDERSTATE, "PointSize_Min",		   D3DRS_POINTSIZE_MIN			 },
	{ FXST_RENDERSTATE, "PointSpriteEnable",	   D3DRS_POINTSPRITEENABLE		 },
	{ FXST_RENDERSTATE, "PointScaleEnable", 	   D3DRS_POINTSCALEENABLE		 },
	{ FXST_RENDERSTATE, "PointScale_A", 		   D3DRS_POINTSCALE_A			 },
	{ FXST_RENDERSTATE, "PointScale_B", 		   D3DRS_POINTSCALE_B			 },
	{ FXST_RENDERSTATE, "PointScale_C", 		   D3DRS_POINTSCALE_C			 },
	{ FXST_RENDERSTATE, "MultiSampleAntiAlias",    D3DRS_MULTISAMPLEANTIALIAS	 },
	{ FXST_RENDERSTATE, "MultiSampleMask",		   D3DRS_MULTISAMPLEMASK		 },
	{ FXST_RENDERSTATE, "PatchEdgeStyle",		   D3DRS_PATCHEDGESTYLE 		 },
	{ FXST_RENDERSTATE, "DebugMonitorToken",	   D3DRS_DEBUGMONITORTOKEN		 },
	{ FXST_RENDERSTATE, "PointSize_Max",		   D3DRS_POINTSIZE_MAX			 },
	{ FXST_RENDERSTATE, "IndexedVertexBlendEnable",D3DRS_INDEXEDVERTEXBLENDENABLE},
	{ FXST_RENDERSTATE, "ColorWriteEnable", 	   D3DRS_COLORWRITEENABLE		 },
	{ FXST_RENDERSTATE, "TweenFactor",			   D3DRS_TWEENFACTOR			 },
	{ FXST_RENDERSTATE, "BlendOp",				   D3DRS_BLENDOP				 },
	{ FXST_RENDERSTATE, "PositionDegree",		   D3DRS_POSITIONDEGREE 		 },
	{ FXST_RENDERSTATE, "NormalDegree", 		   D3DRS_NORMALDEGREE			 },
	{ FXST_RENDERSTATE, "ScissorTestEnable",	   D3DRS_SCISSORTESTENABLE		 },
	{ FXST_RENDERSTATE, "SlopeScaleDepthBias",	   D3DRS_SLOPESCALEDEPTHBIAS	 },
	{ FXST_RENDERSTATE, "AntiAliasedLineEnable",   D3DRS_ANTIALIASEDLINEENABLE	 },
	{ FXST_RENDERSTATE, "MinTessellationLevel",    D3DRS_MINTESSELLATIONLEVEL	 },
	{ FXST_RENDERSTATE, "MaxTessellationLevel",    D3DRS_MAXTESSELLATIONLEVEL	 },
	{ FXST_RENDERSTATE, "TwoSidedStencilMode",	   D3DRS_TWOSIDEDSTENCILMODE	 },
	{ FXST_RENDERSTATE, "CCW_StencilFAIL",		   D3DRS_CCW_STENCILFAIL		 },
	{ FXST_RENDERSTATE, "CCW_StencilZFAIL", 	   D3DRS_CCW_STENCILZFAIL		 },
	{ FXST_RENDERSTATE, "CCW_StencilPASS",		   D3DRS_CCW_STENCILPASS		 },
	{ FXST_RENDERSTATE, "CCW_StencilFUNC",		   D3DRS_CCW_STENCILFUNC		 },
	{ FXST_RENDERSTATE, "ColorWriteEnable1",	   D3DRS_COLORWRITEENABLE1		 },
	{ FXST_RENDERSTATE, "ColorWriteEnable2",	   D3DRS_COLORWRITEENABLE2		 },
	{ FXST_RENDERSTATE, "ColorWriteEnable3",	   D3DRS_COLORWRITEENABLE3		 },
	{ FXST_RENDERSTATE, "BlendFactor",			   D3DRS_BLENDFACTOR			 },
	{ FXST_RENDERSTATE, "SRGBWriteEnable",		   D3DRS_SRGBWRITEENABLE		 },
	{ FXST_RENDERSTATE, "DepthBias",			   D3DRS_DEPTHBIAS				 },
	{ FXST_RENDERSTATE, "Wrap8",				   D3DRS_WRAP8					 },
	{ FXST_RENDERSTATE, "Wrap9",				   D3DRS_WRAP9					 },
	{ FXST_RENDERSTATE, "Wrap10",				   D3DRS_WRAP10 				 },
	{ FXST_RENDERSTATE, "Wrap11",				   D3DRS_WRAP11 				 },
	{ FXST_RENDERSTATE, "Wrap12",				   D3DRS_WRAP12 				 },
	{ FXST_RENDERSTATE, "Wrap13",				   D3DRS_WRAP13 				 },
	{ FXST_RENDERSTATE, "Wrap14",				   D3DRS_WRAP14 				 },
	{ FXST_RENDERSTATE, "Wrap15",				   D3DRS_WRAP15 				 },
	{ FXST_RENDERSTATE, "SeparateAlphaBlendEnable",D3DRS_SEPARATEALPHABLENDENABLE},
	{ FXST_RENDERSTATE, "SrcBlendAlpha",		   D3DRS_SRCBLENDALPHA			 },
	{ FXST_RENDERSTATE, "DestBlendAlpha",		   D3DRS_DESTBLENDALPHA 		 },
	{ FXST_RENDERSTATE, "BlendOpAlpha", 		   D3DRS_BLENDOPALPHA			 },
	{ FXST_TSS, "ColorOp",        D3DTSS_COLOROP       },
	{ FXST_TSS, "ColorArg1",      D3DTSS_COLORARG1     },
	{ FXST_TSS, "ColorArg2",      D3DTSS_COLORARG2     },
	{ FXST_TSS, "AlphaOp",        D3DTSS_ALPHAOP       },
	{ FXST_TSS, "AlphaArg1",      D3DTSS_ALPHAARG1     },
	{ FXST_TSS, "AlphaArg2",      D3DTSS_ALPHAARG2     },
	{ FXST_TSS, "BumpEnvMat00",   D3DTSS_BUMPENVMAT00  },
	{ FXST_TSS, "BumpEnvMat01",   D3DTSS_BUMPENVMAT01  },
	{ FXST_TSS, "BumpEnvMat10",   D3DTSS_BUMPENVMAT10  },
	{ FXST_TSS, "BumpEnvMat11",   D3DTSS_BUMPENVMAT11  },
	{ FXST_TSS, "TexCoordIndex",  D3DTSS_TEXCOORDINDEX },
	{ FXST_TSS, "BumpEnvLScale",  D3DTSS_BUMPENVLSCALE },
	{ FXST_TSS, "BumpEnvLOffset", D3DTSS_BUMPENVLOFFSET},
	{ FXST_TSS, "TextureTransformFlags", D3DTSS_TEXTURETRANSFORMFLAGS},
	{ FXST_TSS, "ColorArg0",      D3DTSS_COLORARG0     },
	{ FXST_TSS, "AlphaArg0",      D3DTSS_ALPHAARG0     },
	{ FXST_TSS, "ResultArg",      D3DTSS_RESULTARG     },
	{ FXST_TSS, "Constant",       D3DTSS_CONSTANT      },
	{ FXST_SAMPLER, "AddressU",      D3DSAMP_ADDRESSU     },
	{ FXST_SAMPLER, "AddressV",      D3DSAMP_ADDRESSV     },
	{ FXST_SAMPLER, "AddressW",      D3DSAMP_ADDRESSW     },
	{ FXST_SAMPLER, "BorderColor",   D3DSAMP_BORDERCOLOR  },
	{ FXST_SAMPLER, "MagFilter",     D3DSAMP_MAGFILTER    },
	{ FXST_SAMPLER, "MinFilter",     D3DSAMP_MINFILTER    },
	{ FXST_SAMPLER, "MipFilter",     D3DSAMP_MIPFILTER    },
	{ FXST_SAMPLER, "MipMapLodBias", D3DSAMP_MIPMAPLODBIAS},
	{ FXST_SAMPLER, "MaxMipLevel",   D3DSAMP_MAXMIPLEVEL  },
	{ FXST_SAMPLER, "MaxAnisotropy", D3DSAMP_MAXANISOTROPY},
	{ FXST_SAMPLER, "SRGBTexture",   D3DSAMP_SRGBTEXTURE  },
	{ FXST_SAMPLER, "ElementIndex",  D3DSAMP_ELEMENTINDEX },
	{ FXST_SAMPLER, "DMapOffset",    D3DSAMP_DMAPOFFSET   },
};

const int FX_STATES_SIZE = sizeof(FX_STATES) / sizeof(FX_STATES[0]);


int	findState( eFxStateType type, int code )
{
	for( int i = 0; i < FX_STATES_SIZE; ++i ) {
		if( FX_STATES[i].type == type && FX_STATES[i].code == code )
			return i;
	}
	ASSERT_FAIL_MSG( "Supplied effect state not found" );
	return -1;
}

int findState( const char* name )
{
	for( int i = 0; i < FX_STATES_SIZE; ++i ) {
		if( 0 == stricmp( FX_STATES[i].name, name ) )
			return i;
	}
	ASSERT_FAIL_MSG( "Supplied effect state not found" );
	return -1;
}


}; // end anonymous namespace


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
	typedef std::vector<SState> TStateVector;

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

	const TStateVector& getStates() const { return mStates; }


	void	debugPrintFx( const char* fileName )
	{
		FILE* f = fopen( fileName, "wt" );
		fprintf( f, "pass P0 {\n" );
		int n = mStates.size();
		for( int i = 0; i < n; ++i ) {
			const SState& s = mStates[i];
			// new pass?
			if( i > 0 && s.pass != mStates[i-1].pass ) {
				fprintf( f, "}\n" );
				fprintf( f, "pass P%i {\n", s.pass );
			}
			// dump states
			if( s.stage < 0 ) {
				fprintf( f, "\t%s = %i\n",
					FX_STATES[s.index].name,
					s.value
				);
			} else {
				fprintf( f, "\t%s[%i] = %i\n",
					FX_STATES[s.index].name,
					s.stage,
					s.value
				);
			}
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
		mStates.push_back( SState( mCurrentPass, index, -1, value ) );
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
		mStates.push_back( SState( mCurrentPass, index, -1, numSegments ) );
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
		mStates.push_back( SState( mCurrentPass, index, -1, shader ? 1 : 0 ) );
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
		mStates.push_back( SState( mCurrentPass, index, -1, shader ? 1 : 0 ) );
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
	/// Inspected states
	TStateVector mStates;
	/// Current pass
	int	mCurrentPass;
};


// --------------------------------------------------------------------------


class CEffectRestorePassGenerator : public boost::noncopyable {
public:
	bool loadConfig( const char* fileName );
	std::string generateRestorePass( const CEffectStateInspector& fx ) const;

private:
	struct SStateRestored {
		SStateRestored( int idx, const std::string& val )
			: index(idx), value(val)
		{
			assert( index >= 0 && index < FX_STATES_SIZE );
			assert( !value.empty() );
		}

		int			index;	///< Index into FX_STATES
		std::string value;	///< Restored to value (plain text, will be interpreted by Effect).
	};
	struct SStateRequired {
		SStateRequired( int idx )
			: index(idx)
		{
			assert( index >= 0 && index < FX_STATES_SIZE );
		}

		int			index;	///< Index into FX_STATES
	};
	struct SStateDependent {
		SStateDependent( int idx, DWORD val )
			: index(idx), value(val)
		{
			assert( index >= 0 && index < FX_STATES_SIZE );
		}

		int			index;			///< If this state (index into FX_STATES)
		DWORD		value;			///< Is set to this value
		std::vector<int> needed;	///< All these states are needed (indices into FX_STATES)
	};

private:
	const SStateRestored* findRestoredState( int index ) const {
		int n = mStatesRestored.size();
		for( int i = 0; i < n; ++i ) {
			if( mStatesRestored[i].index == index )
				return &mStatesRestored[i];
		}
		return NULL;
	}

private:
	std::vector<SStateRestored>		mStatesRestored;
	std::vector<SStateRequired>		mStatesRequired;
	std::vector<SStateDependent>	mStatesDependent;
};


bool CEffectRestorePassGenerator::loadConfig( const char* fileName )
{
	// clear
	mStatesRestored.clear();
	mStatesRestored.reserve( 64 );
	mStatesRequired.clear();
	mStatesRequired.reserve( 16 );
	mStatesDependent.clear();
	mStatesDependent.reserve( 16 );

	// execute file
	CLuaSingleton& lua = CLuaSingleton::getInstance();
	int errorCode = lua.doFile( fileName, false );
	if( errorCode )
		return false; // error

	// read restored states
	CLuaValue luaRestored = lua.getGlobal("restored");
	CLuaArrayIterator itRestored( luaRestored );
	while( itRestored.hasNext() ) {
		CLuaValue& luaSt = itRestored.next();

		std::string name = luaSt.getElement(1).getString();
		luaSt.discard();
		std::string value = luaSt.getElement(2).getString();
		luaSt.discard();

		int index = findState( name.c_str() );
		mStatesRestored.push_back( SStateRestored( index, value ) );
	}
	luaRestored.discard();
	
	// read required states
	CLuaValue luaRequired = lua.getGlobal("required");
	CLuaArrayIterator itRequired( luaRequired );
	while( itRequired.hasNext() ) {
		CLuaValue& luaSt = itRequired.next();
		std::string name = luaSt.getString();

		int index = findState( name.c_str() );
		mStatesRequired.push_back( SStateRequired( index ) );
	}
	luaRequired.discard();

	// read dependent states
	CLuaValue luaDependent = lua.getGlobal("dependent");
	CLuaArrayIterator itDependent( luaDependent );
	while( itDependent.hasNext() ) {
		CLuaValue& luaSt = itDependent.next();
		
		std::string name = luaSt.getElement(1).getString();
		luaSt.discard();
		int value = luaSt.getElement(2).getNumber();
		luaSt.discard();

		int index = findState( name.c_str() );
		mStatesDependent.push_back( SStateDependent( index, value ) );
		SStateDependent& st = mStatesDependent.back();

		CLuaValue luaNames = luaSt.getElement(3);
		CLuaArrayIterator itNames( luaNames );
		while( itNames.hasNext() ) {
			CLuaValue& luaN = itNames.next();
			st.needed.push_back( findState( luaN.getString().c_str() ) );
		}
		luaNames.discard();
	}
	luaDependent.discard();

	return true;
}

std::string CEffectRestorePassGenerator::generateRestorePass( const CEffectStateInspector& fx ) const
{
	int i;
	int nstates = fx.getStates().size();

	// TBD: check required states
	// TBD: check dependent states

	// generate restoring pass
	// NOTE: it seems that (Oct 2004 SDK) fx macros don't support newlines.
	// Oh well, generate one long line...
	std::string res;
	res.reserve( 128 );
	res  = std::string("pass ") + DINGUS_FX_RESTORE_PASS + " {";

	// fill restored states
	// TBD: remove duplicates
	for( i = 0; i < nstates; ++i ) {
		const CEffectStateInspector::SState& state = fx.getStates()[i];
		const SStateRestored* st = findRestoredState( state.index );
		if( !st )
			continue;
		res += " ";
		res += FX_STATES[state.index].name;
		if( state.stage >= 0 ) {
			char buf[10];
			res += '[';
			itoa( state.stage, buf, 10 );
			res += buf;
			res += ']';
		}
		res += '=';
		if( st->value == "@index@" ) {
			assert( stage >= 0 );
			char buf[10];
			itoa( state.stage, buf, 10 );
			res += buf;
		} else {
			res += st->value;
		}
		res += ';';
	}

	res += " }";
	return res;
}


namespace {
	CEffectRestorePassGenerator*	gFxRestorePassGen = 0;
}; // end of anonymous namespace

bool fxloader::initialize( const char* cfgFileName )
{
	assert( !gFxRestorePassGen );
	gFxRestorePassGen = new CEffectRestorePassGenerator();
	return gFxRestorePassGen->loadConfig( cfgFileName );
}

void fxloader::shutdown()
{
	safeDelete( gFxRestorePassGen );
}


// --------------------------------------------------------------------------

bool dingus::fxloader::load(
	const std::string& id, const std::string& fileName,
	CD3DXEffect& dest, std::string& errorMsgs,
	ID3DXEffectPool* pool, ID3DXEffectStateManager* stateManager,
	const D3DXMACRO* macros, int macroCount, bool optimizeShaders, CConsoleChannel& console )
{
	// 1. load effect from file, find valid technique
	// 2. if it has restoring pass, exit: all is done
	// 3. inspect the valid technique
	// 4. generate restoring pass
	// 5. supply restoring pass as macro; load effect again
	// 6. check that it has a restoring pass

	assert( dest.getObject() == NULL );

	console.write( "loading fx '" + id + "'" );

	// add macro RESTORE_PASS to supplied ones, initially empty
	assert( macroCount > 0 );
	D3DXMACRO* newMacros = new D3DXMACRO[macroCount+1];
	memcpy( newMacros, macros, macroCount * sizeof(macros[0]) );
	newMacros[macroCount] = newMacros[macroCount-1];
	newMacros[macroCount-1].Name = "RESTORE_PASS";
	newMacros[macroCount-1].Definition = "";

	// load the effect initially
	ID3DXEffect* fx = NULL;
	ID3DXBuffer* errors = NULL;

	errorMsgs = "";

	assert( pool );
	HRESULT hres = D3DXCreateEffectFromFile(
		&CD3DDevice::getInstance().getDevice(),
		fileName.c_str(), newMacros, NULL, // TBD ==> includes
		optimizeShaders ? 0 : D3DXSHADER_SKIPOPTIMIZATION,
		pool, &fx, &errors );
	if( errors && errors->GetBufferSize() > 1 ) {
		std::string msg = "messages compiling effect '" + fileName + "': ";
		errorMsgs = (const char*)errors->GetBufferPointer();
		msg += errorMsgs;
		CConsole::CON_ERROR.write( msg );
	}
	if( FAILED( hres ) ) {
		delete[] newMacros;
		return false;
	}
	assert( fx );
	
	if( errors )
		errors->Release();


	// initialize effect object
	dest.setObject( fx );

	// if already has restoring pass, return
	if( dest.hasRestoringPass() ) {
		// set state manager
		if( stateManager )
			fx->SetStateManager( stateManager );
		console.write( "fx loaded, already has restoring pass" );
		delete[] newMacros;
		return true;
	}

	// examine effect state assignments
	console.write( "fx loaded, generating state restore pass" );
	static CEffectStateInspector	inspector;

	fx->SetStateManager( &inspector );
	inspector.beginEffect();
	int passes = dest.beginFx();
	for( int i = 0; i < passes; ++i ) {
		inspector.beginPass();
		dest.beginPass( i );
		dest.endPass();
	}
	dest.endFx();

	//inspector.debugPrintFx( (fileName+".ins.fx").c_str() );

	// generate restore pass
	assert( gFxRestorePassGen );
	std::string restorePass = gFxRestorePassGen->generateRestorePass( inspector );
	// debug
	/*{
		FILE* f = fopen( (fileName+".res.fx").c_str(), "wt" );
		fputs( restorePass.c_str(), f );
		fclose( f );
	}*/

	// supply restore pass text as RESTORE_PASS value
	newMacros[macroCount-1].Definition = restorePass.c_str();

	// compile the effect again
	dest.getObject()->Release();
	dest.setObject( NULL );
	fx = NULL;
	errors = NULL;
	errorMsgs = "";
	assert( pool );
	hres = D3DXCreateEffectFromFile(
		&CD3DDevice::getInstance().getDevice(),
		fileName.c_str(), newMacros, NULL, // TBD ==> includes
		optimizeShaders ? 0 : D3DXSHADER_SKIPOPTIMIZATION,
		pool, &fx, &errors );
	if( errors && errors->GetBufferSize() > 1 ) {
		std::string msg = "messages compiling effect #2 '" + fileName + "': ";
		errorMsgs = (const char*)errors->GetBufferPointer();
		msg += errorMsgs;
		CConsole::CON_ERROR.write( msg );
	}
	if( FAILED( hres ) ) {
		delete[] newMacros;
		return false;
	}
	assert( fx );
	if( errors )
		errors->Release();

	// finally initialize effect object
	dest.setObject( fx );

	// it must have the restore pass now
	if( !dest.hasRestoringPass() ) {
		errorMsgs = "Effect does not have restore pass after generation. Perhaps RESTORE_PASS macro missing?";
		CConsole::CON_ERROR.write( errorMsgs );

		dest.getObject()->Release();
		dest.setObject( NULL );
		delete[] newMacros;
		return false;
	}

	// set state manager
	if( stateManager )
		fx->SetStateManager( stateManager );

	delete[] newMacros;
	return true;
}
