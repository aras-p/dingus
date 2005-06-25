#include "stdafx.h"

#include "Demo.h"

#include <dingus/gfx/gui/Gui.h>
#include <dingus/utils/Random.h>


// --------------------------------------------------------------------------
//  Common stuff

IDingusAppContext*	gAppContext;

bool	gFinished = false;
bool	gShowStats = false;



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
// Demo data


CUIDialog*		gUIDlg;
CUISlider*		gUISliderYaw;
CUISlider*		gUISliderPitch;
CUISlider*		gUISliderZoom;

CUICheckBox*	gUIChkDilate;

CUIStatic*		gUIFPS;


enum eFont {
	FNT_NORMAL = 0,
	FNT_HUGE = 1,
	FNT_LARGE = 2,
};


CUIStatic*		gUILetter;		///< The big letter
CUIStatic*		gUIPressKey;	///< Press the key



// --------------------------------------------------------------------------
//  game logic

static const char* LETTERS = "ABCDEFGHIYJKLMNOPRSTUVZ";

int	gCurrLetterIdx;


void CDemo::nextLetter()
{
	++gCurrLetterIdx;
	if( LETTERS[gCurrLetterIdx] == 0 )
		gCurrLetterIdx = 0;

	// set new letter
	char buf[10];
	buf[0] = LETTERS[gCurrLetterIdx];
	buf[1] = 0;
	gUILetter->setText( buf );
}


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

	gUIDlg = new CUIDialog();
	gUIDlg->setFont( FNT_HUGE, "Comic Sans MS", 240, FW_BOLD );
	gUIDlg->setFont( FNT_LARGE, "Comic Sans MS", 32, FW_BOLD );

	gUIDlg->setCallback( gUICallback );

	const int hctl = 16;
	const int hrol = 14;

	// fps
	{
		gUIDlg->addStatic( 0, "(wait)", 5,  460, 200, 20, false, &gUIFPS );
	}
	// zoom
	{
		gUIDlg->addStatic( 0, "yaw ", 210,  5, 40, hctl );
		gUIDlg->addSlider( 0, 250,  5, 100, hctl, -180, 180, 10, false, &gUISliderYaw );
		gUIDlg->addStatic( 0, "ptch", 210, 25, 40, hctl );
		gUIDlg->addSlider( 0, 250, 25, 100, hctl, 0, 90, 5, false, &gUISliderPitch );
		gUIDlg->addStatic( 0, "zoom", 210, 45, 40, hctl );
		gUIDlg->addSlider( 0, 250, 45, 100, hctl, 5, 20, 6, false, &gUISliderZoom );
	}
	//
	{
		gUIDlg->addCheckBox( 0, "Preblur shadows", 5, 50, 100, 20, true, 0, false, &gUIChkDilate );
	}
	// game
	{
		gUIDlg->addStatic( 0, "A", 160, 80, 320, 300, false, &gUILetter );
		gUILetter->getElement(0)->setFont( FNT_HUGE, true, DT_CENTER | DT_VCENTER );
		gCurrLetterIdx = 0;

		gUIDlg->addStatic( 0, "Spausk bet kuri klavisa", 0, 400, 640, 80, false, &gUIPressKey );
		gUIPressKey->getElement(0)->setFont( FNT_LARGE, true, DT_CENTER | DT_VCENTER );
	}
}


// --------------------------------------------------------------------------
// Perform code (main loop)

bool CDemo::msgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	bool done = false;
	if( gUIDlg ) {
		done = gUIDlg->msgProc( hwnd, msg, wparam, lparam );
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

		default:
			if( ke.getAscii() >= ' ' && ke.getMode() == CKeyEvent::KEY_PRESSED ) {
				nextLetter();
			}
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
	dx.clearTargets( true, true, false, 0xFFe0e0e0, 1.0f, 0L );
	dx.sceneBegin();

	// render GUI
	gUIDlg->onRender( dt );

	dx.sceneEnd();

	// sleep a bit
	Sleep( 1 );
}



// --------------------------------------------------------------------------
// Cleanup


void CDemo::shutdown()
{
	safeDelete( gUIDlg );
}
