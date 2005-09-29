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
	typedef std::map<int,CActorEntity*>	TActorEntityMap;

public:
	CEntityManager();
	~CEntityManager();

	void	update( const SLine3& mouseRay, float timeAlpha );
	void	render( eRenderMode rm, bool entityBlobs, bool thirdPerson );
	void	renderMinimap();
	void	onMouseClick();

	void	renderLabels( CUIDialog& dlg, bool thirdPerson );

	int		getSelectedEntityID() const { return mSelectedEntityID; }
	void	setSelectedEntityID( int i ) { mSelectedEntityID = i; }
	
	const CActorEntity* getActorEntityByID( int id ) const {
		if( id == -1 )
			return NULL;
		TActorEntityMap::const_iterator it = mActorEntities.find( id );
		return (it==mActorEntities.end()) ? NULL : it->second;
	}
	CActorEntity* getActorEntityByID( int id ) {
		if( id == -1 )
			return NULL;
		TActorEntityMap::iterator it = mActorEntities.find( id );
		return (it==mActorEntities.end()) ? NULL : it->second;
	}

	void onNewGameEntity( const CGameEntity& e );
	void onNewInjectionPoint( const CGameMap::SPoint& pt );

private:
	int		getCollidedEntityID( const SLine3& ray ) const;

private:
	TActorEntityMap				mActorEntities;
	std::vector<CPointEntity*>	mPointEntities;
	CAttackEntityManager*		mAttackManager;

	int		mLastMouseEntityID;
	int		mSelectedEntityID;
};


#endif
