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
	float		restoreAfter;
	float		restoreDur;
};
std::vector<SFracSphParams>	fracSphParams;


struct SFracYParams {
	double		frame;
	float		y1;
	float		y2;
};
std::vector<SFracYParams>	fracYParams;



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
				float frm;
				int after, dur;
				fscanf( f, "%f %f %f %f %f %i %i\n", &frm, &ep.pos.x, &ep.pos.y, &ep.pos.z, &ep.radius, &after, &dur );
				ep.frame = frm + ANIM_FRAME_OFFSET;
				ep.restoreAfter = float(after) / ANIM_FPS;
				ep.restoreDur = float(dur) / ANIM_FPS;
				fracSphParams.push_back( ep );
			}
			break;
		case 1:
			{
				// fracture Y range event
				SFracYParams ep;
				float frm;
				fscanf( f, "%f %f %f\n", &frm, &ep.y1, &ep.y2 );
				ep.frame = frm + ANIM_FRAME_OFFSET;
				fracYParams.push_back( ep );
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
			TIntVector pieces;
			for( int j = 0; j < CFACE_COUNT; ++j ) {
				if( !walls[j] )
					continue;
				walls[j]->fracturePiecesInSphere( t, ep.pos, ep.radius, pieces, ep.restoreAfter, ep.restoreDur, ep.restoreAfter<0 );
				int npc = pieces.size();
				for( int k = 0; k < npc; ++k ) {
					wall_phys::spawnPiece( lodIndex, j, pieces[k], ep.restoreAfter<0 );
				}
			}
		}
	}

	// fracture sphere events
	n = fracYParams.size();
	for( i = 0; i < n; ++i ) {
		const SFracYParams& ep = fracYParams[i];
		if( ep.frame >= lastUpdateFrame && ep.frame < frame ) {
			TIntVector pieces;
			for( int j = 0; j < CFACE_COUNT; ++j ) {
				if( !walls[j] )
					continue;
				walls[j]->fracturePiecesInYRange( t, ep.y1, ep.y2, pieces );
				int npc = pieces.size();
				for( int k = 0; k < npc; ++k ) {
					wall_phys::spawnPiece( lodIndex, j, pieces[k], true );
				}
			}
		}
	}

	lastUpdateFrame = frame;
}

