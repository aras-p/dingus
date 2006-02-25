#ifndef __HULL_H
#define __HULL_H

namespace dingus { class CDebugRenderer; class SPlane; }

struct HullFace {
	std::vector<SVector3>	v;
};
struct HullObject {
	std::vector<HullFace>	f;
	void clear() { f.clear(); }
};


void CalculateFocusedLightHull( const SMatrix4x4& invViewProj, const SVector3& lightDir, const CAABox& sceneAABB, HullFace& points );

void ClipHullByAABB( HullObject& obj, const CAABox& box );
void ClipHullByPlane( HullObject& obj, const SPlane& A );

void DebugCalcFrustumHull( const SMatrix4x4& invViewProj, HullObject& obj );
void DebugRenderHull( const HullObject& obj, CDebugRenderer& dr, D3DCOLOR color );


#endif
