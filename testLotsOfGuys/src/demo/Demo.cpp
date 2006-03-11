#include "stdafx.h"

#include "Demo.h"
#include "ComplexStuffEntity.h"
#include "IdlePlayer.h"
#include "Entity.h"

#include <dingus/math/MathUtils.h>
#include <dingus/gfx/gui/Gui.h>
#include <dingus/utils/Random.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>
#include <dingus/renderer/EffectParamsNotifier.h>


// --------------------------------------------------------------------------
// Demo variables, constants, etc.

IDingusAppContext*	gAppContext;
bool	gFinished = false;
bool	gShowStats = false;


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

CUIDialog*		gUIDlgHUD;
CUIStatic*		gUIFPS;

IdleConfig		gAnimCfg;
std::vector<CComplexStuffEntity*>	gGuys;
std::vector<IdlePlayer>				gGuyPlayers;


// --------------------------------------------------------------------------


void CDemo::initialize( IDingusAppContext& appContext )
{
	gAppContext = &appContext;
	G_INPUTCTX->addListener( *this );

	// --------------------------------
	// scene

	gCamera.mWorldMat.getOrigin().set( 0, 2, -18 );
	gCamYaw = 0.0f;
	gCamPitch = 0.2f;

	InitIdleConfig( gAnimCfg );

	int GUY_COUNT = 500;
	gGuys.reserve( GUY_COUNT );
	gGuyPlayers.reserve( GUY_COUNT );
	for( int i = 0; i < GUY_COUNT; ++i )
	{
		CComplexStuffEntity* e = new CComplexStuffEntity( "Bicas", "idle_v01" );
		D3DXMatrixRotationY( &e->getWorldMatrix(), gRandom.getFloat(0,D3DX_PI*2) );
		e->getWorldMatrix().getOrigin().set(
			gRandom.getFloat( -25.0f, 25.0f ),
			0.0f,
			gRandom.getFloat( -25.0f, 25.0f )
		);
		time_value startTime = time_value::fromsec( gRandom.getFloat(-20.0f, -1.0f) );
		e->getAnimator().playDefaultAnim( startTime );
		gGuys.push_back( e );
		gGuyPlayers.push_back( IdlePlayer( gAnimCfg ) );
	}

	// --------------------------------
	// GUI

	gUIDlgHUD = new CUIDialog();

	gUIDlgHUD->addStatic( 0, "", 5,  460, 200, 20, false, &gUIFPS );
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
			gCamera.mWorldMat.getOrigin().y += dt * 3;
			break;
		case DIK_PGDN:
			gCamera.mWorldMat.getOrigin().y -= dt * 3;
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
				gCamera.mWorldMat.getOrigin() += dir * dt * 3;
			}
			break;
		case DIK_DOWN:
			{
				SVector3 dir = gCamera.mWorldMat.getAxisZ();
				dir.y = 0.0f;
				dir.normalize();
				gCamera.mWorldMat.getOrigin() -= dir * dt * 3;
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


/// Main loop code.
void CDemo::perform()
{
	char buf[100];

	G_INPUTCTX->perform();
	CDynamicVBManager::getInstance().discard();
	
	time_value tv = CSystemTimer::getInstance().getTime();
	//tv.value /= 4;
	float dt = CSystemTimer::getInstance().getDeltaTimeS();

	int i;
	int nguys = gGuys.size();
	for( i = 0; i < nguys; ++i ) {
		gGuyPlayers[i].update( *gGuys[i], tv );
	}

	CD3DDevice& dx = CD3DDevice::getInstance();

	// FPS
	sprintf( buf, "fps: %6.1f", dx.getStats().getFPS() );
	gUIFPS->setText( buf );

	// camera
	SMatrix4x4& mc = gCamera.mWorldMat;
	SVector3 o = mc.getOrigin();
	D3DXMatrixRotationYawPitchRoll( &mc, gCamYaw, gCamPitch, 0.0f );
	mc.getOrigin() = o;
	const float camnear = 0.1f;
	const float camfar = 100.0f;
	const float camfov = D3DX_PI*0.25f;
	gCamera.setProjectionParams( camfov, dx.getBackBufferAspect(), camnear, camfar );
	gCamera.setOntoRenderContext();

	// render
	dx.sceneBegin();

	dx.clearTargets( true, true, false, 0xFF404040 );

	G_RENDERCTX->applyGlobalEffect();
	for( i = 0; i < nguys; i += 1 )
		gGuys[i]->render();
	G_RENDERCTX->perform();

	gUIDlgHUD->onRender( dt );
	dx.sceneEnd();
}



// --------------------------------------------------------------------------
// Cleanup


void CDemo::shutdown()
{
	safeDelete( gUIDlgHUD );

	//stl_utils::wipe( gScene );
}
