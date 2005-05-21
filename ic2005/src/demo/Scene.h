#ifndef __SCENE_H
#define __SCENE_H

#include "MeshEntity.h"

class CComplexStuffEntity;

namespace dingus {
	class CUIDialog;
};

// --------------------------------------------------------------------------

class CRoomObjectEntity : public CMeshEntity {
public:
	CRoomObjectEntity( const std::string& name );

	void	setMoved() { mMoved = true; }
	void	update( const SVector3& wlightPos );
	void	render( eRenderMode renderMode );

private:
	SMatrix4x4	mInvWorld;
	SVector3	mLightPosOS;	// light pos in object space
	SVector3	mEyePosOS;		// eye pos in object space
	bool		mMoved;
};

// --------------------------------------------------------------------------

bool gReadScene( const char* fileName, std::vector<CRoomObjectEntity*>& scene );


// --------------------------------------------------------------------------

class CScene : public boost::noncopyable {
public:
	typedef std::vector<CMeshEntity*>			TEntityVector;
	typedef std::vector<CComplexStuffEntity*>	TAnimEntityVector;

public:
	CScene();
	virtual ~CScene() = 0;

	void	addEntity( CMeshEntity& e ) { mEntities.push_back( &e ); }
	void	addAnimEntity( CComplexStuffEntity& e ) { mAnimEntities.push_back( &e ); }

	virtual void	update( time_value demoTime, float dt ) = 0;
	virtual void	render( eRenderMode renderMode ) = 0;
	virtual void	renderUI( CUIDialog& dlg ) { }

	virtual const SMatrix4x4* getLightTargetMatrix() const = 0;

	virtual bool	needsReflections() const { return true; }

	// Camera
	const CCameraEntity& getCamera() const { return mCamera; }
	CCameraEntity& getCamera() { return mCamera; }

private:
	CCameraEntity		mCamera;		// Camera for the scene.
	TEntityVector		mEntities;		// Owns entities.
	TAnimEntityVector	mAnimEntities;	// Owns entities.
};



#endif
