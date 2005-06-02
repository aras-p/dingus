#include "stdafx.h"

#include "MeshEntity.h"


// --------------------------------------------------------------------------

CMeshEntity::CMeshEntity( const std::string& name, int lodCount )
:	mMesh(0),
	mLodCount( lodCount )
{
	assert( lodCount > 0 );
	assert( !name.empty() );
	bool oneLod = (lodCount==1);
	std::string descName = name;

	mMesh = RGET_MESH(oneLod ? name : (name+"_0"));
	for( int i = 0; i < RMCOUNT; ++i ) {
		mRenderData[i].renderMeshes = NULL;

		// some objects don't get all rendermodes
		CModelDesc* desc = RTRYGET_MDESC( (RMODE_PREFIX[i] + descName) );
		if( !desc )
			continue;

		mRenderData[i].renderMeshes = new TMeshVector[lodCount];
		for( int lod = 0; lod < lodCount; ++lod ) {
			CMesh* rmesh;
			if( oneLod )
				rmesh = mMesh;
			else {
				std::string lodname = name;
				lodname += '_';
				lodname += ('0'+lod);
				rmesh = RGET_MESH(lodname);
			}
			for( int j = 0; j < desc->getGroupCount(); ++j ) {
				CRenderableMesh* rr = new CRenderableMesh( *rmesh, j, &mWorldMat.getOrigin(), desc->getRenderPriority(j) );
				mRenderData[i].renderMeshes[lod].push_back( rr );
				desc->fillFxParams( j, rr->getParams() );
				addMatricesToParams( rr->getParams() );
			}
		}
	}
}

CMeshEntity::~CMeshEntity()
{
	for( int i = 0; i < RMCOUNT; ++i ) {
		if( mRenderData[i].renderMeshes ) {
			for( int lod = 0; lod < mLodCount; ++lod ) {
				stl_utils::wipe( mRenderData[i].renderMeshes[lod] );
			}
			delete[] mRenderData[i].renderMeshes;
		}
	}
}


void CMeshEntity::render( eRenderMode renderMode, int lod, bool direct )
{
	assert( lod >= 0 && lod < mLodCount );

	if( mRenderData[renderMode].renderMeshes == NULL )
		return;

	updateWVPMatrices();
	if( !direct && frustumCull() )
		return;

	TMeshVector& rms = mRenderData[renderMode].renderMeshes[lod];
	int n = rms.size();
	for( int i = 0; i < n; ++i ) {
		CRenderableMesh& m = *rms[i];
		if( direct )
			G_RENDERCTX->directRender( m );
		else
			G_RENDERCTX->attach( m );
	}
}
