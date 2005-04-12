// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "ModelDescBundle.h"
#include "../gfx/ModelDescSerializer.h"
#include "../utils/Errors.h"

using namespace dingus;


CModelDescBundle::CModelDescBundle()
{
	addExtension( ".dmdl" );
};


bool CModelDescBundle::loadModelDesc( const CResourceId& id, const CResourceId& fullName, CModelDesc& desc ) const
{
	// our own format
	assert( desc.getMeshID().getUniqueName().empty() && desc.getGroupCount() == 0 );
	bool ok = CModelDescSerializer::loadFromFile( fullName.getUniqueName().c_str(), desc );
	if( !ok )
		return false;
	CONSOLE.write( "model desc loaded '" + id.getUniqueName() + "'" );
	return true;
}


CModelDesc* CModelDescBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	CModelDesc* desc = new CModelDesc( "" );
	bool ok = loadModelDesc( id, fullName, *desc );
	if( !ok ) {
		delete desc;
		return NULL;
	}
	return desc;
}
