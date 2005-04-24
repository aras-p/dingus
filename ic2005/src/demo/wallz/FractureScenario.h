#ifndef __FRACTURE_SCENARIO_H
#define __FRACTURE_SCENARIO_H

class CWall3D;

void gReadFractureScenario( const char* fileName );

void gUpdateFractureScenario( double frame, double t, CWall3D** walls );


#endif
