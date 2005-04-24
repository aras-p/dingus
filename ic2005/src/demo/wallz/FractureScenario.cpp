#include "stdafx.h"
#include "FractureScenario.h"

namespace {

struct SFracParams {
	int			frame;
	SVector3	pos;
	float		radius;
};
std::vector<SFracParams>	fracParams;


struct SRestoreParams {
	int			frame0;
	int			frame1;
};
std::vector<SRestoreParams>	restoreParams;

};


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
				fscanf( f, "%i %f %f %f %f\n", &ep.frame, &ep.pos.x, &ep.pos.y, &ep.pos.z, &ep.radius );
				fracParams.push_back( ep );
			}
			break;
		case 1:
			{
				// restore event
				SRestoreParams ep;
				fscanf( f, "%i %i\n", &ep.frame0, &ep.frame1 );
			}
			break;
		default:
			done = true;
		}

	} while(!done);

	fclose( f );
}


void gUpdateFractureScenario( float frame )
{
}

