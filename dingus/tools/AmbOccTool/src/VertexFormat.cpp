// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "VertexFormat.h"

using namespace dingus;

int CVertexFormat::calcFloat3Size( eFloat3Mode flt3 )
{
	assert( flt3 >= FLT3_NONE && flt3 <= FLT3_DEC3N );
	int sizes[] = { 0, 3*4, 4, 4 };
	return sizes[flt3];
}

int CVertexFormat::calcFloat3Type( eFloat3Mode flt3 )
{
	assert( flt3 > FLT3_NONE && flt3 <= FLT3_DEC3N );
	int types[] = { 0, D3DDECLTYPE_FLOAT3, D3DDECLTYPE_D3DCOLOR, D3DDECLTYPE_DEC3N };
	return types[flt3];
}

int CVertexFormat::calcSkinSize() const
{
	eFloat3Mode data = getSkinDataMode();
	if( data == FLT3_NONE )
		return 0;
	eSkinMode skin = getSkinMode();
	int skinsize = 4; // indices
	if( skin == SKIN_1BONE )
		return skinsize;
	if( data != FLT3_FLOAT3 )
		skinsize += 4; // non-float case: all weights packed into single DWORD
	else
		skinsize += skin * 4; // float case: (bones-1) floats
	return skinsize;
}

int CVertexFormat::calcSkinDataType() const
{
	eFloat3Mode data = getSkinDataMode();
	assert( data != FLT3_NONE );
	if( data != FLT3_FLOAT3 )
		return calcFloat3Type( data ); // non-float case
	else {
		// float case
		eSkinMode skin = getSkinMode();
		switch( skin ) {
		case SKIN_2BONE: return D3DDECLTYPE_FLOAT1;
		case SKIN_3BONE: return D3DDECLTYPE_FLOAT2;
		case SKIN_4BONE: return D3DDECLTYPE_FLOAT3;
		}
		assert( false );
		return D3DDECLTYPE_FLOAT1;
	}
}

int CVertexFormat::calcUVType( eUVMode uv )
{
	assert( uv > UV_NONE && uv <= UV_3D );
	int types[] = { 0, D3DDECLTYPE_FLOAT1, D3DDECLTYPE_FLOAT2, D3DDECLTYPE_FLOAT3 };
	return types[uv];
}

int CVertexFormat::calcVertexSize() const
{
	int size = 0;
	// position
	if( hasPosition() )
		size += 3*4;
	// normals, tangents, binormals
	size += calcFloat3Size( getNormalMode() );
	size += calcFloat3Size( getTangentMode() );
	size += calcFloat3Size( getBinormMode() );
	// skin
	size += calcSkinSize();
	// color
	if( hasColor() )
		size += 4;
	// UVs
	for( int i = 0; i < UV_COUNT; ++i ) {
		size += calcUVSize( getUVMode(i) );
	}
	return size;
}

int CVertexFormat::calcComponentCount() const
{
	int size = 0;
	// position
	if( hasPosition() ) ++size;
	// normals, tangents, binormals
	if( getNormalMode() ) ++size;
	if( getTangentMode() ) ++size;
	if( getBinormMode() ) ++size;
	// skin
	if( getSkinDataMode() ) {
		if( getSkinMode() )
			size += 2; // weights and indices
		else
			++size; // just 1-bone indices
	}
	// color
	if( hasColor() ) ++size;
	// UVs
	for( int i = 0; i < UV_COUNT; ++i )
		if( getUVMode(i) ) ++size;
	return size;
}

#define COMMON_ELS { els->Stream = stream; els->Offset = offset; els->Method = D3DDECLMETHOD_DEFAULT; }

void CVertexFormat::calcVertexDecl( D3DVERTEXELEMENT9* els, int stream, int uvIdx ) const
{
	int offset = 0;
	// position
	if( hasPosition() ) {
		COMMON_ELS;
		els->Type = D3DDECLTYPE_FLOAT3; els->Usage = D3DDECLUSAGE_POSITION;
		els->UsageIndex = stream;
		++els;
		offset += 4*3;
	}
	// skin
	if( getSkinDataMode() ) {
		// weights (only if non-1-bone)
		if( getSkinMode() != SKIN_1BONE ) {
			COMMON_ELS;
			els->Type = calcSkinDataType(); els->Usage = D3DDECLUSAGE_BLENDWEIGHT;
			els->UsageIndex = stream;
			++els;
			offset += calcSkinSize()-4;
		}
		// indices
		COMMON_ELS;
		els->Type = D3DDECLTYPE_D3DCOLOR; els->Usage = D3DDECLUSAGE_BLENDINDICES;
		els->UsageIndex = stream;
		++els;
		offset += 4;
	}
	// normal
	if( getNormalMode() ) {
		COMMON_ELS;
		els->Type = calcFloat3Type(getNormalMode()); els->Usage = D3DDECLUSAGE_NORMAL;
		els->UsageIndex = stream;
		++els;
		offset += calcFloat3Size(getNormalMode());
	}
	// tangent
	if( getTangentMode() ) {
		COMMON_ELS;
		els->Type = calcFloat3Type(getTangentMode()); els->Usage = D3DDECLUSAGE_TANGENT;
		els->UsageIndex = stream;
		++els;
		offset += calcFloat3Size(getTangentMode());
	}
	// binormal
	if( getBinormMode() ) {
		COMMON_ELS;
		els->Type = calcFloat3Type(getBinormMode()); els->Usage = D3DDECLUSAGE_BINORMAL;
		els->UsageIndex = stream;
		++els;
		offset += calcFloat3Size(getBinormMode());
	}
	// color
	if( hasColor() ) {
		COMMON_ELS;
		els->Type = D3DDECLTYPE_D3DCOLOR; els->Usage = D3DDECLUSAGE_COLOR;
		els->UsageIndex = stream;
		++els;
		offset += 4;
	}
	// UVs
	for( int i = 0; i < UV_COUNT; ++i ) {
		eUVMode uv = getUVMode(i);
		if( uv ) {
			COMMON_ELS;
			els->Type = calcUVType(uv); els->Usage = D3DDECLUSAGE_TEXCOORD;
			els->UsageIndex = uvIdx + i;
			++els;
			offset += calcUVSize(uv);
		}
	}
}

int	CVertexFormat::calcSkinWeightsOffset() const
{
	return hasPosition() ? 4*3 : 0;
}
int	CVertexFormat::calcSkinIndicesOffset() const
{
	int offset = calcSkinWeightsOffset();
	if( getSkinDataMode() )
		offset += calcSkinSize()-4;
	return offset;
}
