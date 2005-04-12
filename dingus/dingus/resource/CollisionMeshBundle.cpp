// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "CollisionMeshBundle.h"
#include "MeshBundle.h"

using namespace dingus;

CCollisionMeshBundle::CCollisionMeshBundle()
{
}

CCollisionMesh* CCollisionMeshBundle::tryLoadResourceById( const CResourceId& id )
{
	// just loadResourceById - it uses MeshBundle for actual loading, so
	// no predir or extension is required here
	return loadResourceById( id, id );
}

CCollisionMesh* CCollisionMeshBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	CMesh* m = RGET_MESH(id);
	assert( m );

	assert( m->getVertexFormat().hasPosition() );
	assert( m->getIndexStride() == 2 );

	const SVector3* vb = (const SVector3*)m->lockVBRead();
	const unsigned short* ib = (const unsigned short*)m->lockIBRead();

	CCollisionMesh* mesh = new CCollisionMesh( m->getVertexCount(), m->getIndexCount()/3, vb, m->getVertexStride(), ib );

	m->unlockIBRead();
	m->unlockVBRead();
	return mesh;
}
