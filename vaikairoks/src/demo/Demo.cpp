#include "stdafx.h"

#include "Demo.h"

#include <dingus/gfx/gui/Gui.h>
#include <dingus/utils/Random.h>
#include <ctime>


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


enum eFont {
	FNT_NORMAL = 0,
	FNT_HUGE = 1,
	FNT_LARGE = 2,
};



// --------------------------------------------------------------------------
//  game logic

wchar_t LETTERS[1000];
wchar_t PRESSAKEY[1000];

struct SWordDesc {
	wchar_t	word[100];
	char	picture[100];
};
std::vector<SWordDesc>	gWordDescs;


struct SPicture {
	CD3DTexture*	texture;
	RECT			uvs;
	const wchar_t*	word;
};
typedef std::vector<SPicture>	TPictureVector;


std::vector<TPictureVector>	gLetterPictures;



int			gCurrLetterIdx = 0;
int			gCurrPictureIdx = -1;
D3DXCOLOR	gLetterColor = D3DXCOLOR(0,0,0,1);

SUIElement	gUIElem;


void CDemo::initLetters()
{
	FILE* f;

	// read available words
	f = fopen( "data/words.txt", "rb" );
	// skip unicode mark
	fgetc( f ); fgetc( f );
	// first line - letters
	fwscanf( f, L"%ls\n", LETTERS );
	// secong line - press a key
	fgetws( PRESSAKEY, 100, f );
	// remaining lines - words
	while( !feof(f) ) {
		wchar_t apict[100];
		gWordDescs.push_back( SWordDesc() );
		SWordDesc& wd = gWordDescs.back();
		fwscanf( f, L"%ls", wd.word );
		fwscanf( f, L"%s\n", apict );
		wcstombs( wd.picture, apict, sizeof(wd.picture) );
	}
	fclose( f );

	// resize letter pictures vector
	gLetterPictures.resize( wcslen(LETTERS) );

	// read picture atlas definitions
	f = fopen( "data/Atlas.tai", "rt" );
	while( !feof(f) ) {
		char apict[100];
		char aatlas[100];
		int aindex;
		float au, av;
		float adu, adv;
		float dummy;
		fscanf( f, "%s %s %i, 2D, %f, %f, %f, %f, %f\n", apict, aatlas, &aindex, &au, &av, &dummy, &adu, &adv );

		// remove '?.png' from apict
		apict[strlen(apict)-5] = 0;

		// see if we have this word
		bool found = false;
		const wchar_t* word = 0;
		for( int i = 0; i < gWordDescs.size(); ++i ) {
			const SWordDesc& wd = gWordDescs[i];
			if( 0 == stricmp( apict, wd.picture ) ) {
				found = true;
				word = wd.word;
				break;
			}
		}
		if( !found )
			continue;

		// have this word, remember picture info
		int letterIdx = wcschr( LETTERS, word[0] ) - LETTERS;
		assert( letterIdx >= 0 && letterIdx < gLetterPictures.size() );

		SPicture pict;
		// remove '.dds,' from aatlas
		aatlas[strlen(aatlas)-5] = 0;
		pict.texture = RGET_TEX(aatlas);
		D3DSURFACE_DESC desc;
		pict.texture->getObject()->GetLevelDesc( 0, &desc );
		pict.uvs.left = au * desc.Width;
		pict.uvs.top = av * desc.Height;
		pict.uvs.right = (au+adu) * desc.Width;
		pict.uvs.bottom = (av+adv) * desc.Height;
		pict.word = word;
		gLetterPictures[letterIdx].push_back( pict );
	}
	fclose( f );

	// initial data
	gCurrLetterIdx = 0;
	fillLetterParams();
}

void CDemo::nextLetter()
{
	++gCurrLetterIdx;
	if( LETTERS[gCurrLetterIdx] == 0 )
		gCurrLetterIdx = 0;

	fillLetterParams();
}

void CDemo::fillLetterParams()
{
	// color
	gLetterColor.r = gRandom.getFloat( 0.1f, 0.6f );
	gLetterColor.g = gRandom.getFloat( 0.1f, 0.6f );
	gLetterColor.b = gRandom.getFloat( 0.1f, 0.6f );

	// picture index
	const TPictureVector& picts = gLetterPictures[gCurrLetterIdx];
	if( picts.empty() )
		gCurrPictureIdx = -1;
	else {
		gCurrPictureIdx = picts.size() * ((gRandom.getUInt()&1023)/1024.0f);
		CConsole::CON_WARNING << "picts: " << int(picts.size()) << " idx: " << gCurrPictureIdx << endl;
	}
}


void CALLBACK gUIRenderCallback( CUIDialog& dlg )
{
	SFRect r;

	// draw letter
	gUIElem.colorFont.current = gLetterColor;
	gUIElem.fontIdx = FNT_HUGE;
	gUIElem.textFormat = DT_CENTER | DT_VCENTER;

	wchar_t buf[10];
	buf[0] = LETTERS[gCurrLetterIdx];
	buf[1] = 0;
	r.left = 80; r.right = 270; r.top = 80; r.bottom = 300;
	dlg.drawText( buf, &gUIElem, &r, true );

	// draw "press a key"
	gUIElem.fontIdx = FNT_LARGE;
	gUIElem.colorFont.current = 0xFF808080;
	r.left = 0; r.right = 640; r.top = 400; r.bottom = 480;
	dlg.drawText( PRESSAKEY, &gUIElem, &r, true );

	// draw picture if we have one
	if( gCurrPictureIdx >= 0 ) {
		gUIElem.colorTexture.current = 0xFFffffff;
		const SPicture& pict = gLetterPictures[gCurrLetterIdx][gCurrPictureIdx];
		gUIElem.texture = pict.texture;
		gUIElem.textureRect = pict.uvs;

		float dx = (pict.uvs.right - pict.uvs.left) * 0.5f;
		float dy = (pict.uvs.bottom - pict.uvs.top) * 0.5f;
		r.left = 280;
		r.right = r.left + dx - 2;
		r.top = 220 - dy/2 - 20;
		r.bottom = r.top + dy - 2;

		// rectangle
		dlg.drawRect( &r, 0xC0000000 );

		r.left += 1;
		r.right -= 1;
		r.top += 1;
		r.bottom -= 1;
		
		// picture
		dlg.drawSprite( &gUIElem, &r );

		// the word
		r.top = r.bottom + 5;
		r.bottom = r.top + 50;
		gUIElem.fontIdx = FNT_LARGE;
		gUIElem.textFormat = DT_CENTER | DT_TOP | DT_NOCLIP;
		dlg.drawText( pict.word, &gUIElem, &r, true );
	}
}


// --------------------------------------------------------------------------
// initialization


void CALLBACK gUICallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
}

void CDemo::initialize( IDingusAppContext& appContext )
{
	gRandom.seed( time( NULL ) );
	gAppContext = &appContext;

	CD3DDevice& dx = CD3DDevice::getInstance();

	G_INPUTCTX->addListener( *this );

	// --------------------------------
	//  letter pictures

	initLetters();

	// --------------------------------
	// GUI

	gUIDlg = new CUIDialog();
	gUIDlg->setFont( FNT_HUGE, "Comic Sans MS", 240, FW_BOLD );
	gUIDlg->setFont( FNT_LARGE, "Comic Sans MS", 32, FW_BOLD );

	gUIDlg->setCallback( gUICallback );
	gUIDlg->setRenderCallback( gUIRenderCallback );

	const int hctl = 16;
	const int hrol = 14;

	// UI
	{
		gUIElem.fontIdx = FNT_HUGE;
		gUIElem.colorFont.current = 0xFF000000;
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
	//char buf[100];
	//sprintf( buf, "fps: %6.2f", dx.getStats().getFPS() );
	//gUIFPS->setText( buf );

	// render
	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();
	dx.clearTargets( true, true, false, 0xFFf0f0f0, 1.0f, 0L );
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
