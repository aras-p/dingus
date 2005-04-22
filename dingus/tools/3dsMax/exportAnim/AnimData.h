#ifndef __ANIM_DATA_H
#define __ANIM_DATA_H

#include "Max.h"
#include <d3dx9math.h>
#include "../../../dingus/animator/AnimCurve.h"

using namespace dingus;


template<typename T>
struct SAnimGroup {
public:
	typedef Tab<T>			TSampleTab;
	typedef CAnimCurve<T>	TAnimCurve;
	typedef Tab<TAnimCurve> TAnimCurveTab;

public:
	void clear() { samples.ZeroCount(); curves.ZeroCount(); }
	
public:
	TSampleTab		samples;
	TAnimCurveTab	curves;
};

// NOTE: use D3DXQUATERNION instead of Max's Quat
// Using Max's Quat routines produces quaternions that aren't equal to D3DX.
// Maybe it's due to coordinate systems difference, or smth.

typedef SAnimGroup<D3DXVECTOR3>		TVec3AnimGroup;
typedef SAnimGroup<D3DXQUATERNION>	TQuatAnimGroup;
typedef SAnimGroup<D3DCOLOR>		TColorAnimGroup;
typedef SAnimGroup<float>			TFloatAnimGroup;

typedef Tab<D3DXVECTOR3>	TVec3Tab;
typedef Tab<D3DXQUATERNION>	TQuatTab;
typedef Tab<D3DCOLOR>		TColorTab;
typedef Tab<float>			TFloatTab;
typedef Tab<TSTR>			TStringTab;
typedef Tab<int>			TIntTab;

#endif
