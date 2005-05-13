#include "stdafx.h"

#include "MeshEntity.h"
#include <dingus/utils/StringHelper.h>


// --------------------------------------------------------------------------

CMeshEntity::CMeshEntity( const std::string& name )
:	mMesh(0)
,	mName( name )
{
	assert( !name.empty() );
	std::string descName = name;
	std::string meshName = name;

	if( descName == "wall" )
		meshName = "billboard";
	if( CStringHelper::startsWith( name, "Room" ) )
		descName = "Room";

	mMesh = RGET_MESH(meshName);
	for( int i = 0; i < RMCOUNT; ++i ) {
		// some objects don't get all rendermodes
		CModelDesc* desc = RTRYGET_MDESC( RMODE_PREFIX[i] + descName );
		if( !desc )
			continue;
		for( int j = 0; j < desc->getGroupCount(); ++j ) {
			CRenderableMesh* rr = new CRenderableMesh( *mMesh, j, &mWorldMat.getOrigin(), desc->getRenderPriority(j) );
			mRenderMeshes[i].push_back( rr );
			desc->fillFxParams( j, rr->getParams() );
			addMatricesToParams( rr->getParams() );
		}
	}
}

CMeshEntity::~CMeshEntity()
{
	for( int i = 0; i < RMCOUNT; ++i )
		stl_utils::wipe( mRenderMeshes[i] );
}

void CMeshEntity::render( eRenderMode renderMode, bool direct )
{
	if( mRenderMeshes[renderMode].empty() )
		return;
	updateWVPMatrices();
	
	if( !direct && frustumCull() )
		return;

	int n = mRenderMeshes[renderMode].size();
	for( int i = 0; i < n; ++i ) {
		CRenderableMesh& m = *mRenderMeshes[renderMode][i];
		if( direct )
			G_RENDERCTX->directRender( m );
		else
			G_RENDERCTX->attach( m );
	}
}
