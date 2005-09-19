#include "stdafx.h"
#include "GameSetupUI.h"
#include "GameInfo.h"
#include "game/GameDesc.h"


// --------------------------------------------------------------------------


CGameSetupDialog* CGameSetupDialog::mSingleInstance = 0;



CGameSetupDialog::CGameSetupDialog()
:	mState( STATE_ACTIVE )
,	mLastStateQueryTime( -1 )
{
	assert( !mSingleInstance );
	mSingleInstance = this;

	//
	// UI
	
	mDlg = new CUIDialog();
	mDlg->enableNonUserEvents( true );
	mDlg->setCallback( dialogCallback );
	mDlg->setRenderCallback( renderCallback );
	mDlg->setBackgroundColors( 0xA0303030 );
	mDlg->setLocation( 120, 70 );
	mDlg->setSize( 420, 340 );
	mDlg->setFont( 1, "Arial", 22, 50 );
	mDlg->setFont( 2, "Arial", 14, 50 );

	//int yline;
	//const float HC = 18;
	//const float DYLINE = HC+2;

	//CUIStatic* lab;

	//const int xcol = 30;
	//yline = 10;

	/*
	mDlg->addStatic( 0,
		"Arrows\n"
		"Ctrl+Arrows\n"
		"Space\n"
		"M\n"
		"N\n"
		"S\n"
		"E\n"
		"1/2\n"
		"</>\n"
		"PgUp/PgDown\n"
		"Home/End\n"
		"Esc\n"
		, xcol+10, yline+=24, 70, HC*10, false, &lab );
	lab->getElement(0)->setFont( 0, false, DT_LEFT | DT_TOP );
	
	mDlg->addStatic( 0,
		"move & turn\n"
		"strafe\n"
		"togge play/pause\n"
		"toggle megamap/3rdperson\n"
		"toggle minimap\n"
		"toggle game stats\n"
		"toggle selected entity stats\n"
		"select player 1/2 AI\n"
		"scroll time backward/forward\n"
		"zoom in/out (megamap mode)\n"
		"tilt up/down (megamap mode)\n"
		"exit\n"
		, xcol+80, yline, 150, HC*10, false, &lab );
	lab->getElement(0)->setFont( 0, false, DT_LEFT | DT_TOP );
	
	mDlg->addStatic( 0, "Just in case: the keys are for QWERTY keyboard.", xcol, yline+=HC*8, 400, HC );
	mDlg->addStatic( 0, "Of course, most of the tasks can be controlled with the GUI!", xcol, yline+=HC-4, 400, HC );

	mDlg->addStatic( 0, "Credits", xcol-10, yline+=HC+HC/2, 200, 18, false, &lab );
	lab->getElement(0)->setFont( 1, false, DT_LEFT | DT_VCENTER );

	mDlg->addStatic( 0,
		"This 3D player (code/engine/artwork):\n"
		"        Aras Pranckevicius and Paulius Liekis [www.nesnausk.org]\n"
		"\n"
		"Everything else (ideas, SDK, game logic, testing etc.):\n"
		"        Thomas Lucchini, Richard Clark, Cyril du Bois de Maquillé, Vincent Lascaux"
		, xcol, yline+=24, 400, HC*5, false, &lab );
	lab->getElement(0)->setFont( 0, false, DT_LEFT | DT_TOP );
	*/
	

	// buttons
	mDlg->addButton( IDOK, "Close", 340, mDlg->getHeight()-35, 58, 20 );
}


CGameSetupDialog::~CGameSetupDialog()
{
	delete mDlg;
}


/*
void CGameSetupDialog::showDialog()
{
	assert( !mState != STATE_ACTIVE );
	mState = STATE_ACTIVE;
}
*/


void CALLBACK CGameSetupDialog::dialogCallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
	if( evt == UIEVENT_BUTTON_CLICKED && ctrlID == IDOK ) {
		CGameSetupDialog* inst = mSingleInstance;
		assert( inst->mState == STATE_ACTIVE );
		inst->mState = STATE_START;
	}
}


void CALLBACK CGameSetupDialog::renderCallback( CUIDialog& dlg )
{
	const CGameDesc& desc = CGameInfo::getInstance().getGameDesc();
	assert( &desc );

	int i;

	float yline;
	const float HC = 18;
	const float DYLINE = HC+2;

	const float xcol = dlg.getX() + 30;
	yline = dlg.getY() + 10;
	
	SFRect rc;

	// ---- players, join buttons etc.

	rc.set( xcol-10, yline, xcol+400, yline+22 );
	dlg.imDrawText( "Players:", 1, DT_LEFT | DT_VCENTER, 0xFFffffff, rc, false );

	int nplayers = desc.getPlayerCount();

	yline += 24;
	rc.set( xcol, yline, xcol+400, yline+DYLINE );
	dlg.imDrawText( nplayers==1 ? "Single player game" : "Two player game", 0, DT_LEFT | DT_VCENTER, 0xFFffffff, rc, false );

	const net::SServerState& serverState = mSingleInstance->mServerState;

	static const char* PLAYER_TEX_NAMES[G_MAX_PLAYERS] = {
		NULL,
		RID_TEX_PLAYER1,
		RID_TEX_PLAYER2,
	};
	for( i = 1; i < desc.getPlayerCount(); ++i ) {
		char buf[100];

		yline += DYLINE;

		// label
		sprintf( buf, "Player %i:", i );
		rc.set( xcol, yline, xcol+60, yline+DYLINE );
		dlg.imDrawText( buf, 0, DT_LEFT|DT_VCENTER, 0xFFc0c0c0, rc, false );

		// flag
		rc.set( xcol+40+1, yline+1, xcol+40+DYLINE-2, yline+DYLINE-2 );
		RECT uvrc = { 0, 0, CGameDesc::FLAG_SIZE, CGameDesc::FLAG_SIZE };
		dlg.imDrawSprite( 0xFFffffff, uvrc, RGET_S_TEX(PLAYER_TEX_NAMES[i]), rc );

		// name
		rc.set( xcol+40+DYLINE+5, yline, xcol+400, yline+DYLINE );
		dlg.imDrawText( desc.getPlayer(i).name.c_str(), 0, DT_LEFT|DT_VCENTER, 0xFFffffff, rc, false );

		// state
		const char* state = "[replay]";
		if( serverState.playerRealtime[i] ) {
			state = "Realtime, join!";
			if( serverState.playerControlled[i] ) {
				state = "Realtime, controlled";
			}
		}
		rc.set( xcol+200, yline, xcol+400, yline+DYLINE );
		dlg.imDrawText( state, 0, DT_LEFT|DT_VCENTER, 0xFFffffff, rc, false );
	}

	// ---- missions

	yline += 24;
	rc.set( xcol-10, yline, xcol+400, yline+22 );
	dlg.imDrawText( "Missions:", 1, DT_LEFT | DT_VCENTER, 0xFFffffff, rc, false );

	if( !desc.getMissionSummary().empty() ) {
		yline += 24;
		rc.set( xcol, yline, xcol+400, yline+DYLINE );
		dlg.imDrawText( desc.getMissionSummary().c_str(), 0, DT_LEFT|DT_VCENTER, 0xFFffffff, rc, false );
	}

	for( i = 0; i < desc.getMissionCount(); ++i ) {
		yline += 24;
		rc.set( xcol, yline, xcol+400, yline+DYLINE );
		dlg.imDrawText( desc.getMission(i).desc.c_str(), 0, DT_LEFT|DT_VCENTER, 0xFFffffff, rc, false );
	}
}


void CGameSetupDialog::updateViewer( SMatrix4x4& viewer, float& tilt, float& zoom )
{
	time_value currT = CSystemTimer::getInstance().getTime();
	float t = currT.tosec();

	const CGameDesc& desc = CGameInfo::getInstance().getGameDesc();
	const CGameMap& gmap = desc.getMap();

	// periodically query receive server state
	if( currT - mLastStateQueryTime > time_value::fromsec(0.5f) ) {
		mLastStateQueryTime = currT;
		net::receiveServerState( desc.getPlayerCount(), mServerState, mServerStateErrMsg );
	}

	int sizeX = gmap.getCellsX();
	int sizeY = gmap.getCellsY();

	viewer.identify();
	D3DXMatrixRotationY( &viewer, t * 0.02f );
	viewer.getOrigin().set( sizeX/2, 0, -sizeY/2 );

	tilt = 60.0f;
	zoom = (sizeX+sizeY)/2 * 0.5f;
}
