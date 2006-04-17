#include "stdafx.h"

#include "../demo/Demo.h"
#include <dingus/app/DingusSystem.h>
#include <dingus/utils/StringHelper.h>

extern std::string gErrorMsg;


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
	char serverName[1000];
	int serverPort = 0;
	serverName[0] = 0;
	char recordFileNameBuf[1000];
	recordFileNameBuf[0] = 0;

	sscanf( lpCmdLine, "%s %i %s", serverName, &serverPort, recordFileNameBuf );

	if( serverName[0] == 0 || serverPort == 0 ) {
		//MessageBox( 0, "Supply the server name and port in the command line!", "Error", MB_OK );
		//return 0;

		// assume localhost:9000
		strcpy( serverName, "127.0.0.1" ); // TBD: this is only for me!
		serverPort = 9000;
	}

	std::string recordFileName = recordFileNameBuf;
	bool writeRecord = false;
	if( !recordFileName.empty() )
	{
		if( CStringHelper::startsWith( recordFileName, "/rcam:" ) )
		{
			recordFileName = recordFileName.substr( 6, recordFileName.size()-6 );
			writeRecord = true;
		}
		else if( CStringHelper::startsWith( recordFileName, "/pcam:" ) )
		{
			recordFileName = recordFileName.substr( 6, recordFileName.size()-6 );
			writeRecord = false;
		}
		else
		{
			recordFileName = "";
		}
	}

	try {
		CDemo* demo = new CDemo( serverName, serverPort, recordFileName, writeRecord );
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
