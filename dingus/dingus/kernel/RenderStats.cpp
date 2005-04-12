// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "RenderStats.h"

using namespace dingus;


void CRenderStats::reset()
{
	// just set to zero...
	memset( this, 0, sizeof(*this) );
}
