#ifndef __STREAM_ENTITY_H
#define __STREAM_ENTITY_H

#include "MeshEntity.h"
#include "../game/GameMap.h"
class CStreamImpostorsRenderer;


class CStreamEntity : public CMeshEntity {
public:
	CStreamEntity( const CGameMap::SStream& stream, float x, float y, int type );
	virtual ~CStreamEntity();

	// early outs if not visible
	void	render( eRenderMode renderMode, CStreamImpostorsRenderer& impostorer );
	void	update();
	
private:
	SVector4	mColor;
	const CGameMap::SStream*	mStream;
	float	mVelocityX;
	float	mVelocityY;
	int		mType;
};



#endif
