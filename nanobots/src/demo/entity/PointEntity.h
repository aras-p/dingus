#ifndef __POINT_ENTITY_H
#define __POINT_ENTITY_H

#include "MeshEntity.h"
#include "../game/GameMap.h"


class CPointEntity : public CMeshEntity {
public:
	CPointEntity( const CGameMap::SPoint& point );
	virtual ~CPointEntity();

	// early outs if not visible
	void	renderPoint( eRenderMode renderMode );

	void	update();

	void	hide();
	const CGameMap::SPoint* getPoint() const { return mPoint; }
	
private:
	SVector4	mColor;
	double		mTimeOffset;
	float		mAlphaBase, mAlphaAmpl;
	const CGameMap::SPoint*	mPoint;
};



#endif
