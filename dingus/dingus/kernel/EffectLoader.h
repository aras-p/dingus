// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __EFFECT_LOADER_H
#define __EFFECT_LOADER_H

#include "ProxyEffect.h"

namespace dingus {

	class CConsoleChannel;

	namespace fxloader {

		bool load(
			const std::string& id, const std::string& fileName,
			CD3DXEffect& dest, std::string& errorMsgs,
			ID3DXEffectPool* pool, ID3DXEffectStateManager* stateManager,
			const D3DXMACRO* macros, bool optimizeShaders, CConsoleChannel& console );

	};


}; // namespace

#endif
