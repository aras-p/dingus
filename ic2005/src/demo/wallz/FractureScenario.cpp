#include "stdafx.h"
#include "FractureScenario.h"
#include "WallPieces.h"
#include "WallPhysics.h"
#include "../DemoResources.h"


namespace {

struct SFracSphParams {
	double		frame;
	SVector3	pos;
	float		radius;
};
std::vector<SFracSphParams>	fracSphParams;


struct SFracYParams {
	double		frame;
	float		y1;
	float		y2;
};
std::vector<SFracYParams>	fracYParams;


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
				// fracture sphere event
				SFracSphParams ep;
				int frm;
				fscanf( f, "%i %f %f %f %f\n", &frm, &ep.pos.x, &ep.pos.y, &ep.pos.z, &ep.radius );
				ep.frame = frm + ANIM_FRAME_OFFSET;
				fracSphParams.push_back( ep );
			}
			break;
		case 1:
			{
				// fracture Y range event
				SFracYParams ep;
				int frm;
				fscanf( f, "%i %f %f\n", &frm, &ep.y1, &ep.y2 );
				ep.frame = frm + ANIM_FRAME_OFFSET;
				fracYParams.push_back( ep );
			}
			break;
		case 2:
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


void gUpdateFractureScenario( double frame, double t, int lodIndex, CWall3D** walls )
{
	// search last-current frame interval for any fracture/restore events
	int i, n;

	// fracture sphere events
	n = fracSphParams.size();
	for( i = 0; i < n; ++i ) {
		const SFracSphParams& ep = fracSphParams[i];
		if( ep.frame >= lastUpdateFrame && ep.frame < frame ) {
			CConsole::CON_WARNING << "Event: fracture sph " << i << endl;
			TIntVector pieces;
			for( int j = 0; j < CFACE_COUNT; ++j ) {
				walls[j]->fracturePiecesInSphere( t, true, ep.pos, ep.radius, pieces );
				int npc = pieces.size();
				for( int k = 0; k < npc; ++k ) {
					wall_phys::spawnPiece( lodIndex, j, pieces[k] );
				}
			}
		}
	}

	// fracture sphere events
	n = fracYParams.size();
	for( i = 0; i < n; ++i ) {
		const SFracYParams& ep = fracYParams[i];
		if( ep.frame >= lastUpdateFrame && ep.frame < frame ) {
			CConsole::CON_WARNING << "Event: fracture y " << i << endl;
			TIntVector pieces;
			for( int j = 0; j < CFACE_COUNT; ++j ) {
				if( j == CFACE_NY || j == CFACE_PY ) // just skip floor/ceiling :)
					continue;
				walls[j]->fracturePiecesInYRange( t, true, ep.y1, ep.y2, pieces );
				int npc = pieces.size();
				for( int k = 0; k < npc; ++k ) {
					wall_phys::spawnPiece( lodIndex, j, pieces[k] );
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

