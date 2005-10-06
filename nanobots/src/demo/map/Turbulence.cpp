#include "stdafx.h"
#include "Turbulence.h"
#include "../game/GameMap.h"


// --------------------------------------------------------------------------

CTurbulence::CTurbulence( const CGameMap& gmap )
:	mMap(&gmap)
,	mCells(NULL)
{
	mCells = new SCell[gmap.getCellsX()*gmap.getCellsY()];
	memset( mCells, 0, gmap.getCellsX()*gmap.getCellsY()*sizeof(SCell) );
}

CTurbulence::~CTurbulence()
{
	safeDeleteArray( mCells );
}



