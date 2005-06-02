#ifndef __POINTS_MESH_H
#define __POINTS_MESH_H

#include "../DemoResources.h"
#include <dingus/renderer/RenderableMesh.h>

class CGameMap;
class CLevelMesh;


// --------------------------------------------------------------------------


class CPointsMesh : public boost::noncopyable {
public:
	CPointsMesh( const CGameMap& gameMap, const CLevelMesh& levelMesh );
	~CPointsMesh();

	void	render( eRenderMode renderMode );

private:
	CRenderableMesh*	mMesh[RMCOUNT];
};


#endif
