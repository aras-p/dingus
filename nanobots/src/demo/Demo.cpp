#include "stdafx.h"

// Demo application.
// Most of stuff is just kept in global variables :)

#include "Demo.h"
#include "DemoResources.h"
#include "DemoUI.h"
#include "GameSetupUI.h"

#include <dingus/gfx/DebugRenderer.h>
#include <dingus/resource/IndexBufferFillers.h>
#include <dingus/gfx/gui/Gui.h>
#include <dingus/math/MathUtils.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>

#include <dingus/audio/AudioListener.h>
#include <dingus/audio/Sound.h>


#include "GameInfo.h"
#include "game/GameState.h"
#include "game/GameDesc.h"
#include "map/LevelMesh.h"
#include "map/PointsMesh.h"
#include "entity/EntityManager.h"
#include "net/NetInterface.h"


// --------------------------------------------------------------------------
// Demo variables, constants, etc.

const char* RMODE_PREFIX[RMCOUNT] = {
	"normal/",
};

std::string	gServerName;
int			gServerPort;


int			gGlobalCullMode;	// global cull mode
SVector4	gScreenFixUVs;		// UV fixes for fullscreen quads
float		gTimeParam;			// time parameter for effects
SVector4	gFogParam;			// fog params
SVector4	gFogColorParam;		// fog color param

CDebugRenderer*	gDebugRenderer;
IDingusAppContext*	gAppContext;
SD3DSettingsPref*	gD3DSettingsPref;


bool	gFinished = false;
bool	gShowStats = false;

bool			gReadSettings = false;
SAppSettings	gAppSettings;


// --------------------------------------------------------------------------

CDemo::CDemo( const std::string& serverName, int serverPort )
{
	gServerName = serverName;
	gServerPort = serverPort;
}

bool CDemo::checkDevice( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors )
{
	bool ok = true;

	// require VS1.1, or SW vertex processing
	if( caps.getVShaderVersion() < CD3DDeviceCaps::VS_1_1 && vproc < CD3DDeviceCaps::VP_SW ) {
		// don't add error - will turn on swvp and it will be ok
		//errors.addError( "we require vertex shaders!!!" );
		ok = false;
	}

	// require at least this amount of vidmem
	const int videoMB = 8;
	if( caps.getVideoMemoryMB() < videoMB ) {
		char buf[200];
		sprintf( buf, "%iMB of video memory is required, found %iMB", videoMB, caps.getVideoMemoryMB() );
		errors.addError( buf );
		ok = false;
	}

	// GF4MX hack - force SWVP
	if( caps.getPShaderVersion() < CD3DDeviceCaps::PS_1_1 && caps.getVShaderVersion() > CD3DDeviceCaps::VS_FFP ) {
		if( vproc != CD3DDeviceCaps::VP_SW )
			ok = false;
	}

	// return result
	return ok;
}


static const char* CFG_SETTINGS = "data/config.dat";
static const DWORD CFG_VERSION = 20050117;

void CDemo::initD3DSettingsPref( SD3DSettingsPref& pref )
{
	gReadSettings = false;

	gD3DSettingsPref = &pref;
	FILE* f = fopen( CFG_SETTINGS, "rb" );
	if( !f )
		return;
	DWORD version;
	int r;
	r = fread( &version, 1, sizeof(version), f );
	if( r != sizeof(version) || version != CFG_VERSION ) {
		fclose( f );
		return;
	}
	r = fread( &pref, 1, sizeof(pref), f );
	if( r != sizeof(pref) ) {
		fclose( f );
		return;
	}

	SAppSettings settings;
	r = fread( &settings, 1, sizeof(settings), f );
	if( r != sizeof(settings) ) {
		fclose( f );
		return;
	}

	settings.gfxDetail = clamp( settings.gfxDetail, 0, GFX_DETAIL_LEVELS-1 );
	settings.musicVolume = clamp( settings.musicVolume, 0, SFX_MAX_VOLUME );
	settings.soundVolume = clamp( settings.soundVolume, 0, SFX_MAX_VOLUME );
	gReadSettings = true;

	fclose( f );

	gAppSettings = settings;
}

static void gWriteD3DSettingsPref()
{
	const CD3DSettings& s = gAppContext->getD3DSettings();
	gD3DSettingsPref->adapterVendorID = s.getAdapterInfo().adapterID.VendorId;
	gD3DSettingsPref->adapterDeviceID = s.getAdapterInfo().adapterID.DeviceId;
	gD3DSettingsPref->device = s.getDeviceInfo().caps.getDeviceType();
	gD3DSettingsPref->mode = s.mMode;
	gD3DSettingsPref->vsync = s.isVSync() ? 1 : 0;
	gD3DSettingsPref->fsWidth = s.getDisplayMode().Width;
	gD3DSettingsPref->fsHeight = s.getDisplayMode().Height;
	gD3DSettingsPref->fsRefresh = s.getDisplayMode().RefreshRate;
	gD3DSettingsPref->fsFormat = s.getDisplayMode().Format;
	gD3DSettingsPref->fsaaType = s.getMultiSampleType();
	gD3DSettingsPref->fsaaQuality = s.getMultiSampleQuality();
	gD3DSettingsPref->backbuffer = s.getBackBufferFormat();
	gD3DSettingsPref->zstencil = s.getDepthStencilFormat();
	gD3DSettingsPref->vertexProc = s.getVertexProcessing();

	FILE* f = fopen( CFG_SETTINGS, "wb" );
	if( !f )
		return;
	DWORD version = CFG_VERSION;
	fwrite( &version, 1, sizeof(version), f );
	fwrite( gD3DSettingsPref, 1, sizeof(*gD3DSettingsPref), f );
	fwrite( &gAppSettings, 1, sizeof(gAppSettings), f );
	fclose( f );
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

bool	gInitMainStarted = false;
bool	gInitMainDone = false;
bool	gInitGuiDone = false;

const float VIEWER_Y = 1.0f;
const float VIEWER_R = 0.6f;


/// Position/orientation of the base viewer. No zoom outs, etc.
SMatrix4x4		gViewer;

/// Last "inside the level" position of the viewer.
SVector3		gLastViewerValidPos;

// Viewer origin/z moving velocity for camera spring.
SVector3		gViewerVel;
SVector3		gViewerZVel;

CCameraEntity	gCamera;
CCameraEntity	gCameraMiniMap;

bool	gPlayMode = false;

const float	MEGAZOOM_MIN = 3.0f;
const float MEGAZOOM_MAX = 200.0f;

const float MEGATILT_MIN = 45.0f;
const float MEGATILT_MAX = 90.0f;

// mouse
float		gMouseX; // from -1 to 1
float		gMouseY; // from -1 to 1
SVector3	gMouseRay;

// game network updates
time_value	gLastGameUpdateTime;

// background sound
CSound*	gSndHeart;



// --------------------------------------------------------------------------
//  GUI

const int	GID_CHK_MEGAMAP = 1001;
const int	GID_ROL_MINIMAP = 1002;
const int	GID_ROL_STATS = 1003;
//const int	GID_SLD_TIME = 1004; // TBD
const int	GID_SLD_ZOOM = 1005;
const int	GID_SLD_TILT = 1006;
const int	GID_ROL_ESTATS = 1007;

const int	GID_CHK_OPTIONS = 1050;
const int	GID_CHK_HELP = 1051;

//const int	GID_BTN_SELAI1 = 1100; // TBD
//const int	GID_BTN_SELAI2 = 1101;

const int	GID_CHK_AZN_NEED = 1110;
const int	GID_CHK_AZN_COLL = 1111;

//const int	GID_BTN_PLAY = 1200; // TBD


CUIDialog*		gUIDlg;

CDemoSettingsDialog*	gUISettingsDlg;
bool			gSettingsDlgWasActive = false;
CD3DSettings	gSettingsAtDlgStart;

CDemoHelpDialog*	gUIHelpDlg;
bool			gHelpDlgWasActive = false;

CGameSetupDialog*	gUIGameSetupDlg;


CUIStatic*		gUILabelProgress;
CUIImage*		gUIImgLogo;

CUIRollout*		gUIRollMinimap;
CUIStatic*		gUILabelTime;
CUIStatic*		gUILabelFPS;

//CUIButton*		gUIBtnTimeRew; // TBD
//CUIButton*		gUIBtnTimePlay;
//CUIButton*		gUIBtnTimeFfwd;
//CUISlider*		gUISliderTime;

CUIStatic*		gUILabelZoom;
CUISlider*		gUISliderZoom;

CUIStatic*		gUILabelTilt;
CUISlider*		gUISliderTilt;

CUIRollout*		gUIRollStats;
std::vector<CUIControl*>	gUIStatsCtrls;
CUIRollout*		gUIRollEStats;
std::vector<CUIControl*>	gUIEStatsCtrls;
bool			gShowEntityStats;

struct SUIPlayerStats {
	CUIStatic*	score;
	CUIStatic*	botCount;
};
SUIPlayerStats	gUIPlayerStats[G_MAX_PLAYERS];

CUIStatic*		gUIEStType;
CUIStatic*		gUIEStHealth;
CUIStatic*		gUIEStOwner;
CUIStatic*		gUIEStAZN;


static void gSetPlayMode( bool play )
{
	gPlayMode = play;
	// TBD
	//gUIBtnTimePlay->getElement(0)->textureRect.left = play ? 32 : 128;
	//gUIBtnTimePlay->getElement(0)->textureRect.right = play ? 64 : 160;
}


void CALLBACK gUICallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
	CGameInfo& gi = CGameInfo::getInstance();

	switch( evt ) {
	case UIEVENT_BUTTON_CLICKED:
		/*
		// TBD
		switch( ctrlID ) {
		case GID_BTN_PLAY:
			gSetPlayMode( !gPlayMode );
			break;
		case GID_BTN_SELAI1:
			gi.getEntities().setSelectedEntity( gi.getReplay().getPlayer(0).entityAI );
			break;
		case GID_BTN_SELAI2:
			gi.getEntities().setSelectedEntity( gi.getReplay().getPlayer(1).entityAI );
			break;
		}
		*/
		break;
	case UIEVENT_CHECKBOX_CHANGED:
		{
			CUICheckBox* cbox = (CUICheckBox*)ctrl;
			switch( ctrlID ) {
			case GID_CHK_MEGAMAP:
				gAppSettings.followMode = !cbox->isChecked();
				gUISliderZoom->setEnabled( !gAppSettings.followMode );
				gUILabelZoom->setEnabled( !gAppSettings.followMode );
				gUISliderTilt->setEnabled( !gAppSettings.followMode );
				gUILabelTilt->setEnabled( !gAppSettings.followMode );
				if( gAppSettings.followMode ) {
					gViewer.getOrigin() = gLastViewerValidPos;
				}
				break;

			case GID_CHK_AZN_NEED:
				gAppSettings.drawAznNeedle = cbox->isChecked();
				break;

			case GID_CHK_AZN_COLL:
				gAppSettings.drawAznCollector = cbox->isChecked();
				break;

			case GID_ROL_MINIMAP:
				gAppSettings.showMinimap = cbox->isChecked();
				break;

			case GID_ROL_STATS:
				{
					bool vis = cbox->isChecked();
					int nn = gUIStatsCtrls.size();
					for( int i = 0; i < nn; ++i )
						gUIStatsCtrls[i]->setVisible( vis );
					int dy = gUIRollStats->getRolloutHeight() * (vis ? 1 : -1);
					nn = gUIEStatsCtrls.size();
					for( int i = 0; i < nn; ++i ) {
						CUIControl& ct = *gUIEStatsCtrls[i];
						ct.setLocation( ct.mX, ct.mY + dy );
					}
					gUIRollEStats->setLocation( gUIRollEStats->mX, gUIRollEStats->mY + dy );
				}
				break;

			case GID_ROL_ESTATS:
				{
					bool vis = cbox->isChecked();
					if( cbox->isEnabled() )
						gShowEntityStats = vis;
					int nn = gUIEStatsCtrls.size();
					for( int i = 0; i < nn; ++i )
						gUIEStatsCtrls[i]->setVisible( vis );
				}
				break;

			case GID_CHK_OPTIONS:
				{
					bool vis = cbox->isChecked();
					if( vis ) {
						assert( !gSettingsDlgWasActive );
						gSetPlayMode( false );
						gUISettingsDlg->getAppSettings() = gAppSettings;
						gSettingsAtDlgStart = gAppContext->getD3DSettings();
						gUISettingsDlg->showDialog( gSettingsAtDlgStart );
						gSettingsDlgWasActive = true;
					}
				}
				break;
			case GID_CHK_HELP:
				{
					bool vis = cbox->isChecked();
					if( vis ) {
						assert( !gHelpDlgWasActive );
						gSetPlayMode( false );
						gUIHelpDlg->showDialog();
						gHelpDlgWasActive = true;
					}
				}
				break;
			}
		}
		break;

	case UIEVENT_SLIDER_VALUE_CHANGED:
		{
			CUISlider* sld = (CUISlider*)ctrl;
			switch( ctrlID ) {
			/* // TBD
			case GID_SLD_TIME:
				gi.setTime( sld->getValue() );
				gSetPlayMode( false );
				break;
			*/
			case GID_SLD_ZOOM:
				{
					int oldSldVal = gAppSettings.megaZoom * 10;
					int newSldVal = sld->getValue();
					if( newSldVal != oldSldVal )
						gAppSettings.megaZoom = newSldVal * 0.1f;
				}
				break;
			case GID_SLD_TILT:
				{
					int oldSldVal = gAppSettings.megaTilt;
					int newSldVal = sld->getValue();
					if( newSldVal != oldSldVal )
						gAppSettings.megaTilt = newSldVal;
				}
				break;
			}
		}
		break;
	}
}

void CALLBACK gUIRenderCallback( CUIDialog& dlg )
{
	CGameInfo::getInstance().getEntities().renderLabels( dlg, gAppSettings.followMode );
}


#define UISTATS_LABEL \
	label->getElement(0)->setFont( 0, true, DT_LEFT | DT_VCENTER ); \
	gUIStatsCtrls.push_back( label )
#define UISTATS_RLABEL \
	label->getElement(0)->setFont( 0, true, DT_RIGHT | DT_VCENTER ); \
	gUIStatsCtrls.push_back( label )
#define UISTATS_RLABEL_B \
	label->getElement(0)->setFont( 2, true, DT_RIGHT | DT_VCENTER ); \
	gUIStatsCtrls.push_back( label )

#define UIESTATS_LABEL \
	label->getElement(0)->setFont( 0, true, DT_LEFT | DT_VCENTER ); \
	gUIEStatsCtrls.push_back( label )
#define UIESTATS_RLABEL \
	label->getElement(0)->setFont( 0, true, DT_RIGHT | DT_VCENTER ); \
	gUIEStatsCtrls.push_back( label )

const int UIHCTL = 16;
const int UIHROL = 14;
const int UIHLAB = 14;


static void	gSetupGUI()
{
	const CGameInfo& gi = CGameInfo::getInstance();
	const CGameDesc& desc = gi.getGameDesc();
	int nplayers = desc.getPlayerCount();

	gUIGameSetupDlg = new CGameSetupDialog();

	// stats
	{
		int sy = 0;
		CUIStatic* label;

		// rollout
		gUIDlg->addRollout( GID_ROL_STATS, "Stats (S)", 0, sy, 130, UIHROL, 20 + nplayers*2*UIHLAB, true, 'S', false, &gUIRollStats );
		// map name
		gUIDlg->addStatic( 0,
			("Map: "+desc.getMap().getName()).c_str(), 5, sy += UIHLAB, 120, UIHLAB, false, &label );
		UISTATS_LABEL;

		// player stats
		const char* PLAYER_TEX_NAMES[G_MAX_PLAYERS] = {
			NULL,
			RID_TEX_PLAYER1,
			RID_TEX_PLAYER2,
		};
		for( int p = 1; p < nplayers; ++p ) {
			sy += 5;
			const CGameDesc::SPlayer& pl = desc.getPlayer(p);
			SUIPlayerStats& plui = gUIPlayerStats[p];
			sy += UIHLAB;

			// flag
			gUIDlg->addImage( 0, 4, sy+4, 18, 18, *RGET_S_TEX(PLAYER_TEX_NAMES[p]), 0, 0, CGameDesc::FLAG_SIZE, CGameDesc::FLAG_SIZE );

			// name
			std::string plstring;
			plstring = "P";
			plstring += ('0' + p);
			plstring += ": ";
			plstring += pl.name;
			gUIDlg->addStatic( 0, plstring.c_str(), 25, sy, 100, UIHLAB, false, &label );
			UISTATS_LABEL;

			// score
			gUIDlg->addStatic( 0, "Scor:", 30, sy += UIHLAB, 25, UIHLAB, false, &label );
			UISTATS_LABEL;
			gUIDlg->addStatic( 0, "0000", 55, sy, 25, UIHLAB, false, &label );
			UISTATS_RLABEL_B;
			plui.score = label;
			// alive entities
			gUIDlg->addStatic( 0, "Ent:", 90, sy, 20, UIHLAB, false, &label );
			UISTATS_LABEL;
			gUIDlg->addStatic( 0, "00", 110, sy, 12, UIHLAB, false, &label );
			UISTATS_RLABEL;
			plui.botCount = label;
		}
		// AI stats
		{
			sy += 5;
			gUIDlg->addStatic( 0, "Computer", 25, sy += UIHLAB, 100, UIHLAB, false, &label );
			UISTATS_LABEL;
			gUIPlayerStats[0].score = NULL;
			// alive entities
			gUIDlg->addStatic( 0, "Ent:", 90, sy, 20, UIHLAB, false, &label );
			UISTATS_LABEL;
			gUIDlg->addStatic( 0, "00", 110, sy, 12, UIHLAB, false, &label );
			UISTATS_RLABEL;
			gUIPlayerStats[0].botCount = label;
		}
	}
	// selected entity stats
	{
		int sy = gUIRollStats->mY + gUIRollStats->mHeight + gUIRollStats->getRolloutHeight();
		CUIStatic* label;

		gShowEntityStats = true;
		// rollout
		gUIDlg->addRollout( GID_ROL_ESTATS, "Entity stats (E)", 0, sy, 130, UIHROL, 3*UIHLAB+3, true, 'E', false, &gUIRollEStats );
		// type
		gUIDlg->addStatic( 0, "Type:", 5, sy += UIHLAB, 30, UIHLAB, false, &label );
		UIESTATS_LABEL;
		gUIDlg->addStatic( 0, "MMMMM", 35, sy, 45, UIHLAB, false, &label );
		UIESTATS_LABEL;
		gUIEStType = label;
		// owner
		gUIDlg->addStatic( 0, "Own:", 80, sy, 25, UIHLAB, false, &label );
		UIESTATS_LABEL;
		gUIDlg->addStatic( 0, "MMM", 105, sy, 25, UIHLAB, false, &label );
		UIESTATS_LABEL;
		gUIEStOwner = label;
		// health
		gUIDlg->addStatic( 0, "Health:", 5, sy += UIHLAB, 45, UIHLAB, false, &label );
		UIESTATS_LABEL;
		gUIDlg->addStatic( 0, "000/999", 50, sy, 50, UIHLAB, false, &label );
		UIESTATS_LABEL;
		gUIEStHealth = label;
		// azn
		gUIDlg->addStatic( 0, "AZN:", 5, sy += UIHLAB, 45, UIHLAB, false, &label );
		UIESTATS_LABEL;
		gUIDlg->addStatic( 0, "0000", 50, sy, 80, UIHLAB, false, &label );
		UIESTATS_LABEL;
		gUIEStAZN = label;
	}
	// camera/zoom/tilt
	{
		gUIDlg->addCheckBox( GID_CHK_MEGAMAP, "Megamap (M)", 135, 0, 90, UIHCTL, !gAppSettings.followMode, 'M', false, NULL );
		gUIDlg->addStatic( 0, "zoom (pgup/pgdn)", 225, 0, 90, UIHCTL, false, &gUILabelZoom );
		gUIDlg->addSlider( GID_SLD_ZOOM, 320, 0, 120, UIHCTL, MEGAZOOM_MIN*10, MEGAZOOM_MAX*10, gAppSettings.megaZoom*10, false, &gUISliderZoom );
		gUIDlg->addStatic( 0, "tilt (home/end)", 225, UIHCTL, 90, UIHCTL, false, &gUILabelTilt );
		gUIDlg->addSlider( GID_SLD_TILT, 320, UIHCTL, 120, UIHCTL, MEGATILT_MIN, MEGATILT_MAX, gAppSettings.megaTilt, false, &gUISliderTilt );
		gUILabelZoom->getElement(0)->setFont( 0, false, DT_RIGHT | DT_VCENTER );
		gUILabelTilt->getElement(0)->setFont( 0, false, DT_RIGHT | DT_VCENTER );
		gUILabelZoom->setEnabled( !gAppSettings.followMode );
		gUISliderZoom->setEnabled( !gAppSettings.followMode );
		gUILabelTilt->setEnabled( !gAppSettings.followMode );
		gUISliderTilt->setEnabled( !gAppSettings.followMode );
	}
	// minimap
	{
		gUIDlg->addRollout( GID_ROL_MINIMAP, "Minimap (N)", 480, 0, 160, UIHROL, 120, gAppSettings.showMinimap, 'N', false, &gUIRollMinimap );
	}
	// time controls
	{
		//CUIButton* btn;
		/*
		// TBD
		gUIDlg->addButton( 0, "", 285-22*1, 440, 22, 22, 0, false, &btn );
		SetRect( &btn->getElement(0)->textureRect, 64, 413, 96, 443 );
		gUIBtnTimeRew = btn;

		gUIDlg->addButton( GID_BTN_PLAY, "", 355+22*0, 440, 22, 22, ' ', false, &btn );
		SetRect( &btn->getElement(0)->textureRect, 128, 413, 160, 443 );
		gUIBtnTimePlay = btn;
		
		gUIDlg->addButton( 0, "", 355+22*1, 440, 22, 22, 0, false, &btn );
		SetRect( &btn->getElement(0)->textureRect, 96, 413, 128, 443 );
		gUIBtnTimeFfwd = btn;
		*/
		
		gUIDlg->addStatic( 0, "", 289, 440, 62, 22, false, &gUILabelTime );
		gUILabelTime->getElement(0)->setFont( 1, false, DT_RIGHT | DT_VCENTER );

		// TBD
		//gUIDlg->addSlider( GID_SLD_TIME, 285-22, 462, 355-285+22*3, UIHCTL, 0, gi.getReplay().getGameTurnCount()-1, 0, false, &gUISliderTime );
		
		gUIDlg->addStatic( 0, "", 3, 480-UIHLAB-1, 100, UIHLAB, false, &gUILabelFPS );
	}
	// other
	{
		/* // TBD
		gUIDlg->addButton( GID_BTN_SELAI1, "Sel AI1 (1)", 2, 300, 60, UIHLAB, '1' );
		gUIDlg->addButton( GID_BTN_SELAI2, "Sel AI2 (2)", 2, 316, 60, UIHLAB, '2' );
		if( nplayers < 2 )
			gUIDlg->getButton(GID_BTN_SELAI2)->setEnabled( false );
		*/
		
		gUIDlg->addCheckBox( GID_CHK_AZN_NEED, "AZN Needles", 2, 332, 90, UIHCTL, gAppSettings.drawAznNeedle, 0, false, NULL );
		gUIDlg->addCheckBox( GID_CHK_AZN_COLL, "AZN Collectors", 2, 348, 90, UIHCTL, gAppSettings.drawAznCollector, 0, false, NULL );

		gUIDlg->addCheckBox( GID_CHK_OPTIONS, "Options", 560, 480-UIHCTL*2, 80, UIHCTL, gSettingsDlgWasActive );
		gUIDlg->addCheckBox( GID_CHK_HELP, "Help", 560, 480-UIHCTL, 80, UIHCTL, gHelpDlgWasActive );
	}

	gUILabelProgress->setVisible( false );
	gUIImgLogo->setVisible( false );
	gUIDlg->enableNonUserEvents( true );

	gUIDlg->setRenderCallback( gUIRenderCallback );
	
	gInitGuiDone = true;
}



static void gProbablyInitSettings()
{
	// if settings are read from config file - leave them as they are
	if( gReadSettings )
		return;

	//
	// set the graphics detail level

	// default is max. level
	int gfxLevel = GFX_DETAIL_LEVELS-1;
	const CD3DDeviceCaps& caps = CD3DDevice::getInstance().getCaps();
	
	// if we're software VP, decrease level a bit
	if( caps.getVertexProcessing() == CD3DDeviceCaps::VP_SW || caps.getVertexProcessing() == CD3DDeviceCaps::VP_MIXED ) {
		--gfxLevel;
		int cpuMhz = caps.getCpuMhz();
		// below 2GHz - decrease more
		if( cpuMhz < 2000 )
			--gfxLevel;
		// below 1GHz - even more
		if( cpuMhz < 1000 )
			--gfxLevel;
	}
	if( gfxLevel < 0 )
		gfxLevel = 0;
	
	gAppSettings.gfxDetail = gfxLevel;
}

static void gInitiallyPlaceViewer()
{
	// TBD
	/*
	const CGameInfo& gi = CGameInfo::getInstance();
	const CGameMap& gmap = gi.getGameMap();
	const CGameReplay& replay = gi.getReplay();
	const CLevelMesh& level = gi.getLevelMesh();

	const CReplayEntity::SState& pl1st = replay.getEntity( replay.getPlayer(0).entityAI ).getTurnState( 0 );
	int posx = pl1st.posx;
	int posy = pl1st.posy;

	const int XOFF[4] = { 2, 2, -2, -2 };
	const int YOFF[4] = { 2, -2, 2, -2 };
	const float ROT[4] = { D3DX_PI/4*7, D3DX_PI/4*5, D3DX_PI/4*1, D3DX_PI/4*3 };
	*/
	gViewer.identify();
	/*
	gViewer.getOrigin().set( posx, VIEWER_Y, -posy );
	for( int i = 0; i < 4; ++i ) {
		int px = posx+XOFF[i];
		int py = posy+YOFF[i];
		//if( gmap.isBlood( gmap.getCell(px,py).type ) ) {
		if( !level.collideSphere( SVector3(px,VIEWER_Y,-py), VIEWER_R ) ) {
			D3DXMatrixRotationY( &gViewer, ROT[i] );
			gViewer.getOrigin().set( px, VIEWER_Y, -py );
			break;
		}
	}
	*/
	gLastViewerValidPos = gViewer.getOrigin();

	gViewerVel.set(0,0,0);
	gViewerZVel.set(0,0,0);
}


void CDemo::initialize( IDingusAppContext& appContext )
{
	CONS << "Initializing..." << endl;


	gAppContext = &appContext;
	gProbablyInitSettings();

	CSharedTextureBundle& stb = CSharedTextureBundle::getInstance();
	CSharedSurfaceBundle& ssb = CSharedSurfaceBundle::getInstance();

	CD3DDevice& dx = CD3DDevice::getInstance();
	CONS << "Video memory seems to be: " << dx.getCaps().getVideoMemoryMB() << endl;

	G_INPUTCTX->addListener( *this );

	// --------------------------------
	// shared resources

	CIndexBufferBundle::getInstance().registerIB( RID_IB_QUADS, 8000*6, D3DFMT_INDEX16, *(new CIBFillerQuads()) );

	// --------------------------------
	// common params

	gGlobalCullMode = D3DCULL_CW;
	G_RENDERCTX->getGlobalParams().addIntRef( "iCull", &gGlobalCullMode );
	G_RENDERCTX->getGlobalParams().addFloatRef( "fTime", &gTimeParam );
	G_RENDERCTX->getGlobalParams().addVector4Ref( "vFog", gFogParam );
	G_RENDERCTX->getGlobalParams().addVector4Ref( "vFogColor", gFogColorParam );

	gDebugRenderer = new CDebugRenderer( *G_RENDERCTX, *RGET_FX("debug") );

	// --------------------------------
	// game

	CGameInfo::initialize( gServerName, gServerPort, mHwnd );

	// GUI
	gUIDlg = new CUIDialog();
	//gUIDlg->enableKeyboardInput( true );
	gUIDlg->setCallback( gUICallback );
	gUIDlg->setFont( 0, "Tahoma", 12, FW_NORMAL );
	gUIDlg->setFont( 1, "Arial", 22, FW_NORMAL );
	gUIDlg->setFont( 2, "Tahoma", 12, FW_BOLD );

	gUIDlg->addStatic( 0, "", 5, 455, 600, 22, false, &gUILabelProgress );
	gUILabelProgress->getElement(0)->setFont( 1, false, DT_LEFT | DT_VCENTER );

	gUIDlg->addImage( 0, 150, 170, 340, 170, *RGET_TEX("Logo512"), 0, 0, 512, 256, &gUIImgLogo );

	gUISettingsDlg = new CDemoSettingsDialog( appContext.getD3DEnumeration(), appContext.getD3DSettings() );
	gSettingsDlgWasActive = false;
	gUIHelpDlg = new CDemoHelpDialog();
	gHelpDlgWasActive = false;

	gSndHeart = new CSound( *RGET_SOUND(CSoundDesc("heart", true)) );
	gSndHeart->setLooping( true );
	gSndHeart->setVolume( gAppSettings.musicVolume * 0.01f );

	gLastGameUpdateTime = time_value();

	CONS << "Starting..." << endl;
}



// --------------------------------------------------------------------------
// Perform code (main loop)

bool CDemo::msgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	if( msg == net::NET_ASYNC_MESSAGE ) {
		net::onAsyncMsg( wparam, lparam );
		return true;
	}

	if( !gUIDlg )
		return false;

	bool done = false;

	if( gUISettingsDlg->getState() == CDemoSettingsDialog::STATE_ACTIVE ) {
		return gUISettingsDlg->getDialog().msgProc( hwnd, msg, wparam, lparam );
	}
	if( gUIHelpDlg->isActive() ) {
		return gUIHelpDlg->getDialog().msgProc( hwnd, msg, wparam, lparam );
	}
	if( gUIGameSetupDlg && gUIGameSetupDlg->getState() == CGameSetupDialog::STATE_ACTIVE ) {
		return gUIGameSetupDlg->getDialog().msgProc( hwnd, msg, wparam, lparam );
	}

	done = gUIDlg->msgProc( hwnd, msg, wparam, lparam );
	if( done )
		return true;

	// track mouse...
	if( msg == WM_LBUTTONDOWN ) {
		CEntityManager& entities = CGameInfo::getInstance().getEntities();
		if( &entities ) {
			entities.onMouseClick();
		}
	}
	if( msg == WM_MOUSEMOVE ) {
		CD3DDevice& dx = CD3DDevice::getInstance();
		gMouseX = (float(LOWORD(lparam)) / dx.getBackBufferWidth()) * 2 - 1;
		gMouseY = (float(HIWORD(lparam)) / dx.getBackBufferHeight()) * 2 - 1;
	}
	return false;
}

const float	TIME_SPD_NORMAL = 1.5f; // 1.5f
const float TIME_SPD_FAST = 10.0f;


static void gTryPositionViewer( const SVector3& newPos, const SVector3& finalPos )
{
	// if we're in megamap mode - allow it!
	if( !gAppSettings.followMode ) {
		gViewer.getOrigin() = newPos;
		return;
	}

	// else, if the final needed position is very far away - allow it
	SVector3 toFinal = gViewer.getOrigin() - finalPos;
	const float FAR_AWAY = 20.0f;
	if( toFinal.lengthSq() > FAR_AWAY * FAR_AWAY ) {
		gViewer.getOrigin() = newPos;
		return;
	}

	// else, check the new position for collisions
	const CLevelMesh& level = CGameInfo::getInstance().getLevelMesh();
	SVector3 targetPos = newPos;
	level.fitSphere( targetPos, VIEWER_R );
	gViewer.getOrigin() = targetPos;
}

void CDemo::onInputEvent( const CInputEvent& event )
{
	static bool ctrlPressed = false;

	float dt = CSystemTimer::getInstance().getDeltaTimeS();
	float dmove = 8.0f * dt;
	float dturn = 2.0f * dt;

	if( !gAppSettings.followMode ) {
		dmove = (10.0f + gAppSettings.megaZoom * 0.15f) * dt;
		dturn = 3.0f * dt;
	}

	//float gameTime = CGameInfo::getInstance().getTime(); // TBD

	if( event.getType() == CKeyEvent::EVENT_TYPE ) {
		const CKeyEvent& ke = (const CKeyEvent&)event;
		SMatrix4x4 mr;
		switch( ke.getKeyCode() ) {
		case DIK_LCONTROL:
		case DIK_RCONTROL:
			ctrlPressed = (ke.getMode() != CKeyEvent::KEY_RELEASED);
			break;
		case DIK_9:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED )
				gShowStats = !gShowStats;
			break;
		//case DIK_SPACE:
		//	if( ke.getMode() == CKeyEvent::KEY_PRESSED )
		//		gPlayMode = !gPlayMode;
		//	break;
		case DIK_LEFT:
			if( ctrlPressed ) {
				SVector3 newPos = gViewer.getOrigin() - gViewer.getAxisX() * dmove;
				gTryPositionViewer( newPos, newPos );
			} else {
				D3DXMatrixRotationY( &mr, -dturn );
				gViewer = mr * gViewer;
			}
			break;
		case DIK_RIGHT:
			if( ctrlPressed ) {
				SVector3 newPos = gViewer.getOrigin() + gViewer.getAxisX() * dmove;
				gTryPositionViewer( newPos, newPos );
			} else {
				D3DXMatrixRotationY( &mr, +dturn );
				gViewer = mr * gViewer;
			}
			break;
		case DIK_UP:
			{
				SVector3 newPos = gViewer.getOrigin() + gViewer.getAxisZ() * dmove;
				gTryPositionViewer( newPos, newPos );
			}
			break;
		case DIK_DOWN:
			{
				SVector3 newPos = gViewer.getOrigin() - gViewer.getAxisZ() * dmove;
				gTryPositionViewer( newPos, newPos );
			}
			break;
		/* TBD
		case DIK_COMMA:
			gameTime -= TURNS_PER_SEC * TIME_SPD_NORMAL * dt;
			gSetPlayMode( false );
			break;
		case DIK_PERIOD:
			gameTime += TURNS_PER_SEC * TIME_SPD_NORMAL * dt;
			gSetPlayMode( false );
			break; */
		case DIK_PRIOR:
			if( !gAppSettings.followMode ) {
				gAppSettings.megaZoom += dt * 40;
				gAppSettings.megaZoom = clamp( gAppSettings.megaZoom, MEGAZOOM_MIN, MEGAZOOM_MAX );
				gUISliderZoom->setValue( gAppSettings.megaZoom * 10 );
			}
			break;
		case DIK_NEXT:
			if( !gAppSettings.followMode ) {
				gAppSettings.megaZoom -= dt * 40;
				gAppSettings.megaZoom = clamp( gAppSettings.megaZoom, MEGAZOOM_MIN, MEGAZOOM_MAX );
				gUISliderZoom->setValue( gAppSettings.megaZoom * 10 );
			}
			break;
		case DIK_HOME:
			if( !gAppSettings.followMode ) {
				gAppSettings.megaTilt -= dt * 30;
				gAppSettings.megaTilt = clamp( gAppSettings.megaTilt, MEGATILT_MIN, MEGATILT_MAX );
				gUISliderTilt->setValue( gAppSettings.megaTilt );
			}
			break;
		case DIK_END:
			if( !gAppSettings.followMode ) {
				gAppSettings.megaTilt += dt * 30;
				gAppSettings.megaTilt = clamp( gAppSettings.megaTilt, MEGATILT_MIN, MEGATILT_MAX );
				gUISliderTilt->setValue( gAppSettings.megaTilt );
			}
			break;
		}
	}

	/*
	TBD
	if( gameTime < 0 )
		gameTime = 0;
	if( gameTime > CGameInfo::getInstance().getReplay().getGameTurnCount()-1 )
		gameTime = CGameInfo::getInstance().getReplay().getGameTurnCount()-1;
	gUISliderTime->setValue( gameTime );
	CGameInfo::getInstance().setTime( gameTime );
	*/
}

void CDemo::onInputStage()
{
}


static void gRenderMinimap()
{
	CD3DDevice& dx = CD3DDevice::getInstance();
	CUIResourceManager& resmgr = CUIResourceManager::getInstance();

	const float MINI_ZOOM = 160.0f;

	SMatrix4x4& mm = gCameraMiniMap.mWorldMat;
	mm = gViewer;
	SMatrix4x4 mr;
	D3DXMatrixRotationX( &mr, D3DX_PI/2 );
	mm = mr * mm;
	mm.getOrigin() -= mm.getAxisZ() * MINI_ZOOM;

	HRESULT hr;

	D3DVIEWPORT9 viewp;
	viewp.X = resmgr.xToBB( 480+3, dx.getBackBufferWidth() );
	viewp.Y = resmgr.yToBB( 14+3, dx.getBackBufferHeight() );
	viewp.Width = resmgr.xToBB( 640-3, dx.getBackBufferWidth() ) - viewp.X;
	viewp.Height = resmgr.yToBB( 14+120-3, dx.getBackBufferHeight() ) - viewp.Y;
	viewp.MinZ = 0.0f; viewp.MaxZ = 1.0f;
	hr = dx.getDevice().SetViewport( &viewp );
	assert( SUCCEEDED(hr) );

	const float camnear = MINI_ZOOM-20.0f;
	const float camfar = MINI_ZOOM+20.0f;
	const float camfov = D3DX_PI/4;
	gCameraMiniMap.setProjectionParams( camfov, (float)viewp.Width / viewp.Height, camnear, camfar );
	gCameraMiniMap.setOntoRenderContext();

	dx.clearTargets( false, true, false, 0xFF201010, 1.0f, 0L );
	dx.sceneBegin();
	G_RENDERCTX->applyGlobalEffect();
	CGameInfo::getInstance().getLevelMesh().renderMinimap();
	CGameInfo::getInstance().getEntities().renderMinimap();
	G_RENDERCTX->perform();
	dx.sceneEnd();

	viewp.X = 0;
	viewp.Y = 0;
	viewp.Width = dx.getBackBufferWidth();
	viewp.Height = dx.getBackBufferHeight();
	viewp.MinZ = 0.0f; viewp.MaxZ = 1.0f;
	hr = dx.getDevice().SetViewport( &viewp );
	assert( SUCCEEDED(hr) );
}

static const char* UIST_TYPENAMES[ENTITYCOUNT] = {
	"Needle",
	"Explorer",
	"Collector",
	"AI",
	"Container",
	"NeuroCtrl",
	"Blocker",
};
static const char* UIST_OWNERNAMES[G_MAX_PLAYERS] = {
	"CO",
	"P1",
	"P2",
};

static void gUpdateSelEntityStats()
{
	char buf[200];
	static int oldSelID = -1;
	static int oldGameTurn = -1;

	CGameInfo& gi = CGameInfo::getInstance();
	const CGameState& state = gi.getState();
	const CEntityManager& entities = gi.getEntities();

	bool wasSelected = gUIRollEStats->isExpanded();
	int selID = entities.getSelectedEntityID();
	const CActorEntity* selEnt = entities.getActorEntityByID( selID );
	bool isSelected = (selEnt != NULL);

	if( !isSelected ) {
		gUIRollEStats->setEnabled( false );
		gUIRollEStats->setExpanded( false );
	} else {
		if( oldSelID != selID )
			gShowEntityStats = true;
		gUIRollEStats->setEnabled( true );
		gUIRollEStats->setExpanded( gShowEntityStats );

		const CGameEntity& ge = selEnt->getGameEntity();

		// if different - update persistent state
		if( oldSelID != selID ) {
			// type
			gUIEStType->setText( UIST_TYPENAMES[ge.getType()] );
			// owner
			gUIEStOwner->setText( UIST_OWNERNAMES[ge.getOwner()] );
		}
		// if different turn - update variable state
		if( oldGameTurn != state.getTurn() ) {
			const CGameEntity::SState& st = ge.getState();
			// health
			int maxHP = ge.getMaxHealth();
			int hp = st.health;
			sprintf( buf, "%i / %i", hp, maxHP );
			gUIEStHealth->setText( buf );
			// AZN
			// TBD: include others as needed (container, neuroctrl)
			if( ge.getType() == ENTITY_COLLECTOR || ge.getType() == ENTITY_NEEDLE ) {
				sprintf( buf, "%i", st.stock );
				gUIEStAZN->setText( buf );
			} else {
				gUIEStAZN->setText( "n/a" );
			}
		}
	}
	oldSelID = selID;
}


#include <time.h>
static clock_t gTimeRec;

static inline void BEGIN_T() {
	gTimeRec = clock();
}
static inline void END_T( const char* op ) {
	clock_t t = clock();
	float sec = float(t-gTimeRec) / CLOCKS_PER_SEC;
	CConsole::getChannel("time") << op << " " << sec << endl;
}



/**
 *  Main loop code.
 */
void CDemo::perform()
{
	char buf[100];

	time_value tmv = CSystemTimer::getInstance().getTime();
	double t = tmv.tosec();
	float dt = CSystemTimer::getInstance().getDeltaTimeS();
	gTimeParam = float(t);

	CDynamicVBManager::getInstance().discard();

	CD3DDevice& dx = CD3DDevice::getInstance();
	gScreenFixUVs.set( 0.5f/dx.getBackBufferWidth(), 0.5f/dx.getBackBufferHeight(), 0.0f, 0.0f );

	CGameInfo& gi = CGameInfo::getInstance();
	const CGameDesc& desc = gi.getGameDesc();
	CGameState& state = gi.getState();
	const CGameMap& gmap = desc.getMap();

	//
	// should we still perform some initialization steps?

	if( !gInitGuiDone ) {
		// perform multi-step initialization...
		const char* initStepName = NULL;
		if( !gInitMainStarted ) {

			BEGIN_T();
			
			initStepName = gi.initBegin();
			gInitMainStarted = true;
		} else if( !gInitMainDone ) {
			initStepName = gi.initStep();
			if( !initStepName ) {
				initStepName = "Initializing GUI...";
				gInitMainDone = true;
			}
		} else {
			gInitiallyPlaceViewer();
			gSetupGUI();
			gInitGuiDone = true;

			END_T( "initialization" );
			
			return;
		}
		// check for init errors...
		if( !gErrorMsg.empty() ) {
			CONS << "Fatal error: " << gErrorMsg << endl;
			gFinished = true;
			return;
		}
		if( initStepName ) {
			CONS << "Init step: " << initStepName << endl;
		}
		// render progress
		assert( initStepName );
		gUILabelProgress->setText( initStepName );
		dx.clearTargets( true, true, false, 0xFF000000, 1.0f, 0L );
		dx.sceneBegin();
		G_RENDERCTX->applyGlobalEffect();
		gUIDlg->onRender( dt );
		dx.sceneEnd();
		return;
	}

	//
	// check if settings dialog just was closed

	if( gSettingsDlgWasActive && gUISettingsDlg->getState() != CDemoSettingsDialog::STATE_ACTIVE ) {
		gSettingsDlgWasActive = false;
		gUIDlg->getCheckBox( GID_CHK_OPTIONS )->setChecked( false );
		if( gUISettingsDlg->getState() == CDemoSettingsDialog::STATE_OK ) {
			// apply settings, return
			if( gAppSettings.gfxDetail != gUISettingsDlg->getAppSettings().gfxDetail ) {
				gi.getLevelMesh().updateDetailLevel( gUISettingsDlg->getAppSettings().gfxDetail );
			}
			gAppSettings = gUISettingsDlg->getAppSettings();
			CD3DSettings d3dset;
			gUISettingsDlg->getFinalSettings( d3dset );
			// don't apply d3d settings if they didn't change
			if( d3dset != gAppContext->getD3DSettings() )
				gAppContext->applyD3DSettings( d3dset );
			return;
		}
	} else if( gSettingsDlgWasActive ) {
		// if the dialog is active, but the d3d settings just changed (eg. alt-tab),
		// hide it and re-show it
		if( gSettingsAtDlgStart != gAppContext->getD3DSettings() ) {
			gUISettingsDlg->hideDialog();
			gSettingsAtDlgStart = gAppContext->getD3DSettings();
			gUISettingsDlg->showDialog( gSettingsAtDlgStart );
		}
	}

	//
	// check if help dialog just was closed

	if( gHelpDlgWasActive && !gUIHelpDlg->isActive() ) {
		gHelpDlgWasActive = false;
		gUIDlg->getCheckBox( GID_CHK_HELP )->setChecked( false );
	}


	bool gameSetupActive = (gUIGameSetupDlg->getState() == CGameSetupDialog::STATE_ACTIVE);
	bool insideView = gAppSettings.followMode && !gameSetupActive;

	//
	// perform input, audio etc.

	G_INPUTCTX->perform();

	G_AUDIOCTX->beginScene( CSystemTimer::getInstance().getTime() );
	G_AUDIOCTX->updateListener();

	//
	// update game if it's started

	if( !gameSetupActive ) {
		if( tmv - gLastGameUpdateTime >= time_value::fromsec(desc.getTurnDT()*0.5f) ) {
			// TBD: commands to dll
			net::updateGame( 0, 0, 0, state );
			gLastGameUpdateTime = tmv;
		}
	}


	//
	// control time

	// TBD
	/*
	float gameTime = gi.getTime();
	float oldGameTime = gameTime;
	if( gUIBtnTimeRew->isPressed() )
		gameTime -= TURNS_PER_SEC * TIME_SPD_FAST * dt;
	if( gUIBtnTimeFfwd->isPressed() )
		gameTime += TURNS_PER_SEC * TIME_SPD_FAST * dt;
	if( gameTime != oldGameTime )
		gSetPlayMode( false );
	if( gPlayMode )
		gameTime += TURNS_PER_SEC * TIME_SPD_NORMAL * dt;
	if( gameTime < 0 )
		gameTime = 0;
	if( gameTime > replay.getGameTurnCount()-1 ) {
		gameTime = replay.getGameTurnCount()-1;
		gSetPlayMode( false );
	}
	gUIDlg->enableNonUserEvents( false );
	gUISliderTime->setValue( gameTime );
	gUIDlg->enableNonUserEvents( true );
	gi.setTime( gameTime );
	*/

	//
	// camera

	int selEntityID = gi.getEntities().getSelectedEntityID();
	const CActorEntity* selEntity = gi.getEntities().getActorEntityByID( selEntityID );
	bool hasSelected = (selEntity != NULL);
	if( hasSelected ) {
		SVector3 selPos = selEntity->mWorldMat.getOrigin();
		if( !gAppSettings.followMode ) {
			selPos.y = VIEWER_Y;
			SVector3 newPos = smoothCD( gViewer.getOrigin(), selPos, gViewerVel, 0.25f, dt );
			gTryPositionViewer( newPos, selPos );
		} else {
			//SVector3 selPrevPos = selEntity.samplePos( gi.getTime() - 10.0f );
			// TBD
			SVector3 selPrevPos = selPos;
			selPrevPos.y = (selPrevPos.y + VIEWER_Y)*0.5f;
			selPos.y = selPrevPos.y;
			SVector3 dirZ = selPos - selPrevPos;
			dirZ.y = 0.0f;
			const float MIN_DIST = 2.0f;
			if( dirZ.lengthSq() < MIN_DIST*MIN_DIST ) {
				dirZ = gViewer.getAxisZ();
				selPrevPos = selPrevPos - dirZ * MIN_DIST;
			}
			const float PREF_DIST = 5.0f;
			if( dirZ.lengthSq() < PREF_DIST*PREF_DIST ) {
				dirZ.normalize();
				selPrevPos = selPos - dirZ * PREF_DIST;
			} else {
				dirZ.normalize();
			}
			const float SMOOTHT = 0.5f;
			SVector3 newPos = smoothCD( gViewer.getOrigin(), selPrevPos, gViewerVel, SMOOTHT, dt );
			gTryPositionViewer( newPos, selPrevPos );
			dirZ = smoothCD( gViewer.getAxisZ(), dirZ, gViewerZVel, SMOOTHT, dt );
			if( dirZ.lengthSq() < 0.5f ) {
				dirZ = gViewer.getAxisZ();
			} else {
				gViewer.getAxisZ() = dirZ.getNormalized();
			}
			gViewer.getAxisY().set(0,1,0);
			gViewer.getAxisX() = gViewer.getAxisY().cross( gViewer.getAxisZ() );
		}
	} else {
		gViewerVel.set(0,0,0);
		gViewerZVel.set(0,0,0);
	}

	//
	// check if current viewer's position is valid

	{
		SVector3 testViewerPos = gViewer.getOrigin();
		int cx = testViewerPos.x;
		int cy = -testViewerPos.z;
		if( cx >= 0 && cy >= 0 && cx < gmap.getCellsX() && cy < gmap.getCellsY() && gmap.isBlood( gmap.getCell(cx,cy).type ) ) {
			gi.getLevelMesh().fitSphere( testViewerPos, VIEWER_R*2.0f );
			const float SMALL_FIT = 0.2f;
			if( SVector3(testViewerPos - gViewer.getOrigin()).lengthSq() < SMALL_FIT * SMALL_FIT ) {
				gLastViewerValidPos = testViewerPos;
			}
		}
	}

	SMatrix4x4& mm = gCamera.mWorldMat;
	mm = gViewer;
	float camnear, camfar, camfov, fognear, fogfar;
	if( !insideView ) {
		float tilt = gAppSettings.megaTilt;
		float zoom = gAppSettings.megaZoom;
		if( gameSetupActive ) {
			gUIGameSetupDlg->updateViewer( gViewer, tilt, zoom );
		}

		SMatrix4x4 mr;
		D3DXMatrixRotationX( &mr, D3DXToRadian( tilt ) );
		mm = mr * mm;
		mm.getOrigin() -= mm.getAxisZ() * zoom;

		camnear = (zoom - 10.0f) * 0.65f;
		camfar = (zoom + 10.0f) * 2.5f;
		camfov = D3DX_PI/3;
		fognear = (zoom + 10.0f) * 2.0f;
		fogfar = (zoom + 10.0f) * 2.3f;
		gFogColorParam.set( 0, 0, 0, 1 );
	} else {
		camnear = 0.3f;
		camfar = 60.0f;
		camfov = D3DX_PI/4;
		fognear = 20.0f;
		fogfar = camfar-1;
		gFogColorParam.set( 0.25f, 0, 0, 1 );
	}

	if( camnear < 0.1f )
		camnear = 0.1f;
	gCamera.setProjectionParams( camfov, dx.getBackBufferAspect(), camnear, camfar );
	gCamera.setOntoRenderContext();


	//
	// update entities and stats UI

	gMouseRay = gCamera.getWorldRay( gMouseX, gMouseY );
	const SVector3& eyePos = gCamera.mWorldMat.getOrigin();
	SLine3 mouseRay;
	mouseRay.pos = eyePos;
	mouseRay.vec = gMouseRay;

	float timeAlpha = (tmv - gi.getState().getTurnReceivedTime()).tosec() / desc.getTurnDT();
	if( timeAlpha >= 1.0f )
		timeAlpha = 1.0f;
	gi.getEntities().update( mouseRay, timeAlpha );
	
	// stats UI
	int nplayers = desc.getPlayerCount();
	for( int p = 0; p < nplayers; ++p ) {
		const CGameState::SPlayer& pl = state.getPlayer(p);
		SUIPlayerStats& plui = gUIPlayerStats[p];
		if( plui.score ) {
			itoa( pl.score, buf, 10 );
			plui.score->setText( buf );
		}
		if( plui.botCount ) {
			itoa( pl.botCount, buf, 10 );
			plui.botCount->setText( buf );
		}
	}

	// entity stats UI
	gUpdateSelEntityStats();
	
	// time UI
	if( gameSetupActive) {
		gUILabelTime->setVisible( false );
	} else {
		sprintf( buf, "%i", state.getTurn() );
		gUILabelTime->setText( buf );
		gUILabelTime->setVisible( true );
	}
	sprintf( buf, "fps: %.1f", dx.getStats().getFPS() );
	gUILabelFPS->setText( buf );

	// update sound listener
	G_AUDIOCTX->getListener().transform = gCamera.mWorldMat;
	G_AUDIOCTX->getListener().velocity.set(0,0,0);

	float heartVol = gAppSettings.musicVolume * 0.01f;
	if( gSndHeart->getVolume() != heartVol )
		gSndHeart->setVolume( heartVol );
	if( !gPlayMode ) {
		gSndHeart->stop();
	} else if( gSndHeart->isPlaying() ) {
		gSndHeart->update();
	} else {
		gSndHeart->start();
	}

	gFogParam.set( fognear, fogfar, 1.0f/(fogfar-fognear), 0 );


	dx.clearTargets( true, true, false, insideView ? 0xFF400000 : 0xFF000000, 1.0f, 0L );
	dx.sceneBegin();
	G_RENDERCTX->applyGlobalEffect();
	gi.getLevelMesh().render( RM_NORMAL, insideView ? (CLevelMesh::FULL) : (CLevelMesh::NOTOP) );
	gi.getPointsMesh().render( RM_NORMAL );
	gi.getEntities().render( RM_NORMAL, !insideView, insideView );
	G_RENDERCTX->perform();

	// render GUI
	if( !gameSetupActive ) {
		gUIDlg->onRender( dt );
	}

	dx.sceneEnd();

	// render minimap
	if( gAppSettings.showMinimap && !gameSetupActive )
		gRenderMinimap();

	// render GUI #2
	if( gUISettingsDlg->getState() == CDemoSettingsDialog::STATE_ACTIVE ) {
		dx.sceneBegin();
		gUISettingsDlg->getDialog().onRender( dt );
		dx.sceneEnd();
	}
	if( gUIHelpDlg->isActive() ) {
		dx.sceneBegin();
		gUIHelpDlg->getDialog().onRender( dt );
		dx.sceneEnd();
	}
	if( gUIGameSetupDlg->getState() == CGameSetupDialog::STATE_ACTIVE ) {
		dx.sceneBegin();
		gUIGameSetupDlg->getDialog().onRender( dt );
		dx.sceneEnd();
	}

	G_AUDIOCTX->endScene();
}



// --------------------------------------------------------------------------
// Cleanup


void CDemo::shutdown()
{
	gWriteD3DSettingsPref();
	
	CONS << "Shutting down..." << endl;
	CGameInfo::finalize();

	delete gSndHeart;
	delete gDebugRenderer;

	safeDelete( gUIDlg );
	safeDelete( gUISettingsDlg );
	safeDelete( gUIHelpDlg );
	safeDelete( gUIGameSetupDlg );
	CONS << "Done!" << endl;
}
