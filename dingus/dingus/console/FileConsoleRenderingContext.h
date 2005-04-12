// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __FILE_CONSOLE_H
#define __FILE_CONSOLE_H

#include "Console.h"
#include <fstream>

namespace dingus {

	
class CFileConsoleRenderingContext : public IConsoleRenderingContext {
public:
	CFileConsoleRenderingContext( const std::string& fileName );
	virtual ~CFileConsoleRenderingContext();

	virtual void write( const std::string& message );

private:
	std::ofstream	mFile;
};

}; // namespace

#endif
