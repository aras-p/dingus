#ifndef __ENTITY_MANAGER_H
#define __ENTITY_MANAGER_H

#include "ActorEntity.h"
#include "PointEntity.h"
#include "../game/GameState.h"
#include <dingus/math/Line3.h>


class CAttackEntityManager;
namespace dingus {
	class CUIDialog;
};

class CEntityManager {
public:
	/// Player statistics at some time instance
	struct SPlayerStats {
		int		score;					// score
		int		aliveCount;				// alive entity count
		int		counts[ENTITYCOUNT];	// alive entity counts by type
	};
	
public:
	CEntityManager();
	~CEntityManager();

	void	update( const SLine3& mouseRay );
	void	render( eRenderMode rm, bool entityBlobs, bool thirdPerson );
	void	renderMinimap();
	void	onMouseClick();

	void	renderLabels( CUIDialog& dlg, bool thirdPerson );

	const SPlayerStats& getStats( int index ) const { return mStats[index]; }

	int		getSelectedEntity() const { return mSelectedEntity; }
	void	setSelectedEntity( int i ) { mSelectedEntity = i; }
	
	const CActorEntity& getActorEntity( int i ) const { return *mActorEntities[i]; }

private:
	int		getCollidedEntity( const SLine3& ray ) const;

private:
	std::vector<CActorEntity*>	mActorEntities;
	std::vector<char>			mNeedleOnHoshimi;
	std::vector<CPointEntity*>	mPointEntities;
	CAttackEntityManager*		mAttackManager;

	SPlayerStats	mStats[G_MAX_PLAYERS];

	int		mLastMouseEntity;
	int		mSelectedEntity;
};


#endif
