//=============================================================================
// AtiOctree.cpp -- The implementation of the Octree routines.
//=============================================================================
// $File: //depot/3darg/Demos/Sushi/Math/AtiOctree.cpp $ $Revision: #4 $ $Author: gosselin $
//=============================================================================
// (C) 2003 ATI Research, Inc., All rights reserved.
//=============================================================================

#include <string.h>

#include "AtiOctree.h"

// The size of the initial stack for traversing the tree.
static const int32 ATI_INITIAL_OCTREE_STACK_SIZE = 64;
static const float32 ATI_BBOX_CUTOFF = 0.001f;

// EPSILON for cell creation, needs to be at least half as small as CUTOFF
static const float32 ATI_OCT_EPSILON = 0.000001f;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Cell Routines.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Construct a cell
///////////////////////////////////////////////////////////////////////////////
AtiOctreeCell::AtiOctreeCell (void)
{
   memset (&m_boundingBox, 0, sizeof (AtiOctBoundingBox));
   memset (&m_children, 0, sizeof (AtiOctreeCell*) * 8);
   m_numItems = 0;
   m_item = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Destroy this cell, and only this cell!
///////////////////////////////////////////////////////////////////////////////
AtiOctreeCell::~AtiOctreeCell (void)
{
   for (int32 c = 0; c < 8; c++)
   {
      delete m_children[c];
   }
   memset (&m_children, 0, sizeof (AtiOctreeCell*) * 8);
   memset (&m_boundingBox, 0, sizeof (AtiOctBoundingBox));
   m_numItems = 0;
   delete [] m_item;
   m_item = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Tree Routines.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Destroy this cell, and only this cell!
///////////////////////////////////////////////////////////////////////////////
AtiOctree::AtiOctree (void)
{
   m_root = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Destroy a tree
///////////////////////////////////////////////////////////////////////////////
AtiOctree::~AtiOctree (void)
{
   delete m_root;
   m_root = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Gets the bounding box for a given child.
///////////////////////////////////////////////////////////////////////////////
void
AtiOctree::GetBoundingBox (int32 childIndex, AtiOctBoundingBox *bbox, 
                           AtiOctreeCell* child)
{
   // Figure out min/maxs
   switch (childIndex)
   {
      case ATI_OCT_TREE_CHILD_PX_PY_PZ:
         child->m_boundingBox.minX = bbox->centerX - ATI_OCT_EPSILON;
         child->m_boundingBox.maxX = bbox->centerX + bbox->halfX + ATI_OCT_EPSILON;
         child->m_boundingBox.minY = bbox->centerY - ATI_OCT_EPSILON;
         child->m_boundingBox.maxY = bbox->centerY + bbox->halfY + ATI_OCT_EPSILON;
         child->m_boundingBox.minZ = bbox->centerZ - ATI_OCT_EPSILON;
         child->m_boundingBox.maxZ = bbox->centerZ + bbox->halfZ + ATI_OCT_EPSILON;
         break;

      case ATI_OCT_TREE_CHILD_PX_NY_PZ:
         child->m_boundingBox.minX = bbox->centerX - ATI_OCT_EPSILON;
         child->m_boundingBox.maxX = bbox->centerX + bbox->halfX + ATI_OCT_EPSILON;
         child->m_boundingBox.minY = bbox->centerY - bbox->halfY - ATI_OCT_EPSILON;
         child->m_boundingBox.maxY = bbox->centerY + ATI_OCT_EPSILON;
         child->m_boundingBox.minZ = bbox->centerZ - ATI_OCT_EPSILON;
         child->m_boundingBox.maxZ = bbox->centerZ + bbox->halfZ + ATI_OCT_EPSILON;
         break;

      case ATI_OCT_TREE_CHILD_PX_PY_NZ:
         child->m_boundingBox.minX = bbox->centerX - ATI_OCT_EPSILON;
         child->m_boundingBox.maxX = bbox->centerX + bbox->halfX + ATI_OCT_EPSILON;
         child->m_boundingBox.minY = bbox->centerY - ATI_OCT_EPSILON;
         child->m_boundingBox.maxY = bbox->centerY + bbox->halfY + ATI_OCT_EPSILON;
         child->m_boundingBox.minZ = bbox->centerZ - bbox->halfZ - ATI_OCT_EPSILON;
         child->m_boundingBox.maxZ = bbox->centerZ + ATI_OCT_EPSILON;
         break;

      case ATI_OCT_TREE_CHILD_PX_NY_NZ:
         child->m_boundingBox.minX = bbox->centerX - ATI_OCT_EPSILON;
         child->m_boundingBox.maxX = bbox->centerX + bbox->halfX + ATI_OCT_EPSILON;
         child->m_boundingBox.minY = bbox->centerY - bbox->halfY - ATI_OCT_EPSILON;
         child->m_boundingBox.maxY = bbox->centerY + ATI_OCT_EPSILON;
         child->m_boundingBox.minZ = bbox->centerZ - bbox->halfZ - ATI_OCT_EPSILON;
         child->m_boundingBox.maxZ = bbox->centerZ + ATI_OCT_EPSILON;
         break;

      case ATI_OCT_TREE_CHILD_NX_PY_PZ:
         child->m_boundingBox.minX = bbox->centerX - bbox->halfX - ATI_OCT_EPSILON;
         child->m_boundingBox.maxX = bbox->centerX + ATI_OCT_EPSILON;
         child->m_boundingBox.minY = bbox->centerY - ATI_OCT_EPSILON;
         child->m_boundingBox.maxY = bbox->centerY + bbox->halfY + ATI_OCT_EPSILON;
         child->m_boundingBox.minZ = bbox->centerZ - ATI_OCT_EPSILON;
         child->m_boundingBox.maxZ = bbox->centerZ + bbox->halfZ + ATI_OCT_EPSILON;
         break;

      case ATI_OCT_TREE_CHILD_NX_NY_PZ:
         child->m_boundingBox.minX = bbox->centerX - bbox->halfX - ATI_OCT_EPSILON;
         child->m_boundingBox.maxX = bbox->centerX + ATI_OCT_EPSILON;
         child->m_boundingBox.minY = bbox->centerY - bbox->halfY - ATI_OCT_EPSILON;
         child->m_boundingBox.maxY = bbox->centerY + ATI_OCT_EPSILON;
         child->m_boundingBox.minZ = bbox->centerZ - ATI_OCT_EPSILON;
         child->m_boundingBox.maxZ = bbox->centerZ + bbox->halfZ + ATI_OCT_EPSILON;
         break;

      case ATI_OCT_TREE_CHILD_NX_PY_NZ:
         child->m_boundingBox.minX = bbox->centerX - bbox->halfX - ATI_OCT_EPSILON;
         child->m_boundingBox.maxX = bbox->centerX + ATI_OCT_EPSILON;
         child->m_boundingBox.minY = bbox->centerY - ATI_OCT_EPSILON;
         child->m_boundingBox.maxY = bbox->centerY + bbox->halfY + ATI_OCT_EPSILON;
         child->m_boundingBox.minZ = bbox->centerZ - bbox->halfZ - ATI_OCT_EPSILON;
         child->m_boundingBox.maxZ = bbox->centerZ + ATI_OCT_EPSILON;
         break;

      case ATI_OCT_TREE_CHILD_NX_NY_NZ:
         child->m_boundingBox.minX = bbox->centerX - bbox->halfX - ATI_OCT_EPSILON;
         child->m_boundingBox.maxX = bbox->centerX + ATI_OCT_EPSILON;
         child->m_boundingBox.minY = bbox->centerY - bbox->halfY - ATI_OCT_EPSILON;
         child->m_boundingBox.maxY = bbox->centerY + ATI_OCT_EPSILON;
         child->m_boundingBox.minZ = bbox->centerZ - bbox->halfZ - ATI_OCT_EPSILON;
         child->m_boundingBox.maxZ = bbox->centerZ + ATI_OCT_EPSILON;
         break;

      default:
         // badness.
         {int32 i = 0;}
         break;
   }

   // Figure out center and half
   AtiOctBoundingBox* b = &(child->m_boundingBox);
   b->halfX = (b->maxX - b->minX) / 2.0f;
   b->centerX = b->halfX + b->minX;
   b->halfY = (b->maxY - b->minY) / 2.0f;
   b->centerY = b->halfY + b->minY;
   b->halfZ = (b->maxZ - b->minZ) / 2.0f;
   b->centerZ = b->halfZ + b->minZ;
} // end of GetBoundingBox 

///////////////////////////////////////////////////////////////////////////////
// Fill in the octree.
///////////////////////////////////////////////////////////////////////////////
bool8
AtiOctree::FillTree (int32 numItems, void* itemList,
                     AtiOctBoundingBox& boundingBox,
                     AtiPfnItemInBox fnItemInBox, int32 maxInBox,
                     AtiPfnOctProgress fnProgress)
{
   // First create the root node
   m_root = new AtiOctreeCell;
   memcpy (&(m_root->m_boundingBox), &boundingBox, sizeof (AtiOctBoundingBox));
   
   // Expand bounding box a little, just in case there's some floating point
   // strangeness with planes lying on planes.
   m_root->m_boundingBox.minX -= ATI_OCT_EPSILON;
   m_root->m_boundingBox.minY -= ATI_OCT_EPSILON;
   m_root->m_boundingBox.minZ -= ATI_OCT_EPSILON;
   m_root->m_boundingBox.maxX += ATI_OCT_EPSILON;
   m_root->m_boundingBox.maxY += ATI_OCT_EPSILON;
   m_root->m_boundingBox.maxZ += ATI_OCT_EPSILON;
   m_root->m_boundingBox.halfX = (m_root->m_boundingBox.maxX - m_root->m_boundingBox.minX) / 2.0f;
   m_root->m_boundingBox.halfY = (m_root->m_boundingBox.maxY - m_root->m_boundingBox.minY) / 2.0f;
   m_root->m_boundingBox.halfZ = (m_root->m_boundingBox.maxZ - m_root->m_boundingBox.minZ) / 2.0f;

   // Allocate items list for root
   m_root->m_numItems = 0;
   m_root->m_item = new int32 [numItems];
   if (m_root->m_item == NULL)
   {
      return FALSE;
   }
   memset (m_root->m_item, 0, sizeof (int32) * numItems);

   // Fill in items for root.
   for (int32 i = 0; i < numItems; i++)
   {
      // By calling this function when filling this array it lets the
      // calling program weed out some items. Useful in ambient occlusion
      // to have some items not block rays.
      if (fnItemInBox (i, itemList, &(m_root->m_boundingBox)) == TRUE)
      {
         m_root->m_item[m_root->m_numItems] = i;
         m_root->m_numItems++;
      }
   }

   // Allocate a stack to hold the list of cells we are working with.
   AtiOctreeCell** cell = new AtiOctreeCell* [ATI_INITIAL_OCTREE_STACK_SIZE];
   if (cell == NULL)
   {
      return FALSE;
   }
   int32 maxCells = ATI_INITIAL_OCTREE_STACK_SIZE;
   memset (cell, 0, sizeof (AtiOctreeCell*) * maxCells);

   // Now partition
   cell[0] = m_root;
   int32 numCells = 1;
   while (numCells > 0)
   {
      // Show some progress if requested.
      if (fnProgress != NULL)
      {
         fnProgress (0.0f);
      }
      
      // Take the one off the stack.
      numCells--;
      AtiOctreeCell* currCell = cell[numCells];
      
      // First see if we even need to create children
      if ( (currCell->m_numItems < maxInBox) ||
           (currCell->m_boundingBox.halfX <= ATI_BBOX_CUTOFF) ||
           (currCell->m_boundingBox.halfY <= ATI_BBOX_CUTOFF) ||
           (currCell->m_boundingBox.halfZ <= ATI_BBOX_CUTOFF) )
      {
         continue;
      }

      // Create children as needed and add them to our list.
      for (int32 c = 0; c < 8; c++)
      {
         // First create a new cell.
         currCell->m_children[c] = new AtiOctreeCell;
         if (currCell->m_children[c] == NULL)
         {
            delete [] cell;
            return FALSE;
         }
         AtiOctreeCell* child = currCell->m_children[c];
         memset (child, 0, sizeof (AtiOctreeCell));
         
         // Allocate items, use parent since that's the maximum we can have.
         child->m_numItems = 0;
         child->m_item = new int32 [currCell->m_numItems];
         if (child->m_item == NULL)
         {
            delete [] cell;
            return NULL;
         }
         memset (child->m_item, 0, sizeof (int32) * currCell->m_numItems);

         // Now figure out the bounding box.
         GetBoundingBox (c, &currCell->m_boundingBox, child);

         // Run through the items and see if they belong in this cell.
         for (int32 i = 0; i < currCell->m_numItems; i++)
         {
            if (fnItemInBox (currCell->m_item[i], itemList, 
                             &child->m_boundingBox) == TRUE)
            {
               child->m_item[child->m_numItems] = currCell->m_item[i];
               child->m_numItems++;
            }
         } // end for number of items in the current cell.

         // If we didn't find anything in the box, bail
         if (child->m_numItems < 1)
         {
            delete currCell->m_children[c];
            currCell->m_children[c] = NULL;
            child = NULL;
            continue;
         }

         // If we haven't reached our target item size add this into our list
         if ( (child->m_numItems > maxInBox) ||
              (currCell->m_boundingBox.halfX > ATI_BBOX_CUTOFF) ||
              (currCell->m_boundingBox.halfY > ATI_BBOX_CUTOFF) ||
              (currCell->m_boundingBox.halfZ > ATI_BBOX_CUTOFF) )
         {
            // Make sure we have enough room on our stack.
            if (numCells >= maxCells)
            {
               maxCells += ATI_INITIAL_OCTREE_STACK_SIZE;
               AtiOctreeCell** newCells = new AtiOctreeCell* [maxCells];
               memset (newCells, 0, sizeof (AtiOctreeCell*) * maxCells);
               memcpy (newCells, cell, sizeof (AtiOctreeCell*) * numCells);
               delete [] cell;
               cell = newCells;
               newCells = NULL;
            }

            // Add this item into our list.
            cell[numCells] = currCell->m_children[c];
            numCells++;
         }
         child = NULL;
      } // for c (number of children);
   } // While we still have cells.

   // Worked!
   delete [] cell;
   return TRUE;
} // end of FillTree
