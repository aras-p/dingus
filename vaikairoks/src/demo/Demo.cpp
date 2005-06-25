#include "stdafx.h"

#include "Demo.h"

#include <dingus/gfx/gui/Gui.h>
#include <dingus/utils/Random.h>


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
// Demo initialization

CUIDialog*		gUIDlgHUD;
CUISlider*		gUISliderYaw;
CUISlider*		gUISliderPitch;
CUISlider*		gUISliderZoom;

CUICheckBox*	gUIChkDilate;

CUIStatic*		gUIFPS;



// --------------------------------------------------------------------------
// objects



void CALLBACK gUICallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
}


void CDemo::initialize( IDingusAppContext& appContext )
{
	gAppContext = &appContext;

	CD3DDevice& dx = CD3DDevice::getInstance();

	G_INPUTCTX->addListener( *this );

	// --------------------------------
	// GUI

	gUIDlgHUD = new CUIDialog();
	gUIDlgHUD->setCallback( gUICallback );

	const int hctl = 16;
	const int hrol = 14;

	// fps
	{
		gUIDlgHUD->addStatic( 0, "(wait)", 5,  460, 200, 20, false, &gUIFPS );
	}
	// zoom
	{
		gUIDlgHUD->addStatic( 0, "yaw ", 210,  5, 40, hctl );
		gUIDlgHUD->addSlider( 0, 250,  5, 100, hctl, -180, 180, 10, false, &gUISliderYaw );
		gUIDlgHUD->addStatic( 0, "ptch", 210, 25, 40, hctl );
		gUIDlgHUD->addSlider( 0, 250, 25, 100, hctl, 0, 90, 5, false, &gUISliderPitch );
		gUIDlgHUD->addStatic( 0, "zoom", 210, 45, 40, hctl );
		gUIDlgHUD->addSlider( 0, 250, 45, 100, hctl, 5, 20, 6, false, &gUISliderZoom );
	}
	//
	{
		gUIDlgHUD->addCheckBox( 0, "Preblur shadows", 5, 50, 100, 20, true, 0, false, &gUIChkDilate );
	}
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
		}
	}
}

void CDemo::onInputStage()
{
}


/// Main loop code
void CDemo::perform()
{
	G_INPUTCTX->perform();
	
	double t = CSystemTimer::getInstance().getTimeS();
	float dt = CSystemTimer::getInstance().getDeltaTimeS();

	CD3DDevice& dx = CD3DDevice::getInstance();

	// FPS
	char buf[100];
	sprintf( buf, "fps: %6.2f", dx.getStats().getFPS() );
	gUIFPS->setText( buf );

	// render
	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();
	dx.clearTargets( true, true, false, 0xFF000000, 1.0f, 0L );
	dx.sceneBegin();

	// render GUI
	gUIDlgHUD->onRender( dt );

	dx.sceneEnd();

	// sleep a bit
	Sleep( 1 );
}



// --------------------------------------------------------------------------
// Cleanup


void CDemo::shutdown()
{
	safeDelete( gUIDlgHUD );
}
