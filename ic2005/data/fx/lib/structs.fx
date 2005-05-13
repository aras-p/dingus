#ifndef __STRUCTS_FX
#define __STRUCTS_FX

// --------------------------------------------------------------------------
//  common structs

// position
struct SPos {
	float4	pos		: POSITION;
};


// position, normal, various textures/colors
struct SPosN {
	float4	pos		: POSITION;
	float3	normal	: NORMAL;
};
struct SPosNCol {
	float4	pos		: POSITION;
	float3	normal	: NORMAL;
	half4 		color	: COLOR;
};
struct SPosNTex {
	float4	pos		: POSITION;
	float3	normal	: NORMAL;
	float2	uv		: TEXCOORD0;
};
struct SPosNTex2 {
	float4	pos		: POSITION;
	float3	normal	: NORMAL;
	float2	uv[2]	: TEXCOORD0;
};


// position, various textures
struct SPosTex {
	float4 pos		: POSITION;
	float2 uv		: TEXCOORD0;
};
struct SPosTex2 {
	float4 pos		: POSITION;
	float2 uv[2]	: TEXCOORD0;
};
struct SPosTex3 {
	float4 pos		: POSITION;
	float2 uv[3]	: TEXCOORD0;
};
struct SPosTex4 {
	float4 pos		: POSITION;
	float2 uv[4]	: TEXCOORD0;
};


struct SPosTexTexp {
 	float4 pos		: POSITION;
 	float2 uv		: TEXCOORD0;
 	float4 uvp		: TEXCOORD1;
};
struct SPosTexp {
	float4 pos		: POSITION;
	float4 uvp		: TEXCOORD0;
};
struct SPosTexp2 {
	float4 pos		: POSITION;
	float4 uvp[2]	: TEXCOORD0;
};


// position, float
struct SPosZ {
	float4 pos		: POSITION;
	float  z		: TEXCOORD0;
};
struct SPosTexZ {
	float4 pos		: POSITION;
	float2 uv		: TEXCOORD0;
	float  z		: TEXCOORD1;
};


// position, various colors, various textures
struct SPosCol {
	float4 pos		: POSITION;
	half4  color 	: COLOR;
};
struct SPosColTex {
	float4 pos		: POSITION;
	half4  color 	: COLOR;
	float2 uv		: TEXCOORD0;
};
struct SPosColTexp {
	float4 pos		: POSITION;
	half4  color 	: COLOR;
	float4 uvp		: TEXCOORD0;
};
struct SPosColTexp2 {
	float4 pos		: POSITION;
	half4  color 	: COLOR;
	float4 uvp[2]	: TEXCOORD0;
};
struct SPosColTexp3 {
	float4 pos		: POSITION;
	half4  color 	: COLOR;
	float4 uvp[3]	: TEXCOORD0;
};
struct SPosCol2Tex {
	float4 pos		: POSITION;
	half4  color[2]	: COLOR0;
	float2 uv		: TEXCOORD0;
};
struct SPosCol2Tex2 {
	float4 pos		: POSITION;
	half4  color[2]	: COLOR0;
	float2 uv[2]	: TEXCOORD0;
};
struct SPosCol2Tex3 {
	float4 pos		: POSITION;
	half4  color[2]	: COLOR0;
	float2 uv[3]	: TEXCOORD0;
};
struct SPosColTex2 {
	float4 pos		: POSITION;
	half4  color	: COLOR;
	float2 uv[2]	: TEXCOORD0;
};
struct SPosColTex2Z {
	float4 pos		: POSITION;
	half4  color	: COLOR;
	float2 uv[2]	: TEXCOORD0;
	float  z		: TEXCOORD2;
};


// position, tangent space, various textures
struct SPosNTB {
	float4 pos		: POSITION;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float3 binorm	: BINORMAL;
};
struct SPosNTTex {
	float4	pos		: POSITION;
	float3	normal	: NORMAL;
	float3	tangent	: TANGENT;
	float2	uv		: TEXCOORD0;
};
struct SPosNTBTex {
	float4 pos		: POSITION;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float3 binorm	: BINORMAL;
	float2 uv		: TEXCOORD0;
};
struct SPosNTBTex2 {
	float4 pos		: POSITION;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float3 binorm	: BINORMAL;
	float2 uv[2]	: TEXCOORD0;
};



struct SPosTexTanspLHc {
	float4	pos		: POSITION;
	float2 	uv[2]	: TEXCOORD0;
	half3 	l		: COLOR0; // tangent space light
	half3 	h		: COLOR1; // tangent space half
};
struct SPosTexTanspLHV {
	float4	pos		: POSITION;
	float2 	uv		: TEXCOORD0;
	half3 	l		: TEXCOORD1; // tangent space light
	half3 	h		: TEXCOORD2; // tangent space half
	half3	v		: TEXCOORD3; // tangent space view
};
struct SPosTex2TanspLHc {
	float4	pos		: POSITION;
	float2 	uv[3]	: TEXCOORD0;
	half3 	l		: COLOR0; // tangent space light
	half3 	h		: COLOR1; // tangent space half
};
struct SPosTexWorldNV {
	float4	pos 	: POSITION;
	float2 	uv		: TEXCOORD0;
	half3 	n		: TEXCOORD1; // world space normal
	half3 	v		: TEXCOORD2; // world space view
};


#endif
