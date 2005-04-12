// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "SkeletonInfoBundle.h"
#include "MeshSerializer.h"
#include "../utils/Errors.h"

using namespace dingus;

CSkeletonInfoBundle::CSkeletonInfoBundle()
{
	addExtension( ".dmesh" );
}

CSkeletonInfo* CSkeletonInfoBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	CSkeletonInfo* skelInfo = CMeshSerializer::loadSkelInfoFromFile( fullName.getUniqueName().c_str() );
	if( !skelInfo ) {
		return NULL;
	}
	assert( skelInfo );
	CONSOLE.write( "skeleton loaded '" + id.getUniqueName() + "'" );
	return skelInfo;
}
