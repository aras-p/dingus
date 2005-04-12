// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "EffectStateManager.h"
#include "D3DDevice.h"

using namespace dingus;

// define for testing only
//#define DISABLE_FILTERING


CEffectStateManager::CEffectStateManager()
{
	reset();
}

void CEffectStateManager::reset()
{
	memset( mLightEnabled, -1, sizeof(mLightEnabled) );
	memset( mGotRenderStates, 0, sizeof(mGotRenderStates) );
	memset( mTextures, -1, sizeof(mTextures) );
	memset( mGotTextureStates, 0, sizeof(mGotTextureStates) );
	memset( mGotSamplerStates, 0, sizeof(mGotSamplerStates) );
	mVShader = (IDirect3DVertexShader9*)-1;
	mPShader = (IDirect3DPixelShader9*)-1;
}

HRESULT CEffectStateManager::SetTransform( D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// no checking
	++stats.changes.transforms;
	return dx.SetTransform( state, matrix );
}

HRESULT CEffectStateManager::SetMaterial( const D3DMATERIAL9* material ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// no checking
	++stats.changes.lighting;
	return dx.SetMaterial( material );
}

HRESULT CEffectStateManager::SetLight( DWORD index, const D3DLIGHT9* light ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// no checking
	++stats.changes.lighting;
	return dx.SetLight( index, light );
}

HRESULT CEffectStateManager::LightEnable( DWORD index, BOOL enable ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// check first some
#ifndef DISABLE_FILTERING
	if( index < NUM_LIGHTS_CACHED ) {
		if( enable==mLightEnabled[index] ) {
			++stats.filtered.lighting;
			return S_OK;
		}
		mLightEnabled[index] = enable;
	}
#endif
	++stats.changes.lighting;
	return dx.LightEnable( index, enable );
}

HRESULT CEffectStateManager::SetRenderState( D3DRENDERSTATETYPE state, DWORD value ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// check first some
#ifndef DISABLE_FILTERING
	if( state < NUM_RENDERSTATES_CACHED ) {
		if( value==mRenderStates[state] && mGotRenderStates[state] ) {
			++stats.filtered.renderStates;
			return S_OK;
		}
		mRenderStates[state] = value;
		mGotRenderStates[state] = TRUE;
	}
#endif
	++stats.changes.renderStates;
	return dx.SetRenderState( state, value );
}

HRESULT CEffectStateManager::SetTexture( DWORD stage, IDirect3DBaseTexture9* texture ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// check first some
#ifndef DISABLE_FILTERING
	if( stage < NUM_TEXTURES_CACHED ) {
		if( texture==mTextures[stage] ) {
			++stats.filtered.textures;
			return S_OK;
		}
		mTextures[stage] = texture;
	}
#endif
	++stats.changes.textures;
	return dx.SetTexture( stage, texture );
}

HRESULT CEffectStateManager::SetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// check first some
#ifndef DISABLE_FILTERING
	if( stage < NUM_TEXTURES_CACHED && type < NUM_TEXSTATES_CACHED ) {
		if( value==mTextureStates[stage][type] && mGotTextureStates[stage][type] ) {
			++stats.filtered.textureStages;
			return S_OK;
		}
		mTextureStates[stage][type] = value;
		mGotTextureStates[stage][type] = TRUE;
	}
#endif
	++stats.changes.textureStages;
	return dx.SetTextureStageState( stage, type, value );
}

HRESULT CEffectStateManager::SetSamplerState( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// check first some
#ifndef DISABLE_FILTERING
	if( sampler < NUM_SAMPLERS_CACHED && type < NUM_SAMPSTATES_CACHED ) {
		if( value==mSamplerStates[sampler][type] && mGotSamplerStates[sampler][type] ) {
			++stats.filtered.samplers;
			return S_OK;
		}
		mSamplerStates[sampler][type] = value;
		mGotSamplerStates[sampler][type] = TRUE;
	}
#endif
	++stats.changes.samplers;
	return dx.SetSamplerState( sampler, type, value );
}

HRESULT CEffectStateManager::SetNPatchMode( float numSegments ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	// no checking
	return dx.SetNPatchMode( numSegments );
}

HRESULT CEffectStateManager::SetFVF( DWORD fvf ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	device.setDeclarationFVF( fvf ); // route through device
	return S_OK;
}

HRESULT CEffectStateManager::SetVertexShader( IDirect3DVertexShader9* shader ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// check
#ifndef DISABLE_FILTERING
	if( shader == mVShader ) {
		++stats.filtered.vsh;
		return S_OK;
	}
#endif
	mVShader = shader;
	++stats.changes.vsh;
	return dx.SetVertexShader( shader );
}

HRESULT CEffectStateManager::SetVertexShaderConstantF( UINT registerIdx, const float* data, UINT registerCount ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// no checking
	++stats.changes.vshConst;
	return dx.SetVertexShaderConstantF( registerIdx, data, registerCount );
}

HRESULT CEffectStateManager::SetVertexShaderConstantI(THIS_ UINT registerIdx, const int *data, UINT registerCount ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// no checking
	++stats.changes.vshConst;
	return dx.SetVertexShaderConstantI( registerIdx, data, registerCount );
}

HRESULT CEffectStateManager::SetVertexShaderConstantB(THIS_ UINT registerIdx, const BOOL *data, UINT registerCount ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// no checking
	++stats.changes.vshConst;
	return dx.SetVertexShaderConstantB( registerIdx, data, registerCount );
}

HRESULT CEffectStateManager::SetPixelShader( IDirect3DPixelShader9* shader ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// check
#ifndef DISABLE_FILTERING
	if( shader == mPShader ) {
		++stats.filtered.psh;
		return S_OK;
	}
#endif
	mPShader = shader;
	++stats.changes.psh;
	return dx.SetPixelShader( shader );
}

HRESULT CEffectStateManager::SetPixelShaderConstantF( UINT registerIdx, const float *data, UINT registerCount ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// no checking
	++stats.changes.pshConst;
	return dx.SetPixelShaderConstantF( registerIdx, data, registerCount );
}

HRESULT CEffectStateManager::SetPixelShaderConstantI( UINT registerIdx, const int *data, UINT registerCount ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// no checking
	++stats.changes.pshConst;
	return dx.SetPixelShaderConstantI( registerIdx, data, registerCount );
}

HRESULT CEffectStateManager::SetPixelShaderConstantB( UINT registerIdx, const BOOL *data, UINT registerCount ) {
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	// no checking
	++stats.changes.pshConst;
	return dx.SetPixelShaderConstantB( registerIdx, data, registerCount );
}
