// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VISIBILITY_CONTEXT_H
#define __VISIBILITY_CONTEXT_H

#include "VisibilityDB.h"

namespace dingus {



class CVisibilityContext {
public:
	typedef int TVisDBID;
		
public:
	void addDB( TVisDBID dbID, CAbstractVisibilityDB& db );
	CAbstractVisibilityDB& getDB( TVisDBID dbID ) const;

private:
	typedef std::map<TVisDBID, CAbstractVisibilityDB::TSharedPtr> TDatabaseMap;
private:
	TDatabaseMap			mDBMap;
};

}; // namespace

#endif
