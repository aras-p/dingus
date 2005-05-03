#ifndef __SCENE_SHARED_H
#define __SCENE_SHARED_H

#include "DemoResources.h"
#include "Entity.h"


class CWall3D;



class CSceneSharedStuff : public boost::noncopyable {
public:
	CSceneSharedStuff();
	~CSceneSharedStuff();

	void	renderWalls( eRenderMode rm );
	void	updatePhysics();
	void	updateFracture( float demoTimeS );

	CCameraEntity& getWallCamera() { return mWallCamera; }
	CWall3D** getWalls() { return mWalls; }

private:
	CCameraEntity	mWallCamera;
	CWall3D*		mWalls[CFACE_COUNT];
};



#endif
