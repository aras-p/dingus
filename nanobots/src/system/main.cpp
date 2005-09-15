#include "stdafx.h"

#include "../demo/Demo.h"
#include <dingus/app/DingusSystem.h>

extern std::string gErrorMsg;


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
	char serverName[1000];
	int serverPort = 0;
	serverName[0] = 0;

	sscanf( lpCmdLine, "%s %i", serverName, &serverPort );

	if( serverName[0] == 0 || serverPort == 0 ) {
		//MessageBox( 0, "Supply the server name and port in the command line!", "Error", MB_OK );
		//return 0;

		// assume localhost:8000
		strcpy( serverName, "localhost" );
		serverPort = 8000;
	}
	try {
		CDemo* demo = new CDemo( serverName, serverPort );
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
