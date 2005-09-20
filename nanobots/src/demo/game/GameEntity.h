#ifndef __GAME_ENTITY_H
#define __GAME_ENTITY_H

#include <dingus/utils/ringdeque.h>


class CGameEntity {
public:
	//enum { HISTORY_SIZE = 6*3 };

	/// State info
	struct SState {
		short	posx, posy;		// entity position
		short	targx, targy;	// target position (if state requires one)
		short	stock;			// quantity of azn
		short	state;			// state
		short	health;			// hit points
	};

public:
	CGameEntity( unsigned short eid, eEntityType type, int owner, int bornTurn );
	~CGameEntity();

	int		getLastUpdateTurn() const { return mLastUpdateTurn; }

	int		getID() const { return mID; }
	eEntityType	getType() const { return mType; }
	const char* getTypeName() const { return mTypeName; }
	int		getOwner() const { return mOwner; }
	int		getMaxHealth() const { return mMaxHealth; }

	bool	isAlive() const { return mState.state != ENTSTATE_DEAD; }
	const SState& getState() const { return mState; }
	void	updateState( int turn, const SState& state );
	void	markDead();

private:
	/// Last turn number that this entity was received
	int		mLastUpdateTurn;

	/// Unique entity's ID
	int		mID;
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

	SState	mState;
	/// Previous states history. Current is [0], previous is [1] etc.
	//ringdeque<SState,HISTORY_SIZE>	mStateHistory;
};



#endif
