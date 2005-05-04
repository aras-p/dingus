#ifndef __WALL_PHYSICS_H
#define __WALL_PHYSICS_H

#include "../DemoResources.h"

class CWall3D;

namespace dingus {
	class CDebugRenderer;
};

namespace wall_phys {

	struct SStats {
		float	msPhys;
		float	msColl;
		float	msUpdate;
		int		pieceCount;
		int		vertexCount;
		int		triCount;
	};

	void	initialize( float updDT, const SVector3& boundMin, const SVector3& boundMax );

	void	addWall( int lodIndex, const CWall3D& wall );

	void	shutdown();

	void	spawnPiece( int lodIndex, int wallID, int index );

	void	update();

	void	render( eRenderMode rm );

	const SStats&	getStats();

}; // namespace wall_phys


#endif
