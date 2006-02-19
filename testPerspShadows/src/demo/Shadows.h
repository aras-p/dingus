#ifndef __SHADOWS_H
#define __SHADOWS_H

#include "Entity.h"
#include "DemoResources.h"
#include <dingus/renderer/RenderableMesh.h>


class SceneEntity;
struct Light;
struct ShadowBuffer;

typedef std::vector<SceneEntity*>	SceneEntityPtrs;
typedef std::vector<Light*>	LightPtrs;


const int kShaderShadowTextureIndex = 4;


// --------------------------------------------------------------------------

class SceneEntity : public CAbstractEntity {
public:
	SceneEntity( const std::string& name );
	virtual ~SceneEntity();

	const CAABox& getAABB() { return mMesh->getTotalAABB(); }

	/// Updates WVP, adds to render context
	void	render( eRenderMode renderMode, bool updateWVP, bool direct );

	CRenderableMesh* getRenderMesh( eRenderMode renderMode ) { return mRenderMeshes[renderMode]; }

private:
	CRenderableMesh*	mRenderMeshes[RMCOUNT];
	CMesh*		mMesh;

public:
	SVector3			m_Color;
	bool				m_CanAnimate;
};


extern SceneEntityPtrs	gScene;
extern CAABox gSceneBounds, gCasterBounds, gReceiverBounds;


void	CalculateSceneBounds();


// --------------------------------------------------------------------------

struct Light : public CCameraEntity
{
	SVector3			m_Color;
	
	// render target data
	SMatrix4x4		m_TextureProjMatrix;
	int				m_RTSize;
	CD3DTexture*	m_RTTexture;
	CD3DSurface*	m_RTSurface;

	SMatrix4x4		m_DebugViewProjMatrix;

	void	PrepareAndSetOntoRenderContext( const CCameraEntity& cam, bool lispsm );
};

extern LightPtrs gLights;


// --------------------------------------------------------------------------

void CalculateSceneBounds();
void RenderSceneWithShadows( CCameraEntity& camera, CCameraEntity& actualCamera, float shadowQuality, bool lispsm );


#endif
