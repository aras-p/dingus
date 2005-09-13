#ifndef __ACTOR_ENTITY_H
#define __ACTOR_ENTITY_H

#include "MeshEntity.h"
#include "../DemoResources.h"
#include <dingus/audio/Sound.h>

class CGameEntity;


class CActorEntity : public CMeshEntity {
public:
	enum { LOD_COUNT = GFX_DETAIL_LEVELS };

public:
	CActorEntity( const CGameEntity& ge );
	virtual ~CActorEntity();

	void	update();

	const CGameEntity& getGameEntity() const { return *mGameEntity; }
	D3DCOLOR	getColorMinimap() const { return mColorMinimap; }
	D3DCOLOR	getColorBlob() const { return mColorBlob; }

	//SVector3 samplePos( float t ) const;
	//const SVector3& getPos( int i ) const { return mPositions[i]; }

	void	setOutline( float ttl ) { mOutlineTTL = ttl; }
	float	getOutline() const { return mOutlineTTL; }

	float	getHealthBarDY() const { return mHealthBarDY; }
	float	getBlobDY() const { return mBlobDY; }
	float	getOutlineDY() const { return mOutlineDY; }

private:
	void	updateSounds( bool dead );

private:
	CSound*	mSndAttack;
	CSound*	mSndBirth;
	CSound*	mSndInjured;

	int		mSoundPlayedTurn;

	const CGameEntity*	mGameEntity;
	//SVector3*	mPositions;

	D3DCOLOR	mColorMinimap;
	D3DCOLOR	mColorBlob;

	float	mHealthBarDY;
	float	mBlobDY;
	float	mOutlineDY;

	float	mOutlineTTL;

	bool	mOnGround;
	bool	mOnAir;
	bool	mOnSine;
	float	mBaseAltitude;
};



#endif
