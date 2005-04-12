// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "VisibilityContext.h"

using namespace dingus;


void CVisibilityContext::addDB( TVisDBID dbID, CAbstractVisibilityDB& db )
{
	mDBMap.insert( std::make_pair( dbID, &db ) );
}

CAbstractVisibilityDB& CVisibilityContext::getDB( TVisDBID dbID ) const
{
	TDatabaseMap::const_iterator db = mDBMap.find( dbID );
	if( db == mDBMap.end() ) {
		// TBD: throw exception here
		ASSERT_FAIL_MSG( "Visibility DB not found" );
	}
	assert( db->second );
	return *db->second;
}
