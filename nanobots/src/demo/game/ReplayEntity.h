#ifndef __REPLAY_ENTITY_H
#define __REPLAY_ENTITY_H

#include <dingus/math/Vector3.h>


class CReplayEntity {
public:
	/// Full entity state info
	struct SState {
		short	posx, posy;		// entity position
		short	targx, targy;	// target position (if state requires one)
		short	azn;			// quantity of azn
		short	state;			// state
		short	health;			// hit points
	};

public:
	CReplayEntity( eEntityType type, int owner, int bornTurn, int deathTurn, const SState* states );
	~CReplayEntity();

	eEntityType	getType() const { return mType; }
	const char* getTypeName() const { return mTypeName; }
	int		getOwner() const { return mOwner; }
	int		getBornTurn() const { return mBornTurn; }
	int		getDeathTurn() const { return mDeathTurn; }
	int		getAliveTurns() const { return mAliveTurns; }
	int		getMaxHealth() const { return mMaxHealth; }

	bool	isAlive( float t ) const { return t >= mBornTurn && t < mDeathTurn; }
	
	const SState& getTurnState( int turn ) const {
		turn -= mBornTurn;
		if( turn < 0 )
			turn = 0;
		if( turn >= mAliveTurns )
			turn = mAliveTurns-1;
		return mStates[turn];
	}

	float	getDeathAlpha( float t ) const {
		int turn = mDeathTurn;
		if( t < turn-1 || t >= turn+7 )
			return -1.0f;
		return (t-(turn-1)) * (1.0f/8.0f);
	}

private:
	/// Entity type.
	eEntityType	mType;
	/// Entity's type name
	const char*	mTypeName;
	/// Entity's owner: player index (0..count-2 are players; count-1 is AI).
	int		mOwner;

	/// Hit points when entity is born
	int		mMaxHealth;

	/// The turn entity is born.
	int		mBornTurn;
	/// The turn entity dies.
	int		mDeathTurn;
	/// The number of turns it was alive (death-born+1).
	int		mAliveTurns;

	/// Entity's states for each turn it lives.
	SState*	mStates;
};





#endif
