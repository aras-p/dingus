#include "stdafx.h"

#include "../demo/Demo.h"
#include <dingus/app/DingusSystem.h>


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	CDemo* demo = NULL;
	CDingusSystem* system = NULL;
	try {
		demo = new CDemo();
		system = new CDingusSystem( *demo );
		if( SUCCEEDED( system->create( hInst, false ) ) )
			system->run();
	} catch( std::exception& e ) {
		if( system )
			system->getOutOfFullscreen();
		
		std::string willExit = "\n\nThe application will now exit.";
		MessageBox( 0, (e.what()+willExit).c_str(), "Fatal error", MB_OK | MB_SETFOREGROUND | MB_TOPMOST );
		return 0;
	}
	return 0;
}
