// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "ModelDesc.h"
#include "../resource/EffectBundle.h"
#include "../resource/TextureBundle.h"
#include "../resource/SharedTextureBundle.h"
#include "../resource/CubeTextureBundle.h"

using namespace dingus;

DEFINE_POOLED_ALLOC(dingus::CModelDesc,128,false);

CModelDesc::SParams::SParams( const SParams& rhs )
:	textures(rhs.textures),
	cubemaps(rhs.cubemaps),
	stextures(rhs.stextures),
	vectors3(rhs.vectors3),
	vectors4(rhs.vectors4),
	floats(rhs.floats)
{
}

const CModelDesc::SParams& CModelDesc::SParams::operator=( const SParams& rhs )
{
	textures = rhs.textures;
	cubemaps = rhs.cubemaps;
	stextures = rhs.stextures;
	vectors3 = rhs.vectors3;
	vectors4 = rhs.vectors4;
	floats = rhs.floats;
	return *this;
}

CModelDesc::SGroup::SGroup( const SGroup& rhs )
:	params(rhs.params),
	fxID(rhs.fxID),
	renderPriority(rhs.renderPriority)
{
}

const CModelDesc::SGroup& CModelDesc::SGroup::operator=( const SGroup& rhs )
{
	params = rhs.params;
	fxID = rhs.fxID;
	renderPriority = rhs.renderPriority;
	return *this;
}

void CModelDesc::addGroup( int renderPriority, const CResourceId& fxID )
{
	mGroups.push_back( SGroup(fxID,renderPriority) );
}

void CModelDesc::addParamTexture( int group, const std::string& name, const std::string& value )
{
	assert(group>=0&&group<getGroupCount());
	SGroup& g = mGroups[group];
	g.params.textures.push_back( std::make_pair(name,value) );
}

void CModelDesc::addParamCubemap( int group, const std::string& name, const std::string& value )
{
	assert(group>=0&&group<getGroupCount());
	SGroup& g = mGroups[group];
	g.params.cubemaps.push_back( std::make_pair(name,value) );
}

void CModelDesc::addParamSTexture( int group, const std::string& name, const std::string& value )
{
	assert(group>=0&&group<getGroupCount());
	SGroup& g = mGroups[group];
	g.params.stextures.push_back( std::make_pair(name,value) );
}

void CModelDesc::addParamVec3( int group, const std::string& name, const SVector3& value )
{
	assert(group>=0&&group<getGroupCount());
	SGroup& g = mGroups[group];
	g.params.vectors3.push_back( std::make_pair(name,value) );
}

void CModelDesc::addParamVec4( int group, const std::string& name, const SVector4& value )
{
	assert(group>=0&&group<getGroupCount());
	SGroup& g = mGroups[group];
	g.params.vectors4.push_back( std::make_pair(name,value) );
}

void CModelDesc::addParamFloat( int group, const std::string& name, float value )
{
	assert(group>=0&&group<getGroupCount());
	SGroup& g = mGroups[group];
	g.params.floats.push_back( std::make_pair(name,value) );
}

void CModelDesc::fillFxParams( int group, CEffectParams& dest ) const
{
	const SParams& p = getParams( group );
	dest.setEffect( *RGET_FX( getFxID(group) ) );

	size_t i;
	for( i = 0; i < p.textures.size(); ++i ) {
		const SParams::TNameIDPair& e = p.textures[i];	
		dest.addTexture( e.first.c_str(), *RGET_TEX( e.second ) );
	}
	for( i = 0; i < p.cubemaps.size(); ++i ) {
		const SParams::TNameIDPair& e = p.cubemaps[i];	
		dest.addCubeTexture( e.first.c_str(), *RGET_CUBETEX( e.second ) );
	}
	for( i = 0; i < p.stextures.size(); ++i ) {
		const SParams::TNameIDPair& e = p.stextures[i];	
		dest.addTexture( e.first.c_str(), *RGET_S_TEX( e.second ) );
	}
	for( i = 0; i < p.vectors3.size(); ++i ) {
		const SParams::TNameVec3Pair& e = p.vectors3[i];	
		dest.addVector3( e.first.c_str(), e.second );
	}
	for( i = 0; i < p.vectors4.size(); ++i ) {
		const SParams::TNameVec4Pair& e = p.vectors4[i];	
		dest.addVector4( e.first.c_str(), e.second );
	}
	for( i = 0; i < p.floats.size(); ++i ) {
		const SParams::TNameFloatPair& e = p.floats[i];	
		dest.addFloat( e.first.c_str(), e.second );
	}
}

