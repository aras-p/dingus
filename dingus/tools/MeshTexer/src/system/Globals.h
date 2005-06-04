#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <dingus/renderer/RenderContext.h>
#include <dingus/kernel/D3DDevice.h>
#include <dingus/input/InputContext.h>

#include <dingus/resource/CubeTextureBundle.h>
#include <dingus/resource/EffectBundle.h>
#include <dingus/resource/IndexBufferBundle.h>
#include <dingus/resource/MeshBundle.h>
#include <dingus/resource/SharedMeshBundle.h>
#include <dingus/resource/SharedTextureBundle.h>
#include <dingus/resource/SharedSurfaceBundle.h>
#include <dingus/resource/TextureBundle.h>
#include <dingus/resource/VertexDeclBundle.h>

using namespace dingus;

extern CRenderContext*	G_RENDERCTX;
extern CInputContext*	G_INPUTCTX;

extern CConsoleChannel&	CONS;


const int GUI_X = 640;
const int GUI_Y = 480;



#endif
