#include "stdafx.h"

#include "../demo/Demo.h"
#include <dingus/app/DingusSystem.h>
#include <dingus/utils/StringHelper.h>

extern std::string gErrorMsg;


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
	try {
		CDemo* demo = new CDemo();
		CDingusSystem* system = new CDingusSystem( *demo );
		if( SUCCEEDED( system->create( hInst, false ) ) )
			system->run();
		delete demo;
		delete system;
	} catch( std::exception& e ) {
		std::string willExit = "\n\nThe application will now exit.";
		MessageBox( 0, (e.what()+willExit).c_str(), "Fatal error", MB_OK );
		return 0;
	}
	if( !gErrorMsg.empty() ) {
		std::string willExit = "\n\nThe application will now exit.";
		MessageBox( 0, (gErrorMsg+willExit).c_str(), "Initialization error", MB_OK );
	}
	return 0;
}
