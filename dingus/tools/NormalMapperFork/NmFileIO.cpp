/******************************************************************************
 *  NmFileIO.cpp -- File IO routines for normal mapper.
 ******************************************************************************
 $Header: //depot/3darg/Tools/NormalMapper/NmFileIO.cpp#24 $
 ******************************************************************************
 *  (C) 2000 ATI Research, Inc.  All rights reserved.
 ******************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <math.h>

#include "NmFileIO.h"

#ifdef ATI_MAC_OS
 #include "MacSpecific.h"
#endif

using namespace std;

//#define NEW_WAY

#define EPSILON 1.0e-7
static const double ATI_VERT_TINY = EPSILON;
static const double ATI_NORM_TINY = EPSILON;
static const double ATI_TEX_TINY = EPSILON;
static const double ATI_TAN_ANGLE = 0.3;

// Compare function definition
typedef int (* PFNNMCOMPARED)(NmTangentPointD* v0, NmTangentPointD* v1);

////////////////////////////////////////////////////////////////////
// Write a block of triangles to the file
////////////////////////////////////////////////////////////////////
bool
NmWriteTriangles (FILE* fp, int numTris, NmRawTriangle* tris)
{
   // Write out header information
   NmHeader header;
   strncpy (header.hdr, NMF_HEADER_TAG, 4);
   header.size = sizeof (NmHeader) + sizeof (int) + 
                 (sizeof(NmRawTriangle) * numTris);
   
#ifdef ATI_MAC_OS
   NmHeader bsHeader;
   memcpy(&bsHeader, &header, sizeof(NmHeader));
   AtiByteSwap_NmHeader(&bsHeader, 1);
   if (fwrite (&bsHeader, sizeof (NmHeader), 1, fp) != 1)
#else  
   if (fwrite (&header, sizeof (NmHeader), 1, fp) != 1)
#endif
   {
      cerr << "Unable to write NMF header\n";
      return false;
   }

   // Write out out triangle chunk header
   strncpy (header.hdr, NMF_TRIANGLE_TAG, 4);
   header.size = sizeof (int) + sizeof(NmRawTriangle) * numTris;
   
#ifdef ATI_MAC_OS
   memcpy(&bsHeader, &header, sizeof(NmHeader));
   AtiByteSwap_NmHeader(&bsHeader, 1);   
   if (fwrite (&bsHeader, sizeof (NmHeader), 1, fp) != 1)
#else  
   if (fwrite (&header, sizeof (NmHeader), 1, fp) != 1)
#endif
   {
      cerr << "Unable to write triangle header\n";
      return false;
   }
   
   // Write the number of triangles.
#ifdef ATI_MAC_OS
   int bsNumTris = numTris;
   AtiByteSwap_int32(&bsNumTris, 1);
   if (fwrite (&bsNumTris, sizeof (int), 1, fp) != 1)
#else
   if (fwrite (&numTris, sizeof (int), 1, fp) != 1)
#endif
   {
      cerr << "Unable to write number of triangles\n";
      return false;
   }
   
   // Write the triangles.
#ifdef ATI_MAC_OS
   NmRawTriangle* bsTris = (NmRawTriangle*)malloc((numTris+1)*sizeof(NmRawTriangle));
   if (bsTris == NULL)
   {
      cerr << "Unable to write triangles\n";
      return false;
   }
   
   memcpy(bsTris, tris, numTris*sizeof(NmRawTriangle));
   AtiByteSwap_NmRawTriangle(bsTris, numTris);
     
   if (fwrite (bsTris, sizeof(NmRawTriangle), numTris, fp) != (unsigned)numTris)
   {
      cerr << "Unable to write triangles\n";
      //free(bsTris);
      return false;
   }

   free(bsTris);
#else
   if (fwrite (tris, sizeof(NmRawTriangle), numTris, fp) != (unsigned)numTris)
   {
      cerr << "Unable to write triangles\n";
      return false;
   }
#endif
     
   return true;
} // end NmWriteTriangles

////////////////////////////////////////////////////////////////////
// Read a block of triangles from the file
////////////////////////////////////////////////////////////////////
bool
NmReadTriangleChunk (FILE* fp, int* numTris, NmRawTriangle** tris)
{
   // Read the number of triangles.
   if (fread (numTris, sizeof (int), 1, fp) != 1)
   {
      cerr << "Unable to read number of triangles\n";
      return false;
   }
   
   #ifdef ATI_MAC_OS
    AtiByteSwap_int32(numTris, 1);
   #endif
   
   if ((*numTris) < 1)
   {
      cerr << "No triangles found in file\n";
      return false;
   }
   
   // Allocate space for triangles
   (*tris) = new NmRawTriangle [(*numTris)];
   if ((*tris) == NULL)
   {
      cerr << "Unable to allocate space for " << (*tris) << " triangles\n";
      return false;
   }

   // Now read the triangles into the allocated space.
   if (fread ((*tris), sizeof (NmRawTriangle), (*numTris), fp) != 
       (unsigned)(*numTris))
   {
      cerr << "Unable to read triangles\n";
#ifdef _DEBUG
      if (ferror (fp))
      {
         perror ("Read Triangles");
      }
      else if (feof (fp))
      {
         cerr << "End of file!\n";
      }
#endif
      return false;
   }
   
   #ifdef ATI_MAC_OS
    AtiByteSwap_NmRawTriangle((*tris), (*numTris));
   #endif
   
   // success
   return true;
}

////////////////////////////////////////////////////////////////////
// See if this file is an NMF file and it has a triangle chunk.
////////////////////////////////////////////////////////////////////
bool
NmFileHasTriangles (FILE* fp)
{
   NmHeader header;
   while (!feof (fp))
   {
      // Read chunk header.
      if (fread (&header, sizeof (NmHeader), 1, fp) != 1)
      {
         return false;
      }
      
      #ifdef ATI_MAC_OS
       AtiByteSwap_NmHeader(&header, 1);
      #endif

      // Check if it's the NMF chunk
      if (strncmp (header.hdr, NMF_HEADER_TAG, 4) == 0)
      {
        while (!feof (fp))
        {
           // Read chunk header.
           if (fread (&header, sizeof (NmHeader), 1, fp) != 1)
           {
              return false;
           }
           
           #ifdef ATI_MAC_OS
            AtiByteSwap_NmHeader(&header, 1);
           #endif

           // Check if it's the triangle chunk we are looking for.
           if (strncmp (header.hdr, NMF_TRIANGLE_TAG, 4) == 0)
           {
             return true;
           }
           else
           {
              // Skip over chunk since it's not what we want.
              if (fseek (fp, header.size, SEEK_CUR) != 0)
              {
                 return false;
              }
           }
        }
      }
      else
      {
         // Skip over chunk since it's not what we want.
         if (fseek (fp, header.size, SEEK_CUR) != 0)
         {
            return false;
         }
      }
   }
   // No NMF file chunk found.
   return false;
} // end NmFileHasTriangles

////////////////////////////////////////////////////////////////////
// Read a block of triangles from the file
////////////////////////////////////////////////////////////////////
bool
NmReadTriangles (FILE* fp, int* numTris, NmRawTriangle** tris)
{
   NmHeader header;
   while (!feof (fp))
   {
      // Read chunk header.
      if (fread (&header, sizeof (NmHeader), 1, fp) != 1)
      {
         cerr << "Unable to read header\n";
         return false;
      }
      
      #ifdef ATI_MAC_OS
       AtiByteSwap_NmHeader(&header, 1);
      #endif

      // Check if it's the NMF chunk
      if (strncmp (header.hdr, NMF_HEADER_TAG, 4) == 0)
      {
        while (!feof (fp))
        {
           // Read chunk header.
           if (fread (&header, sizeof (NmHeader), 1, fp) != 1)
           {
              cerr << "Unable to read header\n";
              return false;
           }
           
           #ifdef ATI_MAC_OS
            AtiByteSwap_NmHeader(&header, 1);
           #endif

           // Check if it's the triangle chunk we are looking for.
           if (strncmp (header.hdr, NMF_TRIANGLE_TAG, 4) == 0)
           {
              // We can currently on handle one triangle chunk.
              return NmReadTriangleChunk (fp, numTris, tris);
           }
           else
           {
              // Skip over chunk since it's not what we want.
              if (fseek (fp, header.size, SEEK_CUR) != 0)
              {
                 cerr << "Chunk size doesn't match file size\n";
                 return false;
              }
           }
        }
      }
      else
      {
         // Skip over chunk since it's not what we want.
         if (fseek (fp, header.size, SEEK_CUR) != 0)
         {
            cerr << "Chunk size doesn't match file size\n";
            return false;
         }
      }
   }
   cerr << "No NMF data found in file\n";
   return false;
} // end NmReadTriangles

////////////////////////////////////////////////////////////////////
// Converts an older (pre chunk) NMF file into the current format
////////////////////////////////////////////////////////////////////
bool 
NmConvertFile (FILE* fpIn, FILE* fpOut)
{
   int numTris; 
   NmRawTriangle* tris;
   if (!NmReadTriangleChunk (fpIn, &numTris, &tris))
   {
      cerr << "Unable to read input file\n";
      return false;
   }
   if (!NmWriteTriangles (fpOut, numTris, tris))
   {
      cerr << "Unable to write output file\n";
      return false;
   }
   return true;
}

////////////////////////////////////////////////////////////////////
// Compute tangent space for the given triangle list
////////////////////////////////////////////////////////////////////
bool 
NmComputeTangents (int numTris, NmRawTriangle* tris, NmRawTangentSpace** tan)
{
   // Check inputs
   if ((numTris == 0) || (tris == NULL) || (tan == NULL))
   {
      return false;
   }

   // First we need to allocate up the tangent space storage
   (*tan) = new NmRawTangentSpace [numTris];
   if ((*tan) == NULL)
   {
      return false;
   }

   // Cheat and use the double version to compute then convert into floats
   NmRawTangentSpaceD* tanD;
   if (!NmComputeTangentsD(numTris, tris, &tanD))
   {
      return false;
   }

   // Now find tangent space
   for (int i = 0; i < numTris; i++)
   {
      for (int j = 0; j < 3; j++)
      {
         (*tan)[i].tangent[j].x = (float)(tanD[i].tangent[j].x);
         (*tan)[i].tangent[j].y = (float)(tanD[i].tangent[j].y);
         (*tan)[i].tangent[j].z = (float)(tanD[i].tangent[j].z);
         (*tan)[i].binormal[j].x = (float)(tanD[i].binormal[j].x);
         (*tan)[i].binormal[j].y = (float)(tanD[i].binormal[j].y);
         (*tan)[i].binormal[j].z = (float)(tanD[i].binormal[j].z);
      }
   }
   delete [] tanD;
   return true;
} // end NmComputeTangents

//==========================================================================
// Normalize a vector
//==========================================================================
static inline void
NormalizeD (double v[3])
{
   double len = sqrt((v[0]*v[0])+(v[1]*v[1])+(v[2]*v[2]));
   if (len < EPSILON)
   {
      v[0] = 1.0;
      v[1] = 0.0;
      v[2] = 0.0;
   }
   else
   {
      v[0] = v[0]/len;
      v[1] = v[1]/len;
      v[2] = v[2]/len;
   }
}

//==========================================================================
// Calculates the dot product of two vectors.
//==========================================================================
static inline double
DotProduct3D (double vectorA[3], double vectorB[3])
{
   return( (vectorA[0] * vectorB[0]) + (vectorA[1] * vectorB[1]) +
           (vectorA[2] * vectorB[2]) );
} // end of DotProduct


//=============================================================================
// Compare the values
//=============================================================================
static int
NmCompareD (NmTangentPointD* v0, NmTangentPointD* v1)
{
#ifdef ATI_MAC_OS
   int i;
#endif // ATI_MAC_OS
     
   // Positions.
   for (int i = 0; i < 3; i++)
   {
      if (fabs(v0->vertex[i] - v1->vertex[i]) > ATI_VERT_TINY)
      {
         if (v0->vertex[i] > v1->vertex[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }

   // Normal
   for (i = 0; i < 3; i++)
   {
      if (fabs(v0->normal[i] - v1->normal[i]) > ATI_NORM_TINY)
      {
         if (v0->normal[i] > v1->normal[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }

   // Texture Coordinates.
   for (i = 0; i < 2; i++)
   {
      if (fabs(v0->uv[i] - v1->uv[i]) > ATI_TEX_TINY)
      {
         if (v0->uv[i] > v1->uv[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }

   }

   // Tangent
   double t0[3] = {v0->tangent[0], v0->tangent[1], v0->tangent[2]};
   NormalizeD (t0);
   double t1[3] = {v1->tangent[0], v1->tangent[1], v1->tangent[2]};
   NormalizeD (t1);
   double dp3 = DotProduct3D (t0, t1);
   if (dp3 < ATI_TAN_ANGLE)
   {
      for (int i = 0; i < 3; i++)
      {
         if (v0->tangent[i] > v1->tangent[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }

   // Binormal
   double b0[3] = {v0->binormal[0], v0->binormal[1], v0->binormal[2]};
   NormalizeD (b0);
   double b1[3] = {v1->binormal[0], v1->binormal[1], v1->binormal[2]};
   NormalizeD (b1);
   dp3 = DotProduct3D (b0, b1);
   if (dp3 < ATI_TAN_ANGLE)
   {
      for (int i = 0; i < 3; i++)
      {
         if (v0->binormal[i] > v1->binormal[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }
   return 0;
}

//=============================================================================
// Compare the values
//=============================================================================
static int
NmCompareWithTexD (NmTangentPointD* v0, NmTangentPointD* v1)
{
#ifdef ATI_MAC_OS
   int i;
#endif // ATI_MAC_OS

   // Positions.
   for (int i = 0; i < 3; i++)
   {
      if (fabs(v0->vertex[i] - v1->vertex[i]) > ATI_VERT_TINY)
      {
         if (v0->vertex[i] > v1->vertex[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }

   // Normal
   for (i = 0; i < 3; i++)
   {
      if (fabs(v0->normal[i] - v1->normal[i]) > ATI_NORM_TINY)
      {
         if (v0->normal[i] > v1->normal[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }

   // Texture Coordinates.
   for (i = 0; i < 2; i++)
   {
      if (fabs(v0->uv[i] - v1->uv[i]) > ATI_TEX_TINY)
      {
         if (v0->uv[i] > v1->uv[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }

   }

   // Tangent
   for (i = 0; i < 3; i++)
   {
      if (fabs(v0->tangent[i] - v1->tangent[i]) > ATI_NORM_TINY)
      {
         if (v0->tangent[i] > v1->tangent[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }

   // Binormal
   for (i = 0; i < 3; i++)
   {
      if (fabs(v0->binormal[i] - v1->binormal[i]) > ATI_NORM_TINY)
      {
         if (v0->binormal[i] > v1->binormal[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }

   // Matches
   return 0;
}

//=============================================================================
// Compare the values
//=============================================================================
static int
NmCompareNoTexD (NmTangentPointD* v0, NmTangentPointD* v1)
{
#ifdef ATI_MAC_OS
   int i;
#endif // ATI_MAC_OS

   // Positions.
   for (int i = 0; i < 3; i++)
   {
      if (fabs(v0->vertex[i] - v1->vertex[i]) > ATI_VERT_TINY)
      {
         if (v0->vertex[i] > v1->vertex[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }

   // Normal
   for (i = 0; i < 3; i++)
   {
      if (fabs(v0->normal[i] - v1->normal[i]) > ATI_NORM_TINY)
      {
         if (v0->normal[i] > v1->normal[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }

   // Tangent
   double t0[3] = {v0->tangent[0], v0->tangent[1], v0->tangent[2]};
   NormalizeD (t0);
   double t1[3] = {v1->tangent[0], v1->tangent[1], v1->tangent[2]};
   NormalizeD (t1);
   double dp3 = DotProduct3D (t0, t1);
   if (dp3 < ATI_TAN_ANGLE)
   {
      for (int i = 0; i < 3; i++)
      {
         if (v0->tangent[i] > v1->tangent[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }

   // Binormal
   double b0[3] = {v0->binormal[0], v0->binormal[1], v0->binormal[2]};
   NormalizeD (b0);
   double b1[3] = {v1->binormal[0], v1->binormal[1], v1->binormal[2]};
   NormalizeD (b1);
   dp3 = DotProduct3D (b0, b1);
   if (dp3 < ATI_TAN_ANGLE)
   {
      for (int i = 0; i < 3; i++)
      {
         if (v0->binormal[i] > v1->binormal[i])
         {
            return -1;
         }
         else
         {
            return 1;
         }
      }
   }
   return 0;
}

//=============================================================================
// Binary traversal function
//=============================================================================
static int 
NmIndexBinaryTraverseD (NmTangentPointD* value, // The reference element
                        NmTangentPointD* data,// pointer to the indexed data
                        int* indices,         // pointer index
                        int count,            // number of items in the array
                        int* result,          // The result of the last compare
                        PFNNMCOMPARED compare) // Compare function. 
{
   int high;
   int low;
   int mid;
   int nextMid;
   
   high = count;
   low = 0;
   mid = low + ((high - low) >> 1);
   *result = -1;

   while (low != high) 
   {
      *result = compare (value, &(data[indices[mid]])); 
      if (*result == 0)
      {
         break;
      }
      else if (*result < 0)
      {
         high = mid;
      }
      else
      {
         low = mid;
      }
     
      nextMid = low + ((high - low) >> 1);
      if (mid == nextMid)
      {
         break;
      }

      mid = nextMid;
   }

   return mid;
}

#define DcVec2Add(r, a, b)           ((r)[0] = (a)[0] + (b)[0], (r)[1] = (a)[1] + (b)[1])
#define DcVec2AddTo(a, b)            ((a)[0] += (b)[0], (a)[1] += (b)[1])
#define DcVec2AddScalar(r, a, b)     ((r)[0] = (a)[0] + (b), (r)[1] = (a)[1] + (b))
#define DcVec2AddScalarTo(a, b)      ((a)[0] += (b), (a)[1] += (b))
#define DcVec2Copy(a, b)             ((b)[0] = (a)[0], (b)[1] += (a)[1])
#define DcVec2Divide(r, a, b)        ((r)[0] = (a)[0] / (b)[0], (r)[1] = (a)[1] / (b)[1])
#define DcVec2DivideBy(a, b)         ((a)[0] /= (b)[0], (a)[1] /= (b)[1])
#define DcVec2DivideByScalar(v, s)   ((v)[0] /= s, (v)[1] /= s)
#define DcVec2DivideScalar(r, v, s)  ((r)[0] = (v)[0] / s, (r)[1] = (v)[1] / s)
#define DcVec2MidPoint(c, a, b)      ((r)[0] = (b)[0] + ((a)[0] - (b)[0]) * 0.5f, (r)[1] = (b)[1] + ((a)[1] - (b)[1]) * 0.5f)
#define DcVec2Mult(a, b, c)          ((c)[0] = (a)[0] * (b)[0], (c)[1] = (a)[1] * (b)[1])
#define DcVec2MultBy(a, b)           ((a)[0] *= (b)[0], (a)[1] *= (b)[1])
#define DcVec2MultByScalar(v, s)     ((v)[0] *= s, (v)[1] *= s)
#define DcVec2MultScalar(r, v, s)    ((r)[0] = (v)[0] * s, (r)[1] = (v)[1] * s)
#define DcVec2Mad(r, a, b, c)        ((r)[0] = (a)[0] * (b)[0] + (c)[0], (r)[1] = (a)[1] * (b)[1] + (c)[1])
#define DcVec2Negate(a, b)           ((b)[0] = -(a)[0], (b)[1] = -(a)[1])
#define DcVec2Scale(r, v, s)         ((r)[0] = (v)[0] * s, (r)[1] = (v)[1] * s)
#define DcVec2ScaleBy(v, s)          ((v)[0] *= s, (v)[1] *= s)
#define DcVec2Set(v, x, y)           ((v)[0] = x, (v)[1] = y)
#define DcVec2Subtract(r, a, b)      ((r)[0] = (a)[0] - (b)[0], (r)[1] = (a)[1] - (b)[1])
#define DcVec2SubtractFrom(a, b)     ((a)[0] -= (b)[0], (a)[1] -= (b)[1])
#define DcVec2Magnitude(v)           ( sqrtf((v)[0] * (v)[0] + (v)[1] * (v)[1]))
#define DcVec2MagnitudeDouble(v)     ( sqrt((v)[0] * (v)[0] + (v)[1] * (v)[1]))
#define DcVec2NormalizeDouble(v)     { double __mag = 1.0 / DcVec2MagnitudeDouble(v); \
                                      (v)[0] *= __mag; (v)[1] *= __mag; }
#define DcVec2Normalize(v)           { float __mag = 1.0f / (float)DcVec2Magnitude(v); \
                                      (v)[0] *= __mag; (v)[1] *= __mag; }


#define DcVecAdd(r, a, b)            ((r)[0] = (a)[0] + (b)[0], (r)[1] = (a)[1] + (b)[1], (r)[2] = (a)[2] + (b)[2])
#define DcVecAddTo(a, b)             ((a)[0] += (b)[0], (a)[1] += (b)[1], (a)[2] += (b)[2])
#define DcVecAddScalar(r, a, b)      ((r)[0] = (a)[0] + (b), (r)[1] = (a)[1] + (b), (r)[2] = (a)[2] + (b))
#define DcVecAddScalarTo(a, b)       ((a)[0] += (b), (a)[1] += (b), (a)[2] += (b))
#define DcVecCopy(a, b)              ((b)[0] = (a)[0], (b)[1] += (a)[1], (b)[2] += (a)[2])
#define DcVecCross(r, a, b)          ((r)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1], (r)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2], (r)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0])
#define DcVecDivide(r, a, b)         ((r)[0] = (a)[0] / (b)[0], (r)[1] = (a)[1] / (b)[1], (r)[2] = (a)[2] / (b)[2])
#define DcVecDivideBy(a, b)          ((a)[0] /= (b)[0], (a)[1] /= (b)[1], (a)[2] /= (b)[2])
#define DcVecDivideByScalar(v, s)    ((v)[0] /= s, (v)[1] /= s, (v)[2] /= s)
#define DcVecDivideScalar(r, v, s)   ((r)[0] = (v)[0] / s, (r)[1] = (v)[1] / s, (r)[2] = v[2] / s)
#define DcVecMidPoint(c, a, b)       ((r)[0] = (b)[0] + ((a)[0] - (b)[0]) * 0.5f, (r)[1] = (b)[1] + ((a)[1] - (b)[1]) * 0.5f, (r)[2] = (b)[2] + ((a)[2] - (b)[2]) * 0.5f)
#define DcVecMult(a, b, c)           ((c)[0] = (a)[0] * (b)[0], (c)[1] = (a)[1] * (b)[1], (c)[2] = (a)[2] * (b)[2])
#define DcVecMultBy(a, b)            ((a)[0] *= (b)[0], (a)[1] *= (b)[1], (a)[2] *= (b)[2])
#define DcVecMultByScalar(v, s)      ((v)[0] *= s, (v)[1] *= s, (v)[2] *= s)
#define DcVecMultScalar(r, v, s)     ((r)[0] = (v)[0] * s, (r)[1] = (v)[1] * s, (r)[2] = v[2] * s)
#define DcVecMad(r, a, b, c)         ((r)[0] = (a)[0] * (b)[0] + (c)[0], (r)[1] = (a)[1] * (b)[1] + (c)[1], (r)[2] = (a)[2] * (b)[2] + (c)[2])
#define DcVecNegate(a)               ((a)[0] = -(a)[0], (a)[1] = -(a)[1], (a)[2] = -(a)[2])
#define DcVecScale(r, v, s)          ((r)[0] = (v)[0] * s, (r)[1] = (v)[1] * s, (r)[2] = v[2] * s)
#define DcVecScaleBy(v, s)           ((v)[0] *= s, (v)[1] *= s, (v)[2] *= s)
#define DcVecSet(v, x, y, z)         ((v)[0] = x, (v)[1] = y, (v)[2] = z)
#define DcVecSubtract(r, a, b)       ((r)[0] = (a)[0] - (b)[0], (r)[1] = (a)[1] - (b)[1], (r)[2] = (a)[2] - (b)[2])
#define DcVecSubtractFrom(a, b)      ((a)[0] -= (b)[0], (a)[1] -= (b)[1], (a)[2] -= (b)[2])
#define DcVecMagnitude(v)            ( sqrtf((v)[0] * (v)[0] + (v)[1] * (v)[1] + (v)[2] * (v)[2]))
#define DcVecMagnitudeDouble(v)      ( sqrt((v)[0] * (v)[0] + (v)[1] * (v)[1] + (v)[2] * (v)[2]))
#define DcVecNormalizeDouble(v,def)  { double __mag = DcVecMagnitudeDouble(v); \
                                       if (__mag < EPSILON) {(v)[0] = (def)[0]; (v)[1] = (def)[1]; (v)[2] = (def)[2];} else \
                                       {__mag = 1.0/__mag;(v)[0] *= __mag; (v)[1] *= __mag; (v)[2] *= __mag;} }
#define DcVecNormalize(v)            { float __mag = 1.0f / (float)DcVecMagnitude(v); \
                                       (v)[0] *= __mag; (v)[1] *= __mag; (v)[2] *= __mag; }
#define DcVecLineLength(a, b)        ( sqrtf(((a)[0] - (b)[0]) * ((a)[0] - (b)[0]) + \
                                             ((a)[1] - (b)[1]) * ((a)[1] - (b)[1]) + \
                                             ((a)[2] - (b)[2]) * ((a)[2] - (b)[2])))
#define DcVecLerp(r, a, b, c)        { DcVecSubtract(r, b, a);  \
                                       DcVecMultByScalar(r, c);   \
                                       DcVecAddTo(r, a); } 

#define DcVecDotProduct(a, b)        ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])
#define DcVecDot4(a, b)              ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2] + (a)[3] * (b)[3])
#define DcVecDot3(a, b)              ((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])

#define DC_INVALID_ARG false
#define DC_OK true

bool DcTangentSpace(float* v1, float* v2, float* v3, 
                   float* t1, float* t2, float* t3, 
                   double* normal, double *tangent, double *binormal)
{
   typedef struct
   {
      double    v[3];
      double    t[2];
   } tanSpaceVert;
   
   tanSpaceVert   verts[3];
   tanSpaceVert   tempVert;
   double          dot;
   double          interp;
   double          tempVector[3];

   //===================================//
   // make sure the arguments are valid //
   //===================================//
   if ((v1 != NULL) && 
       (v2 != NULL) && 
       (v3 != NULL) &&
       (t1 != NULL) && 
       (t2 != NULL) && 
       (t3 != NULL) &&
       (normal != NULL) && 
       (tangent != NULL) && 
       (binormal != NULL))
   {
      //========================================//
      // fill in the tangent space vertex array //
      //========================================//
      verts[0].v[0] = v1[0];
      verts[0].v[1] = v1[1];
      verts[0].v[2] = v1[2];
      verts[0].t[0] = t1[0];
      verts[0].t[1] = t1[1];

      verts[1].v[0] = v2[0];
      verts[1].v[1] = v2[1];
      verts[1].v[2] = v2[2];
      verts[1].t[0] = t2[0];
      verts[1].t[1] = t2[1];

      verts[2].v[0] = v3[0];
      verts[2].v[1] = v3[1];
      verts[2].v[2] = v3[2];
      verts[2].t[0] = t3[0];
      verts[2].t[1] = t3[1];

      //============================//
      // compute the tangent vector //
      //============================//
      {
         //=======================================================//
         // sort the vertices based on their t texture coordinate //
         //=======================================================//
         if (verts[0].t[1] < verts[1].t[1])
         {
            tempVert = verts[0];
            verts[0] = verts[1];
            verts[1] = tempVert;
         }
         if (verts[0].t[1] < verts[2].t[1])
         {
            tempVert = verts[0];
            verts[0] = verts[2];
            verts[2] = tempVert;
         }
         if (verts[1].t[1] < verts[2].t[1])
         {
            tempVert = verts[1];
            verts[1] = verts[2];
            verts[2] = tempVert;
         }

         //=======================================================================//
         // compute the parametric offset along edge02 to the middle t coordinate //
         //=======================================================================//
         if (fabs((verts[2].t[1] - verts[0].t[1])) < EPSILON)
         {
            interp = 1.0;
         }
         else
         {
            interp = (verts[1].t[1] - verts[0].t[1]) / (verts[2].t[1] - verts[0].t[1]);
         }

         //============================================================================//
         // use the interpolation paramter to compute the vertex position along edge02 //
         // that coresponds to the same t coordinate as v[1]                           //
         //============================================================================//
         DcVecLerp(tempVector, verts[0].v, verts[2].v, interp);
      
         //========================================//
         // compute the interpolated s coord value //
         //========================================//
         interp = verts[0].t[0] + (verts[2].t[0] - verts[0].t[0]) * interp;

         //===========================================================================//
         // the tangent vector is the ray from the middle vertex to the lerped vertex //
         //===========================================================================//
         DcVecSubtract(tangent, tempVector, verts[1].v);

         //=====================================================//
         // make sure the tangent points in the right direction //
         //=====================================================//
         if (interp < verts[1].t[0])
         {
            DcVecNegate(tangent);
         }

         //=============================================================//
         // make sure the tangent vector is perpendicular to the normal //
         //=============================================================//
         dot = DcVecDot3(normal, tangent);            
         tangent[0] = tangent[0] - normal[0] * dot;
         tangent[1] = tangent[1] - normal[1] * dot;
         tangent[2] = tangent[2] - normal[2] * dot;

         //==============================//
         // normalize the tangent vector //
         //==============================//
         static double defTan[3] = {1.0, 0.0, 0.0};
         DcVecNormalizeDouble(tangent, defTan);
      }

      //=============================//
      // compute the binormal vector //
      //=============================//
      {
         //=======================================================//
         // sort the vertices based on their s texture coordinate //
         //=======================================================//
         if (verts[0].t[0] < verts[1].t[0])
         {
            tempVert = verts[0];
            verts[0] = verts[1];
            verts[1] = tempVert;
         }
         if (verts[0].t[0] < verts[2].t[0])
         {
            tempVert = verts[0];
            verts[0] = verts[2];
            verts[2] = tempVert;
         }
         if (verts[1].t[0] < verts[2].t[0])
         {
            tempVert = verts[1];
            verts[1] = verts[2];
            verts[2] = tempVert;
         }

         //=======================================================================//
         // compute the parametric offset along edge02 to the middle s coordinate //
         //=======================================================================//
         if (fabs((verts[2].t[0] - verts[0].t[0])) < EPSILON)
         {
            interp = 1.0;
         }
         else
         {
            interp = (verts[1].t[0] - verts[0].t[0]) / (verts[2].t[0] - verts[0].t[0]);
         }

         //============================================================================//
         // use the interpolation paramter to compute the vertex position along edge02 //
         // that coresponds to the same t coordinate as v[1]                           //
         //============================================================================//
         DcVecLerp(tempVector, verts[0].v, verts[2].v, interp);
      
         //========================================//
         // compute the interpolated t coord value //
         //========================================//
         interp = verts[0].t[1] + (verts[2].t[1] - verts[0].t[1]) * interp;

         //============================================================================//
         // the binormal vector is the ray from the middle vertex to the lerped vertex //
         //============================================================================//
         DcVecSubtract(binormal, tempVector, verts[1].v);

         //======================================================//
         // make sure the binormal points in the right direction //
         //======================================================//
         if (interp < verts[1].t[1])
         {
            DcVecNegate(binormal);
         }

         //==============================================================//
         // make sure the binormal vector is perpendicular to the normal //
         //==============================================================//
         dot = DcVecDot3(normal, binormal);            
         binormal[0] = binormal[0] - normal[0] * dot;
         binormal[1] = binormal[1] - normal[1] * dot;
         binormal[2] = binormal[2] - normal[2] * dot;
      
         //===============================//
         // normalize the binormal vector //
         //===============================//
         static double defBi[3] = {0.0, 0.0, 1.0};
         DcVecNormalizeDouble(binormal, defBi);
      }
   }
   else
   {
      return DC_INVALID_ARG;
   }

   return DC_OK;
}

////////////////////////////////////////////////////////////////////
// Compute tangent space for the given triangle
////////////////////////////////////////////////////////////////////
static void
ComputeTangentVectorsD (NmRawTriangle* pgon, int idx,
                        double tan[3], double norm[3], double binorm[3])
{
   // Clear outputs.
   tan[0] = 1.0; tan[1] = 0.0; tan[2] = 0.0;
   binorm[0] = 0.0; binorm[1] = 0.0; binorm[2] = 1.0;
   norm[0] = pgon->norm[idx].x;
   norm[1] = pgon->norm[idx].y;
   norm[2] = pgon->norm[idx].z;

   // Make sure we have valid data.
   if (pgon->texCoord[0].u == 0.0 && 
       pgon->texCoord[1].u == 0.0 && 
       pgon->texCoord[2].u == 0.0)
   {
      return;
   }

   DcTangentSpace(pgon->vert[0].v, pgon->vert[1].v, pgon->vert[2].v, 
                  pgon->texCoord[0].uv, pgon->texCoord[1].uv,
                  pgon->texCoord[2].uv, norm, tan, binorm);
} // end ComputeTangentVectors

////////////////////////////////////////////////////////////////////
// Copy data into a point structure
////////////////////////////////////////////////////////////////////
static void
NmCopyPointD (NmRawTriangle* tri, int v, NmTangentPointD* point)
{
   point->vertex[0] = tri->vert[v].x;
   point->vertex[1] = tri->vert[v].y;
   point->vertex[2] = tri->vert[v].z;
   point->normal[0] = tri->norm[v].x;
   point->normal[1] = tri->norm[v].y;
   point->normal[2] = tri->norm[v].z;
   point->uv[0] = tri->texCoord[v].u;
   point->uv[1] = tri->texCoord[v].v;
   double norm[3];
   ComputeTangentVectorsD (tri, v, point->tangent, norm, point->binormal);
   point->count = 1;
}

////////////////////////////////////////////////////////////////////
// Copy data into a point structure
////////////////////////////////////////////////////////////////////
static void
NmCopyPointTangentD (NmRawTriangle* tri, NmRawTangentSpaceD* tan,
                     int v, NmTangentPointD* point)
{
   point->vertex[0] = tri->vert[v].x;
   point->vertex[1] = tri->vert[v].y;
   point->vertex[2] = tri->vert[v].z;
   point->normal[0] = tri->norm[v].x;
   point->normal[1] = tri->norm[v].y;
   point->normal[2] = tri->norm[v].z;
   point->uv[0] = tri->texCoord[v].u;
   point->uv[1] = tri->texCoord[v].v;
   point->tangent[0] = tan->tangent[v].x;
   point->tangent[1] = tan->tangent[v].y;
   point->tangent[2] = tan->tangent[v].z;
   point->binormal[0] = tan->binormal[v].x;
   point->binormal[1] = tan->binormal[v].y;
   point->binormal[2] = tan->binormal[v].z;
   point->count = 1;
}

////////////////////////////////////////////////////////////////////
// Insert a point and get it's index.
////////////////////////////////////////////////////////////////////
static int
NmInsertD (NmRawTriangle* tri, int v, int* num, int* sortIndex,
           NmTangentPointD* point)
{
   // Make sure we have some stuff to check.
   int ret = 0;
   if ((*num) > 0)
   {
      // Copy point into available slot.
      NmTangentPointD* pt = &(point[(*num)]);
      NmCopyPointD (tri, v, pt);

      // Search for it.
      int compValue;
#ifdef NEW_WAY
      int pos = NmIndexBinaryTraverseD (pt, point, sortIndex, (*num),
                                        &compValue, NmCompareNoTexD);
#else
      int pos = NmIndexBinaryTraverseD (pt, point, sortIndex, (*num),
                                        &compValue, NmCompareD);
#endif
      
      // Now see if we need to insert.
      if (compValue == 0)
      {
         point[sortIndex[pos]].tangent[0] += pt->tangent[0];
         point[sortIndex[pos]].tangent[1] += pt->tangent[1];
         point[sortIndex[pos]].tangent[2] += pt->tangent[2];
         point[sortIndex[pos]].binormal[0] += pt->binormal[0];
         point[sortIndex[pos]].binormal[1] += pt->binormal[1];
         point[sortIndex[pos]].binormal[2] += pt->binormal[2];
         point[sortIndex[pos]].count++;
         ret = sortIndex[pos];
      }
      else if (compValue < 0) // we are inserting before this index
      {
         ret = (*num);
         memmove (&(sortIndex[pos + 1]), &(sortIndex[pos]), 
                  ((*num) - pos) * sizeof(int));
         
         sortIndex[pos] = (*num);
         (*num)++;
      }
      else // we are appending after this index
      {
         ret = (*num);
         if (pos < ((*num) - 1))
         {
            memmove(&(sortIndex[pos + 2]), &(sortIndex[pos + 1]), 
                    ((*num) - pos - 1) * sizeof(int));
         }
         sortIndex[pos + 1] = (*num);
         (*num)++;
      }
   }
   else
   {  // First point just add it into our list.
      NmCopyPointD (tri, v, &(point[(*num)]));
      sortIndex[(*num)] = 0;
      ret = (*num);
      (*num)++;
   }
   return ret;
}


////////////////////////////////////////////////////////////////////
// Insert a point and get it's index.
////////////////////////////////////////////////////////////////////
static int
NmInsertForVBD (NmRawTriangle* tri, NmRawTangentSpaceD* tan, 
                int v, int* num, int* sortIndex, NmTangentPointD* point)
{
   // Make sure we have some stuff to check.
   int ret = 0;
   if ((*num) > 0)
   {
      // Copy point into available slot.
      NmTangentPointD* pt = &(point[(*num)]);
      NmCopyPointTangentD (tri, tan, v, pt);

      // Search for it.
      int compValue;
      int pos = NmIndexBinaryTraverseD (pt, point, sortIndex, (*num),
                                        &compValue, NmCompareWithTexD);
      
      // Now see if we need to insert.
      if (compValue < 0) // we are inserting before this index
      {
         ret = (*num);
         memmove (&(sortIndex[pos + 1]), &(sortIndex[pos]), 
                  ((*num) - pos) * sizeof(int));
         
         sortIndex[pos] = (*num);
         (*num)++;
      }
      else // we are appending after this index
      {
         ret = (*num);
         if (pos < ((*num) - 1))
         {
            memmove(&(sortIndex[pos + 2]), &(sortIndex[pos + 1]), 
                    ((*num) - pos - 1) * sizeof(int));
         }
         sortIndex[pos + 1] = (*num);
         (*num)++;
      }
   }
   else
   {  // First point just add it into our list.
      NmCopyPointTangentD (tri, tan, v, &(point[(*num)]));
      sortIndex[(*num)] = 0;
      ret = (*num);
      (*num)++;
   }
   return ret;
}

////////////////////////////////////////////////////////////////////
// Compute tangent space for the given triangle list
////////////////////////////////////////////////////////////////////
bool 
NmComputeTangentsD (int numTris, NmRawTriangle* tris, NmRawTangentSpaceD** tan)
{
   // Check inputs
   if ((numTris == 0) || (tris == NULL) || (tan == NULL))
   {
      return false;
   }

   // First we need to allocate up the tangent space storage
   (*tan) = new NmRawTangentSpaceD [numTris];
   if ((*tan) == NULL)
   {
      return false;
   }

   // Allocate storage structures
   NmIndex* index = new NmIndex [numTris];
   if (index == NULL)
   {
      return false;
   }
   int* sortIndex = new int [numTris*3]; // Brute force it
   NmTangentPointD* point = new NmTangentPointD [numTris*3]; // Brute force it
   if (point == NULL)
   {
      return false;
   }
   
   // Now go through finding matching vertices and computing tangents.
   int count = 0;
   int i;
   for (i = 0; i < numTris; i++)
   {
      for (int j = 0; j < 3; j++)
      {
         index[i].idx[j] = NmInsertD (&tris[i], j, &count, sortIndex, point);
      }      
   }

   // Next we renormalize
   for (i = 0; i < count; i++)
   {
      point[i].tangent[0] = point[i].tangent[0]/(double)point[i].count;
      point[i].tangent[1] = point[i].tangent[1]/(double)point[i].count;
      point[i].tangent[2] = point[i].tangent[2]/(double)point[i].count;
      NormalizeD (point[i].tangent);

      point[i].binormal[0] = point[i].binormal[0]/(double)point[i].count;
      point[i].binormal[1] = point[i].binormal[1]/(double)point[i].count;
      point[i].binormal[2] = point[i].binormal[2]/(double)point[i].count;
      NormalizeD (point[i].binormal);
   }

   // Copy tangent data into structures
   for (i = 0; i < numTris; i++)
   {
      for (int j = 0; j < 3; j++)
      {
         (*tan)[i].tangent[j].x = point[index[i].idx[j]].tangent[0];
         (*tan)[i].tangent[j].y = point[index[i].idx[j]].tangent[1];
         (*tan)[i].tangent[j].z = point[index[i].idx[j]].tangent[2];
         (*tan)[i].binormal[j].x = point[index[i].idx[j]].binormal[0];
         (*tan)[i].binormal[j].y = point[index[i].idx[j]].binormal[1];
         (*tan)[i].binormal[j].z = point[index[i].idx[j]].binormal[2];
      }
   }

   // Clean up
   delete [] index;
   delete [] sortIndex;
   delete [] point;

   // Success!
   return true;
} // end NmComputeTangent

////////////////////////////////////////////////////////////////////
// Turn raw triangles into vertex/index buffers.
////////////////////////////////////////////////////////////////////
bool 
NmCreateVertexBuffers (int numTris, NmRawTriangle* tris, 
                       int* numVerts, NmTangentPointD** verts,
                       NmIndex** indices)
{
   // Check inputs
   if ((numTris == 0) || (tris == NULL) || (verts == NULL) ||
       (indices == NULL))
   {
      return false;
   }

#ifdef NEW_WAY
   // First compute tangents
   NmRawTangentSpaceD* tan;
   if (!NmComputeTangentsD (numTris, tris, &tan))
   {
      return false;
   }
#endif

   // Allocate storage structures
   (*indices) = new NmIndex [numTris];
   if ((*indices) == NULL)
   {
      return false;
   }
   NmIndex* index = (*indices);
   int* sortIndex = new int [numTris*3]; // Brute force it
   if (sortIndex == NULL)
   {
      return false;
   }
   (*verts) = new NmTangentPointD [numTris*3]; // Brute force it
   if ((*verts) == NULL)
   {
      return false;
   }
   NmTangentPointD* point = (*verts);
   
   // Now go through finding unique vertices.
   int count = 0;
   int i;
   for (i = 0; i < numTris; i++)
   {
      for (int j = 0; j < 3; j++)
      {
#ifdef NEW_WAY
         index[i].idx[j] = NmInsertForVBD (&tris[i], &tan[i], j, &count,
                                           sortIndex, point);
#else
         index[i].idx[j] = NmInsertD (&tris[i], j, &count, sortIndex, point);
#endif
      }      
   }
   (*numVerts) = count;

   // Next we renormalize
#ifndef NEW_WAY
   for (i = 0; i < count; i++)
   {
      point[i].tangent[0] = point[i].tangent[0]/(double)point[i].count;
      point[i].tangent[1] = point[i].tangent[1]/(double)point[i].count;
      point[i].tangent[2] = point[i].tangent[2]/(double)point[i].count;
      NormalizeD (point[i].tangent);

      point[i].binormal[0] = point[i].binormal[0]/(double)point[i].count;
      point[i].binormal[1] = point[i].binormal[1]/(double)point[i].count;
      point[i].binormal[2] = point[i].binormal[2]/(double)point[i].count;
      NormalizeD (point[i].binormal);
   }
#endif

   // Success!
   delete [] sortIndex;
#ifdef NEW_WAY
   delete [] tan;
#endif
   return true;
} // end of NmCreateVertexBuffers
