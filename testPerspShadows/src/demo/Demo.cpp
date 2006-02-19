#include "stdafx.h"

#include "Demo.h"
#include "Shadows.h"
#include "ShadowBufferRTManager.h"

#include <dingus/math/MathUtils.h>
#include <dingus/gfx/gui/Gui.h>
#include <dingus/utils/Random.h>
#include <dingus/gfx/DebugRenderer.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>
#include <dingus/renderer/EffectParamsNotifier.h>


// --------------------------------------------------------------------------
// Demo variables, constants, etc.

IDingusAppContext*	gAppContext;
bool	gFinished = false;
bool	gShowStats = false;
CDebugRenderer*	gDebugRenderer;

bool			gUseDSTShadows;


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
float			gCamYaw, gCamPitch;

CCameraEntity	gDummyCamera;


CUIDialog*		gUIDlgHUD;
CUIStatic*		gUIFPS;

const float kMeterSize = 100.0f;


CUICheckBox*	gUIChkAnimObjects;
CUICheckBox*	gUIChkDrawLights;
CUICheckBox*	gUIChkDrawStats;
CUISlider*		gUISldShadowQuality;

CUIComboBox*	gUICmbShadowFilter;
CUICheckBox*	gUIChkLISPSM;


// --------------------------------------------------------------------------

void CALLBACK gUIRenderCallback( CUIDialog& dlg );

void CALLBACK gUICallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
	if( evt == UIEVENT_COMBOBOX_SELECTION_CHANGED )
	{
		CUIComboBox* c = (CUIComboBox*)ctrl;
		if( c == gUICmbShadowFilter )
		{
			CEffectBundle& bfx = CEffectBundle::getInstance();
			bfx.setMacro( "D_SHD_FILTER", (const char*)c->getSelectedData() );
			bfx.reload();
			CEffectParamsNotifier::getInstance().notify();
		}
	}
}

// --------------------------------------------------------------------------


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
			SVector3 diff;
			fscanf( f, "%s pos %f %f %f rot %f %f %f %f diff %f %f %f\n",
				name,
				&pos.x, &pos.y, &pos.z,
				&rot.x, &rot.y, &rot.z, &rot.w,
				&diff.x, &diff.y, &diff.z
			);
			SceneEntity* obj = new SceneEntity( name );
			obj->mWorldMat = SMatrix4x4( pos, rot );
			obj->m_Color = diff * (1.0f / 255.0f);
			gScene.push_back( obj );
		}
		else if( !strcmp(type,"dir") )
		{
			SVector3 dir, color;
			fscanf( f, "%s dir %f %f %f color %f %f %f\n",
				name,
				&dir.x, &dir.y, &dir.z,
				&color.x, &color.y, &color.z
			);
			Light* l = new Light();
			l->mWorldMat.getAxisZ() = -dir;
			l->m_Color = color * (1.0f/255.0f);
			l->mWorldMat.spaceFromAxisZ();
			gLights.push_back( l );
		}
		else
		{
			break;
		}
	}

	CalculateSceneBounds();
}



void CDemo::initialize( IDingusAppContext& appContext )
{
	gAppContext = &appContext;
	G_INPUTCTX->addListener( *this );

	gUseDSTShadows = CD3DDevice::getInstance().getCaps().hasShadowMaps();
	//gUseDSTShadows = false;

	if( gUseDSTShadows ) {
		CEffectBundle::getInstance().setMacro( "DST_SHADOWS", "1" );
	}

	gDebugRenderer = new CDebugRenderer( *G_RENDERCTX, *RGET_FX("debug") );

	// --------------------------------
	// shadows

	gShadowRTManager = new ShadowBufferRTManager();

	// --------------------------------
	// scene

	gInitializeScene( "data/scene.txt" );

	gCamera.mWorldMat.getOrigin() = gSceneBounds.getCenter();
	gCamera.mWorldMat.getOrigin().y *= 1.2f;
	gCamYaw = 0.0f;
	//gCamPitch = 0.2f;
	gCamPitch = 0.0f;

	gDummyCamera.mWorldMat.getOrigin().z = -200;
	gDummyCamera.mWorldMat.getOrigin().y = gSceneBounds.getCenter().y*0.6f;
	gDummyCamera.setProjectionParams( D3DX_PI/4, 1.0f, 50.0f, 500.0f );

	// --------------------------------
	// GUI

	gUIDlgHUD = new CUIDialog();
	gUIDlgHUD->setCallback( gUICallback );
	gUIDlgHUD->setRenderCallback( gUIRenderCallback );

	gUIDlgHUD->addStatic( 0, "", 5,  460, 200, 20, false, &gUIFPS );

	const int hctl = 16;
	int y = 100-hctl;

	gUIDlgHUD->addStatic( 0, "Shadow quality:", 5, y+=hctl, 100, hctl );
	gUIDlgHUD->addSlider( 0, 100, y, 70, hctl, 7, 11, 10, false, &gUISldShadowQuality );

	gUIDlgHUD->addStatic( 0, "Shadow filter:", 5, y+=hctl, 100, hctl );
	gUIDlgHUD->addComboBox( 0, 100, y, 70, hctl, 0, false, &gUICmbShadowFilter );
	gUICmbShadowFilter->addItem( "None", "0", true );
	gUICmbShadowFilter->addItem( "3x3 PCF", "1", false );
	gUICmbShadowFilter->addItem( "4x4 PCF", "2", false );
	if( CD3DDevice::getInstance().getCaps().getPShaderVersion() >= CD3DDeviceCaps::PS_3_0 )
		gUICmbShadowFilter->addItem( "4x4 PCF Dither", "3", false ); // only on SM3.0
	gUIDlgHUD->addCheckBox( 0, "LisPSM",   5, y+=hctl, 100, 16, false, 0, false, &gUIChkLISPSM );

	gUIDlgHUD->addCheckBox( 0, "Anim objects", 5, y+=hctl, 100, 16, false, 0, false, &gUIChkAnimObjects );
	gUIDlgHUD->addCheckBox( 0, "Draw lights",  5, y+=hctl, 100, 16, true, 0, false, &gUIChkDrawLights );
	gUIDlgHUD->addCheckBox( 0, "Draw stats",   5, y+=hctl, 100, 16, true, 0, false, &gUIChkDrawStats );
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
		case DIK_PGUP:
			gCamera.mWorldMat.getOrigin().y += kMeterSize * dt * 3;
			break;
		case DIK_PGDN:
			gCamera.mWorldMat.getOrigin().y -= kMeterSize * dt * 3;
			break;
		case DIK_A:
			gCamPitch -= dt;
			break;
		case DIK_Z:
			gCamPitch += dt;
			break;
		case DIK_UP:
			{
				SVector3 dir = gCamera.mWorldMat.getAxisZ();
				dir.y = 0.0f;
				dir.normalize();
				gCamera.mWorldMat.getOrigin() += dir * kMeterSize * dt * 3;
			}
			break;
		case DIK_DOWN:
			{
				SVector3 dir = gCamera.mWorldMat.getAxisZ();
				dir.y = 0.0f;
				dir.normalize();
				gCamera.mWorldMat.getOrigin() -= dir * kMeterSize * dt * 3;
			}
			break;
		}
	}

	gCamPitch = clamp( gCamPitch, -D3DX_PI*0.4f, D3DX_PI*0.4f );
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

	SMatrix4x4 vp = gDummyCamera.mWorldMat;
	vp.invert();
	vp *= gDummyCamera.getProjectionMatrix();

	gDebugRenderer->renderFrustum( vp, kMeterSize*0.03f, 0x80FF0000 );
	
	gDebugRenderer->renderAABB( gCasterBounds, kMeterSize*0.02f, 0x400000FF );
	gDebugRenderer->renderAABB( gReceiverBounds, kMeterSize*0.02f, 0x4000FFFF );

	size_t i;
	size_t nLights = gLights.size();
	for( i = 0; i < nLights; ++i )
	{
		Light& l = *gLights[i];
		// render light cone
		if( gUIChkDrawLights->isChecked() )
		{
			gDebugRenderer->renderFrustum( l.m_DebugViewProjMatrix, kMeterSize*0.05f, 0x8000FF00 );
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
	float y = 260;
	const float dy = 15;
	dlg.imDrawText( "objects:", 0, DT_LEFT|DT_TOP, 0xC0ffffff, SFRect(x1,y,x2,y+dy), true );
	dlg.imDrawText( itoa((int)gScene.size(),buf,10), 0, DT_LEFT|DT_TOP, 0xFFffffff, SFRect(x2,y,x2+100,y+dy), true );
	y += dy;
	dlg.imDrawText( "lights:", 0, DT_LEFT|DT_TOP, 0xC0ffffff, SFRect(x1,y,x2,y+dy), true );
	dlg.imDrawText( itoa((int)gLights.size(),buf,10), 0, DT_LEFT|DT_TOP, 0xFFffffff, SFRect(x2,y,x2+100,y+dy), true );
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


static void gAnimate( float dt )
{
	if( gUIChkAnimObjects->isChecked() )
	{
		size_t nobjs = gScene.size();
		for( size_t i = 0; i < nobjs; ++i )
		{
			SceneEntity& obj = *gScene[i];
			if( !obj.m_CanAnimate )
				continue;

			SMatrix4x4 mr;
			D3DXMatrixRotationYawPitchRoll(
				&mr,
				cosf(i)*dt*1.2f,
				sinf(i)*dt*0.7f,
				cosf(i)*dt*0.2f
			);
			obj.mWorldMat = mr * obj.mWorldMat;
		}
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

	gAnimate( dt );

	// FPS
	sprintf( buf, "fps: %6.1f", dx.getStats().getFPS() );
	gUIFPS->setText( buf );

	float shadowQuality = gUISldShadowQuality->getValue();

	// camera
	SMatrix4x4& mc = gCamera.mWorldMat;
	SVector3 o = mc.getOrigin();
	D3DXMatrixRotationYawPitchRoll( &mc, gCamYaw, gCamPitch, 0.0f );
	mc.getOrigin() = o;
	const float sceneSize = SVector3(gSceneBounds.getMax() - gSceneBounds.getMin()).length();
	const float camnear = sceneSize * 0.005f;
	const float camfar = sceneSize * 0.7f;
	const float camfov = D3DX_PI*0.25f;
	gCamera.setProjectionParams( camfov, dx.getBackBufferAspect(), camnear, camfar );

	// render
	dx.sceneBegin();

	RenderSceneWithShadows( gCamera, gCamera, shadowQuality, gUIChkLISPSM->isChecked() );
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
