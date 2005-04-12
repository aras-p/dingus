// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VERTICES_H
#define __VERTICES_H

#include "../math/Vector3.h"
#include "../math/Vector4.h"


namespace dingus {



#pragma pack(1)

/// Vertex format: position.
const DWORD FVF_XYZ = D3DFVF_XYZ;
/// Vertex: position.
struct SVertexXyz {
	SVector3	p;
};

/// Vertex format: position, normal.
const DWORD FVF_XYZ_NORMAL = D3DFVF_XYZ | D3DFVF_NORMAL;
/// Vertex: position, normal.
struct SVertexXyzNormal {
	SVector3	p;
	SVector3	n;
};

/// Vertex format: position, UV.
const DWORD FVF_XYZ_TEX1 = D3DFVF_XYZ | D3DFVF_TEX1;
/// Vertex: position, UV.
struct SVertexXyzTex1 {
	SVector3	p;
	float		tu, tv;
};

/// Vertex format: position, color.
const DWORD FVF_XYZ_DIFFUSE = D3DFVF_XYZ | D3DFVF_DIFFUSE;
/// Vertex: position, color.
struct SVertexXyzDiffuse {
	SVector3	p;
	D3DCOLOR	diffuse;
};

/// Vertex format: position, color, UV.
const DWORD FVF_XYZ_DIFFUSE_TEX1 = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
/// Vertex: position, color, UV.
struct SVertexXyzDiffuseTex1 {
	SVector3	p;
	D3DCOLOR	diffuse;
	float		tu, tv;
};

/// Vertex format: transformed position, UV.
const DWORD FVF_XYZRHW_TEX1 = D3DFVF_XYZRHW | D3DFVF_TEX1;
/// Vertex: transformed position, UV.
struct SVertexXyzRhwTex1 {
	SVector4	p;
	float		tu, tv;
};

/// Vertex format: transformed position, color, UV.
const DWORD FVF_XYZRHW_DIFFUSE_TEX1 = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
/// Vertex: transformed position, color, UV.
struct SVertexXyzRhwDiffuseTex1 {
	SVector4	p;
	D3DCOLOR	diffuse;
	float		tu, tv;
};

/// Vertex format: position, normal, UV.
const DWORD FVF_XYZ_NORMAL_TEX1 = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
/// Vertex: position, normal, UV.
struct SVertexXyzNormalTex1 {
	SVector3	p;
	SVector3	n;
	float		tu, tv;
};

/// Vertex format: position, normal, UV1, UV2.
const DWORD FVF_XYZ_NORMAL_TEX2 = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2;
/// Vertex: position, normal, UV.
struct SVertexXyzNormalTex2 {
	SVector3	p;
	SVector3	n;
	float		tu1, tv1;
	float		tu2, tv2;
};

/// Vertex format: position, normal, color, UV.
const DWORD FVF_XYZ_NORMAL_DIFFUSE_TEX1 = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;
/// Vertex: position, normal, color, UV.
struct SVertexXyzNormalDiffuseTex1 {
	SVector3	p;
	SVector3	n;
	D3DCOLOR	diffuse;
	float		tu, tv;
};

/// Vertex format: position, normal, color.
const DWORD FVF_XYZ_NORMAL_DIFFUSE = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;
/// Vertex: position, normal, color.
struct SVertexXyzNormalDiffuse {
	SVector3	p;
	SVector3	n;
	D3DCOLOR	diffuse;
};

#pragma pack()


}; // namespace


#endif
