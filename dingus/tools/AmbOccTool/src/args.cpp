#include "stdafx.h"
#include "args.h"
#include <cstring>
#include <cstdlib>


CCmdlineArgs::CCmdlineArgs( int argc, const char** argv )
{
	// don't put first
	mArgs.reserve( argc );
	for( int i = 1; i < argc; ++i ) {
		mArgs.push_back( argv[i] );
	}
}

int CCmdlineArgs::find( const char* name ) const
{
	for( int i = mArgs.size()-1; i >= 0; --i ) {
		if( !strcmp(mArgs[i], name) )
			return i;
	}
	return -1;
}

const char* CCmdlineArgs::getString( const char* name ) const
{
	int idx = find(name)+1;
	if( idx <= 0 || idx >= mArgs.size() )
		return NULL;
	return mArgs[idx];
}

int CCmdlineArgs::getInt( int def, const char* name ) const
{
	int idx = find(name)+1;
	if( idx <= 0 || idx >= mArgs.size() )
		return def;
	return atoi( mArgs[idx] );
}

float CCmdlineArgs::getFloat( float def, const char* name ) const
{
	int idx = find(name)+1;
	if( idx <= 0 || idx >= mArgs.size() )
		return def;
	return float( atof( mArgs[idx] ) );
}
