// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __GFX_MESH_SERIALIZER_H
#define __GFX_MESH_SERIALIZER_H

#include "../gfx/Mesh.h"
#include "../gfx/skeleton/SkeletonInfo.h"


namespace dingus {


class CMeshSerializer {
public:
	/// Return true if successful.
	static bool loadMeshFromFile( const char* fileName, CMesh& mesh );
	///// Return true if successful
	//static bool saveMeshToFile( const char* fileName, const CMesh& mesh );
	
	static CSkeletonInfo* loadSkelInfoFromFile( const char* fileName );
};


}; // namespace


#endif
