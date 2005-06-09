//=============================================================================
// NormalMapper.cpp -- Program that converts a low and high res model into
//                     a normal map, lots of options see main() for details.
//=============================================================================
// $File: //depot/3darg/Tools/NormalMapper/NormalMapper.cpp $ $Revision: #73 $ $Author: gosselin $
//=============================================================================
// (C) 2002 ATI Research, Inc., All rights reserved.
//=============================================================================

#ifndef ATI_MAC_OS
 #include <windows.h>
#endif
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "NmFileIO.h"
#include "TGAIO.h"
#include "ArgFileIO.h"
#include "AtiOctree.h"
#include "AtiTriBoxMoller.h"

static char* versionString =  "NormalMapper v03.02.02\n";

//#define USE_SMD_FILES

#define PACKINTOBYTE_MINUS1TO1(X)  ((BYTE)((X)*127.5+127.5))
#define UNPACKBYTE_MINUS1TO1(x)    ((((float)(x)-127.5)/127.5))
#define PACKINTOBYTE_0TO1(x)       ((BYTE)((x)*255))
#define UNPACKBYTE_0TO1(x)         (((float)(x)/255.0f))

#define PACKINTOSHORT_0TO1(x)      ((unsigned short)((x)*65535))
#define UNPACKSHORT_0TO1(x)        (((float)(x)/65535.0f))
#define PACKINTOSHORT_MINUS1TO1(X)  ((short)((X)*32767.5+32767.5))
#define UNPACKSHORT_MINUS1TO1(x)    ((((float)(x)-32767.5)/32767.5))
#define PACKINTOSHORT_SMINUS1TO1(x) ((short)((x)*32767.5))
#define UNPACKSHORT_SMINUS1TO1(x)   (((float)(x))/32767.5)

#define VEC_Subtract(a, b, c) ((c)[0] = (a)[0] - (b)[0], \
                               (c)[1] = (a)[1] - (b)[1], \
                               (c)[2] = (a)[2] - (b)[2])
#define VEC_Add(a, b, c) ((c)[0] = (a)[0] + (b)[0], \
                          (c)[1] = (a)[1] + (b)[1], \
                          (c)[2] = (a)[2] + (b)[2])
#define VEC_Cross(a, b, c) ((c)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1], \
                            (c)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2], \
                            (c)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0])
#define VEC_DotProduct(a, b) ((a)[0] * (b)[0] + \
                              (a)[1] * (b)[1] + \
                              (a)[2] * (b)[2])
#define INT_ROUND_TEXCOORD_U(X)  (int)(((X)*(float)(gWidth-1))+0.5f)
#define INT_ROUND_TEXCOORD_V(X)  (int)(((X)*(float)(gHeight-1))+0.5f)
#define INT_TEXCOORD_U(X)  (int)((X)*(float)(gWidth-1))
#define INT_TEXCOORD_V(X)  (int)((X)*(float)(gHeight-1))

// Value that's close enough to be called 0.0
#define EPSILON 1.0e-7

static const double PI = 3.1415926535897932384626433832795;

// Edge structure.
typedef struct
{
   int idx0;
   int idx1;

   // Min/max info
   int yMin;
   int yMax;
   int x;
   int x1;
   int increment;
   int numerator;
   int denominator;
} NmEdge;

// Tangent space structure.
typedef struct
{
   double m[3][9];
} NmTangentMatrix;

// Experimental pixel format.      
typedef union
{
   struct { BYTE r, g, b, a; };
   struct { BYTE v[4]; };
} NmExpPixel;

// Local print routines.
void NmErrorLog (const char *szFmt, ...);
void NmErrorLog (char *szFmt);
#define NmPrint NmErrorLog

// Width and height of the resultant texture
static int gWidth;
static int gHeight;

// Supersampling variables.
static int gNumSamples = 0;
typedef union
{
   struct { double x, y; };
   struct { double v[2]; };
} NmSample;
NmSample* gSamples = NULL;

// Rules for figuring out which normal is better.
enum
{
   NORM_RULE_CLOSEST = 0,
   NORM_RULE_BEST_CLOSEST,
   NORM_RULE_BEST_FARTHEST,
   NORM_RULE_FARTHEST,
   NORM_RULE_MIXED,
   NORM_RULE_BEST_MIXED,
   NORM_RULE_BEST,
   NORM_RULE_FRONT_FURTHEST,
   NORM_RULE_FRONT_BEST_FURTHEST,
   NORM_RULE_FRONT_CLOSEST,
   NORM_RULE_FRONT_BEST_CLOSEST
};
static int gNormalRules = NORM_RULE_BEST_CLOSEST;

// The output format
enum
{
   NORM_OUTPUT_8_8_8_TGA = 0,
   NORM_OUTPUT_8_8_8_8_TGA,
   NORM_OUTPUT_EXP_TGA,
   NORM_OUTPUT_16_16_ARG,
   NORM_OUTPUT_16_16_16_16_ARG,
   NORM_OUTPUT_10_10_10_2_ARG,
   NORM_OUTPUT_10_10_10_2_ARG_MS,
   NORM_OUTPUT_11_11_10_ARG_MS,
};
static int gOutput = NORM_OUTPUT_8_8_8_TGA;

// Octree stack.
static const int MAX_TREE_NODES = 64;
static int gMaxCells = 0;
static AtiOctreeCell** gCell = NULL;

// Occlusion constants
static double gDistanceOffset = 0.00001;
static double gZVec[3] = {0.0f, 0.0f, 1.0f};

// How to generate mip levels
enum
{
   MIP_NONE = 0,
   MIP_RECOMPUTE,
   MIP_BOX
};
static int gComputeMipLevels = MIP_NONE;

// How quiet we should be.
enum
{
   NM_VERBOSE = 0,
   NM_QUIET,
   NM_SILENT
};
static int gQuiet = NM_VERBOSE;

static bool gDisplacements = false; // Generate displacements?
static int gDispIdx;
static bool gOcclusion = false;     // Generate the occlusion term?
static int gOcclIdx = 0;
static bool gBentNormal = false;    // Save off bent normal?
static double gEpsilon = 0.0;       // Tolerance value
static double gDistance = FLT_MAX;  // Maximum distance a normal is considered
static bool gInTangentSpace = true; // Put the normals into tangent space?
static bool gExpandTexels = true;   // Expand the border texels so we don't get
                                    // crud when bi/trilinear filtering
static bool gBoxFilter = false;     // Perform a post-box filter on normal map?
static double gMaxAngle = 0.1;      // Determines cutoff angle to see if the
                                    // normal is roughly in the right direction
static int gNumDivisions = 8;       // Number of divisions for the hemisphere
                                    // of rays (controls number of rays for
                                    // occlusion and bent normal.
static int gMaxInBox = 30;          // "Maximum" number of triangles per
                                    // octree node. This is a trade off between
                                    // speed of building the tree and speed of
                                    // testing the intersections. More per bin
                                    // means faster creation, but slower testing
                                    // less means the reverse. If tree creation
                                    // can't split a bin any further there may
                                    // be more than this number in a bin. Think
                                    // of vertices with a large number of tris.
static bool gEdgeCopy = true;       // Perform computation of "snapped" edges
                                    // in an effort to reduce the harsh
                                    // transition between texture boundaries
static int gTrianglePad = 2;        // How many texels to pad around each
                                    // triangle in an effort to get all
                                    // possible relevant samples.
static bool gAddTexelCorners = false;// Add texel corners to samples
static int gDilateTexels = 10;      // How many texels to dilate.

// Some statistic counters.
static int gMaxTrisTested = 0;
static int gMaxTrisHit = 0;
static int gMaxCellsTested = 0;
static int gAOMaxTrisTested = 0;
static int gAOMaxCellsTested = 0;

//#define DEBUG_INTERSECTION
#ifdef DEBUG_INTERSECTION
static bool gDbgIntersection = false;
static const int gDbgNum = 1;
static int gDbgX[gDbgNum] = {157};
// Flip Y coordinate from Photoshop (subtract from height)
static int gDbgY[gDbgNum] = {1023 - 570};
#endif

//////////////////////////////////////////////////////////////////////////////
// Get the sample offsets for supersampling.
//////////////////////////////////////////////////////////////////////////////
static void
GetSamples (int* numSamples, NmSample** samples)
{
#ifdef __DEBUG
   if ((samples == NULL) || (numSamples == NULL))
   {
      NmPrint ("ERROR: GetSamples given NULL pointer!\n");
      exit (-1);
   }
   if ((*numSamples) < 0)
   {
      NmPrint ("ERROR: Number of samples out of range (%d)!\n", (*numSamples));
      exit (-1);
   }
#endif

   // Figure out if we need extra samples.
   int extraSamples = 1; // center texel
   if (gAddTexelCorners)
   {
      extraSamples += 4; // corners.
   }
   
   // Figure out how many samples we really need.
   (*numSamples)++;
   int num = extraSamples + ((*numSamples)*(*numSamples));
   if ((*numSamples) == 1)
   {
      num = extraSamples;
   }

   // Allocate sample memory
   (*samples) = new NmSample[num];
   if ((*samples) == NULL)
   {
      NmPrint ("ERROR: Unable to allocate sample array!\n");
      exit (-1);
   }

   // Now fill in static "extra" samples.
   (*samples)[0].x =  0.0; (*samples)[0].y =  0.0;
   if (gAddTexelCorners)
   {
      (*samples)[1].x =  0.5; (*samples)[1].y =  0.5;
      (*samples)[2].x =  0.5; (*samples)[2].y = -0.5;
      (*samples)[3].x = -0.5; (*samples)[3].y =  0.5;
      (*samples)[4].x = -0.5; (*samples)[4].y = -0.5;
   }

   // Now fill in the rest of the samples.
   int idx = extraSamples;
   if ((*numSamples) > 1)
   {
      for (int32 y = 0; y < (*numSamples); y++)
      {
         double sy = ((1.0/((double)(*numSamples) + 1.0))*((double)(y) + 1.0)) - 0.5;
         for (int32 x = 0; x < (*numSamples); x++)
         {
            double sx = ((1.0/((double)(*numSamples) + 1.0))*((double)(x) + 1.0))- 0.5;
            (*samples)[idx].x = sx;
            (*samples)[idx].y = sy;
            idx++;
         }
      }
   }

   // Paranoid check.
#ifdef _DEBUG
   if (idx != num)
   {
      NmPrint ("ERROR: We didn't fill sample array correctly (%d %d)\n", idx, num);
      exit (-1);
   }
#endif
   
   // Update number of samples
   (*numSamples) = num;
} // GetSamples

//////////////////////////////////////////////////////////////////////////////
// Normalize a vector
//////////////////////////////////////////////////////////////////////////////
static void
Normalize(double v[3])
{
   if (v == NULL)
   {
      NmPrint ("ERROR: NULL pointer passed to Normalize!\n");
      exit (-1);
   }
   double len = sqrt((v[0]*v[0])+(v[1]*v[1])+(v[2]*v[2]));
   if (len < EPSILON)
   {
      v[0] = 1.0f;
      v[1] = 0.0f;
      v[2] = 0.0f;
   }
   else
   {
      v[0] = v[0]/len;
      v[1] = v[1]/len;
      v[2] = v[2]/len;
   }
}

//////////////////////////////////////////////////////////////////////////
// Test if the new normal is a better fit than the last one.
//////////////////////////////////////////////////////////////////////////
static inline bool
IntersectionIsBetter (int rule, NmRawPointD* norm,
                      double nNorm[3], NmRawPointD* nIntersect,
                      double lNorm[3], NmRawPointD* lIntersect)
{
#ifdef _DEBUG
   if ((rule < 0) || (rule > NORM_RULE_FRONT_BEST_CLOSEST))
   {
      NmPrint ("ERROR: Unknown rule passed to IntersectionIsBetter (%d)!\n", rule);
      exit (-1);
   }
   if ((norm == NULL) || (nNorm == NULL) || (nIntersect == NULL) ||
       (lNorm == NULL) || (lIntersect == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to IntersectionIsBetter!\n");
      exit (-1);
   }
#endif
   // First see if the normal is roughly in the same direction as the low
   // resoultion normal.
   if (VEC_DotProduct (nNorm, norm->v) > gMaxAngle)
   {
      // If this is the first intersection we've found.
      bool first = false;
      if ((lNorm[0] == 0.0) && (lNorm[1] == 0.0) && (lNorm[2] == 0.0))
      {
         first = true;
         //return true;
      }
     
      // Which ruleset to use.
      switch (rule)
      {
         default:
            NmPrint ("Error: Unknown rules set (%d)!\n", rule);
            exit (-1);
               
         case NORM_RULE_CLOSEST:
            // Pick the closest
             if ( (fabs (nIntersect->x) < gDistance) &&
                  ((fabs(nIntersect->x) < fabs(lIntersect->x)) || first) )
            {
               return true;
            }
            return false;

        case NORM_RULE_FRONT_CLOSEST:
            // Pick the closest in front of the low res.
            if ( (nIntersect->x >= -gEpsilon) && 
                 (nIntersect->x < gDistance) &&
                 ((nIntersect->x < lIntersect->x) || first) )
            {
               return true;
            }
            return false;


         case NORM_RULE_BEST_CLOSEST:
            // Pick the closest, if equal pick the one closer to low res norm
            if ( (fabs(nIntersect->x) < gDistance) &&
                 ((fabs(nIntersect->x) < fabs(lIntersect->x)) || first) )
            {
               return true;
            }
            else if ( (fabs(nIntersect->x) < gDistance) &&
                      (fabs(nIntersect->x) == fabs(lIntersect->x)) &&
                      (VEC_DotProduct (nNorm, norm->v) >
                       VEC_DotProduct (lNorm, norm->v)) )
            {
               return true;
            }
            return false;

        case NORM_RULE_FRONT_BEST_CLOSEST:
            // Pick the closest in front of low res,
            // if equal pick the one closer to low res norm
            if ( (nIntersect->x >= -gEpsilon) && 
                 (nIntersect->x < gDistance) &&
                 ((nIntersect->x < lIntersect->x) || first) )
            {
               return true;
            }
            else if ( (nIntersect->x < gDistance) &&
                      (nIntersect->x == lIntersect->x) &&
                      (VEC_DotProduct (nNorm, norm->v) >
                       VEC_DotProduct (lNorm, norm->v)))
            {
               return true;
            }
            return false;

         case NORM_RULE_FARTHEST:
            // Pick the furthest
            if ( (fabs(nIntersect->x) < gDistance) &&
                 ((fabs(nIntersect->x) > fabs(lIntersect->x)) || first) )
            {
               return true;
            }
            return false;

         case NORM_RULE_FRONT_FURTHEST:
            // Pick the furthest in front of low res
            if ( (nIntersect->x >= -gEpsilon) && 
                 (nIntersect->x < gDistance) &&
                 ((nIntersect->x > lIntersect->x) || first) )
            {
               return true;
            }
            return false;

         case NORM_RULE_BEST_FARTHEST:
            // Pick the furthest, if equal pick the one closer to low res norm
            if ( (fabs (nIntersect->x) < gDistance) &&
                 ((fabs(nIntersect->x) > fabs(lIntersect->x)) || first) )
            {
               return true;
            }
            else if ( (fabs (nIntersect->x) < gDistance) && 
                      (fabs(nIntersect->x) == fabs(lIntersect->x)) &&
                      (VEC_DotProduct (nNorm, norm->v) >
                       VEC_DotProduct (lNorm, norm->v)) )
            {
               return true;
            }
            return false;

        case NORM_RULE_FRONT_BEST_FURTHEST:
            // Pick the furthest in front of low res,
            // if equal pick the one closer to low res norm
            if ( (nIntersect->x >= -gEpsilon) && 
                 (nIntersect->x < gDistance) && 
                 ((nIntersect->x > lIntersect->x) || first) )
            {
               return true;
            }
            else if ( (nIntersect->x < gDistance) &&  
                      (fabs(nIntersect->x) == fabs(lIntersect->x)) &&
                      (VEC_DotProduct (nNorm, norm->v) >
                       VEC_DotProduct (lNorm, norm->v)) )
            {
               return true;
            }
            return false;

         case NORM_RULE_MIXED:
            // See if this is an intersection behind low res face
            if (nIntersect->x <= 0)
            {
               // We prefer normals that are in front of the low res face, if
               // the last intersection was in front ignore this one.
               if ((lIntersect->x > 0) && !first)
               {
                  return false;
               }
               
               // Pick the closer of the two since this intersection is behind
               // the low res face.
               if (  (fabs (nIntersect->x) < gDistance) &&  
                     ((fabs(nIntersect->x) < fabs(lIntersect->x)) || first) )
               {
                  return true;
               }
            }
            else // Intersection in front of low res face
            {
               if ( (lIntersect->x < 0) && (fabs (nIntersect->x) < gDistance) )
               {
                  // We automatically accept this one since it's in front
                  // and the last one was behind.
                  return true;
               } 
               else if ( (fabs (nIntersect->x) < gDistance) &&  
                         ((fabs(nIntersect->x) > fabs(lIntersect->x)) ||
                          first) )
               {
                  // This one is the furthest in front so pick this one.
                  return true;
               }
            }
            return false;

         case NORM_RULE_BEST_MIXED:
            // See if this is an intersection behind low res face
            if (nIntersect->x <= 0)
            {
               // We prefer normals that are in front of the low res face, if
               // the last intersection was in front ignore this one.
               if ((lIntersect->x > 0) && !first)
               {
                  return false;
               }
               
               // Pick the closer of the two since this intersection is behind
               // the low res face. If the two intersections are equidistant
               // pick the normal that more closely matches the low res normal
               if ( (lIntersect->x < 0) && (fabs (nIntersect->x) < gDistance) )
               {
                  // We automatically accept this one since it's in front
                  // and the last one was behind.
                  return true;
               }
               else  if ( (fabs (nIntersect->x) < gDistance) &&  
                          ((fabs(nIntersect->x) < fabs(lIntersect->x)) || first) )
               {
                  return true;
               }
               else if ( (fabs (nIntersect->x) < gDistance) &&  
                         (fabs(nIntersect->x) == fabs(lIntersect->x)) &&
                         (VEC_DotProduct (nNorm, norm->v) >
                          VEC_DotProduct (lNorm, norm->v)) )
               {
                  return true;
               }
            }
            else // Intersection in front of low res face
            {
               // Pick the furthest intersection since the intersection is in
               // front of the low res face. If they are equidistant pick the
               // normal that most closely matches the low res normal.
               if ((fabs(nIntersect->x) > fabs(lIntersect->x)) || first)
               {
                  return true;
               }
               else if ( (fabs (nIntersect->x) < gDistance) &&  
                         (fabs(nIntersect->x) == fabs(lIntersect->x)) &&
                         (VEC_DotProduct (nNorm, norm->v) >
                          VEC_DotProduct (lNorm, norm->v)) )
               {
                  return true;
               }
            }
            return false;

         case NORM_RULE_BEST:
            // Pick the one closer to low res norm
            if ( (fabs (nIntersect->x) < gDistance) &&  
                 (VEC_DotProduct (nNorm, norm->v) >
                  VEC_DotProduct (lNorm, norm->v)) )
            {
               return true;
            }
            return false;
      }
   } // end if we pass angle test

   return false;
}

//////////////////////////////////////////////////////////////////////////
// See if the ray intersects the triangle.
// Drew's adaption from Real Time Rendering
// Optimized based on Moller's work in http://www.acm.org/jgt/
// http://www.ce.chalmers.se/staff/tomasm/raytri/raytri.c
//////////////////////////////////////////////////////////////////////////
static inline bool
IntersectTriangle (double *orig, double *dir,
                   float *v1, float *v2, float *v3,
                   double *t, double *u, double *v)
{
#ifdef _DEBUG
   if ((orig == NULL) || (dir == NULL) || (v1 == NULL) || (v2 == NULL) ||
       (v3 == NULL) || (t == NULL) || (u == NULL) || (v == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to IntersectTriangle!\n");
      exit (-1);
   }
#endif
   double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   double det,inv_det;

   // find vectors for two edges sharing vert0
   VEC_Subtract (v2, v1, edge1);
   VEC_Subtract (v3, v1, edge2);
   
   // begin calculating determinant - also used to calculate U parameter
   VEC_Cross (dir, edge2, pvec);

   // if determinant is near zero, ray lies in plane of triangle
   det = VEC_DotProduct (edge1, pvec);

   // calculate distance from vert0 to ray origin
   VEC_Subtract (orig, v1, tvec);
   inv_det = 1.0 / det;

   VEC_Cross (tvec, edge1, qvec);

   if (det > gEpsilon)
   {
      *u = VEC_DotProduct (tvec, pvec);
      if (*u < 0.0 || *u > det)
      {
         return false;
      }
            
      // Calculate V parameter and test bounds
      *v = VEC_DotProduct (dir, qvec);
      if (*v < 0.0 || *u + *v > det)
      {
         return false;
      }
   }
   else if (det < -gEpsilon)
   {
      // Calculate U parameter and test bounds
      *u = VEC_DotProduct (tvec, pvec);
      if (*u > 0.0 || *u < det)
      {
         return false;
      }
      
      // Calculate V parameter and test bounds
      *v = VEC_DotProduct (dir, qvec);
      if (*v > 0.0 || *u + *v < det)
      {
         return false;
      }
   }
   else
   {
      // Ray is parallell to the plane of the triangle
      return false;
   }

   *t = VEC_DotProduct (edge2, qvec) * inv_det;
   (*u) *= inv_det;
   (*v) *= inv_det;

   return true;
}

///////////////////////////////////////////////////////////////////////////////
// Determine if a ray intersects a box TRUE if it does, FALSE if not.
// Algorithm from:
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
///////////////////////////////////////////////////////////////////////////////
static inline bool
RayIntersectsBox (NmRawPointD* position, NmRawPointD* direction,
                  AtiOctBoundingBox* box)
{
#ifdef _DEBUG
   if ((position == NULL) || (direction == NULL) || (box == NULL))
   {
      return false;
   }
#endif
   // Clear near/far intersection.
   double tNear = -FLT_MAX;
   double tFar = FLT_MAX;
   
   // X plane
   // If ray is parallel to the plane
   int p = 0;
   if (direction->v[p] == 0.0)
   {
      if ((position->v[p] < box->min[p]) || (position->v[p] > box->max[p]))
      {
         return false;
      }
   }
   else
   {  // Not parallel
      double t1 = (box->min[p] - position->v[p]) / direction->v[p];
      double t2 = (box->max[p] - position->v[p]) / direction->v[p];
      
      // Swap since T1 intersection with near plane
      if (t1 > t2)
      { 
         double tmp = t1;
         t1 = t2;
         t2 = tmp;
      }

      // want largest Tnear
      if (t1 > tNear)
      {
         tNear = t1;
      }
      
      // want smallest Tfar
      if (t2 < tFar)
      {
         tFar = t2;
      }

      // Box missed!
      if (tNear > tFar)
      {
         return false;
      }
         
      // Box behind ray.
      if (tFar < 0)
      {
         return false;
      }
   } // end else not parallel

   // Y plane
   // If ray is parallel to the plane
   p = 1;
   if (direction->v[p] == 0.0)
   {
      if ((position->v[p] < box->min[p]) || (position->v[p] > box->max[p]))
      {
         return false;
      }
   }
   else
   {  // Not parallel
      double t1 = (box->min[p] - position->v[p]) / direction->v[p];
      double t2 = (box->max[p] - position->v[p]) / direction->v[p];
      
      // Swap since T1 intersection with near plane
      if (t1 > t2)
      { 
         double tmp = t1;
         t1 = t2;
         t2 = tmp;
      }

      // want largest Tnear
      if (t1 > tNear)
      {
         tNear = t1;
      }

      // want smallest Tfar
      if (t2 < tFar)
      {
         tFar = t2;
      }

      // Box missed!
      if (tNear > tFar)
      {
         return false;
      }
         
      // Box behind ray.
      if (tFar < 0)
      {
         return false;
      }
   } // end else not parallel

   // Z plane
   // If ray is parallel to the plane
   p = 2;
   if (direction->v[p] == 0.0)
   {
      if ((position->v[p] < box->min[p]) || (position->v[p] > box->max[p]))
      {
         return false;
      }
   }
   else
   {  // Not parallel
      double t1 = (box->min[p] - position->v[p]) / direction->v[p];
      double t2 = (box->max[p] - position->v[p]) / direction->v[p];
      
      // Swap since T1 intersection with near plane
      if (t1 > t2)
      { 
         double tmp = t1;
         t1 = t2;
         t2 = tmp;
      }

      // want largest Tnear
      if (t1 > tNear)
      {
         tNear = t1;
      }

      // want smallest Tfar
      if (t2 < tFar)
      {
         tFar = t2;
      }

      // Box missed!
      if (tNear > tFar)
      {
         return false;
      }
         
      // Box behind ray.
      if (tFar < 0)
      {
         return false;
      }
   } // end else not parallel

   return true;
} // end RayIntersectsBox

///////////////////////////////////////////////////////////////////////////////
// Routine to determine if the given triangle is in the box
///////////////////////////////////////////////////////////////////////////////
bool8
TriInBox (int32 itemIndex, void* itemList, AtiOctBoundingBox* bbox)
{
#ifdef _DEBUG
   if ((itemIndex < 0) || (itemList == NULL) || (bbox == NULL))
   {
      return false;
   }
#endif
   NmRawTriangle* tri = (NmRawTriangle*)itemList;
   return AtiTriBoxMoller (bbox->center, bbox->half, tri[itemIndex].vert[0].v,
                           tri[itemIndex].vert[1].v, tri[itemIndex].vert[2].v);
}

//////////////////////////////////////////////////////////////////////////////
// Get the list of rays for a given number of divisions (in a quadrant)
// http://www-2.cs.cmu.edu/~mws/rpos.html
//////////////////////////////////////////////////////////////////////////////
static bool
GetRays (int numDivisions, int* numRays, NmRawPointD** rays,
         double** rayWeight)
{
   // Check parameters
   if ((numDivisions < 1) || (numRays == NULL) || (rays == NULL))
   {
      return false;
   }

   // Allocate new storage.
   (*rays) = new NmRawPointD [numDivisions*numDivisions*4 + 5];
   if ((*rays) == NULL)
   {
      return false;
   }

   // Generate the first quadrant.
   double divp1 = (double)(numDivisions + 1);
   (*numRays) = 0;
   for (int i = 0; i < numDivisions; i++)
   {
      double z = (double)(i+1) / divp1;
      double theta = asin (z);
      for (int j = 0; j < numDivisions; j++)
      {
         double phi = ((float64)(j + 1) / divp1) * (PI/2.0);
         (*rays)[(*numRays)].x = cos (theta) * cos (phi);
         (*rays)[(*numRays)].y = cos (theta) * sin (phi);
         (*rays)[(*numRays)].z = z;
         (*numRays)++;
      }
   }

   // Second quadrant + - +
   int count = (*numRays);
   for (int i = 0; i < count; i++)
   {
      (*rays)[(*numRays)].x = (*rays)[i].x;
      (*rays)[(*numRays)].y = -(*rays)[i].y;
      (*rays)[(*numRays)].z = (*rays)[i].z;
      (*numRays)++;
   }

   // Third - + +
   for (int i = 0; i < count; i++)
   {
      (*rays)[(*numRays)].x = -(*rays)[i].x;
      (*rays)[(*numRays)].y = (*rays)[i].y;
      (*rays)[(*numRays)].z = (*rays)[i].z;
      (*numRays)++;
   }

   // Fourth - - +
   for (int i = 0; i < count; i++)
   {
      (*rays)[(*numRays)].x = -(*rays)[i].x;
      (*rays)[(*numRays)].y = -(*rays)[i].y;
      (*rays)[(*numRays)].z = (*rays)[i].z;
      (*numRays)++;
   }

   // Add each axis
   (*rays)[(*numRays)].x = 1.0f;
   (*rays)[(*numRays)].y = 0.0f;
   (*rays)[(*numRays)].z = 0.0f;
   (*numRays)++;
   (*rays)[(*numRays)].x = -1.0f;
   (*rays)[(*numRays)].y = 0.0f;
   (*rays)[(*numRays)].z = 0.0f;
   (*numRays)++;
   (*rays)[(*numRays)].x = 0.0f;
   (*rays)[(*numRays)].y = 1.0f;
   (*rays)[(*numRays)].z = 0.0f;
   (*numRays)++;
   (*rays)[(*numRays)].x = 0.0f;
   (*rays)[(*numRays)].y = -1.0f;
   (*rays)[(*numRays)].z = 0.0f;
   (*numRays)++;
   (*rays)[(*numRays)].x = 0.0f;
   (*rays)[(*numRays)].y = 0.0f;
   (*rays)[(*numRays)].z = 1.0f;
   (*numRays)++;

   // Compute ray weights if needed
   if (rayWeight != NULL)
   {
      // Allocate array
      (*rayWeight) = new double [(*numRays)];
      if ((*rayWeight) == NULL)
      {
         delete [] (*rays);
         (*rays) = NULL;
         (*numRays) = 0;
         return false;
      }
      
      // Do the computation
      double zVec[3] = {0.0f, 0.0f, 1.0f};
      for (int r = 0; r < (*numRays); r++)
      {
         (*rayWeight)[r] = VEC_DotProduct(zVec, (*rays)[r].v);
      }
   }

   // Done!
   return true;
} // GetRays

//==========================================================================
// Constructs a matrix to rotate the from vector to the to vector. Or in
// the authors own words:
//   
// "We describe an efficient (no square roots or trigonometric functions)
//  method to construct the 3×3 matrix that rotates a unit vector f into
//  another unit vector t, rotating about the axis f×t. We give experimental
//  results showing this method is faster than previously known methods.
//  An implementation in C is provided."
//
// I tweaked this to match the types an general coding style of this app.
//
// From:
//   Paper: http://www.acm.org/jgt/papers/MollerHughes99/
//   Code:  http://www.acm.org/jgt/papers/MollerHughes99/code.html
//==========================================================================
static void 
FromToRotation (double mtx[16], double from[3], double to[3])
{
   mtx[3] = 0.0f;
   mtx[7] = 0.0f;
   mtx[11] = 0.0f;
   mtx[12] = 0.0f;
   mtx[13] = 0.0f;
   mtx[14] = 0.0f;
   mtx[15] = 1.0f;

   double v[3];
   double e, h, f;

   VEC_Cross (from, to, v);
   e = VEC_DotProduct (from, to);
   f = (e < 0.0f)? -e:e;

   if (f > 1.0f - EPSILON)     /* "from" and "to"-vector almost parallel */
   {
      double u[3], v[3]; /* temporary storage vectors */
      double x[3];       /* vector most nearly orthogonal to "from" */
      double c1, c2, c3; /* coefficients for later use */
      int i, j;

      x[0] = (from[0] > 0.0f)? from[0] : -from[0];
      x[1] = (from[1] > 0.0f)? from[1] : -from[1];
      x[2] = (from[2] > 0.0f)? from[2] : -from[2];

      if (x[0] < x[1]) 
      {
         if (x[0] < x[2]) 
         {
            x[0] = 1.0f; x[1] = x[2] = 0.0f;
         }
         else 
         {
            x[2] = 1.0f; x[0] = x[1] = 0.0f;
         }
      }
      else 
      {
         if (x[1] < x[2]) 
         {
            x[1] = 1.0f; x[0] = x[2] = 0.0f;
         }
         else 
         {
            x[2] = 1.0f; x[0] = x[1] = 0.0f;
         }
      }

      u[0] = x[0] - from[0]; u[1] = x[1] - from[1]; u[2] = x[2] - from[2];
      v[0] = x[0] - to[0];   v[1] = x[1] - to[1];   v[2] = x[2] - to[2];
      
      c1 = 2.0f / VEC_DotProduct(u, u);
      c2 = 2.0f / VEC_DotProduct (v, v);
      c3 = c1 * c2  * VEC_DotProduct(u, v);
    
      for (i = 0; i < 3; i++) 
      {
         for (j = 0; j < 3; j++) 
         {
            mtx[i + j*4] =  - c1 * u[i] * u[j]
                            - c2 * v[i] * v[j]
                            + c3 * v[i] * u[j];
         }
         mtx[i + i*4] += 1.0f;
      }

   }
   else  /* the most common case, unless "from"="to", or "from"=-"to" */
   {
      /* ...otherwise use this hand optimized version (9 mults less) */
      double hvx, hvz, hvxy, hvxz, hvyz;
      h = (1.0f - e)/VEC_DotProduct(v, v);
      hvx = h * v[0];
      hvz = h * v[2];
      hvxy = hvx * v[1];
      hvxz = hvx * v[2];
      hvyz = hvz * v[1];
      mtx[0 + 0*4] = e + hvx * v[0]; 
      mtx[0 + 1*4] = hvxy - v[2];     
      mtx[0 + 2*4] = hvxz + v[1];

      mtx[1 + 0*4] = hvxy + v[2];  
      mtx[1 + 1*4] = e + h * v[1] * v[1]; 
      mtx[1 + 2*4] = hvyz - v[0];
      
      mtx[2 + 0*4] = hvxz - v[1];  
      mtx[2 + 1*4] = hvyz + v[0];     
      mtx[2 + 2*4] = e + hvz * v[2];
   }
} // FromToRotation

/////////////////////////////////////
// Get edge info
/////////////////////////////////////
static inline void
GetEdge (NmEdge* edge, NmRawTriangle* tri, int idx0, int idx1)
{
#ifdef _DEBUG
   if ((idx0 < 0) || (idx0 > 3))
   {
      NmPrint ("ERROR: idx0 out of range (GetEdge)!\n");
      exit (-1);
   }
   if ((idx1 < 0) || (idx1 > 3))
   {
      NmPrint ("ERROR: idx1 out of range (GetEdge)!\n");
      exit (-1);
   }
   if ((edge == NULL) || (tri == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to GetEdge!\n");
      exit (-1);
   }
#endif
   // Based on the Y/v coordinate fill in the structure.
   if (tri->texCoord[idx0].v <= tri->texCoord[idx1].v)
   {
      edge->idx0 = idx0;
      edge->idx1 = idx1;
      edge->yMin = INT_ROUND_TEXCOORD_V (tri->texCoord[idx0].v);
      edge->yMax = INT_ROUND_TEXCOORD_V (tri->texCoord[idx1].v);
      edge->x = INT_ROUND_TEXCOORD_U (tri->texCoord[idx0].u);
      edge->x1 = INT_ROUND_TEXCOORD_U (tri->texCoord[idx1].u);
      edge->increment = edge->denominator = edge->yMax - edge->yMin;
      edge->numerator = edge->x1 - edge->x;
   }
   else
   {
      edge->idx0 = idx1;
      edge->idx1 = idx0;
      edge->yMin = INT_ROUND_TEXCOORD_V (tri->texCoord[idx1].v);
      edge->yMax = INT_ROUND_TEXCOORD_V (tri->texCoord[idx0].v);
      edge->x = INT_ROUND_TEXCOORD_U (tri->texCoord[idx1].u);
      edge->x1 = INT_ROUND_TEXCOORD_U (tri->texCoord[idx0].u);
      edge->increment = edge->denominator = edge->yMax - edge->yMin;
      edge->numerator = edge->x1 - edge->x;
   }
} // GetEdge

///////////////////////////////////////////////////////////////////////////////
// Simple spinner display for progress indication
///////////////////////////////////////////////////////////////////////////////
static int gSpinner = 0;
static void
ShowSpinner (char* header = NULL)
{
   if (gQuiet > NM_VERBOSE)
   {
      return;
   }
   char* lineStart = header;
   if (header == NULL)
   {
      static char empty[] = "";
      lineStart = empty;
   }
   switch (gSpinner)
   {
      case 0:
         printf ("\r%s\\", lineStart);
         gSpinner++;
         break;
      case 1:
         printf ("\r%s|", lineStart);
         gSpinner++;
         break;
      case 2:
         printf ("\r%s/", lineStart);
         gSpinner++;
         break;
      default:
      case 3:
         printf ("\r%s-", lineStart);
         gSpinner = 0;
         break;
   }
}

//////////////////////////////////////////////////////////////////////////////
// Show some progress
//////////////////////////////////////////////////////////////////////////////
void
NmOctreeProgress (float progress)
{
   ShowSpinner ();
}

////////////////////////////////////////////////////////////////////
// Multiply a 3x3 matrix with a 3 space vector (assuming w = 1), ignoring
// the last row of the matrix
////////////////////////////////////////////////////////////////////
static void
ConvertFromTangentSpace (double* m, double *vec, double *result)
{
   if ((m == NULL) || (vec == NULL) || (result == NULL))
   {
      NmPrint ("ERROR: NULL pointer pased to ConvertFromTangentSpace\n");
      exit (-1); 
   }

   double tmp[3];
   tmp[0] = vec[0]*m[0] + vec[1]*m[1] + vec[2]*m[2];
   tmp[1] = vec[0]*m[3] + vec[1]*m[4] + vec[2]*m[5];
   tmp[2] = vec[0]*m[6] + vec[1]*m[7] + vec[2]*m[8];

   result[0] = tmp[0];
   result[1] = tmp[1];
   result[2] = tmp[2];
}

////////////////////////////////////////////////////////////////////
// Multiply a 3x3 matrix with a 3 space vector (assuming w = 1), ignoring
// the last row of the matrix
////////////////////////////////////////////////////////////////////
static void
ConvertToTangentSpace (double* m, double *vec, double *result)
{
   if ((m == NULL) || (vec == NULL) || (result == NULL))
   {
      NmPrint ("ERROR: NULL pointer pased to ConvertFromTangentSpace\n");
      exit (-1);
   }

   double tmp[3];
   tmp[0] = vec[0]*m[0] + vec[1]*m[3] + vec[2]*m[6];
   tmp[1] = vec[0]*m[1] + vec[1]*m[4] + vec[2]*m[7];
   tmp[2] = vec[0]*m[2] + vec[1]*m[5] + vec[2]*m[8];

   result[0] = tmp[0];
   result[1] = tmp[1];
   result[2] = tmp[2];
}

//////////////////////////////////////////////////////////////////////////
// Compute plane equation
//////////////////////////////////////////////////////////////////////////
static void
ComputePlaneEqn (float v0[3], float v1[3], float norm[3], double plane[4])
{
   if ((v0 == NULL) || (v1 == NULL) || (norm == NULL) || (plane == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to ComputePlaneEqn\n");
      exit (-1);
   }

   // Find A, B, C - normal to plane
   double v[3];
   VEC_Subtract (v1, v0, v);
   VEC_Cross (v, norm, plane);
   double tmp = sqrt (plane[0]*plane[0] + plane[1]*plane[1]+plane[2]*plane[2]);
   if (tmp < EPSILON)
   {
      plane[0] = 0;
      plane[1] = 0;
      plane[2] = 1;
   }
   else
   {
      plane[0] = plane[0]/tmp;
      plane[1] = plane[1]/tmp;
      plane[2] = plane[2]/tmp;
   }

   // Find D (A*x + B*y + C*z = D)
   plane[3] = VEC_DotProduct (plane, v0);
}

//////////////////////////////////////////////////////////////////////////
// Is the triangle "inside" the plane - if any of the three points lie on
// or above the given plane then the triangle is "inside". Given the
// plane equation above if the dot product with ABC is >= D it's in!
//////////////////////////////////////////////////////////////////////////
static inline bool
IsInside (NmRawTriangle* tri, double plane[4])
{
#ifdef _DEBUG
   if ((tri == NULL) || (plane == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to IsInside!\n");
      exit (-1);
   }
#endif
   if (VEC_DotProduct (tri->vert[0].v, plane) >= plane[3])
   {
      return true;
   }
   if (VEC_DotProduct (tri->vert[1].v, plane) >= plane[3])
   {
      return true;
   }
   if (VEC_DotProduct (tri->vert[2].v, plane) >= plane[3])
   {
      return true;
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////
// Convert to experimental pixel format. Divide by the maximum of the
// absloute value of x, y, or 1-z and store the maximum in alpha.
//////////////////////////////////////////////////////////////////////////
static void
ConvertToExpPixel (double x, double y, double z, NmExpPixel* jp)
{
   if (jp == NULL)
   {
      NmPrint ("ERROR: NULL pointer passed to ConvertToExpPixel!\n");
      exit (-1);
   }


   // Compute one minus z
   double omz = 1.0 - z;
   double ax = fabs (x);
   double ay = fabs (y);
   double aomz = fabs (omz);

   // Find max.
   double max;
   if (ax > ay)
   {
      if (ax > aomz)
      {
         max = ax;
      }
      else
      {
         max = aomz;
      }
   }
   else
   {
      if (ay > aomz)
      {
         max = ay;
      }
      else
      {
         max = aomz;
      }
   }

   // Now compute values.
   jp->r = PACKINTOBYTE_MINUS1TO1(x/max);
   jp->g = PACKINTOBYTE_MINUS1TO1(y/max);
   jp->b = PACKINTOBYTE_MINUS1TO1(omz/max);
   jp->a = PACKINTOBYTE_0TO1(max);
}

//////////////////////////////////////////////////////////////////////////
// Fetch from the normal map given the uv.
//////////////////////////////////////////////////////////////////////////
static void
Fetch (float* map, int width, int height, double u, double v, double bumpN[3])
{
   if ((map == NULL) || (bumpN == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to Fetch\n");
      exit (-1);
   }
   if (width < 1) 
   {
      NmPrint ("ERROR: Invaild width %d! (Fetch)\n", width);
      exit (-1);
   }
   if (height < 1)
   {
      NmPrint ("ERROR: Invaild height %d! (Fetch)\n", height);
      exit (-1);
   }

   // Get coordinates in 0-1 range
   if ((u < 0.0) || (u > 1.0))
   {
      u -= floor(u);
   }
   if ((v < 0.0) || (v > 1.0))
   {
      v -= floor(v);
   }

   // Now figure out the texel information for u coordinate
   double up = (double)(width-1) * u;
   double umin = floor (up);
   double umax = ceil (up);
   double ufrac = up - umin;

   // Now figure out the texel information for v coordinate
   double vp = (double)(height-1) * v;
   double vmin = floor (vp);
   double vmax = ceil (vp);
   double vfrac = vp - vmin;

   // First term umin/vmin
   int idx = (int)(vmin)*width*3 + (int)(umin)*3;
   bumpN[0] = ((1.0-ufrac)*(1.0-vfrac)*(double)(map[idx]));
   bumpN[1] = ((1.0-ufrac)*(1.0-vfrac)*(double)(map[idx+1]));
   bumpN[2] = ((1.0-ufrac)*(1.0-vfrac)*(double)(map[idx+2]));

   // Second term umax/vmin
   idx = (int)(vmin)*width*3 + (int)(umax)*3;
   bumpN[0] += (ufrac*(1.0-vfrac)*(double)(map[idx]));
   bumpN[1] += (ufrac*(1.0-vfrac)*(double)(map[idx+1]));
   bumpN[2] += (ufrac*(1.0-vfrac)*(double)(map[idx+2]));

   // Third term umin/vmax
   idx = (int)(vmax)*width*3 + (int)(umin)*3;
   bumpN[0] += ((1.0-ufrac)*vfrac*(double)(map[idx]));
   bumpN[1] += ((1.0-ufrac)*vfrac*(double)(map[idx+1]));
   bumpN[2] += ((1.0-ufrac)*vfrac*(double)(map[idx+2]));

   // Fourth term umax/vmax
   idx = (int)(vmax)*width*3 + (int)(umax)*3;
   bumpN[0] += (ufrac*vfrac*(double)(map[idx]));
   bumpN[1] += (ufrac*vfrac*(double)(map[idx+1]));
   bumpN[2] += (ufrac*vfrac*(double)(map[idx+2]));
}

//////////////////////////////////////////////////////////////////////////
// Get a pixel from the image.
//////////////////////////////////////////////////////////////////////////
static inline void 
ReadPixel (BYTE* image, int width, int off, pixel* pix, int x, int y)
{
#ifdef _DEBUG
   if ((image == NULL) || (pix == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to Readpixel!\n");
      exit (-1);
   }
#endif
   int idx = y*width*off + x*off;
   if (off > 0)
   {
      pix->red = image[idx];
   }
   if (off > 1)
   {
      pix->blue = image[idx + 1];
   }
   if (off > 2)
   {
      pix->green = image[idx + 2];
   }
}

//////////////////////////////////////////////////////////////////////////
// Reads a height field file from disk and converts it into a normal for
// use in perturbing the normals generated from the high res model
//////////////////////////////////////////////////////////////////////////
static void
GetBumpMapFromHeightMap (char* bumpName, int* bumpWidth,  int* bumpHeight,
                         float** bumpMap, float scale)
{
   if ((bumpWidth == NULL) ||(bumpHeight == NULL) || (bumpMap == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to GetBumpMapFromHeightMap!\n");
      exit (-1);
   }

   // No height field
   if (bumpName == NULL)
   {
      (*bumpWidth) = 0;
      (*bumpHeight) = 0;
      (*bumpMap) = NULL;
      return;
   }

   // First read in the heightmap.
   FILE* fp = fopen (bumpName, "rb");
   if (fp == NULL)
   {
      NmPrint ("ERROR: Unable to open %s\n", bumpName);
      exit (-1);
   }
   BYTE* image;
   int bpp;
   if (!TGAReadImage (fp, bumpWidth, bumpHeight, &bpp, &image))
   {
      NmPrint ("ERROR: Unable to read %s\n", bumpName);
      exit (-1);
   }
   fclose (fp);

   // Allocate normal image.
   (*bumpMap) = new float [(*bumpWidth)*(*bumpHeight)*3];
   if ((*bumpMap) == NULL)
   {
      NmPrint ("ERROR: Unable to allocate normal map memory!");
      exit (-1);
   }
   
   // Get offset
   int off = 0;
   switch (bpp)
   {
      case 8:
         off = 1;
         break;
      case 16:
         off = 2;
         break;
      case 24:
         off = 3;
         break;
      case 32:
         off = 4;
         break;
      default:
         NmPrint ("ERROR: Unhandled bit depth for bump map!\n");
         exit (-1);
   }

   // Sobel the image to get normals.
   float dX, dY, nX, nY, nZ, oolen;
   pixel pix;
   for (int y = 0; y < (*bumpHeight); y++)
   {
      for (int x = 0; x < (*bumpWidth); x++)
      {
         // Do Y Sobel filter
         ReadPixel (image, (*bumpWidth), off,
                    &pix, (x-1+(*bumpWidth)) % (*bumpWidth), (y+1) % (*bumpHeight));
         dY  = ((((float) pix.red) / 255.0f)*scale) * -1.0f;
            
         ReadPixel(image, (*bumpWidth), off,
                   &pix,   x   % (*bumpWidth), (y+1) % (*bumpHeight));
         dY += ((((float) pix.red) / 255.0f)*scale) * -2.0f;
            
         ReadPixel(image, (*bumpWidth), off,
                   &pix, (x+1) % (*bumpWidth), (y+1) % (*bumpHeight));
         dY += ((((float) pix.red) / 255.0f)*scale) * -1.0f;
            
         ReadPixel(image, (*bumpWidth), off,
                   &pix, (x-1+(*bumpWidth)) % (*bumpWidth), (y-1+(*bumpHeight)) % (*bumpHeight));
         dY += ((((float) pix.red) / 255.0f)*scale) *  1.0f;
            
         ReadPixel(image, (*bumpWidth), off,
                   &pix,   x   % (*bumpWidth), (y-1+(*bumpHeight)) % (*bumpHeight));
         dY += ((((float) pix.red) / 255.0f)*scale) *  2.0f;
            
         ReadPixel(image, (*bumpWidth), off,
                   &pix, (x+1) % (*bumpWidth), (y-1+(*bumpHeight)) % (*bumpHeight));
         dY += ((((float) pix.red) / 255.0f)*scale) *  1.0f;
            
         // Do X Sobel filter
         ReadPixel(image, (*bumpWidth), off,
                   &pix, (x-1+(*bumpWidth)) % (*bumpWidth), (y-1+(*bumpHeight)) % (*bumpHeight));
         dX  = ((((float) pix.red) / 255.0f)*scale) * -1.0f;
            
         ReadPixel(image, (*bumpWidth), off,
                   &pix, (x-1+(*bumpWidth)) % (*bumpWidth),   y   % (*bumpHeight));
         dX += ((((float) pix.red) / 255.0f)*scale) * -2.0f;
            
         ReadPixel(image, (*bumpWidth), off,
                   &pix, (x-1+(*bumpWidth)) % (*bumpWidth), (y+1) % (*bumpHeight));
         dX += ((((float) pix.red) / 255.0f)*scale) * -1.0f;
            
         ReadPixel(image, (*bumpWidth), off,
                   &pix, (x+1) % (*bumpWidth), (y-1+(*bumpHeight)) % (*bumpHeight));
         dX += ((((float) pix.red) / 255.0f)*scale) *  1.0f;
            
         ReadPixel(image, (*bumpWidth), off,
                   &pix, (x+1) % (*bumpWidth),   y   % (*bumpHeight));
         dX += ((((float) pix.red) / 255.0f)*scale) *  2.0f;
            
         ReadPixel(image, (*bumpWidth), off,
                   &pix, (x+1) % (*bumpWidth), (y+1) % (*bumpHeight));
         dX += ((((float) pix.red) / 255.0f)*scale) *  1.0f;
            
            
         // Cross Product of components of gradient reduces to
         nX = -dX;
         nY = -dY;
         nZ = 1;
            
         // Normalize
         oolen = 1.0f/((float) sqrt(nX*nX + nY*nY + nZ*nZ));
         nX *= oolen;
         nY *= oolen;
         nZ *= oolen;

         int idx = y*(*bumpWidth)*3 + x*3;
         (*bumpMap)[idx] = nX;
         (*bumpMap)[idx+1] = nY;
         (*bumpMap)[idx+2] = nZ;
      }
   }
} // GetBumpMapFromHeightMap

//////////////////////////////////////////////////////////////////////////
// Read in a model file
//////////////////////////////////////////////////////////////////////////
static void
ReadModel (char* name, char* type, int* numTris, NmRawTriangle** tris,
           double bbox[6], bool checkTex)
{
   // Check arguments
   if ((name == NULL) || (type == NULL) || (numTris == NULL) || (tris == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to ReadModel!\n");
      exit (-1);
   }

   // Read the file
   NmPrint ("Reading %s poly model %s\n", type, name);
   FILE* fp = fopen (name, "rb");
   if (fp == NULL)
   {
      NmPrint ("ERROR: Unable to open %s\n", name);
      exit (-1);
   }
#ifdef USE_SMD_FILES
   extern bool SMDReadTriangles (FILE* fp, int* numTris, NmRawTriangle** tris);
   if (SMDReadTriangles (fp, numTris, tris) == false)
#else
   if (NmReadTriangles (fp, numTris, tris) == false)
#endif
   {
      NmPrint ("ERROR: Unable to read %s\n", name);
      fclose (fp);
      exit (-1);
   }
   fclose (fp);
   NmPrint ("Found %d triangles in %s res model\n", (*numTris), type);

   // Figure out bounding box and check texture coordinates.
   bbox[0] = FLT_MAX; // X min
   bbox[1] = FLT_MAX; // Y min
   bbox[2] = FLT_MAX; // Z min
   bbox[3] = -FLT_MAX; // X max
   bbox[4] = -FLT_MAX; // Y max
   bbox[5] = -FLT_MAX; // Z max
   for (int i = 0; i < (*numTris); i++)
   {
      for (int j = 0; j < 3; j++)
      {
         // Find bounding box.
         for (int k = 0; k < 3; k++)
         {
            if ((*tris)[i].vert[j].v[k] < bbox[k])
            {
               bbox[k] = (*tris)[i].vert[j].v[k];
            }
            if ((*tris)[i].vert[j].v[k] > bbox[k + 3])
            {
               bbox[k + 3] = (*tris)[i].vert[j].v[k];
            }
         }

         // Check texture coordinates
         if (checkTex)
         {
            if (((*tris)[i].texCoord[j].u < 0.0) ||
                ((*tris)[i].texCoord[j].u > 1.0))
            {
               if (fabs((*tris)[i].texCoord[j].u) > EPSILON)
               {
                  NmPrint ("ERROR: Texture coordinates must lie in the 0.0 - 1.0 range for the %s res model (u: %f)!\n", type, (*tris)[i].texCoord[j].u);
                  exit (-1);
               }
               else
               {
                  (*tris)[i].texCoord[j].u = 0.0;
               }
            }
            if (((*tris)[i].texCoord[j].v < 0.0) ||
                ((*tris)[i].texCoord[j].v > 1.0))
            {
               if (fabs((*tris)[i].texCoord[j].v) > EPSILON)
               {
                  NmPrint ("ERROR: Texture coordinates must lie in the 0.0 - 1.0 range for the %s res model! (v %f)\n", type, (*tris)[i].texCoord[j].v);
                  exit (-1);
               }
               else
               {
                  (*tris)[i].texCoord[j].v = 0.0;
               }
            }
         }
      }
   }
   NmPrint ("%s poly bounding box: (%10.3f %10.3f %10.3f)\n", type, bbox[0],
           bbox[1], bbox[2]);
   NmPrint ("                       (%10.3f %10.3f %10.3f)\n", bbox[3],
            bbox[4], bbox[5]);
} // ReadModel

//////////////////////////////////////////////////////////////////////////
// Create tangent space matrices for a model
//////////////////////////////////////////////////////////////////////////
static void
GenerateTangents (NmTangentMatrix** tangentSpace, int numTris,
                  NmRawTriangle* tris)
{
   // Check arguments
   if ((tangentSpace == NULL) || (tris == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to GenerateTangents\n");
      exit (-1);
   }
   if (numTris < 1)
   {
      NmPrint ("ERROR: No triangles passed to GenerateTangents\n");
      exit (-1);
   }

   // Get tangent space.
   NmPrint ("Computing tangents\n");
   NmRawTangentSpaceD* tan = NULL;
   if (NmComputeTangentsD (numTris, tris, &tan) == false)
   {
      NmPrint ("ERROR: Unable to compute tangent space!\n");
      exit (-1);
   }
      
   // Create tangent matrices
   (*tangentSpace) = new NmTangentMatrix [numTris];
   for (int j = 0; j < numTris; j++)
   {
      for (int k = 0; k < 3; k++)
      {
         (*tangentSpace)[j].m[k][0] = tan[j].tangent[k].x;
         (*tangentSpace)[j].m[k][3] = tan[j].tangent[k].y;
         (*tangentSpace)[j].m[k][6] = tan[j].tangent[k].z;

         (*tangentSpace)[j].m[k][1] = tan[j].binormal[k].x;
         (*tangentSpace)[j].m[k][4] = tan[j].binormal[k].y;
         (*tangentSpace)[j].m[k][7] = tan[j].binormal[k].z;

         (*tangentSpace)[j].m[k][2] = tris[j].norm[k].x;
         (*tangentSpace)[j].m[k][5] = tris[j].norm[k].y;
         (*tangentSpace)[j].m[k][8] = tris[j].norm[k].z;
      }
   }
   delete [] tan;
}

//////////////////////////////////////////////////////////////////////////
// Interpolate position and normal given the Barycentric cooridnates.
//////////////////////////////////////////////////////////////////////////
static inline void
BaryInterpolate (NmRawTriangle* tri, double b1, double b2, double b3,
                 double pos[3], double nrm[3])
{
   pos[0] = (tri->vert[0].x * b1)+(tri->vert[1].x * b2)+(tri->vert[2].x * b3);
   pos[1] = (tri->vert[0].y * b1)+(tri->vert[1].y * b2)+(tri->vert[2].y * b3);
   pos[2] = (tri->vert[0].z * b1)+(tri->vert[1].z * b2)+(tri->vert[2].z * b3);
   
   nrm[0] = (tri->norm[0].x * b1)+(tri->norm[1].x * b2)+(tri->norm[2].x * b3);
   nrm[1] = (tri->norm[0].y * b1)+(tri->norm[1].y * b2)+(tri->norm[2].y * b3);
   nrm[2] = (tri->norm[0].z * b1)+(tri->norm[1].z * b2)+(tri->norm[2].z * b3);
   Normalize (nrm);
}

//////////////////////////////////////////////////////////////////////////
// Get the sorted edges from the given triangle
//////////////////////////////////////////////////////////////////////////
static void
GetSortedEdges (NmEdge edge[3], NmRawTriangle* tri)
{
   if (tri == NULL)
   {
      NmPrint ("ERROR: NULL pointer passed to GetStortedEdges!\n");
      exit (-1);
   }

   // Get the edges from the triangle
   GetEdge (&edge[0], tri, 0, 1);
   GetEdge (&edge[1], tri, 0, 2);
   GetEdge (&edge[2], tri, 1, 2);

   // Sort by minimum Y
   NmEdge tmp;
   if (edge[2].yMin < edge[1].yMin)
   {
      memcpy (&tmp, &edge[1], sizeof (NmEdge));
      memcpy (&edge[1], &edge[2], sizeof (NmEdge));
      memcpy (&edge[2], &tmp, sizeof (NmEdge));
   }
   if (edge[1].yMin < edge[0].yMin)
   {
      memcpy (&tmp, &edge[0], sizeof (NmEdge));
      memcpy (&edge[0], &edge[1], sizeof (NmEdge));
      memcpy (&edge[1], &tmp, sizeof (NmEdge));
   }
}

//////////////////////////////////////////////////////////////////////////
// Find the minimum and maximum Y value for the given set of edges.
//////////////////////////////////////////////////////////////////////////
static void
GetYMinMax (NmEdge edge[3], int* minY, int* maxY)
{
   // Make sure we have valid parameters
   if ((minY == NULL) || (maxY == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to GetYMinMax!\n");
      exit (-1);
   }

   // Find the minimum and maximum Y values for these edges.
   (*minY) = edge[0].yMin;
   (*maxY )= edge[0].yMax;
   if (edge[1].yMax > (*maxY))
   {
      (*maxY) = edge[1].yMax;
   }
   if (edge[2].yMax > (*maxY))
   {
      (*maxY) = edge[2].yMax;
   }
}

//////////////////////////////////////////////////////////////////////////
// Find the minimum and maximum X value for the given set of edges and
// a given Y value.
//////////////////////////////////////////////////////////////////////////
static void
GetXMinMax (NmEdge edge[3], int y, int* minX, int* maxX)
{
   // Make sure we have valid parameters
   if ((minX == NULL) || (maxX == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to GetXMinMax!\n");
      exit (-1);
   }

   // Using active edges find the min/max X values.
   (*minX) = 32767;
   (*maxX) = 0;
   for (int e = 0; e < 3; e++)
   {
      // See if this edge is active.
      if ((edge[e].yMin <= y) && (edge[e].yMax >= y))
      {
         // Check it's X values to see if they are min or max.
         if (edge[e].x < (*minX))
         {
            (*minX) = edge[e].x;
         }
         if (edge[e].x > (*maxX))
         {
            (*maxX) = edge[e].x;
         }
                     
         // Update X for next scanline
         edge[e].increment += edge[e].numerator;
         if (edge[e].denominator != 0)
         {
            if (edge[e].numerator < 0)
            {
               while (edge[e].increment <= 0)
               {
                  edge[e].x--;
                  edge[e].increment += edge[e].denominator;
               }
            }
            else
            {
               while (edge[e].increment > edge[e].denominator)
               {
                  edge[e].x++;
                  edge[e].increment -= edge[e].denominator;
               }
            }
         }
      } // end if edge is active
   } // end for number of edges

   // No active edges.
   if (((*minX) == 32767) && ((*maxX) == 0))
   {
#ifdef _DEBUG
      NmPrint ("Warning: No active edge.\n");
#endif
      return;
   }
}

//////////////////////////////////////////////////////////////////////////
// Normalize the given image.
//////////////////////////////////////////////////////////////////////////
static inline void
GetPerturbedNormal (NmRawTriangle* tri, double b0, double b1, double b2,
                    float* bumpMap, int bumpWidth, int bumpHeight,
                    double m[3][9], double norm[3])
{
   // Check parameters
#ifdef _DEBUG
   if ((tri == NULL) || (bumpMap == NULL) || (m == NULL) || (norm == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to GetPerturbedNormal!\n");
      exit (-1);
   }
#endif

   // Figure out texture coordinates.
   double u = (tri->texCoord[0].u * b0) +
              (tri->texCoord[1].u * b1) +
              (tri->texCoord[2].u * b2);
   double v = (tri->texCoord[0].v * b0) +
              (tri->texCoord[1].v * b1) +
              (tri->texCoord[2].v * b2);
                                 
   // Fetch from the bump map.
   double bumpN[3];
   Fetch (bumpMap, bumpWidth, bumpHeight, u, v, bumpN);

   // Interpolate tangent space on high res model
   double ts[9];
   for (int t = 0; t < 9; t++)
   {
      ts[t] = (m[0][t] * b0) + (m[1][t] * b1) + (m[2][t] * b2);
   }

   // Convert normal into object space.
   ConvertFromTangentSpace (ts, bumpN, bumpN);
   norm[0] = bumpN[0];
   norm[1] = bumpN[1];
   norm[2] = bumpN[2];
}

//////////////////////////////////////////////////////////////////////////
// Add a cell into our list.
//////////////////////////////////////////////////////////////////////////
static inline void
AddCell (AtiOctreeCell* cell, int* numCells)
{
   // See if we have enough space first.
   if ((*numCells) >= gMaxCells)
   {
      gMaxCells += MAX_TREE_NODES;
      AtiOctreeCell** tmp = new AtiOctreeCell* [gMaxCells];
      if (tmp == NULL)
      {
         NmPrint ("ERROR: Unable to allocate cell stack!\n");
         exit (-1);
      }
      memset (tmp, 0, sizeof (AtiOctreeCell*) * gMaxCells);
      memcpy (tmp, gCell, sizeof (AtiOctreeCell*) * (*numCells));
      delete [] gCell;
      gCell = tmp;
      tmp = NULL;
   }

   // Place the pointer in the list
   gCell[(*numCells)] = cell;
   (*numCells)++;
}

//////////////////////////////////////////////////////////////////////////
// Given the position, normal, and set of rays compute the bent normal and
// occlusion term.
//////////////////////////////////////////////////////////////////////////
static void
ComputeOcclusion (NmRawPointD* newPos, NmRawPointD* newNorm, int numTris,
                  NmRawTriangle* tri, AtiOctree* octree, int numRays,
                  NmRawPointD* rays, double* rayWeights,
                  NmRawPointD* bentNormal, double* occlusion)
{
#ifdef _DEBUG
   if ((newPos == NULL) || (newNorm == NULL) || (tri == NULL) ||
       (octree == NULL) || (rays == NULL) || (rayWeights == NULL) ||
       (bentNormal == NULL) || (occlusion == NULL))
   {
      NmPrint ("ERROR: Incorrect arguments passed!\n");
      exit (-1);
   }
#endif
   // Clear results. 
   // Bent normal should at minimum be the regular normal (I think).
   bentNormal->x = newNorm->x;
   bentNormal->y = newNorm->y;
   bentNormal->z = newNorm->z;
   (*occlusion) = 0.0;
   double hit = 0.0f;
   double num = 0.0f;

   // Compute offset vertex
   NmRawPointD pos;
   pos.x = newPos->x + (newNorm->x * gDistanceOffset);
   pos.y = newPos->y + (newNorm->y * gDistanceOffset);
   pos.z = newPos->z + (newNorm->z * gDistanceOffset);

   // Compute rotation matrix to match hemisphere to normal
   double rotMat[16];
   FromToRotation (rotMat, gZVec, newNorm->v);
      
   // Shoot the rays
   for (int r = 0; r < numRays; r++)
   {
      // First rotate the ray into position
      NmRawPointD oRay;
      oRay.x = rays[r].x*rotMat[0] + rays[r].y*rotMat[4] + rays[r].z*rotMat[8];
      oRay.y = rays[r].x*rotMat[1] + rays[r].y*rotMat[5] + rays[r].z*rotMat[9];
      oRay.z = rays[r].x*rotMat[2] + rays[r].y*rotMat[6] + rays[r].z*rotMat[10];
       
      // Walk the Octree to find triangle intersections.
      bool intersect = false;
      int numCells = 0;
      int cellCount = 0;
      int triCount = 0;
      AddCell (octree->m_root, &numCells);
      while ((numCells > 0) && !intersect)
      {
         // Take the cell from the list.
         cellCount++;
         numCells--;
         AtiOctreeCell* currCell = gCell[numCells];

         // See if this is a leaf node
         bool leaf = true;
         for (int c = 0; c < 8; c++)
         {
            if (currCell->m_children[c] != NULL)
            {
               leaf = false;
               break;
            }
         }

         // If we are a leaf check the triangles
         if (leaf)
         {
            // Run through the triangles seeing if the ray intersects.
            for (int t = 0; t < currCell->m_numItems; t++)
            {
               // Save off current triangle.
               NmRawTriangle* currTri = &(tri[currCell->m_item[t]]);
               triCount++;

               // See if it intersects.
               double oT, oU, oV;
               if (IntersectTriangle (pos.v, oRay.v, currTri->vert[0].v,
                                      currTri->vert[1].v, currTri->vert[2].v,
                                      &oT, &oU, &oV))
               {
                  if (oT > 0.0f)
                  {
                     intersect = true;
                     break;
                  }
               }
            } // end for t (num triangles in this cell)
         } // end if leaf
         else
         {  // Non-leaf, add the children to the list if their bounding
            // box intersects the ray.
            for (int c = 0; c < 8; c++)
            {
               if (currCell->m_children[c] != NULL)
               {
                  // Save off current child.
                  AtiOctreeCell* child = currCell->m_children[c];
                  
                  // If the ray intersects the box
                  if (RayIntersectsBox (&pos, &oRay, &child->m_boundingBox))
                  {
                     AddCell (child, &numCells);
                  } // end if the ray intersects this bounding box.
                  // else do nothing, we'll never intersect any triangles
                  // for it's children.
               } // end if we have a cell
            } // end for c (8 children)
         } // end else non-leaf node.
      } // end while cells

      // Update our running results based on if we found and intersection.
      num += rayWeights[r];
      if (!intersect)
      {
         bentNormal->x += (oRay.x * rayWeights[r]);
         bentNormal->y += (oRay.y * rayWeights[r]);
         bentNormal->z += (oRay.z * rayWeights[r]);
      }
      else
      {
         hit += rayWeights[r];
      }
      
      // Save off some stats.
      if (triCount > gAOMaxTrisTested)
      {
         gAOMaxTrisTested = triCount;
      }
      if (cellCount > gAOMaxCellsTested)
      {
         gAOMaxCellsTested = cellCount;
      }
   } // end for r (number of rays)
   
   // Normalize result
   Normalize (bentNormal->v);
   (*occlusion) = (num - hit) / num;
} // end of ComputeOcclusion

//////////////////////////////////////////////////////////////////////////
// Figure out if we have an intersection from the given point, if we do
// make sure it's the "best" one. Returns true if it found an intersection
// false otherwise. It places the normal into newNormal and position into
// newPos.
//////////////////////////////////////////////////////////////////////////
static inline bool
FindBestIntersection (NmRawPointD& pos, NmRawPointD& norm, AtiOctree* octree,
                      NmRawTriangle* highTris, NmTangentMatrix* hTangentSpace,
                      float* bumpMap, int bumpHeight, int bumpWidth,
                      double newNorm[3], double newPos[3],
                      double* displacement)
{
   // Clear outputs.
   newNorm[0] = 0.0;
   newNorm[1] = 0.0;
   newNorm[2] = 0.0;
   newPos[0] = 0.0;
   newPos[1] = 0.0;
   newPos[2] = 0.0;

   // Create negative normal
   NmRawPointD negNorm;
   negNorm.x = -norm.x;
   negNorm.y = -norm.y;
   negNorm.z = -norm.z;

   // Some stats.
   int cellCount = 0;
   int hitTriCount = 0;
   int triCount = 0;

   // Walk the octree looking for intersections.
   NmRawPointD intersect;
   NmRawPointD lastIntersect;
   int numCells = 0;
   AddCell (octree->m_root, &numCells);
   while (numCells > 0)
   {
      // Take the cell from the list.
      cellCount++;
      numCells--;
      AtiOctreeCell* currCell = gCell[numCells];

      // See if this is a leaf node
      bool leaf = true;
      for (int c = 0; c < 8; c++)
      {
         if (currCell->m_children[c] != NULL)
         {
            leaf = false;
            break;
         }
      }

      // If we are a leaf check the triangles
      if (leaf)
      {
         // Run through the triangles seeing if the ray intersects.
         for (int t = 0; t < currCell->m_numItems; t++)
         {
            // Save off current triangle.
            NmRawTriangle* hTri = &highTris[currCell->m_item[t]];
            
            // See if it intersects.
            triCount++;
            if (IntersectTriangle (pos.v, norm.v, hTri->vert[0].v,
                                   hTri->vert[1].v, hTri->vert[2].v,
                                   &intersect.x, &intersect.y,
                                   &intersect.z))
            {
               // Keep some statistics.
               hitTriCount++;

               // Figure out new normal and position
               double b0 = 1.0 - intersect.y - intersect.z;
               double np[3]; 
               double nn[3];
               BaryInterpolate (hTri, b0, intersect.y, intersect.z, np, nn);
         
               // Debug this intersection test if requested.
#ifdef DEBUG_INTERSECTION
               if (gDbgIntersection)
               {
                  NmPrint ("      High Triangle %d:\n", currCell->m_item[t]);
                  NmPrint ("         bcc:  %8.4f %8.4f %8.4f   dist: %8.4f\n",
                           b0, intersect.y, intersect.z, intersect.x);
                  NmPrint ("         nPos: %8.4f %8.4f %8.4f\n", np[0], np[1], np[2]);
                  NmPrint ("         nNrm: %8.4f %8.4f %8.4f\n", nn[0], nn[1], nn[2]);
               }
#endif

               // See if this should be the normal for the map.
               if (IntersectionIsBetter (gNormalRules, &norm,
                                         nn, &intersect,
                                         newNorm, &lastIntersect))
               {
#ifdef DEBUG_INTERSECTION
                  if (gDbgIntersection)
                  {
                     NmPrint ("      >>> Intersection is better!\n");
                  }
#endif
                  // Perturb by bump map
                  if (bumpMap != NULL)
                  {
                     GetPerturbedNormal (hTri, b0, intersect.y,
                                         intersect.z, bumpMap,
                                         bumpWidth, bumpHeight,
                                         hTangentSpace[currCell->m_item[t]].m,
                                         nn);
                  }
            
                  // Copy over values
                  memcpy (newNorm, nn, sizeof (double)*3);
                  memcpy (newPos, np, sizeof (double)*3);
                  memcpy (&lastIntersect, &intersect,
                          sizeof (NmRawPointD));
               } // end if this intersection is better
#ifdef DEBUG_INTERSECTION
               else if (gDbgIntersection)
               {
                  NmPrint ("      <<< Intersection is worse!\n");
               }
#endif
            } // end if we have an intersection
         } // end for t (num triangles in this cell)
      } // end if leaf
      else
      {  // Non-leaf, add the children to the list if their bounding
         // box intersects the ray.
         for (int c = 0; c < 8; c++)
         {
            if (currCell->m_children[c] != NULL)
            {
               // Save off current child.
               AtiOctreeCell* child = currCell->m_children[c];
                  
               // If the ray intersects the box
               if (RayIntersectsBox (&pos, &norm, &child->m_boundingBox))
               {
                  AddCell (child, &numCells);
               } // end if the ray intersects this bounding box.
               else if (RayIntersectsBox (&pos, &negNorm, &child->m_boundingBox))
               {
                  AddCell (child, &numCells);
               } // end if the ray intersects this bounding box.
               // else do nothing, we'll never intersect any triangles
               // for it's children.
            } // end if we have a cell
         } // end for c (8 children)
      } // end else non-leaf node.
   } // end while cells

   // Save off some stats.
   if (triCount > gMaxTrisTested)
   {
      gMaxTrisTested = triCount;
   }
   if (hitTriCount > gMaxTrisHit)
   {
      gMaxTrisHit = hitTriCount;
   }
   if (cellCount > gMaxCellsTested)
   {
      gMaxCellsTested = cellCount;
   }

   // Test if we found an intersection.
   if ( (newNorm[0] != 0.0) || (newNorm[1] != 0.0) || (newNorm[2] != 0.0) )
   {
      (*displacement) = lastIntersect.x;
      return true;
   }
   return false;
} // end of FindBestIntersection


//////////////////////////////////////////////////////////////////////////
// Print out the bounding box.
//////////////////////////////////////////////////////////////////////////
static void
PrintBBox (AtiOctBoundingBox* bbox)
{
   NmPrint ("min:    %7.3f %7.3f %7.3f\n", bbox->minX, bbox->minY, bbox->minZ);
   NmPrint ("max:    %7.3f %7.3f %7.3f\n", bbox->maxX, bbox->maxY, bbox->maxZ);
   NmPrint ("center: %7.3f %7.3f %7.3f\n", bbox->centerX, bbox->centerY, bbox->centerZ);
   NmPrint ("half:   %7.3f %7.3f %7.3f\n", bbox->halfX, bbox->halfY, bbox->halfZ);
}

//////////////////////////////////////////////////////////////////////////
// Print out some stats about the octree.
//////////////////////////////////////////////////////////////////////////
static void
PrintOctreeStats (AtiOctree* octree)
{
   // Bail if we don't have an octree
   if (octree == NULL)
   {
      NmPrint ("ERROR: No octree passed to PrintOctreeStats\n");
      exit (-1);
   }

   // Initialize counter.
   int totalCells = 0;
   int leafCells = 0;
   int nonLeafCells = 0;
   int numItemsInLeaves = 0;
   int minItemsInLeaves = 1000000;
   int maxItemsInLeaves = 0;

   // Walk the tree.
   int numCells = 0;
   AddCell (octree->m_root, &numCells);
   while (numCells > 0)
   {
      // Take the cell from the list.
      numCells--;
      AtiOctreeCell* currCell = gCell[numCells];

      // See if this is a leaf node
      bool leaf = true;
      for (int c = 0; c < 8; c++)
      {
         if (currCell->m_children[c] != NULL)
         {
            leaf = false;
            break;
         }
      }

      // If we are a leaf check the triangles
      if (leaf)
      {
#ifdef VERBOSE_OCTREE
         NmPrint ("-------------- leaf --------------\n");
         NmPrint ("%d triangles\n", currCell->m_numItems);
         PrintBBox (&currCell->m_boundingBox);
#endif
         leafCells++;
         totalCells++;
         numItemsInLeaves += currCell->m_numItems;
         if (currCell->m_numItems < minItemsInLeaves)
         {
            minItemsInLeaves = currCell->m_numItems;
         }
         if (currCell->m_numItems > maxItemsInLeaves)
         {
            maxItemsInLeaves = currCell->m_numItems;
         }
      } // end if leaf
      else
      {  // Non-leaf
#ifdef VERBOSE_OCTREE
         NmPrint ("************ non-leaf ************\n");
         NmPrint ("%d triangles\n", currCell->m_numItems);
         PrintBBox (&currCell->m_boundingBox);
#endif

         // Handle children
         totalCells++;
         nonLeafCells++;
         int childCount = 0;
         for (int c = 0; c < 8; c++)
         {
            if (currCell->m_children[c] != NULL)
            {
               childCount++;
               AtiOctreeCell* child = currCell->m_children[c];
               AddCell (child, &numCells);
            } // end if we have a cell
         } // end for c (8 children)
#ifdef VERBOSE_OCTREE
         NmPrint ("%d children\n", childCount);
#endif
      } // end else non-leaf node.
   } // end while we still have cells

   // Print stats.
   NmPrint ("%d Octree cells (%d leaf %d non-leaf)\n",
            totalCells, leafCells, nonLeafCells);
   NmPrint ("     %d total items (%.3f average in leaves)\n",  numItemsInLeaves,
            (float)(numItemsInLeaves)/(float)(leafCells));
   NmPrint ("     %d min in leaves  %d max in leaves\n", minItemsInLeaves,
            maxItemsInLeaves);
} // end PrintOctreeStats

//////////////////////////////////////////////////////////////////////////
// Figure out the name for the output file, we possibly need to tack on
// a number designating the mip level.
//////////////////////////////////////////////////////////////////////////
static void
GetOutputFilename (char* name, char* original, int mipCount)
{
   // Check arguments
   if ((name == NULL) || (original == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to GetOutputFilename\n");
      exit (-1);
   }

   // Copy over name
   strcpy (name, original);

   // If we are computing the mipmaps tack on an appropriate number
   if (gComputeMipLevels > 0)
   {
      char* dot = strrchr (name, '.');
      char ext[24];
      strcpy (ext, dot);
      if (dot != NULL)
      {
         sprintf (dot, "%02d%s", mipCount, ext);
      }
      else
      {
         sprintf (ext, "%02d.tga", mipCount);
         strcat (name, ext);
      }
   }
}

//////////////////////////////////////////////////////////////////////////
// Normalize the given image.
//////////////////////////////////////////////////////////////////////////
static void
ComputeNextMipLevel (int* lastWidth, int* lastHeight)
{
#ifdef _DEBUG
   if ( (lastWidth == NULL) || (lastHeight == NULL) )
   {
      NmPrint ("ERROR: Bad parameter to ComputeNextMipLevel!\n");
      exit (-1);
   }
#endif
   if (gComputeMipLevels > 0)
   {
      (*lastWidth) = gWidth;
      (*lastHeight) = gHeight;
      gWidth = gWidth / 2;
      gHeight = gHeight / 2;
      if (gWidth < 1)
      {
         if (gHeight > 1)
         {
            gWidth = 1;
         }
      }
      else if (gHeight < 1)
      {
         gHeight = 1;
      }
   }
   else
   {
      gWidth = 0;
      gHeight = 0;
   }
}

//////////////////////////////////////////////////////////////////////////
// Copy over any texels in img2 where img1 is empty
//////////////////////////////////////////////////////////////////////////
static void
CopyEdges (float* img1, float* img2, int numComponents)
{
#ifdef _DEBUG
   if ((img1 == NULL) || (img2 == NULL) || (numComponents < 1))
   {
      NmPrint ("ERROR: Bad parameter to CopyEdges\n");
      exit (-1);
   }
#endif
   NmPrint ("\rCopy Edges\n");
   for (int y = 0; y < gHeight; y++)
   {
      ShowSpinner ();
      for (int x = 0; x < gWidth; x++)
      {
         // See if this pixel is empty
         int idx = y*gWidth*numComponents + x*numComponents;
         if ( ( (fabs(img1[idx]) < EPSILON) &&
                (fabs(img1[idx + 1]) < EPSILON) &&
                (fabs(img1[idx + 2]) < EPSILON) ) &&
              ( (fabs(img2[idx]) >= EPSILON) ||
                (fabs(img2[idx + 1]) >= EPSILON) || 
                (fabs(img2[idx + 2]) >= EPSILON) ) )
         {
            img1[idx + 0] = img2[idx + 0];
            img1[idx + 1] = img2[idx + 1];
            img1[idx + 2] = img2[idx + 2];
            if (gOcclusion)
            {
               img1[idx + gOcclIdx] = img2[idx + gOcclIdx];
            }
            if (gDisplacements)
            {
               img1[idx + gDispIdx] = img2[idx + gDispIdx];
            }
         }
      }
   }
   if (gQuiet == NM_VERBOSE)
   {
      printf ("\r");
   }
}

//////////////////////////////////////////////////////////////////////////
// Normalize the given image.
//////////////////////////////////////////////////////////////////////////
static void
NormalizeImage (float* img, int numComponents)
{
#ifdef _DEBUG
   if ((img == NULL) || (numComponents < 1))
   {
      NmPrint ("ERROR: Bad parameter to NormalizeImage\n");
      exit (-1);
   }
#endif
   NmPrint ("\rRenormalize\n");
   for (int y = 0; y < gHeight; y++)
   {
      ShowSpinner ();
      for (int x = 0; x < gWidth; x++)
      {
         // See if this pixel is empty
         int idx = y*gWidth*numComponents + x*numComponents;
         if ((fabs(img[idx]) >= EPSILON) ||
             (fabs(img[idx + 1]) >= EPSILON) || 
             (fabs(img[idx + 2]) >= EPSILON))
         {
            double iNorm[3];
            iNorm[0] = img[idx + 0];
            iNorm[1] = img[idx + 1];
            iNorm[2] = img[idx + 2];
            Normalize (iNorm);
            img[idx + 0] = (float)iNorm[0];
            img[idx + 1] = (float)iNorm[1];
            img[idx + 2] = (float)iNorm[2];
            if (gOcclusion)
            {
               img[idx + gOcclIdx] = img[idx + gOcclIdx];
            }
            if (gDisplacements)
            {
               img[idx + gDispIdx] = img[idx + gDispIdx];
            }
         }
      }
   }
   if (gQuiet == NM_VERBOSE)
   {
      printf ("\r");
   }
} // NormalizeImage

//////////////////////////////////////////////////////////////////////////
// Dilate image, by expanding neighbors to filled pixels. This fixes
// issues seen when blending. Otherwise the blend would be performed
// against a zero vector. This routine swaps the buffers around, in the 
// end img will contain the output and scratch will contain the last iteration
// of the dilation.
//////////////////////////////////////////////////////////////////////////
static void
DilateImage (float** img, float** scratch, int numComponents)
{
   if ((img == NULL) || (scratch == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to DilateImage\n");
      exit (-1);
   }
   NmPrint ("\rFilling borders\n");
   for (int f = 0; f < gDilateTexels; f++)
   {
      // Loop over the old image and create a new one.
      for (int y = 0; y < gHeight; y++)
      {
         ShowSpinner ();
         for (int x = 0; x < gWidth; x++)
         {
            // See if this pixel is empty
            int idx = y*gWidth*numComponents + x*numComponents;
            if ((fabs((*img)[idx]) <= EPSILON) &&
                (fabs((*img)[idx + 1]) <= EPSILON) && 
                (fabs((*img)[idx + 2]) <= EPSILON))
            {
               // Accumulate the near filled pixels
               double nn[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
               int ncount = 0;
               int yv;
               int xv;
               int nidx;
                        
               // -Y
               yv = (y - 1)%gHeight;
               if (yv < 0) 
               {
                  yv = gHeight -1;
               }
               xv = (x)%gWidth;
               if (xv < 0) 
               {
                  xv = gWidth -1;
               }
               nidx = yv*gWidth*numComponents + xv*numComponents;
               if ((fabs((*img)[nidx]) > EPSILON) ||
                   (fabs((*img)[nidx + 1]) > EPSILON) ||
                   (fabs((*img)[nidx + 2]) > EPSILON))
               {
                  nn[0] += (*img)[nidx];
                  nn[1] += (*img)[nidx+1];
                  nn[2] += (*img)[nidx+2];
                  if (gOcclusion)
                  {
                     nn[gOcclIdx] += (*img)[nidx + gOcclIdx];
                  }
                  if (gDisplacements)
                  {
                     nn[gDispIdx] += (*img)[nidx + gDispIdx];
                  }
                  ncount++;
               }
                        
               // +Y
               yv = (y + 1)%gHeight;
               if (yv < 0) 
               {
                  yv = gHeight -1;
               }
               xv = (x)%gWidth;
               if (xv < 0) 
               {
                  xv = gWidth -1;
               }
               nidx = yv*gWidth*numComponents + xv*numComponents;
               if ((fabs((*img)[nidx]) > EPSILON) ||
                   (fabs((*img)[nidx + 1]) > EPSILON) ||
                   (fabs((*img)[nidx + 2]) > EPSILON))
               {
                  nn[0] += (*img)[nidx];
                  nn[1] += (*img)[nidx+1];
                  nn[2] += (*img)[nidx+2];
                  if (gOcclusion)
                  {
                     nn[gOcclIdx] += (*img)[nidx + gOcclIdx];
                  }
                  if (gDisplacements)
                  {
                     nn[gDispIdx] += (*img)[nidx + gDispIdx];
                  }
                  ncount++;
               }
               
               // -X
               yv = (y)%gHeight;
               if (yv < 0) 
               {
                  yv = gHeight -1;
               }
               xv = (x-1)%gWidth;
               if (xv < 0) 
               {
                  xv = gWidth -1;
               }
               nidx = yv*gWidth*numComponents + xv*numComponents;
               if ((fabs((*img)[nidx]) > EPSILON) ||
                   (fabs((*img)[nidx + 1]) > EPSILON) ||
                   (fabs((*img)[nidx + 2]) > EPSILON))
               {
                  nn[0] += (*img)[nidx];
                  nn[1] += (*img)[nidx+1];
                  nn[2] += (*img)[nidx+2];
                  if (gOcclusion)
                  {
                     nn[gOcclIdx] += (*img)[nidx + gOcclIdx];
                  }
                  if (gDisplacements)
                  {
                     nn[gDispIdx] += (*img)[nidx + gDispIdx];
                  }
                  ncount++;
               }
                        
               // -X
               yv = (y)%gHeight;
               if (yv < 0) 
               {
                  yv = gHeight -1;
               }
               xv = (x+1)%gWidth;
               if (xv < 0) 
               {
                  xv = gWidth -1;
               }
               nidx = yv*gWidth*numComponents + xv*numComponents;
               if ((fabs((*img)[nidx]) > EPSILON) ||
                   (fabs((*img)[nidx + 1]) > EPSILON) ||
                   (fabs((*img)[nidx + 2]) > EPSILON))
               {
                  nn[0] += (*img)[nidx];
                  nn[1] += (*img)[nidx+1];
                  nn[2] += (*img)[nidx+2];
                  if (gOcclusion)
                  {
                     nn[gOcclIdx] += (*img)[nidx + gOcclIdx];
                  }
                  if (gDisplacements)
                  {
                     nn[gDispIdx] += (*img)[nidx + gDispIdx];
                  }
                  ncount++;
               }
                        
               // If we found some neighbors that were filled, fill
               // this one with the average, otherwise copy
               if (ncount > 0)
               {
                  Normalize (nn);
                  (*scratch)[idx] = (float)nn[0];
                  (*scratch)[idx + 1] = (float)nn[1];
                  (*scratch)[idx + 2] = (float)nn[2];
                  if (gOcclusion)
                  {
                     (*scratch)[idx + gOcclIdx] = ((float)nn[gOcclIdx]/ncount);
                  }
                  if (gDisplacements)
                  {
                     (*scratch)[idx + gDispIdx] = ((float)nn[gDispIdx]/ncount);
                  }
               }
               else
               {
                  (*scratch)[idx] = (*img)[idx];
                  (*scratch)[idx + 1] = (*img)[idx  + 1];
                  (*scratch)[idx + 2] = (*img)[idx + 2];
                  if (gOcclusion)
                  {
                     (*scratch)[idx + gOcclIdx] = (*img)[idx + gOcclIdx];
                  }
                  if (gDisplacements)
                  {
                     (*scratch)[idx + gDispIdx] = (*img)[idx + gDispIdx];
                  }
               }
            } // end if this pixel is empty
            else
            {
               (*scratch)[idx] = (*img)[idx];
               (*scratch)[idx + 1] = (*img)[idx + 1];
               (*scratch)[idx + 2] = (*img)[idx + 2];
               if (gOcclusion)
               {
                  (*scratch)[idx + gOcclIdx] = (*img)[idx + gOcclIdx];
               }
               if (gDisplacements)
               {
                  (*scratch)[idx + gDispIdx] = (*img)[idx + gDispIdx];
               }
            }
         } // end for x
      } // end for y
      
      // Swap images
      float* i1 = (*scratch);
      (*scratch) = (*img);
      (*img) = i1;
   } // end for f
   if (gQuiet == NM_VERBOSE)
   {
      printf ("\r");
   }
} // DilateImage

//////////////////////////////////////////////////////////////////////////
// Box filter image. Last width determines if this routine reduces the
// image or just blurs it.
//////////////////////////////////////////////////////////////////////////
static void
BoxFilter (float* result, float* source, int lastWidth, int numComponents)
{
   // Check arguments.
   if ((result == NULL) || (source == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to BoxFilter!\n");
      exit (-1);
   }
   if (lastWidth < 1)
   {
      NmPrint ("ERROR: Bad width %d passed to BoxFilter!\n", lastWidth);
      exit (-1);
   }

   // Figure out width and height
   int height = gHeight;
   int width = gWidth;
   int mult = 2;
   if (lastWidth == gWidth)
   {
      mult = 1;
      height--;
      width--;
   }
   
   // Now do the filter.
   NmPrint ("Box filtering\n");
   for (int y = 0; y < height; y++)
   {
      ShowSpinner ();
      for (int x = 0; x < width; x++)
      {
         float norm[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
         int oldIdx = (y*mult)*lastWidth*numComponents + (x*mult)*numComponents;
         norm[0] += source[oldIdx];
         norm[1] += source[oldIdx+1];
         norm[2] += source[oldIdx+2];
         if (gOcclusion)
         {
            norm[gOcclIdx] += source[oldIdx+gOcclIdx];
         }
         if (gDisplacements)
         {
            norm[gDispIdx] += source[oldIdx+gDispIdx];
         }
         oldIdx = ((y*mult)+1)*lastWidth*numComponents + (x*mult)*numComponents;
         norm[0] += source[oldIdx];
         norm[1] += source[oldIdx+1];
         norm[2] += source[oldIdx+2];
         if (gOcclusion)
         {
            norm[gOcclIdx] += source[oldIdx+gOcclIdx];
         }
         if (gDisplacements)
         {
            norm[gDispIdx] += source[oldIdx+gDispIdx];
         }
         oldIdx = ((y*mult)+1)*lastWidth*numComponents + ((x*mult)+1)*numComponents;
         norm[0] += source[oldIdx];
         norm[1] += source[oldIdx+1];
         norm[2] += source[oldIdx+2];
         if (gOcclusion)
         {
            norm[gOcclIdx] += source[oldIdx+gOcclIdx];
         }
         if (gDisplacements)
         {
            norm[gDispIdx] += source[oldIdx+gDispIdx];
         }
         oldIdx = (y*mult)*lastWidth*numComponents + ((x*mult)+1)*numComponents;
         norm[0] += source[oldIdx];
         norm[1] += source[oldIdx+1];
         norm[2] += source[oldIdx+2];
         int idx = y*gWidth*numComponents + x*numComponents;
         if (gOcclusion)
         {
            norm[gOcclIdx] += source[oldIdx+gOcclIdx];
         }
         if (gDisplacements)
         {
            norm[gDispIdx] += source[oldIdx+gDispIdx];
         }
         double len = sqrt((norm[0]*norm[0])+(norm[1]*norm[1])+(norm[2]*norm[2]));
         if (len < EPSILON)
         {
            result[idx] = 0.0;
            result[idx + 1] = 0.0;
            result[idx + 2] = 0.0;
         }
         else
         {
            result[idx] = (float)(norm[0]/len);
            result[idx + 1] = (float)(norm[1]/len);
            result[idx + 2] = (float)(norm[2]/len);
         }
         if (gOcclusion)
         {
            result[idx + gOcclIdx] = (float)(norm[gOcclIdx]/4);
         }
         if (gDisplacements)
         {
            result[idx + gDispIdx] = (float)(norm[gDispIdx]/4);
         }
      }
   }
   if (gQuiet == NM_VERBOSE)
   {
      printf ("\r");
   }
}

//////////////////////////////////////////////////////////////////////////
// Write the output image in the format specified
//////////////////////////////////////////////////////////////////////////
static void
WriteOutputImage (char* fName, float* img, int numComponents)
{
   if ((fName == NULL) || (img == NULL))
   {
      NmPrint ("ERROR: NULL pointer passed to WriteOutputImage\n");
      exit (-1);
   }
   switch (gOutput)
   {
      case NORM_OUTPUT_8_8_8_TGA:
         {
            // Convert image
            BYTE* outImg = new BYTE[gWidth*gHeight*3];
            NmPrint ("\rConverting to 8x8x8 Targa\n");
            for (int y = 0; y < gHeight; y++)
            {
               ShowSpinner ();
               for (int x = 0; x < gWidth; x++)
               {
                  int idx = y*gWidth*numComponents + x*numComponents;
                  int oIdx = y*gWidth*3 + x*3;
                  outImg[oIdx + 0] = PACKINTOBYTE_MINUS1TO1(img[idx]);
                  outImg[oIdx + 1] = PACKINTOBYTE_MINUS1TO1(img[idx+1]);
                  outImg[oIdx + 2] = PACKINTOBYTE_MINUS1TO1(img[idx+2]);
               }
            }
            if (gQuiet == NM_VERBOSE)
            {
               printf ("\r");
            }
               
            // Write out image
            FILE* fp = fopen (fName, "wb");
            if (fp == NULL)
            {
               NmPrint ("ERROR: Unable to open %s\n", fName);
               exit (-1);
            }
            if (TGAWriteImage (fp, gWidth, gHeight, 24, outImg) != true)
            {
               NmPrint ("ERROR: Unable to write %s\n", fName);
               fclose (fp);
               exit (-1);
            }
            fclose (fp);
            NmPrint ("Wrote %s\n", fName);
            delete [] outImg;
         }
         break;

      case NORM_OUTPUT_8_8_8_8_TGA:
         {
            // Convert image
            BYTE* outImg = new BYTE[gWidth*gHeight*4];
            NmPrint ("\rConverting to 8x8x8x8 Targa\n");
            for (int y = 0; y < gHeight; y++)
            {
               ShowSpinner ();
               for (int x = 0; x < gWidth; x++)
               {
                  int idx = y*gWidth*numComponents + x*numComponents;
                  int oIdx = y*gWidth*4 + x*4;
                  outImg[oIdx + 0] = PACKINTOBYTE_MINUS1TO1(img[idx]);
                  outImg[oIdx + 1] = PACKINTOBYTE_MINUS1TO1(img[idx+1]);
                  outImg[oIdx + 2] = PACKINTOBYTE_MINUS1TO1(img[idx+2]);
                  if (gOcclusion)
                  {
                     outImg[oIdx + 3] = PACKINTOBYTE_0TO1(img[idx+gOcclIdx]);
                  }
                  else
                  {
                     outImg[oIdx + 3] = PACKINTOBYTE_0TO1(0);
                  }
               }
            }
            if (gQuiet == NM_VERBOSE)
            {
               printf ("\r");
            }
               
            // Write out image
            FILE* fp = fopen (fName, "wb");
            if (fp == NULL)
            {
               NmPrint ("ERROR: Unable to open %s\n", fName);
               exit (-1);
            }
            if (TGAWriteImage (fp, gWidth, gHeight, 32, outImg) != true)
            {
               NmPrint ("ERROR: Unable to write %s\n", fName);
               fclose (fp);
               exit (-1);
            }
            fclose (fp);
            NmPrint ("Wrote %s\n", fName);
            delete [] outImg;
         }
         break;
            
      case NORM_OUTPUT_EXP_TGA:
         {
            // Convert image
            BYTE* outImg = new BYTE[gWidth*gHeight*4];
            NmExpPixel jp;
            NmPrint ("\rConverting to experimental format\n");
            for (int y = 0; y < gHeight; y++)
            {
               ShowSpinner ();
               for (int x = 0; x < gWidth; x++)
               {
                  int fIdx = y*gWidth*numComponents + x*numComponents;
                  ConvertToExpPixel (img[fIdx], img[fIdx+1], img[fIdx+2], &jp);
                  int idx = y*gWidth*4 + x*4;
                  outImg[idx + 0] = jp.r;
                  outImg[idx + 1] = jp.g;
                  outImg[idx + 2] = jp.b;
                  outImg[idx + 3] = jp.a;
               }
            }
            if (gQuiet == NM_VERBOSE)
            {
               printf ("\r");
            }
               
            // Write out image
            FILE* fp = fopen (fName, "wb");
            if (fp == NULL)
            {
               NmPrint ("ERROR: Unable to open %s\n", fName);
               exit (-1);
            }
            if (TGAWriteImage (fp, gWidth, gHeight, 32, outImg) != true)
            {
               NmPrint ("ERROR: Unable to write %s\n", fName);
               fclose (fp);
               exit (-1);
            }
            fclose (fp);
            NmPrint ("Wrote %s\n", fName);
            delete [] outImg;
         }
         break;
            
      case NORM_OUTPUT_16_16_ARG:
         {
            // Convert
            short* outImg = new short[gWidth*gHeight*2];
            NmPrint ("\rConverting to 16x16 signed format\n");
            for (int y = 0; y < gHeight; y++)
            {
               ShowSpinner ();
               for (int x = 0; x < gWidth; x++)
               {
                  int fIdx = y*gWidth*numComponents + x*numComponents;
                  int idx = y*gWidth*2 + x*2;
                  outImg[idx + 0] = PACKINTOSHORT_SMINUS1TO1(img[fIdx]);
                  outImg[idx + 1] = PACKINTOSHORT_SMINUS1TO1(img[fIdx+1]);
               }
            }
            if (gQuiet == NM_VERBOSE)
            {
               printf ("\r");
            }
               
            // Now write the image.
            AtiBitmap atiOut;
            atiOut.width = gWidth;
            atiOut.height = gHeight;
            atiOut.bitDepth = 32;
            atiOut.format = ATI_BITMAP_FORMAT_S1616;
            atiOut.pixels = (unsigned char*)(outImg);
            if (!AtiWrite3DARGImageFile (&atiOut, fName))
            {
               NmPrint ("ERROR: Unable to write %s\n", fName);
               exit (-1);
            }
            delete [] outImg;
            NmPrint ("Wrote %s\n", fName);
         }
         break;
            
      case NORM_OUTPUT_16_16_16_16_ARG:
         {
            // Convert image
            short* outImg = new short[gWidth*gHeight*4];
            NmPrint ("\rConverting to 16x16x16x16 signed format\n");
            for (int y = 0; y < gHeight; y++)
            {
               ShowSpinner ();
               for (int x = 0; x < gWidth; x++)
               {
                  int fIdx = y*gWidth*numComponents + x*numComponents;
                  int idx = y*gWidth*4 + x*4;
                  outImg[idx + 0] = PACKINTOSHORT_SMINUS1TO1(img[fIdx]);
                  outImg[idx + 1] = PACKINTOSHORT_SMINUS1TO1(img[fIdx+1]);
                  outImg[idx + 2] = PACKINTOSHORT_SMINUS1TO1(img[fIdx+2]);
                  if (gOcclusion)
                  {
                     outImg[idx + 3] = PACKINTOSHORT_0TO1(img[fIdx+gOcclIdx]);
                  }
                  else
                  {
                     outImg[idx + 3] = PACKINTOSHORT_SMINUS1TO1(0);
                  }
               }
            }
            if (gQuiet == NM_VERBOSE)
            {
               printf ("\r");
            }

            // Now write the image.
            AtiBitmap atiOut;
            atiOut.width = gWidth;
            atiOut.height = gHeight;
            atiOut.bitDepth = 64;
            atiOut.format = ATI_BITMAP_FORMAT_S16161616;
            atiOut.pixels = (unsigned char*)(outImg);
            if (!AtiWrite3DARGImageFile (&atiOut, fName))
            {
               NmPrint ("ERROR: Unable to write %s\n", fName);
               exit (-1);
            }
            delete [] outImg;
            NmPrint ("Wrote %s\n", fName);
         }
         break;
            
      case NORM_OUTPUT_10_10_10_2_ARG:
         {
            // Convert image
            long* outImg = new long[gWidth*gHeight];
            NmPrint ("\rConverting to 10x10x10x2 format\n");
            for (int y = 0; y < gHeight; y++)
            {
               ShowSpinner ();
               for (int x = 0; x < gWidth; x++)
               {
                  int fIdx = y*gWidth*numComponents + x*numComponents;
                  int idx = y*gWidth + x;
#define MASK9  0x1ff
#define MASK10 0x3ff
#define MASK11 0x7ff
                  outImg[idx] = ((((unsigned long)(img[fIdx  ]*MASK9) + MASK9)&MASK10)<<22) |
                                ((((unsigned long)(img[fIdx+1]*MASK9) + MASK9)&MASK10)<<12) |
                                ((((unsigned long)(img[fIdx+2]*MASK9) + MASK9)&MASK10)<<2);
               }
            }
            if (gQuiet == NM_VERBOSE)
            {
               printf ("\r");
            }

            // Now write the image.
            AtiBitmap atiOut;
            atiOut.width = gWidth;
            atiOut.height = gHeight;
            atiOut.bitDepth = 32;
            atiOut.format = ATI_BITMAP_FORMAT_1010102;
            atiOut.pixels = (unsigned char*)(outImg);
            if (!AtiWrite3DARGImageFile (&atiOut, fName))
            {
               NmPrint ("ERROR: Unable to write %s\n", fName);
               exit (-1);
            }
            delete [] outImg;
            NmPrint ("Wrote %s\n", fName);
         }
         break;

      case NORM_OUTPUT_10_10_10_2_ARG_MS:
         {
            // Convert image
            short* outImg = new short[gWidth*gHeight*4];
            NmPrint ("\rConverting to 10x10x10x2 test format (bit chopped 16bpc)\n");
            for (int y = 0; y < gHeight; y++)
            {
               ShowSpinner ();
               for (int x = 0; x < gWidth; x++)
               {
#define MASK_10BITS 0xffc0
                  int fIdx = y*gWidth*numComponents + x*numComponents;
                  int idx = y*gWidth*4 + x*4;
                  outImg[idx + 0] = (PACKINTOSHORT_MINUS1TO1(img[fIdx]))&MASK_10BITS;
                  outImg[idx + 1] = (PACKINTOSHORT_MINUS1TO1(img[fIdx+1]))&MASK_10BITS;
                  outImg[idx + 2] = (PACKINTOSHORT_MINUS1TO1(img[fIdx+2]))&MASK_10BITS;
                  outImg[idx + 3] = PACKINTOSHORT_MINUS1TO1(0);
               }
            }
            if (gQuiet == NM_VERBOSE)
            {
               printf ("\r");
            }

            // Now write the image.
            AtiBitmap atiOut;
            atiOut.width = gWidth;
            atiOut.height = gHeight;
            atiOut.bitDepth = 64;
            atiOut.format = ATI_BITMAP_FORMAT_16161616;
            atiOut.pixels = (unsigned char*)(outImg);
            if (!AtiWrite3DARGImageFile (&atiOut, fName))
            {
               NmPrint ("ERROR: Unable to write %s\n", fName);
               exit (-1);
            }
            delete [] outImg;
            NmPrint ("Wrote %s\n", fName);
         }
         break;

      case NORM_OUTPUT_11_11_10_ARG_MS:
         {
            // Convert image
            short* outImg = new short[gWidth*gHeight*4];
            NmPrint ("\rConverting to 11x11x10 test format (bit chopped 16bpc)\n");
            for (int y = 0; y < gHeight; y++)
            {
               ShowSpinner ();
               for (int x = 0; x < gWidth; x++)
               {
#define MASK_11BITS 0xffe0
                  int fIdx = y*gWidth*numComponents + x*numComponents;
                  int idx = y*gWidth*4 + x*4;
                  outImg[idx + 0] = (PACKINTOSHORT_MINUS1TO1(img[fIdx]))&MASK_11BITS;
                  outImg[idx + 1] = (PACKINTOSHORT_MINUS1TO1(img[fIdx+1]))&MASK_11BITS;
                  outImg[idx + 2] = (PACKINTOSHORT_MINUS1TO1(img[fIdx+2]))&MASK_10BITS;
                  outImg[idx + 3] = PACKINTOSHORT_MINUS1TO1(0);
               }
            }
            if (gQuiet == NM_VERBOSE)
            {
               printf ("\r");
            }
            
            // Now write the image.
            AtiBitmap atiOut;
            atiOut.width = gWidth;
            atiOut.height = gHeight;
            atiOut.bitDepth = 64;
            atiOut.format = ATI_BITMAP_FORMAT_16161616;
            atiOut.pixels = (unsigned char*)(outImg);
            if (!AtiWrite3DARGImageFile (&atiOut, fName))
            {
               NmPrint ("ERROR: Unable to write %s\n", fName);
               exit (-1);
            }
            delete [] outImg;
            NmPrint ("Wrote %s\n", fName);
         }
         break;
            
      default:
         NmPrint ("ERROR: Unknown output format!\n");
         exit (-1);
   } // end switch on output type.

   // If we were asked to generate displacements spit out a 16bit .arg file.
   if (gDisplacements)
   {
      // Convert
      short* outImg = new short[gWidth*gHeight];
      NmPrint ("\rConverting displacements to 16 bit signed format\n");
      float maxDisp = 0.0f;
      for (int y = 0; y < gHeight; y++)
      {
         ShowSpinner ();
         for (int x = 0; x < gWidth; x++)
         {
            int fIdx = y*gWidth*numComponents + x*numComponents + gDispIdx;
            if (fabs (img[fIdx]) > maxDisp)
            {
               maxDisp = img[fIdx];
            }
         }
      }
      NmPrint ("Normalizing displacements by maximum displacement (%8.4f)\n",
               maxDisp);
      for (int y = 0; y < gHeight; y++)
      {
         ShowSpinner ();
         for (int x = 0; x < gWidth; x++)
         {
            int fIdx = y*gWidth*numComponents + x*numComponents + gDispIdx;
            int idx = y*gWidth + x;
            outImg[idx + 0] = PACKINTOSHORT_SMINUS1TO1(img[fIdx]/maxDisp);
         }
      }
      if (gQuiet == NM_VERBOSE)
      {
         printf ("\r");
      }
               
      // Now write the image.
      AtiBitmap atiOut;
      atiOut.width = gWidth;
      atiOut.height = gHeight;
      atiOut.bitDepth = 16;
      atiOut.format = ATI_BITMAP_FORMAT_16;
      atiOut.pixels = (unsigned char*)(outImg);
      char8 dName[256];
      sprintf (dName, "disp-%s.arg", fName);
      if (!AtiWrite3DARGImageFile (&atiOut, dName))
      {
         NmPrint ("ERROR: Unable to write %s\n", dName);
         exit (-1);
      }
      delete [] outImg;
      NmPrint ("Wrote %s\n", dName);
   } // end if we are writing displacements.
} // WriteOutputImage

//////////////////////////////////////////////////////////////////////////
// Check argument flags and set the appropriate values.
//////////////////////////////////////////////////////////////////////////
static void
TestArgs (char* args)
{
   // Super-sample number
   if (strstr (args, "1") != NULL)
   {
      gNumSamples = 1;
   }
   if (strstr (args, "2") != NULL)
   {
      gNumSamples = 2;
   }
   if (strstr (args, "3") != NULL)
   {
      gNumSamples = 3;
   }
   if (strstr (args, "4") != NULL)
   {
      gNumSamples = 4;
   }
   if (strstr (args, "5") != NULL)
   {
      gNumSamples = 5;
   }
   if (strstr (args, "6") != NULL)
   {
      gNumSamples = 6;
   }
   if (strstr (args, "7") != NULL)
   {
      gNumSamples = 7;
   }
   if (strstr (args, "8") != NULL)
   {
      gNumSamples = 8;
   }
   if (strstr (args, "9") != NULL)
   {
      gNumSamples = 9;
   }
   if (strstr (args, "0") != NULL)
   {
      gNumSamples = 10;
   }

   // Rulesets for determining best normals
   if (strstr (args, "c") != NULL)
   {
      gNormalRules = NORM_RULE_CLOSEST;
      if (strstr (args, "X") != NULL)
      {
         gNormalRules = NORM_RULE_FRONT_CLOSEST;
      }
   }
   if (strstr (args, "C") != NULL)
   {
      gNormalRules = NORM_RULE_BEST_CLOSEST;
      if (strstr (args, "X") != NULL)
      {
         gNormalRules = NORM_RULE_FRONT_BEST_CLOSEST;
      }
   }
   if (strstr (args, "f") != NULL)
   {
      gNormalRules = NORM_RULE_FARTHEST;
      if (strstr (args, "X") != NULL)
      {
         gNormalRules = NORM_RULE_FRONT_FURTHEST;
      }
   }
   if (strstr (args, "F") != NULL)
   {
      gNormalRules = NORM_RULE_BEST_FARTHEST;
      if (strstr (args, "X") != NULL)
      {
         gNormalRules = NORM_RULE_FRONT_BEST_FURTHEST;
      }
   }
   if (strstr (args, "d") != NULL)
   {
      gNormalRules = NORM_RULE_MIXED;
   }
   if (strstr (args, "D") != NULL)
   {
      gNormalRules = NORM_RULE_BEST_MIXED;
   }
   if (strstr (args, "b") != NULL)
   {
      gNormalRules = NORM_RULE_BEST;
   }

   // Misc flags
   if (strstr (args, "r") != NULL)
   {
      gOcclusion = true;
      gNumDivisions = 4;
   }
   if (strstr (args, "R") != NULL)
   {
      gOcclusion = true;
      gNumDivisions = 6;
   }
   if (strstr (args, "o") != NULL)
   {
      gOcclusion = true;
      gNumDivisions = 8;
   }
   if (strstr (args, "O") != NULL)
   {
      gOcclusion = true;
      gNumDivisions = 12;
   }
   if (strstr (args, "n") != NULL)
   {
      gBentNormal = true;
   }
   if (strstr (args, "N") != NULL)
   {
      gBentNormal = true;
      gNumDivisions = 12;
   }
   if (strstr (args, "m") != NULL)
   {
      gComputeMipLevels = MIP_RECOMPUTE;
   }
   if (strstr (args, "M") != NULL)
   {
      gComputeMipLevels = MIP_BOX;
   }
   if ((strstr (args, "w") != NULL) ||
       (strstr (args, "W") != NULL))
   {
      gInTangentSpace = false;
   }
   if (strstr (args, "e") != NULL)
   {
      gExpandTexels = false;
   }
   if (strstr (args, "E") != NULL)
   {
      gDilateTexels = 15;
   }
   if (strstr (args, "g") != NULL)
   {
      gDilateTexels = 20;
   }
   if (strstr (args, "G") != NULL)
   {
      gDilateTexels = 30;
   }
   if (strstr (args, "t") != NULL)
   {
      gEpsilon = EPSILON;
   }
   if (strstr (args, "T") != NULL)
   {
      gEpsilon = 0.1;
   }
   if (strstr (args, "h") != NULL)
   {
      gOutput = NORM_OUTPUT_16_16_ARG;
   }
   if (strstr (args, "H") != NULL)
   {
      gOutput = NORM_OUTPUT_16_16_16_16_ARG;
   }
   if (strstr (args, "i") != NULL)
   {
      gOutput = NORM_OUTPUT_10_10_10_2_ARG;
   }
   if (strstr (args, "s") != NULL)
   {
      gOutput = NORM_OUTPUT_10_10_10_2_ARG_MS;
   }
   if (strstr (args, "S") != NULL)
   {
      gOutput = NORM_OUTPUT_11_11_10_ARG_MS;
   }
   if (gOcclusion)
   {
      if (gOutput == NORM_OUTPUT_8_8_8_TGA)
      {
         gOutput = NORM_OUTPUT_8_8_8_8_TGA;
      }
      else if (gOutput != NORM_OUTPUT_16_16_16_16_ARG)
      {
         NmPrint ("Warning: Specified output format will NOT output occlusion. Right now only 8x8x8x8 TARGA (default) and 16x16x16x16 ARG (-H) will output occlusion!\n");
      }
   }
   if (strstr (args, "B") != NULL)
   {
      gBoxFilter = true;
   }
   if (strstr (args, "a") != NULL)
   {
      gMaxAngle = 0.4;
   }
   if (strstr (args, "A") != NULL)
   {
      gMaxAngle = 0.7;
   }
   if (strstr (args, "l") != NULL)
   {
      gMaxAngle = -0.1;
   }
   if (strstr (args, "L") != NULL)
   {
      gMaxAngle = -0.4;
   }
   if (strstr (args, "q") != NULL)
   {
      gQuiet = NM_QUIET;
   }
   if (strstr (args, "Q") != NULL)
   {
      gQuiet = NM_SILENT;
   }
   if (strstr (args, "z") != NULL)
   {
      gEdgeCopy = false;
   }
   if (strstr (args, "p") != NULL)
   {
      gTrianglePad = 3;
   }
   if (strstr (args, "P") != NULL)
   {
      gTrianglePad = 4;
   }
   if (strstr (args, "y") != NULL)
   {
      gDisplacements = true;
   }
   if (strstr (args, "k") != NULL)
   {
      gAddTexelCorners = true;
   }
} // TestArgs

//////////////////////////////////////////////////////////////////////////
// Process arguments
//////////////////////////////////////////////////////////////////////////
static void
ProcessArgs (int argc, char **argv, char** lowName, char** highName,
             char** bumpName, double* bumpScale, char** outName)
{
   // A common usage line so everything is consistent.
   char* flags = NULL;
   static char* usage = "Usage: NormalMapper [-0123456789aAbBcCdDeEfFgGhHiklLmMnNoOpPqQrRtTvwXyz] lowres highres Width Height [value] [heightmap heightscale] outputname\n";

   // Print out verbose message if 0 or 1 arguments given.
   if ((argc == 1) || (argc == 2))
   {
      NmPrint (usage);
      NmPrint ("                     h  - Output 16x16 .arg texture\n");
      NmPrint ("                     H  - Output 16x16x16x16 .arg texture\n");
      NmPrint ("                     i  - Output 10x10x10x2 .arg texture\n");
      NmPrint ("                     w  - Use worldspace normals\n");
      NmPrint ("                     m  - Create mip chain (remap)\n");
      NmPrint ("                     M  - Create mip chain (box filter)\n");
      NmPrint ("                     B  - Box filter final image\n");
      NmPrint ("                     e  - Don't expand border texels\n");
      NmPrint ("                     E  - expand border texels more (15 texels)\n");
      NmPrint ("                     g  - more dilation (20 texels)\n");
      NmPrint ("                     G  - even more dilation (30 texels)\n");
      NmPrint ("                     t  - larger tolerance on compare\n");
      NmPrint ("                     T  - largest tolerance on compare\n");
      NmPrint ("                     a  - smaller angle for cutoff\n");
      NmPrint ("                     A  - smallest angle for cutoff\n");
      NmPrint ("                     l  - larger angle for cutoff\n");
      NmPrint ("                     L  - largest angle for cutoff\n");
      NmPrint ("                     X  - only normals in front\n");
      NmPrint ("                     z  - don't perform edge copy\n");
      NmPrint ("                     r  - Generate occlusion term (69 rays)\n");
      NmPrint ("                     R  - Generate occlusion term (149 rays)\n");
      NmPrint ("                     o  - Generate occlusion term (261 rays)\n");
      NmPrint ("                     O  - Generate occlusion term (581 rays)\n");
      NmPrint ("                     n  - Generate bent normal (261 rays)\n");
      NmPrint ("                     N  - Generate bent normal (581 rays)\n");
      NmPrint ("                     y  - Generate displacement values\n");
      NmPrint ("                     p  - more triangle padding\n");
      NmPrint ("                     P  - even more triangle padding\n");
      NmPrint ("                     q  - Quiet mode, no spinners or percentages\n");
      NmPrint ("                     Q  - Very quiet mode, no messages\n");
      NmPrint ("                     v  - value is the maximum distance\n");
      NmPrint ("                     1  - Five samples per texel\n");
      NmPrint ("                     2  - Ten samples per texel\n");
      NmPrint ("                     3  - Seventeen samples per texel\n");
      NmPrint ("                     4  - Twenty six samples per texel\n");
      NmPrint ("                     5  - Thirty seven samples per texel\n");
      NmPrint ("                     6  - Fifty samples per texel\n");
      NmPrint ("                     7  - Sixty five samples per texel\n");
      NmPrint ("                     8  - Eighty two samples per texel\n");
      NmPrint ("                     9  - One hundred and one samples per texel\n");
      NmPrint ("                     0  - One hundred and twenty two samples per texel\n");
      NmPrint ("                     k  - Add four samples for texel corners\n");
      NmPrint ("                     c  - Pick closest intersection,\n");
      NmPrint ("                          first if equidistant\n");
      NmPrint ("                     C  - Pick closest intersection,\n");
      NmPrint ("                          normal closer to low res if equidistant\n");
      NmPrint ("                     f  - Pick furthest intersection,\n");
      NmPrint ("                          first if equidistant\n");
      NmPrint ("                     F  - Pick furthest intersection,\n");
      NmPrint ("                          normal closer to low res if equidistant\n");
      NmPrint ("                     d  - Pick furthest intersection in front,\n");
      NmPrint ("                          closest intersection behind,\n");
      NmPrint ("                          first if equidistant\n");
      NmPrint ("                     D  - Pick Pick furthest intersection in front,\n");
      NmPrint ("                          normal closer to low res if equidistant,\n");
      NmPrint ("                          closest intersection behind,\n");
      NmPrint ("                     b  - normal closest to low res\n");
      exit (-1);
   }

   // Make sure the right number of arguments are present.
   if ((argc != 6) && (argc != 7) && (argc != 8) && (argc != 9) && (argc != 10))
   {
      NmPrint (usage);
      exit (-1);
   }

   // No heightmap, no arguments.
   gEpsilon = 0.0;
   if (argc == 6)
   {
      (*lowName) = argv[1];
      (*highName) = argv[2];
      gWidth = atoi (argv[3]);
      gHeight = atoi (argv[4]);
      (*outName) = argv[5];
   }

   // Command line arguments
   if (argc == 7)
   {
      (*lowName) = argv[2];
      (*highName) = argv[3];
      gWidth = atoi (argv[4]);
      gHeight = atoi (argv[5]);
      (*outName) = argv[6];
      TestArgs (argv[1]);
      flags = argv[1];
   }

   // Height map no command line args, or distance with arguments.
   if (argc == 8)
   {

      if (strstr (argv[1], "-") != NULL)
      {
         if (strstr (argv[1], "v") == NULL)
         {
            NmPrint (usage);
            exit (-1);
         }
         (*lowName) = argv[2];
         (*highName) = argv[3];
         gWidth = atoi (argv[4]);
         gHeight = atoi (argv[5]);
         gDistance = atof (argv[6]);
         (*outName) = argv[7];
         TestArgs (argv[1]);
         flags = argv[1];
      }
      else
      {
         gNormalRules = NORM_RULE_CLOSEST;
         (*lowName) = argv[1];
         (*highName) = argv[2];
         gWidth = atoi (argv[3]);
         gHeight = atoi (argv[4]);
         (*bumpName) = argv[5];
         (*bumpScale) = atof (argv[6]);
         (*outName) = argv[7];
      }
   }

   // Commandline arguments and heightmap
   if (argc == 9)
   {
         gNormalRules = NORM_RULE_CLOSEST;
         (*lowName) = argv[2];
         (*highName) = argv[3];
         gWidth = atoi (argv[4]);
         gHeight = atoi (argv[5]);
         (*bumpName) = argv[6];
         (*bumpScale) = atof (argv[7]);
         (*outName) = argv[8];
         TestArgs (argv[1]);
         flags = argv[1];
   }

   // Commandline arguments, heightmap, and distance
   if (argc == 10)
   {
         if (strstr (argv[1], "v") == NULL)
         {
            NmPrint (usage);
            exit (-1);
         }

         gNormalRules = NORM_RULE_CLOSEST;
         (*lowName) = argv[2];
         (*highName) = argv[3];
         gWidth = atoi (argv[4]);
         gHeight = atoi (argv[5]);
         gDistance = atof (argv[6]);
         (*bumpName) = argv[7];
         (*bumpScale) = atof (argv[8]);
         (*outName) = argv[9];
         TestArgs (argv[1]);
         flags = argv[1];
   }

   // Check width and height
   if ((gWidth % 2) != 0)
   {
      NmPrint ("ERROR: Width must be a power of two!\n");
      exit (-1);
   }
   if ((gHeight % 2) != 0)
   {
      NmPrint ("ERROR: Height must be a power of two!\n");
      exit (-1);
   }
   if (gWidth < 1)
   {
      NmPrint ("ERROR: Width must be greater than 0\n");
      exit (-1);
   }
   if (gHeight < 1)
   {
      NmPrint ("ERROR: Height must be greater than 0\n");
      exit (-1);
   }

   // Print out options
   NmPrint (versionString);
   if (flags != NULL)
   {
      NmPrint ("Flags: %s\n", flags);
   }
   else
   {
      NmPrint ("Flags:\n");
   }
   NmPrint ("Width: %d Height: %d\n", gWidth, gHeight);
   GetSamples (&gNumSamples, &gSamples);
   NmPrint ("%d sample(s) per texel\n", gNumSamples);
   NmPrint ("%d texel triangle pad\n", gTrianglePad);
   if (gBentNormal)
   {
      NmPrint ("Storing bent normal\n");
   }
   if (gOcclusion)
   {
      NmPrint ("Storing occlusion term\n");
   }
   switch (gNormalRules)
   {
      case NORM_RULE_CLOSEST:
         NmPrint ("Normal Rule: Closest\n");
         break;
      case NORM_RULE_BEST_CLOSEST:
         NmPrint ("Normal Rule: Best Closest\n");
         break;
      case NORM_RULE_BEST_FARTHEST:
         NmPrint ("Normal Rule: Best Furthest\n");
         break;
      case NORM_RULE_FARTHEST:
         NmPrint ("Normal Rule: Furthest\n");
         break;
      case NORM_RULE_MIXED:
         NmPrint ("Normal Rule: Mixed\n");
         break;
      case NORM_RULE_BEST_MIXED:
         NmPrint ("Normal Rule: Best Mixed\n");
         break;
      case NORM_RULE_BEST:
         NmPrint ("Normal Rule: Best\n");
         break;
      case NORM_RULE_FRONT_FURTHEST:
         NmPrint ("Normal Rule: Front Furthest\n");
         break;
      case NORM_RULE_FRONT_BEST_FURTHEST:
         NmPrint ("Normal Rule: Front Best Furthest\n");
         break;
      case NORM_RULE_FRONT_CLOSEST:
         NmPrint ("Normal Rule: Front Closest\n");
         break;
      case NORM_RULE_FRONT_BEST_CLOSEST:
         NmPrint ("Normal Rule: Front Best Closest\n");
         break;
      default:
         NmPrint ("Normal Rule: UKNOWN!\n");
         break;
   }
   if (gDistance != FLT_MAX)
   {
      NmPrint ("Max distance: %4.12f\n", gDistance);
   }
   NmPrint ("Epsilon: %1.10f\n", gEpsilon);
   NmPrint ("Cutoff Angle: %5.1f\n", (acos (gMaxAngle) * (180.0/PI))*2.0);
   switch (gComputeMipLevels)
   {
      case MIP_NONE:
         NmPrint ("No mipmap generation\n");
         break;
      case MIP_RECOMPUTE:
         NmPrint ("Re-cast mipmap generation.\n");
         break;
      case MIP_BOX:
         NmPrint ("Box filter mip map generation.\n");
         break;
      default:
         NmPrint ("Unknown mip map generation\n");
         break;
   }
   if (gInTangentSpace)
   {
      NmPrint ("Normals in tangent space\n");
   }
   else
   {
      NmPrint ("Normals in world space\n");
   }
   if (gExpandTexels)
   {
      NmPrint ("Expand border texels (%d texels)\n", gDilateTexels);
   }
   else
   {
      NmPrint ("Don't Expand border texels\n");
   }
   if (gEdgeCopy)
   {
      NmPrint ("Perform edge copy\n");
   }
   else
   {
      NmPrint ("Don't perform edge copy\n");
   }
   if (gBoxFilter)
   {
      NmPrint ("Postprocess box filter\n");
   }
   else
   {
      NmPrint ("No postprocess filter\n");
   }
} // ProcessArgs

//////////////////////////////////////////////////////////////////////////
// Entry point
//////////////////////////////////////////////////////////////////////////
int 
main (int argc, char **argv)
{

   // Get the arguments.
   char* lowName = NULL;
   char* highName = NULL;
   char* bumpName = NULL;
   double bumpScale = 1.0;
   char* outName = NULL;
   ProcessArgs (argc, argv, &lowName, &highName, &bumpName, &bumpScale,
                &outName);

   // Get bump map from height map.
   float* bumpMap = NULL;
   int bumpWidth = 0;
   int bumpHeight = 0;
   if (bumpName != NULL)
   {
      NmPrint ("Bump Scale: %2.4f\n", bumpScale);
      NmPrint ("Reading in height map: %s\n", bumpName);
      GetBumpMapFromHeightMap (bumpName, &bumpWidth,  &bumpHeight, &bumpMap,
                               (float)bumpScale);
   }

   // Read in the low res model
   int lowNumTris;
   NmRawTriangle* lowTris;
   double lowBBox[6];
   ReadModel (lowName, "low", &lowNumTris, &lowTris, lowBBox, true);

   // If the flag is set compute tangent space for all the low res triangles.
   NmTangentMatrix* tangentSpace = NULL;
   if (gInTangentSpace == true)
   {
      GenerateTangents (&tangentSpace, lowNumTris, lowTris);
   }

   // Read in the high res model
   int highNumTris;
   NmRawTriangle* highTris;
   double highBBox[6];
   ReadModel (highName, "high", &highNumTris, &highTris, highBBox, false);


   // Tangents for high res if we need them.
   NmTangentMatrix* hTangentSpace = NULL;
   if (bumpName != NULL)
   {
      GenerateTangents (&hTangentSpace, highNumTris, highTris);
   }
   
   // If we are doing occlusion term or bent normal put the high res model
   // into an octree to speed up the ray casting.
   AtiOctree* octree = NULL;
   int numRays = 0;
   NmRawPointD* rays = NULL;
   double* rayWeights = NULL;

   // Figure out the bounding box for the high res model
   AtiOctBoundingBox bbox;
   bbox.minX = (float)highBBox[0];
   bbox.minY = (float)highBBox[1];
   bbox.minZ = (float)highBBox[2];
   bbox.maxX = (float)highBBox[3];
   bbox.maxY = (float)highBBox[4];
   bbox.maxZ = (float)highBBox[5];
   bbox.halfX = (bbox.maxX - bbox.minX) / 2.0f;
   bbox.centerX = bbox.halfX + bbox.minX;
   bbox.halfY = (bbox.maxY - bbox.minY) / 2.0f;
   bbox.centerY = bbox.halfY + bbox.minY;
   bbox.halfZ = (bbox.maxZ - bbox.minZ) / 2.0f;
   bbox.centerZ = bbox.halfZ + bbox.minZ;

   // Create and fill the octree.
   NmPrint ("Creating octree\n");
   octree = new AtiOctree;
   if (octree == NULL)
   {
     NmPrint ("ERROR: Unable to create octree!\n");
     exit (-1);
   }
   if (octree->FillTree (highNumTris, (void*)(highTris), bbox, TriInBox,
                         gMaxInBox, NmOctreeProgress) == FALSE)
   {
     NmPrint ("ERROR: Unable to fill octree!\n");
     exit (-1);
   }
   if (gQuiet == NM_VERBOSE)
   {
      printf ("\r");
   }
   PrintOctreeStats (octree);

   // If we are doing occlusion stuff get ray hemisphere.
   if (gOcclusion || gBentNormal)
   {
      // Now fill up the hemisphere of rays.
      if (!GetRays (gNumDivisions, &numRays, &rays, &rayWeights))
      {
         NmPrint ("ERROR: Unable to create hemisphere of rays!\n");
         exit (-1);
      }
      NmPrint ("Using %d rays for occlusion testing\n",
               gNumDivisions*gNumDivisions*4 + 5);
   }
   
   // Create images
   int numComponents = 3;
   if (gOcclusion)
   {
      gOcclIdx = numComponents;
      numComponents++;
   }
   if (gDisplacements)
   {
      gDispIdx = numComponents;
      numComponents++;
   }
   float* img = new float[gWidth*gHeight*numComponents+numComponents];
   if (img == NULL)
   {
      NmPrint ("ERROR: Unable to allocate texture\n");
      exit (-1);
   }
   float* img2 = new float[gWidth*gHeight*numComponents+numComponents];
   if (img2 == NULL)
   {
      NmPrint ("ERROR: Unable to allocate texture\n");
      exit (-1);
   }

   // loop over mip levels
   int mipCount = 0;
   int lastWidth = gWidth;
   int lastHeight = gHeight;
   while ((gWidth > 0) && (gHeight > 0))
   {
      // A little info
      NmPrint ("Output normal map %d by %d\n", gWidth, gHeight);

      // If this is the first time, or we are recomputing each mip level
      // find the normals by ray casting.
      if ((gComputeMipLevels == MIP_RECOMPUTE) || (mipCount == 0))
      {
         // Zero out memory
         memset (img, 0, sizeof (float)*gWidth*gHeight*numComponents);
         memset (img2, 0, sizeof (float)*gWidth*gHeight*numComponents);
         
         // Loop over the triangles in the low res model and rasterize them
         // into the normal texture based on the texture coordinates.
         NmPrint ("Computing normals\n");
         if (gQuiet == NM_VERBOSE)
         {
            printf ("  0%%");
         }
         NmEdge edge[3];
         for (int l = 0; l < lowNumTris; l++)
         {
            // Show some progress
            char prog[24];
            if (gQuiet == NM_VERBOSE)
            {
               float progress = ((float)(l)/(float)(lowNumTris))*100.0f;
               sprintf (prog, "%6.2f%% ", progress);
               printf ("\r%s", prog);
            }

            // Save off a pointer to the triangle
            NmRawTriangle* lTri = &lowTris[l];

            // Find initial Barycentric parameter. Algorithm described at: 
            // http://research.microsoft.com/~hollasch/cgindex/math/barycentric.html
            double x1 = lTri->texCoord[0].u*((double)(gWidth)-1.0);
            double x2 = lTri->texCoord[1].u*((double)(gWidth)-1.0);
            double x3 = lTri->texCoord[2].u*((double)(gWidth)-1.0);
            double y1 = lTri->texCoord[0].v*((double)(gHeight)-1.0);
            double y2 = lTri->texCoord[1].v*((double)(gHeight)-1.0);
            double y3 = lTri->texCoord[2].v*((double)(gHeight)-1.0);
            double b0 = ((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)); 

            // We will be dividing by b0 so it better not be zero.
            if (fabs(b0) < EPSILON)
            {
#ifdef _DEBUG
               NmPrint ("\rWarning b0 == 0.0\n");
#endif
               continue;
            }

            // Find edges and sort by minimum Y.
            GetSortedEdges (edge, lTri);
                        
            // Find min/max Y.
            int minY, maxY;
            GetYMinMax (edge, &minY, &maxY);

            // Figure out Y bounds using padding
            int minX = 0;
            int maxX = 0;
            int pad = gTrianglePad;
            int firstY = (minY - pad);
            int lastY = (maxY + pad);

            // Now loop over Ys doing each "scanline"
            for (int y = firstY; y <= lastY; y++)
            {
               // Make sure this is a valid texture coordinate
               if ((y < 0) || (y >= gHeight))
               {
                  continue;
               }

               // Find min and max X for this scanline.
               // Since we are doing a little bit more this is a little funky.
               // Basically the first time through we are at minY-X so we need
               // to use minY as our Y value, which will give us the original
               // minX maxX. The once we are at minY we don't want to recompute
               // because we already computed min/maxX and further computation
               // that would update edge information, so we skip the
               // computation. After that we proceed as normal until we walk
               // off of maxY at which point we just use maxY's min/maxX. The
               // idea here is that we want to make sure we catch the corners
               // for of texels when we are on a texture boundary, those
               // corners will be 0 effectively and we want a real value in
               // in them. The Barycentric test will mask out any samples
               // that don't fall within our actual triangle.
               if (y == firstY)
               {
                  GetXMinMax (edge, minY, &minX, &maxX);
               }
               else if ((y > minY) && (y <= maxY))
               {
                  GetXMinMax (edge, y, &minX, &maxX);
               }

               // Loop over the Xs filling in each texel of the normal map
               for (int x = (minX - pad); x <= (maxX + pad); x++)
               {
#ifdef DEBUG_INTERSECTION
                  gDbgIntersection = false;
                  for (int d = 0; d < gDbgNum; d++)
                  {
                     if ((y == gDbgY[d]) && (x == gDbgX[d]))
                     {
                        gDbgIntersection = true;
                     }
                  }
                  if (gDbgIntersection)
                  {
                     NmPrint ("============================================\n");
                     NmPrint ("Debug at texel: %d %d\n", x, y);
                     NmPrint ("   Triangle %d\n", l);
                     NmPrint ("      v1:   %8.4f  %8.4f  %8.4f\n", 
                              lTri->vert[0].x, lTri->vert[0].y,lTri->vert[0].z);
                     NmPrint ("      n1:   %8.4f  %8.4f  %8.4f\n", 
                              lTri->norm[0].x, lTri->norm[0].y,lTri->norm[0].z);
                     NmPrint ("      t1: %8.4f  %8.4f\n", lTri->texCoord[0].u,
                              lTri->texCoord[0].v);
                     NmPrint ("      v2:   %8.4f  %8.4f  %8.4f\n", 
                              lTri->vert[1].x, lTri->vert[1].y,lTri->vert[1].z);
                     NmPrint ("      n2:   %8.4f  %8.4f  %8.4f\n", 
                              lTri->norm[1].x, lTri->norm[1].y,lTri->norm[1].z);
                     NmPrint ("      t2: %8.4f  %8.4f\n", lTri->texCoord[1].u,
                              lTri->texCoord[1].v);
                     NmPrint ("      v3:   %8.4f  %8.4f  %8.4f\n", 
                              lTri->vert[2].x, lTri->vert[2].y,lTri->vert[2].z);
                     NmPrint ("      n3:   %8.4f  %8.4f  %8.4f\n", 
                              lTri->norm[2].x, lTri->norm[2].y,lTri->norm[2].z);
                     NmPrint ("      t3: %8.4f  %8.4f\n", lTri->texCoord[2].u,
                              lTri->texCoord[2].v);
                     NmPrint ("      x1/y1: %8.4f %8.4f\n", x1, y1);
                     NmPrint ("      x2/y2: %8.4f %8.4f\n", x2, y2);
                     NmPrint ("      x3/y3: %8.4f %8.4f\n", x3, y3);
                     NmPrint ("      b0: %8.4f\n", b0);
                  }
#endif
                  // Make sure this is a valid texture coordinate
                  if ((x < 0) || (x >= gWidth))
                  {
                     continue;
                  }

                  // Find Barycentric coordinates.
                  double b1 = ((x2-x) * (y3-y) - (x3-x) * (y2-y)) / b0;
                  double b2 = ((x3-x) * (y1-y) - (x1-x) * (y3-y)) / b0;
                  double b3 = ((x1-x) * (y2-y) - (x2-x) * (y1-y)) / b0;

                  // Interpolate tangent space.
                  double ts[9];
                  if (gInTangentSpace == true)
                  {
                     for (int t = 0; t < 9; t++)
                     {
                        ts[t] = (tangentSpace[l].m[0][t] * b1) + 
                                (tangentSpace[l].m[1][t] * b2) +
                                (tangentSpace[l].m[2][t] * b3);
                     }
                  }
                  
                  // For each sample accumulate the normal
                  double sNorm[3] = {0.0, 0.0, 0.0};
                  double sOcclusion = 0.0;
                  double sDisplacement = 0.0;
                  int sFound = 0;
                  double aNorm[3] = {0.0, 0.0, 0.0};
                  for (int s = 0; s < gNumSamples; s++)
                  {
#ifdef DEBUG_INTERSECTION
                     if (gDbgIntersection)
                     {
                        NmPrint ("> Sample %d (%8.4f, %8.4f)\n", s, 
                                 gSamples[s].x, gSamples[s].y);
                     }
#endif
                     // Compute new x & y
                     double sX = (double)(x) + gSamples[s].x;
                     double sY = (double)(y) + gSamples[s].y;
                     
                     // Find Barycentric coordinates.
                     double b1 = ((x2-sX) * (y3-sY) - (x3-sX) * (y2-sY)) / b0;
                     double b2 = ((x3-sX) * (y1-sY) - (x1-sX) * (y3-sY)) / b0;
                     double b3 = ((x1-sX) * (y2-sY) - (x2-sX) * (y1-sY)) / b0;
#ifdef DEBUG_INTERSECTION
                     if (gDbgIntersection)
                     {
                        NmPrint ("   sX/Y: %8.4f  %8.4f\n", sX, sY);
                        NmPrint ("   bcc: %8.4f %8.4f %8.4f\n", b1, b2, b3);
                     }
#endif

                     // Use Barycentric coordinates to deterimine if we are
                     // are outside of the triangle.
                     if ((b1 > 1.0) || (b1 < 0.0) ||
                         (b2 > 1.0) || (b2 < 0.0) ||
                         (b3 > 1.0) || (b3 < 0.0))
                     {
#ifdef DEBUG_INTERSECTION
                        if (gDbgIntersection)
                        {
                           NmPrint ("   Outside triangle!\n");
                        }
#endif
                        continue;
                     }

                     // Compute position and normal
                     NmRawPointD pos;
                     NmRawPointD norm;
                     BaryInterpolate (lTri, b1, b2, b3, pos.v, norm.v);
#ifdef DEBUG_INTERSECTION
                     if (gDbgIntersection)
                     {
                        NmPrint ("      nPos: %8.4f  %8.4f  %8.4f\n", 
                                 pos.x, pos.y, pos.z);
                        NmPrint ("      nNrm: %8.4f  %8.4f  %8.4f\n", 
                                 norm.x, norm.y, norm.z);
                     }
#endif

                     // First see if we even have an intersection.
                     double newNorm[3];
                     double newPos[3];
                     double newDisplacement = 0.0f;
                     if (FindBestIntersection (pos, norm, octree, highTris,
                                               hTangentSpace, bumpMap, 
                                               bumpHeight, bumpWidth, newNorm, 
                                               newPos, &newDisplacement))
                     {
                        // Normalize the new normal
                        sDisplacement += newDisplacement;
                        Normalize (newNorm);
                        
                        // Do bent normal/occlusion if needed.
                        if (gOcclusion || gBentNormal)
                        {
                           // First do the ray casting.
                           NmRawPointD bentNormal;
                           double occlusion;
                           ComputeOcclusion ((NmRawPointD*)(newPos),
                                             (NmRawPointD*)(newNorm),
                                             highNumTris, highTris, octree,
                                             numRays, rays, rayWeights,
                                             &bentNormal, &occlusion);
                           
                           // Add it to our sample sum.
                           if (gBentNormal)
                           {
                              sNorm[0] += bentNormal.x;
                              sNorm[1] += bentNormal.y;
                              sNorm[2] += bentNormal.z;
                           }
                           else
                           {
                              sNorm[0] += newNorm[0];
                              sNorm[1] += newNorm[1];
                              sNorm[2] += newNorm[2];
                           }
                           sOcclusion += occlusion;
                           sFound++;
                        }
                        else
                        {  // Plain old normal map
                           sNorm[0] += newNorm[0];
                           sNorm[1] += newNorm[1];
                           sNorm[2] += newNorm[2];
                           sFound++;
                        }
                     } // end if we found a normal
                  } // end for samples (s);

                  // If we found a normal put it in the image.
                  if (sFound > 0)
                  {
                     // Convert to tangent space if needed
                     if (gInTangentSpace == true)
                     {
                        ConvertToTangentSpace (ts, sNorm, sNorm);
                     }

                     // Add it to the image.
                     int idx = y*gWidth*numComponents + x*numComponents;
                     img[idx + 0] += (float)sNorm[0];
                     img[idx + 1] += (float)sNorm[1];
                     img[idx + 2] += (float)sNorm[2];
                     if (gOcclusion)
                     {
                        img[idx + gOcclIdx] = (float)(sOcclusion/sFound);
                     }
                     if (gDisplacements)
                     {
                        img[idx + gDispIdx] = (float)(sDisplacement/sFound);
                     }
                  } // end if we found a normal
                  else if (gEdgeCopy)
                  {
                     // Since we didn't find a normal, "snap" the center
                     // sample's barycentric coordinates to the edge of the
                     // triangle and find the normal there. Store this in
                     // a temporary buffer that we'll use to dilate the image
                     // in a way that doesn't produce texture seams.
                     double sX = (double)x;
                     double sY = (double)y;
                     double b1 = ((x2-sX) * (y3-sY) - (x3-sX) * (y2-sY)) / b0;
                     double b2 = ((x3-sX) * (y1-sY) - (x1-sX) * (y3-sY)) / b0;
                     double b3 = ((x1-sX) * (y2-sY) - (x2-sX) * (y1-sY)) / b0;
                     
                     // "Snap" barycentric coordinates.
                     if (b1 > 1.0)
                     {
                        double diff = b1 - 1.0;
                        b1 = 1.0;
                        b2 += (diff/2.0);
                        b3 += (diff/2.0);
                     }
                     if (b1 < 0.0)
                     {
                        double diff = b1;
                        b1 = 0.0;
                        b2 += (diff/2.0);
                        b3 += (diff/2.0);
                     }
                     if (b2 > 1.0)
                     {
                        double diff = b2 - 1.0;
                        b1 = 1.0;
                        b1 += (diff/2.0);
                        b3 += (diff/2.0);
                     }
                     if (b2 < 0.0)
                     {
                        double diff = b2;
                        b2 = 0.0;
                        b1 += (diff/2.0);
                        b3 += (diff/2.0);
                     }
                     if (b3 > 1.0)
                     {
                        double diff = b3 - 1.0;
                        b3 = 1.0;
                        b2 += (diff/2.0);
                        b1 += (diff/2.0);
                     }
                     if (b3 < 0.0)
                     {
                        double diff = b3;
                        b3 = 0.0;
                        b2 += (diff/2.0);
                        b1 += (diff/2.0);
                     }

                     // Compute position and normal
                     NmRawPointD pos;
                     NmRawPointD norm;
                     BaryInterpolate (lTri, b1, b2, b3, pos.v, norm.v);

                     // First see if we even have an intersection.
                     double newNorm[3];
                     double newPos[3];
                     double newOcclusion = 0.0;
                     double newDisplacement = 0.0;
                     if (FindBestIntersection (pos, norm, octree, highTris,
                                               hTangentSpace, bumpMap, 
                                               bumpHeight, bumpWidth, newNorm, 
                                               newPos, &newDisplacement))
                     {
                        // Normalize the new normal
                        Normalize (newNorm);
                        
                        // Do bent normal/occlusion if needed.
                        if (gOcclusion || gBentNormal)
                        {
                           // First do the ray casting.
                           NmRawPointD bentNormal;
                           ComputeOcclusion ((NmRawPointD*)(newPos),
                                             (NmRawPointD*)(newNorm),
                                             highNumTris, highTris, octree,
                                             numRays, rays, rayWeights,
                                             &bentNormal, &newOcclusion);
                           
                           // Replace normal with bent normal if needed
                           if (gBentNormal)
                           {
                              newNorm[0] += bentNormal.x;
                              newNorm[1] += bentNormal.y;
                              newNorm[2] += bentNormal.z;
                           }
                        } // do ambient occlusion if needed

                        
                        // Convert to tangent space if needed
                        if (gInTangentSpace == true)
                        {
                           ConvertToTangentSpace (ts, newNorm, newNorm);
                        }

                        // Add it to the image.
                        int idx = y*gWidth*numComponents + x*numComponents;
                        img2[idx + 0] += (float)newNorm[0];
                        img2[idx + 1] += (float)newNorm[1];
                        img2[idx + 2] += (float)newNorm[2];
                        if (gOcclusion)
                        {
                           img2[idx + gOcclIdx] = (float)(newOcclusion);
                        }
                        if (gDisplacements)
                        {
                           img2[idx + gDispIdx] = (float)(newDisplacement);
                        }
                     } // end if we found a normal
                     // else don't add anything to either image.
                  } // end else find a "snapped" normal

                  // Spin!
                  ShowSpinner (prog);
               } // end for x
            } // end for y
         } // end for low res triangles
         if (gQuiet == NM_VERBOSE)
         {
            printf ("\r100.00%%\r");
         }

         // Write out some stats.
         NmPrint ("Maximum octree cells tested for a ray: %d\n", gMaxCellsTested);
         NmPrint ("Maximum triangles tested for a ray: %d\n", gMaxTrisTested);
         NmPrint ("Maximum triangles hit for a ray: %d\n", gMaxTrisHit);
         if (gOcclusion || gBentNormal)
         {
            NmPrint ("Maximum octree cells tested for occlusion ray: %d\n",
                     gAOMaxTrisTested);
            NmPrint ("Maximum triangles tested for occlusion ray: %d\n",
                     gAOMaxCellsTested);
         }

         // Now renormalize
         if (gEdgeCopy)
         {
            CopyEdges (img, img2, numComponents);
         }
         NormalizeImage (img, numComponents);
         
         // Fill unused areas based on surrounding colors to prevent artifacts.
         if (gExpandTexels)
         {
            DilateImage (&img, &img2, numComponents);
         }

         // Box filter image
         if (gBoxFilter)
         {
            BoxFilter (img, img2, gWidth, numComponents);
            float* i1 = img2;
            img2 = img;
            img = i1;
         }
      } // end if we are recomputing mip chains or this is the first time.
      else // Could do compute mip == box here, but it's our only option ATM.
      {
         BoxFilter (img2, img, lastWidth, numComponents);
         float* i1 = img2;
         img2 = img;
         img = i1;
      } // end box filter old image down for mip levels

      // Tack mip chain number onto name if needed then write image.
      char fName[256];
      GetOutputFilename (fName, outName, mipCount);
      WriteOutputImage (fName, img, numComponents);

      // Update gWidth & gHeight here to go to the next mip level
      ComputeNextMipLevel (&lastWidth, &lastHeight);
      mipCount++;
   } // end while we still have mip levels to generate.
   
   return 0;
} // end main

#define ERROR_LOG_FILE "NormalMapperLog.txt"
//=============================================================================
void 
NmErrorLog (const char *szFmt, ...)
{
   // If we are being silent don't print anything.
   if (gQuiet == NM_SILENT)
   {
      return;
   }

   // Print into a local buffer.
   char sz[4096], szMsg[4096];
   va_list va;
   va_start(va, szFmt);
   vsprintf(szMsg, szFmt, va);
   va_end(va);
   
   sprintf(sz, "%s", szMsg);
   
   sz[sizeof(sz)-1] = '\0';
   
#if 1
   printf (sz);
#endif
#ifdef _DEBUG
#if 1
   OutputDebugString(sz);
#endif
#endif
#if 0
   static bool first = true;
   FILE *fp;
   if (first == true)
   {
      fp = fopen(ERROR_LOG_FILE, "w");
      if (fp == NULL)
         return;
      first = false;
   }
   else
   {
      fp = fopen(ERROR_LOG_FILE, "a");
      if (fp == NULL)
         return;
   }
   
   fprintf (fp, sz);
   fclose (fp);
   fflush (stdout);
#endif
}

//=============================================================================
void 
NmErrorLog (char *szFmt)
{
   NmErrorLog((const char *)szFmt);
}
