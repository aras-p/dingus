#include "stdafx.h"
#include "GameSetupUI.h"
#include "GameInfo.h"
#include "game/GameDesc.h"

#include "game/GameState.h" // TBD: remove me

// --------------------------------------------------------------------------


CGameSetupDialog* CGameSetupDialog::mSingleInstance = 0;


const int GID_BTN_START = 7000;
const int GID_BTN_JOIN = 7010;



CGameSetupDialog::CGameSetupDialog()
:	mState( STATE_ACTIVE )
,	mLastStateQueryTime( -1 )
,	mJoinAcceptedForPlayer(-1)
,	mStartClicked(false)
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


	const CGameDesc& desc = CGameInfo::getInstance().getGameDesc();
	assert( &desc );

	// buttons
	for( int i = 1; i < desc.getPlayerCount(); ++i ) {
		// join buttons
		mDlg->addButton( GID_BTN_JOIN+i, "Join", 0, 0, 40, 16, 0, false, &mBtnJoin[i] );
	}

	mDlg->addButton( GID_BTN_START, "Start game", 330, mDlg->getHeight()-35, 68, 20, 0, false, &mBtnStart );
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
	if( evt == UIEVENT_BUTTON_CLICKED  ) {
		CGameSetupDialog* inst = mSingleInstance;
		assert( inst->mState == STATE_ACTIVE );

		if( ctrlID > GID_BTN_JOIN && ctrlID < GID_BTN_JOIN+G_MAX_PLAYERS ) {

			// issue join request
			int playerID = ctrlID - GID_BTN_JOIN;
			inst->mJoinAcceptedForPlayer = -1;
			bool accepted = net::requestJoin( playerID );
			if( accepted ) {
				inst->mJoinAcceptedForPlayer = playerID;
				ctrl->setEnabled( false );
			}
		} else if( ctrlID == GID_BTN_START ) {
			const CGameDesc& desc = CGameInfo::getInstance().getGameDesc();
			assert( &desc );
			net::receiveServerState( desc.getPlayerCount(), inst->mServerState, inst->mServerStateErrMsg, true );
			inst->mStartClicked = true;
		}
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

	char buf[200];

	rc.set( xcol-10, yline, xcol+400, yline+22 );
	dlg.imDrawText( "Game:", 1, DT_LEFT | DT_VCENTER, 0xFFffffff, rc, false );

	CGameSetupDialog& me = *mSingleInstance;
	const net::SServerState& serverState = me.mServerState;
	int nplayers = desc.getPlayerCount();

	static const char* GAMESTATES[GSTCOUNT] = {
		"server not started",
		"waiting for players",
		"game starting",
		"game running",
		"game ended",
		"ready to start",
	};

	yline += 24;
	sprintf( buf, "%s, game state: %s",
		nplayers==1 ? "Single player game" : "Two player game",
		GAMESTATES[serverState.state]
	);
	rc.set( xcol, yline, xcol+400, yline+DYLINE );
	dlg.imDrawText( buf, 0, DT_LEFT | DT_VCENTER, 0xFFffffff, rc, false );


	static const char* PLAYER_TEX_NAMES[G_MAX_PLAYERS] = {
		NULL,
		RID_TEX_PLAYER1,
		RID_TEX_PLAYER2,
	};
	for( i = 1; i < desc.getPlayerCount(); ++i ) {
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
		rc.set( xcol+40+DYLINE+5, yline, xcol+145, yline+DYLINE );
		dlg.imDrawText( desc.getPlayer(i).name.c_str(), 0, DT_LEFT|DT_VCENTER, 0xFFffffff, rc, false );

		// state
		const char* state = "From replay";
		me.mBtnJoin[i]->setEnabled( false );
		me.mBtnJoin[i]->setLocation( xcol + 150 - dlg.getX(), yline + 2 - dlg.getY() );
		if( serverState.playerRealtime[i] ) {
			state = "You can control it!";
			if( serverState.playerControlled[i] ) {
				state = (me.mJoinAcceptedForPlayer==i) ? "You will control it" : "Already controlled";
			} else {
				// enable join button
				me.mBtnJoin[i]->setEnabled( true );
			}
		}
		rc.set( xcol+200, yline, xcol+350, yline+DYLINE );
		dlg.imDrawText( state, 0, DT_LEFT|DT_VCENTER, 0xFFffffff, rc, false );
	}

	// ---- missions

	yline += 24;
	rc.set( xcol-10, yline, xcol+400, yline+22 );
	dlg.imDrawText( "Missions:", 1, DT_LEFT | DT_VCENTER, 0xFFffffff, rc, false );

	if( !desc.getMissionSummary().empty() ) {
		yline += 24;
		rc.set( xcol, yline, xcol+400, yline+14*desc.getMissionSummaryLines() );
		dlg.imDrawText( desc.getMissionSummary().c_str(), 0, DT_LEFT|DT_TOP, 0xFFffffff, rc, false );
		yline += 14*desc.getMissionSummaryLines();
	}

	for( i = 0; i < desc.getMissionCount(); ++i ) {
		yline += 4;
		rc.set( xcol, yline, xcol+400, yline+14*desc.getMission(i).descLines );
		dlg.imDrawText( desc.getMission(i).desc.c_str(), 0, DT_LEFT|DT_TOP, 0xFFffffff, rc, false );
		yline += 14*desc.getMission(i).descLines;
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
		net::receiveServerState( desc.getPlayerCount(), mServerState, mServerStateErrMsg, false );

		if( mServerState.state == GST_STARTING || mServerState.state == GST_STARTED ) {
			mBtnStart->setText( "View game" );
			if( mStartClicked )
				mState = STATE_START;
		}
	}

	int sizeX = gmap.getCellsX();
	int sizeY = gmap.getCellsY();

	viewer.identify();
	D3DXMatrixRotationY( &viewer, t * 0.02f );
	viewer.getOrigin().set( sizeX/2, 0, -sizeY/2 );

	tilt = 60.0f;
	zoom = (sizeX+sizeY)/2 * 0.5f;
}
