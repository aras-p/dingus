// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __EFFECT_LOADER_H
#define __EFFECT_LOADER_H

#include "ProxyEffect.h"

namespace dingus {

	class CConsoleChannel;

	namespace fxloader {

		/**
		 *	Initialize effect loading system.
		 *  @param cfgFileName State groups configuration file.
		 */
		bool initialize( const char* cfgFileName );
		/**
		 *	Shutdown effect loading system.
		 */
		void shutdown();
		
		/**
		 *	Load effect. This will examine the effect, complain about
		 *	missing states, generate restore pass and return
		 *	ready-to-use effect.
		 */
		bool load(
			const std::string& id, const std::string& fileName,
			CD3DXEffect& dest, std::string& errorMsgs,
			ID3DXEffectPool* pool, ID3DXEffectStateManager* stateManager,
			const D3DXMACRO* macros, int macroCount, bool optimizeShaders, CConsoleChannel& console );

	};


}; // namespace

#endif
