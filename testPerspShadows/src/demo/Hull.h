#ifndef __HULL_H
#define __HULL_H


struct HullFace {
	std::vector<SVector3>	v;
};
struct HullObject {
	std::vector<HullFace>	f;
};


void CalculateFocusedLightHull( const SMatrix4x4& invViewProj, const SVector3& lightDir, const CAABox& sceneAABB, HullFace& points );


#endif
