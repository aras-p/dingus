// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __GFX_UTILS_H
#define __GFX_UTILS_H

#include "../math/Matrix4x4.h"
#include "../math/Plane.h"


namespace dingus {

namespace gfx {


/**
 *  Computes texture projection matrix for world-space vertices.
 *
 *  @param projectorMatrix Projector's view*projection matrix.
 *  @param dest Destination matrix.
 */
void textureProjectionWorld(
	const SMatrix4x4& projectorMatrix, float texX, float texY,
	SMatrix4x4& dest );

/**
 *  Computes texture projection matrix for view-space vertices.
 *
 *  @param renderCameraMatrix Rendering camera's matrix.
 *  @param projectorMatrix Projector's view*projection matrix.
 *  @param dest Destination matrix.
 */
void textureProjectionView(
	const SMatrix4x4& renderCameraMatrix,
	const SMatrix4x4& projectorMatrix, float texX, float texY,
	SMatrix4x4& dest );





/**
 *  Compute projection matrix that does oblique near frustum clipping.
 *  Implemented as presented at nVidia developer site.
 *
 *  @param view View matrix.
 *	@param proj Projection matrix.
 *	@param clipPlane World space clip plane.
 *	@param clippedProj Resulting projection matrix that has clipPlane as near plane.
 */
void clippedProjection(
	const SMatrix4x4& view, const SMatrix4x4& proj, const SPlane& clipPlane,
	SMatrix4x4& clippedProj );


};

}; // namespace


#endif
