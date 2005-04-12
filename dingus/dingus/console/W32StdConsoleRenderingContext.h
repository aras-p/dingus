// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __WIN32_CONSOLE_H
#define __WIN32_CONSOLE_H

#include "Console.h"

namespace dingus {

	
class CW32StdConsoleRenderingContext : public IConsoleRenderingContext {
public:
	CW32StdConsoleRenderingContext();
	virtual ~CW32StdConsoleRenderingContext();

	void write( const std::string& message );

private:
	HANDLE	mConsoleHandle;
};

}; // namespace

#endif
