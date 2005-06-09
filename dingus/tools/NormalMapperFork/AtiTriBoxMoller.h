//==============================================================================
// AtiRayTriMoller.h -- Triangle box intersection routine.
// Found at: http://www.ce.chalmers.se/staff/tomasm/code/tribox2.txt
//==============================================================================
// $File: //depot/3darg/Demos/Sushi/Math/AtiTriBoxMoller.h $ $Revision: #1 $ $Author: gosselin $
//==============================================================================
// (C) 2003 ATI Research, Inc., All rights reserved.
//==============================================================================

#ifndef __ATI_TRI_BOX_MOLLER__H
#define __ATI_TRI_BOX_MOLLER__H

// INCLUDES ====================================================================
#include "Types.h"

// DEFINITIONS =================================================================

// Let's you define which kind of math you want to perform
#ifdef ATI_DOUBLE_TRI_BOX_INTERSECTION_TEST
#define TBM_FLOAT float64
#define TBM_ZERO 0.0
#else
#define TBM_FLOAT float32
#define TBM_ZERO 0.0f
#endif

// FUNCTIONS ===================================================================

extern bool8 AtiTriBoxMoller (TBM_FLOAT boxcenter[3], TBM_FLOAT boxhalfsize[3],
                              TBM_FLOAT triVert0[3], TBM_FLOAT triVert1[3],
                              TBM_FLOAT triVert2[3]);

#endif
