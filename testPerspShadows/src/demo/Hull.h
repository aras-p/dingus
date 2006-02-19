#ifndef __HULL_H
#define __HULL_H


struct HullFace {
	std::vector<int>	idx;
};
struct HullObject {
	std::vector<SVector3>	verts;
	std::vector<HullFace>	faces;
};



#endif
