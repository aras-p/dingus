#include "stdafx.h"

#include "Demo.h"
#include "Shadows.h"
#include "ShadowBufferRTManager.h"

#include <dingus/math/MathUtils.h>
#include <dingus/gfx/gui/Gui.h>
#include <dingus/utils/Random.h>
#include <dingus/gfx/DebugRenderer.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>


// --------------------------------------------------------------------------
// Demo variables, constants, etc.

IDingusAppContext*	gAppContext;
bool	gFinished = false;
bool	gShowStats = false;
CDebugRenderer*	gDebugRenderer;

// --------------------------------------------------------------------------

CDemo::CDemo()
{
}

bool CDemo::checkDevice( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors )
{
	bool ok = true;
	if( caps.getPShaderVersion() < CD3DDeviceCaps::PS_2_0 ) {
		errors.addError( "pixel shaders 2.0 required" );
		ok = false;
	}
	if( !caps.hasFloatTextures() ) {
		errors.addError( "floating point texture support required" );
		ok = false;
	}
	if( caps.getVShaderVersion() < CD3DDeviceCaps::VS_1_1 ) {
		if( vproc != CD3DDeviceCaps::VP_SW )
			ok = false;
	}
	return ok;
}

bool CDemo::shouldFinish()
{
	return gFinished;
}
bool CDemo::shouldShowStats()
{
	return gShowStats;
}

// --------------------------------------------------------------------------
// Variables

CCameraEntity	gCamera;

CUIDialog*		gUIDlgHUD;
CUIStatic*		gUIFPS;

float	gCamYaw;
float	gCamPitch;
float	gCamDist;

const float kMeterSize = 100.0f;


CUICheckBox*	gUIChkDrawLights;
CUICheckBox*	gUIChkDrawCones;
CUICheckBox*	gUIChkDrawBounds;
CUICheckBox*	gUIChkDrawStats;
CUISlider*		gUISldShadowQuality;


// --------------------------------------------------------------------------

void CALLBACK gUIRenderCallback( CUIDialog& dlg );

void CALLBACK gUICallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
}

// --------------------------------------------------------------------------


/*
void gInitializeScene()
{
	int i;

	const int kLightCount = 5;
	const int kAICount = 200;

	for( i = 0; i < kLightCount; ++i )
	{
		Light* l = new Light();
		l->mWorldMat.getOrigin().set(
			gRandom.getFloat( -10.0f, 10.0f ),
			10.0f,
			gRandom.getFloat( -10.0f, 10.0f )
		);
		l->mWorldMat.getAxisZ().set( 0, -1, 0 );
		l->mWorldMat.spaceFromAxisZ();
		l->setProjectionParams( D3DX_PI/4, 1.0f, 1.0f, 20.0f );
		l->updateViewConeAngleFOV();
		l->updateViewCone();
		gLights.push_back( l );
	}

	for( i = 0; i < kAICount; ++i )
	{
		SceneEntity* obj = new SceneEntity( "AI_0" );
		obj->mWorldMat.getOrigin().set(
			gRandom.getFloat( -10.0f,  10.0f ),
			0,//gRandom.getFloat( -10.0f,   0.0f ),
			gRandom.getFloat( -10.0f,  10.0f )
		);
		gScene.push_back( obj );
	}
}
*/

void gInitializeScene( const char* fileName )
{
	FILE* f = fopen( fileName, "rt" );
	char type[100], name[100];

	while( true )
	{
		fscanf( f, "%s", type );
		if( !strcmp(type,"mesh") )
		{
			SVector3 pos;
			SQuaternion rot;
			fscanf( f, "%s pos %f %f %f rot %f %f %f %f\n",
				name,
				&pos.x, &pos.y, &pos.z,
				&rot.x, &rot.y, &rot.z, &rot.w
			);
			SceneEntity* obj = new SceneEntity( name );
			obj->mWorldMat = SMatrix4x4( pos, rot );
			gScene.push_back( obj );
		}
		else if( !strcmp(type,"spot") )
		{
			SVector3 pos, dir;
			float ang;
			fscanf( f, "%s pos %f %f %f dir %f %f %f angle %f\n",
				name,
				&pos.x, &pos.y, &pos.z,
				&dir.x, &dir.y, &dir.z,
				&ang
			);
			Light* l = new Light();
			l->mWorldMat.getOrigin() = pos;
			l->mWorldMat.getAxisZ() = -dir;
			l->mWorldMat.spaceFromAxisZ();
			l->setProjectionParams( D3DXToRadian(ang), 1.0f, 10.0f, 1000.0f );
			l->updateViewConeAngleFOV();
			l->updateViewCone();
			gLights.push_back( l );
		}
		else
		{
			break;
		}
	}
}



void CDemo::initialize( IDingusAppContext& appContext )
{
	gAppContext = &appContext;
	G_INPUTCTX->addListener( *this );

	//CEffectBundle::getInstance().setSkipConstants( "mShadowProj;mLightViewProj;vLightDir;vLightPos;fLightAngle;" );

	gDebugRenderer = new CDebugRenderer( *G_RENDERCTX, *RGET_FX("debug") );

	// --------------------------------
	// shadows

	gShadowRTManager = new ShadowBufferRTManager();

	// --------------------------------
	// scene

	gInitializeScene( "data/scene.txt" );

	gCamYaw = 0.0f;
	gCamPitch = 0.1f;
	gCamDist = 3.0f * kMeterSize;

	// --------------------------------
	// GUI

	gUIDlgHUD = new CUIDialog();
	gUIDlgHUD->setCallback( gUICallback );
	gUIDlgHUD->setRenderCallback( gUIRenderCallback );

	const int hctl = 16;
	int y = 100-hctl;

	gUIDlgHUD->addStatic( 0, "", 5,  460, 200, 20, false, &gUIFPS );

	gUIDlgHUD->addStatic( 0, "Shadow quality:", 5, y+=hctl, 100, hctl );
	gUIDlgHUD->addSlider( 0, 100, y, 70, hctl, 1, 10, 6, false, &gUISldShadowQuality );

	gUIDlgHUD->addCheckBox( 0, "Draw lights", 5, y+=hctl, 100, 16, true, 0, false, &gUIChkDrawLights );
	gUIDlgHUD->addCheckBox( 0, "Draw cones",  5, y+=hctl, 100, 16, false, 0, false, &gUIChkDrawCones );
	gUIDlgHUD->addCheckBox( 0, "Draw bounds", 5, y+=hctl, 100, 16, false, 0, false, &gUIChkDrawBounds );
	gUIDlgHUD->addCheckBox( 0, "Draw stats",  5, y+=hctl, 100, 16, true, 0, false, &gUIChkDrawStats );
}



// --------------------------------------------------------------------------
// Perform code (main loop)

bool CDemo::msgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	bool done = false;
	if( gUIDlgHUD ) {
		done = gUIDlgHUD->msgProc( hwnd, msg, wparam, lparam );
		if( done )
			return true;
	}
	return false;
}

void CDemo::onInputEvent( const CInputEvent& event )
{
	float dt = CSystemTimer::getInstance().getDeltaTimeS();

	if( event.getType() == CKeyEvent::EVENT_TYPE ) {
		const CKeyEvent& ke = (const CKeyEvent&)event;
		switch( ke.getKeyCode() ) {
		case DIK_9:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED )
				gShowStats = !gShowStats;
			break;

		case DIK_LEFT:
			gCamYaw -= dt;
			break;
		case DIK_RIGHT:
			gCamYaw += dt;
			break;
		case DIK_UP:
			gCamPitch -= dt;
			break;
		case DIK_DOWN:
			gCamPitch += dt;
			break;
		case DIK_A:
			gCamDist -= dt * kMeterSize * 3;
			break;
		case DIK_Z:
			gCamDist += dt * kMeterSize * 3;
			break;
		}
	}

	gCamPitch = clamp( gCamPitch, -D3DX_PI*0.4f, D3DX_PI*0.4f );
	gCamDist = clamp( gCamDist, kMeterSize * 0.2f, kMeterSize * 50.0f );
}

void CDemo::onInputStage()
{
}

// --------------------------------------------------------------------------


void gRenderDebug()
{
	gCamera.setOntoRenderContext();
	G_RENDERCTX->applyGlobalEffect();

	gDebugRenderer->beginDebug();

	size_t i;
	size_t nLights = gLights.size();
	for( i = 0; i < nLights; ++i )
	{
		Light* l = gLights[i];
		// render light cone
		if( gUIChkDrawLights->isChecked() )
		{
			gDebugRenderer->renderCone( l->mWorldMat.getOrigin(), l->getViewCone().axis, cosf(l->getFOV()/2), sinf(l->getFOV()/2), l->getZFar(), kMeterSize*0.01f, 0x2000FF00 );
			gDebugRenderer->renderSphere( l->mWorldMat.getOrigin(), kMeterSize*0.007f, 0x2000FF00 );
		}

		// render all shadow buffer cones of this light
		if( gUIChkDrawCones->isChecked() )
		{
			size_t nSBs = l->m_ShadowBuffers.size();
			for( size_t j = 0; j < nSBs; ++j )
			{
				ShadowBuffer* sb = l->m_ShadowBuffers[j];
				gDebugRenderer->renderCone( l->mWorldMat.getOrigin(), sb->m_LSpaceCone.axis, sb->m_LSpaceCone.cosAngle, sb->m_LSpaceCone.sinAngle, l->getZFar()*0.5f, kMeterSize*0.007f, 0x100000FF );
			}
		}
	}

	if( gUIChkDrawBounds->isChecked() )
	{
		for( i = 0; i < gScene.size(); ++i )
		{
			SceneEntity* obj = gScene[i];
			gDebugRenderer->renderSphere( obj->mWorldMat.getOrigin(), obj->getRadius(), 0x10FF0000 );
		}
	}

	gDebugRenderer->endDebug();
}

void CALLBACK gUIRenderCallback( CUIDialog& dlg )
{
	if( !gUIChkDrawStats->isChecked() )
		return;

	char buf[1000];
	const float x1 = 5;
	const float x2 = 60;
	float y = 200;
	const float dy = 15;
	dlg.imDrawText( "objects:", 0, DT_LEFT|DT_TOP, 0xC0ffffff, SFRect(x1,y,x2,y+dy), true );
	dlg.imDrawText( itoa((int)gScene.size(),buf,10), 0, DT_LEFT|DT_TOP, 0xFFffffff, SFRect(x2,y,x2+100,y+dy), true );
	y += dy;
	dlg.imDrawText( "lights:", 0, DT_LEFT|DT_TOP, 0xC0ffffff, SFRect(x1,y,x2,y+dy), true );
	dlg.imDrawText( itoa((int)gLights.size(),buf,10), 0, DT_LEFT|DT_TOP, 0xFFffffff, SFRect(x2,y,x2+100,y+dy), true );
	y += dy;
	dlg.imDrawText( "cones:", 0, DT_LEFT|DT_TOP, 0xC0ffffff, SFRect(x1,y,x2,y+dy), true );
	dlg.imDrawText( itoa((int)gShadowBuffers.size(),buf,10), 0, DT_LEFT|DT_TOP, 0xFFffffff, SFRect(x2,y,x2+100,y+dy), true );
	y += dy;

	for( int sbSize = kMinShadowRTSize; sbSize <= kMaxShadowRTSize; sbSize *= 2 )
	{
		sprintf( buf, "%i:", sbSize );
		dlg.imDrawText( buf, 0, DT_RIGHT|DT_TOP, 0xA0ffffff, SFRect(x1,y,x2-10,y+dy), true );
		int act, tot;
		gShadowRTManager->GetSizeStats( sbSize, act, tot );
		if( act != 0 )
		{
			dlg.imDrawText( itoa(act,buf,10), 0, DT_LEFT|DT_TOP, 0xC0ffffff, SFRect(x2,y,x2+100,y+dy), true );
		}
		if( tot != 0 )
		{
			dlg.imDrawText( itoa(tot,buf,10), 0, DT_LEFT|DT_TOP, 0xC0ffffff, SFRect(x2+15,y,x2+100,y+dy), true );
		}
		y += dy;
	}
}


/// Main loop code.
void CDemo::perform()
{
	char buf[100];

	G_INPUTCTX->perform();
	CDynamicVBManager::getInstance().discard();
	
	double t = CSystemTimer::getInstance().getTimeS();
	float dt = CSystemTimer::getInstance().getDeltaTimeS();

	CD3DDevice& dx = CD3DDevice::getInstance();

	// FPS
	sprintf( buf, "fps: %6.1f", dx.getStats().getFPS() );
	gUIFPS->setText( buf );

	float shadowQuality = gUISldShadowQuality->getValue() * 200;

	// camera
	SMatrix4x4& mc = gCamera.mWorldMat;
	D3DXMatrixRotationYawPitchRoll( &mc, gCamYaw, gCamPitch, 0.0f );
	mc.getOrigin() -= mc.getAxisZ() * gCamDist;
	const float camnear = kMeterSize * 0.1f;
	const float camfar = kMeterSize * 20.0f;
	const float camfov = D3DX_PI*0.25f;
	gCamera.setProjectionParams( camfov, dx.getBackBufferAspect(), camnear, camfar );

	// render
	dx.sceneBegin();

	RenderSceneWithShadows( gCamera, shadowQuality );
	gRenderDebug();

	gUIDlgHUD->onRender( dt );
	dx.sceneEnd();
}



// --------------------------------------------------------------------------
// Cleanup


void CDemo::shutdown()
{
	safeDelete( gUIDlgHUD );

	stl_utils::wipe( gScene );
	stl_utils::wipe( gLights );

	safeDelete( gShadowRTManager );
	safeDelete( gDebugRenderer );
}
