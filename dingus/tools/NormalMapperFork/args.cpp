#include "args.h"
#include <cstring>
#include <cstdlib>


CmdlineArgs::CmdlineArgs( int argc, char** argv )
:	mCount(argc-1)
{
	mArgs = new const char*[ mCount ];
	for( int i = 0; i < mCount; ++i ) {
		mArgs[i] = argv[i+1];
	}
}

CmdlineArgs::~CmdlineArgs()
{
	delete[] mArgs;
}

int CmdlineArgs::find( const char* name ) const
{
	for( int i = mCount-1; i >= 0; --i ) {
		if( !strcmp(mArgs[i], name) )
			return i;
	}
	return -1;
}

const char* CmdlineArgs::getString( const char* name ) const
{
	int idx = find(name)+1;
	if( idx <= 0 || idx >= mCount )
		return NULL;
	return mArgs[idx];
}

int CmdlineArgs::getInt( int def, const char* name ) const
{
	int idx = find(name)+1;
	if( idx <= 0 || idx >= mCount )
		return def;
	return atoi( mArgs[idx] );
}

double CmdlineArgs::getFloat( double def, const char* name ) const
{
	int idx = find(name)+1;
	if( idx <= 0 || idx >= mCount )
		return def;
	return atof( mArgs[idx] );
}
