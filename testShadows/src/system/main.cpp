#include "stdafx.h"

#include "../demo/Demo.h"
#include <dingus/app/DingusSystem.h>


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
	return 0;
}
