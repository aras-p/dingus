// skinning stuff

#ifndef __SKINNING_FX
#define __SKINNING_FX

#include "shared.fx"

// bone matrices
static const int MAX_MATRICES = 25;
float4x3	mSkin[MAX_MATRICES];


// --------------------------------------------------------------------------
// 1-bone skinning

static inline void gSkinning(
	float4	ipos,
	float3	inormal,
	float4	findices,
	out float3 outPos,
	out float3 outNormal )
{
	// compensate for lack of UBYTE4 on Geforce3
	int4 indices = D3DCOLORtoUBYTE4( findices );

	outPos = mul( ipos, mSkin[indices[0]] );
	float3	normal = mul( inormal, (float3x3)mSkin[indices[0]] );

	// normalize normal
	outNormal = normalize( normal );
}

static inline void gSkinningPos(
	float4	ipos,
	float4	findices,
	out float3 outPos )
{
	// compensate for lack of UBYTE4 on Geforce3
	int4 indices = D3DCOLORtoUBYTE4( findices );

	outPos = mul( ipos, mSkin[indices[0]] );
}


// --------------------------------------------------------------------------
// 2-4 bone skinning

static inline void gSkinning(
	float4	ipos,
	float3	inormal,
	float4	findices,
	float3	weights,
	out float3 outPos,
	out float3 outNormal,
	uniform int NUM_BONES )
{
	// compensate for lack of UBYTE4 on Geforce3
	int4 indices = D3DCOLORtoUBYTE4( findices );

	// calculate the pos/normal using the "normal" weights and accumulate
	// the weights to calculate the last weight
	outPos = 0.0;
	float3	normal = 0.0;
	float	lastWeight = 0.0;
	for( int b = 0; b < NUM_BONES-1; ++b ) {
		float w = weights[b];
		lastWeight = lastWeight + w;
		int idx = indices[b];
		outPos += mul( ipos, mSkin[idx] ) * w;
		normal += mul( inormal, (float3x3)mSkin[idx] ) * w;
	}

	// add in last influence
	lastWeight = 1.0f - lastWeight;
	outPos += mul( ipos, mSkin[indices[NUM_BONES-1]] ) * lastWeight;
	normal += mul( inormal, (float3x3)mSkin[indices[NUM_BONES-1]] ) * lastWeight;
	
	// normalize normal
	outNormal = normalize( normal );
}

static inline void gSkinningPos(
	float4	ipos,
	float4	findices,
	float3	weights,
	out float3 outPos,
	uniform int NUM_BONES )
{
	// compensate for lack of UBYTE4 on Geforce3
	int4 indices = D3DCOLORtoUBYTE4( findices );

	// calculate the pos/normal using the "normal" weights and accumulate
	// the weights to calculate the last weight
	outPos = 0.0;
	float	lastWeight = 0.0;
	for( int b = 0; b < NUM_BONES-1; ++b ) {
		float w = weights[b];
		lastWeight = lastWeight + w;
		int idx = indices[b];
		outPos += mul( ipos, mSkin[idx] ) * w;
	}

	// add in last influence
	lastWeight = 1.0f - lastWeight;
	outPos += mul( ipos, mSkin[indices[NUM_BONES-1]] ) * lastWeight;
}


#endif
