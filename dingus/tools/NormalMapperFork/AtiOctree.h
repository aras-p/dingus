//==============================================================================
// AtiOctree.h -- Header for Octree routines.
//==============================================================================
// $File: //depot/3darg/Demos/Sushi/Math/AtiOctree.h $ $Revision: #2 $ $Author: gosselin $
//==============================================================================
// (C) 2003 ATI Research, Inc., All rights reserved.
//==============================================================================

#ifndef __ATI_OCTREE__H
#define __ATI_OCTREE__H

// INCLUDES ====================================================================
#include "Types.h"

// DEFINITIONS =================================================================

// Enumerations for various children.
// PX = Positive X
// PY = Positive Y
// PZ = Positive Z
// NX = Negative X
// NY = Negative Y
// NZ = Negative Z
enum
{
   ATI_OCT_TREE_CHILD_PX_PY_PZ = 0,
   ATI_OCT_TREE_CHILD_PX_NY_PZ,
   ATI_OCT_TREE_CHILD_PX_PY_NZ,
   ATI_OCT_TREE_CHILD_PX_NY_NZ,
   ATI_OCT_TREE_CHILD_NX_PY_PZ,
   ATI_OCT_TREE_CHILD_NX_NY_PZ,
   ATI_OCT_TREE_CHILD_NX_PY_NZ,
   ATI_OCT_TREE_CHILD_NX_NY_NZ,
};

// A bounding box structure
typedef union
{
   struct
   {
      float32 box[6];
      float32 centerHalf[6];
   };
   struct
   {
      float32 min[3];
      float32 max[3];
      float32 center[3];
      float32 half[3];
   };
   struct
   {
      float32 minX, minY, minZ;
      float32 maxX, maxY, maxZ;
      float32 centerX, centerY, centerZ;
      float32 halfX, halfY, halfZ;
   };
} AtiOctBoundingBox;

// A cell within the Octree.
class AtiOctreeCell
{
   public:
      // Bounding box for the cell.
      AtiOctBoundingBox m_boundingBox;

      // The eight children of this cell
      AtiOctreeCell* m_children[8];

      // List of stuff in this cell
      int32 m_numItems;
      int32* m_item;

	  bool  m_leaf;

      // Con/de-struction
      AtiOctreeCell (void);
      ~AtiOctreeCell (void);

   private:
      // Disable copying
      AtiOctreeCell& operator = (AtiOctreeCell& o) { return *this; };
};

// Returns TRUE if the item falls into the given box, FALSE otherwise.
typedef bool8 (* AtiPfnItemInBox)(int32 itemIndex, void* itemList,
                                  AtiOctBoundingBox* m_boundingBox);

// A callback to show some progress, right now it always passes 0.0.
typedef void (* AtiPfnOctProgress)(float32 progress);

// Interface into the tree itself.
class AtiOctree
{
   public:
      // Top cell in the octree.
      AtiOctreeCell* m_root;

      // Fill the tree
      bool8 FillTree (int32 numItems, void* itemList,
                      AtiOctBoundingBox& boundingBox,
                      AtiPfnItemInBox fnItemInBox, int32 maxInBox,
                      AtiPfnOctProgress fnProgress = NULL);

      // Con/de-struction
      AtiOctree (void);
      ~AtiOctree (void);

   private:
      // No void construction
      // Disable copying
      AtiOctree& operator = (AtiOctree& o) { return *this; };

      // Internal routine to chop up a bounding box.
      void GetBoundingBox (int32 childIndex, AtiOctBoundingBox *bbox, 
                           AtiOctreeCell* child);
};

#endif
