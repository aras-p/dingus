// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MESH_CREATOR_H
#define __MESH_CREATOR_H

#include "../gfx/Mesh.h"


namespace dingus {


class IMeshCreator : public CRefCounted {
public:
	typedef DingusSmartPtr<IMeshCreator>	TSharedPtr;
public:
	virtual ~IMeshCreator() = 0 { }
	virtual void createMesh( CMesh& mesh ) = 0;
};



}; // namespace

#endif
