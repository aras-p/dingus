#include "stdafx.h"
#include "MeshEntity.h"


// --------------------------------------------------------------------------

CMeshEntity::CMeshEntity( const std::string& meshName )
:	mMesh(0)
{
	mMesh = RGET_MESH(meshName);
}
