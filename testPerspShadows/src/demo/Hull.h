#ifndef __HULL_H
#define __HULL_H


struct HullFace {
	std::vector<SVector3>	v;
};
struct HullObject {
	std::vector<HullFace>	f;
};


#endif
