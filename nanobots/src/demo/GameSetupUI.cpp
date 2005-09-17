#include "stdafx.h"
#include "GameSetupUI.h"
#include "GameInfo.h"
#include "game/GameDesc.h"


CGameSetupUI::CGameSetupUI()
:	mFinished(false)
{
}


CGameSetupUI::~CGameSetupUI()
{
}


void CGameSetupUI::updateViewer( SMatrix4x4& viewer, float& tilt, float& zoom )
{
	float t = CSystemTimer::getInstance().getTimeS();

	const CGameMap& gmap = CGameInfo::getInstance().getGameDesc().getMap();
	int sizeX = gmap.getCellsX();
	int sizeY = gmap.getCellsY();

	viewer.identify();
	D3DXMatrixRotationY( &viewer, t * 0.02f );
	viewer.getOrigin().set( sizeX/2, 0, -sizeY/2 );

	tilt = 60.0f;
	zoom = (sizeX+sizeY)/2 * 0.5f;
}
