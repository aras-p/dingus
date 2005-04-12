// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __WIN32_DEBUG_CONSOLE_H
#define __WIN32_DEBUG_CONSOLE_H

#include "Console.h"

namespace dingus {

	
class CWDebugConsoleRenderingContext : public IConsoleRenderingContext {
public:
	CWDebugConsoleRenderingContext() { };
	virtual ~CWDebugConsoleRenderingContext() { };

	void write( const std::string& message );
};

}; // namespace

#endif