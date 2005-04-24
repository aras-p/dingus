#include "stdafx.h"
#include "FractureScenario.h"
#include "WallPieces.h"
#include "WallPhysics.h"
#include "../DemoResources.h"


namespace {

struct SFracParams {
	double		frame;
	SVector3	pos;
	float		radius;
};
std::vector<SFracParams>	fracParams;


struct SRestoreParams {
	double		frame0;
	double		frame1;
};
std::vector<SRestoreParams>	restoreParams;


double	lastUpdateFrame = -1.0;

};


static const int ANIM_FRAME_OFFSET = 950;


void gReadFractureScenario( const char* fileName )
{
	FILE* f = fopen( fileName, "rt" );

	bool done = false;
	do {
		int eventType;
		fscanf( f, "%i", &eventType );

		switch( eventType ) {
		case 0:
			{
				// fracture event
				SFracParams ep;
				int frm;
				fscanf( f, "%i %f %f %f %f\n", &frm, &ep.pos.x, &ep.pos.y, &ep.pos.z, &ep.radius );
				ep.frame = frm + ANIM_FRAME_OFFSET;
				fracParams.push_back( ep );
			}
			break;
		case 1:
			{
				// restore event
				SRestoreParams ep;
				int frm0, frm1;
				fscanf( f, "%i %i\n", &frm0, &frm1 );
				ep.frame0 = frm0 + ANIM_FRAME_OFFSET;
				ep.frame1 = frm1 + ANIM_FRAME_OFFSET;
			}
			break;
		default:
			done = true;
		}

	} while(!done);

	fclose( f );
}


void gUpdateFractureScenario( double frame, double t, CWall3D** walls )
{
	// search last-current frame interval for any fracture/restore events
	int i, n;
	n = fracParams.size();
	for( i = 0; i < n; ++i ) {
		const SFracParams& ep = fracParams[i];
		if( ep.frame >= lastUpdateFrame && ep.frame < frame ) {
			CConsole::CON_WARNING << "Event: fracture " << i << endl;
			TIntVector pieces;
			for( int j = 0; j < CFACE_COUNT; ++j ) {
				walls[j]->fracturePiecesInSphere( t, true, ep.pos, ep.radius, pieces );
				int npc = pieces.size();
				CConsole::CON_WARNING << "  fractured " << npc << " in wall " << j << endl;
				for( int k = 0; k < npc; ++k ) {
					wall_phys::spawnPiece( j, pieces[k] );
				}
			}
		}
	}

	n = restoreParams.size();
	for( i = 0; i < n; ++i ) {
		const SRestoreParams& ep = restoreParams[i];
		// TBD
		if( ep.frame1 >= lastUpdateFrame && ep.frame1 < frame ) {
			// TBD
		}
	}


	lastUpdateFrame = frame;
}

