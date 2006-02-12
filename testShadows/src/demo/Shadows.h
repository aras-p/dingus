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
typedef std::vector<ShadowBuffer*>	ShadowBufferPtrs;


const int kShaderShadowTextureIndex = 4;


// --------------------------------------------------------------------------

class SceneEntity : public CAbstractEntity {
public:
	SceneEntity( const std::string& name );
	virtual ~SceneEntity();

	float	getRadius() const { return mRadius; }

	/// Updates WVP, adds to render context
	void	render( eRenderMode renderMode, bool updateWVP, bool direct );

	CRenderableMesh* getRenderMesh( eRenderMode renderMode ) { return mRenderMeshes[renderMode]; }

private:
	CRenderableMesh*	mRenderMeshes[RMCOUNT];
	CMesh*		mMesh;
	float		mRadius;

public:
	ShadowBufferPtrs	m_ShadowBuffers;
};


extern SceneEntityPtrs	gScene;


// --------------------------------------------------------------------------

struct ShadowBuffer
{
	Light*			m_Light;		// light this SB belongs to
	float			m_PixelDensity;	// pixel density of this SB
#if _DEBUG
	SceneEntityPtrs	m_Entities;		// entities using this SB
#endif
	SceneEntityPtrs	m_Casters;		// casters in this SB
	ViewCone		m_LSpaceCone;	// light space cone of this SB

	// render target data
	SMatrix4x4		m_ViewProjMatrix;
	SMatrix4x4		m_TextureProjMatrix;
	int				m_RTSize;
	CD3DTexture*	m_RTTexture;
	CD3DSurface*	m_RTSurface;

	void	SetOntoRenderContext();
};

extern ShadowBufferPtrs	gShadowBuffers;


// --------------------------------------------------------------------------

struct Light : public CCameraEntity
{
	ShadowBufferPtrs	m_ShadowBuffers;	// SBs that this light uses
	SceneEntityPtrs		m_Entities;			// objects in light
};

extern LightPtrs gLights;


// --------------------------------------------------------------------------

void RenderSceneWithShadows( CCameraEntity& camera, float shadowQuality );


#endif
