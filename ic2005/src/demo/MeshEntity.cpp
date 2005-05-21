#include "stdafx.h"

#include "MeshEntity.h"
#include <dingus/utils/StringHelper.h>


// --------------------------------------------------------------------------

CMeshEntity::CMeshEntity( const std::string& descName )
:	mMesh(0)
,	mDescName( descName )
{
	init( descName, descName );
}

CMeshEntity::CMeshEntity( const std::string& descName, const std::string& meshName )
:	mMesh(0)
,	mDescName( descName )
{
	init( descName, meshName );
}

CMeshEntity::~CMeshEntity()
{
	for( int i = 0; i < RMCOUNT; ++i )
		stl_utils::wipe( mRenderMeshes[i] );
}


void CMeshEntity::init( const std::string& descName, const std::string& meshName )
{
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

void CMeshEntity::addLightToParams( const SVector3& lpos )
{
	for( int i = 0; i < RMCOUNT; ++i ) {
		if( !mRenderMeshes[i].empty() ) {
			mRenderMeshes[i][0]->getParams().addVector3( "vLightPos", lpos );
		}
	}
}

