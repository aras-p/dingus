// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "Console.h"
#include <time.h>

using namespace dingus;

CConsole* CConsole::mSingleInstance = 0;


CConsoleChannel& CConsole::CON_WARNING	= CConsole::getChannel( "warning" );
CConsoleChannel& CConsole::CON_ERROR	= CConsole::getChannel( "error" );


//---------------------------------------------------------------------------
// CConsole
//---------------------------------------------------------------------------

CConsole::CConsole()
:	mDefaultRenderingCtx( NULL ),
	mDefaultFilter( NULL )
{
}

CConsole::~CConsole()
{
	for( TChannelList::iterator i = mChannels.begin(); i != mChannels.end(); ++i )
		delete *i;
}

CConsoleChannel& CConsole::openChannel( const std::string& name )
{
	CConsoleChannel* channel = NULL;

	for( TChannelList::iterator i = mChannels.begin(); i != mChannels.end(); ++i ) {
		if( (*i)->getName() == name )
			channel = *i;
	}

	if( !channel ) {
		channel = new CConsoleChannel( *this, name, mDefaultRenderingCtx, mDefaultFilter );
		mChannels.push_back( channel );
	}

	return *channel;
}

void CConsole::closeChannel( CConsoleChannel& channel )
{
	for( TChannelList::iterator i = mChannels.begin(); i != mChannels.end(); ++i ) {
		if( *i == &channel ) {
			delete *i;
			i = mChannels.erase( i );
		}
	}
}

void CConsole::setDefaultRenderingContext( IConsoleRenderingContext& ctx )
{ 
	mDefaultRenderingCtx = &ctx;

	for( TChannelList::iterator i = mChannels.begin(); i != mChannels.end(); ++i ) {
		if( !(*i)->getRenderingContext() )
			(*i)->setRenderingContext( ctx );
	}
}


//---------------------------------------------------------------------------
// CConsoleChannel
//---------------------------------------------------------------------------

CConsoleChannel::CConsoleChannel( CConsole& console, const std::string& name, IConsoleRenderingContext* renderingCtx, IConsoleFilter* filter )
:	mParentConsole( console ),
	mName( name ),
	mRenderingCtx( renderingCtx ),
	mFilter( filter ),
	mWritingChannel( true ),
	mLoggingTime( false )
{
}

CConsoleChannel::~CConsoleChannel()
{
}

void CConsoleChannel::write( const std::string& methodName, const std::string& message )
{
	write( message );
}

void CConsoleChannel::write( const std::string& message )
{
	if( !mRenderingCtx )
		return;

	if( mFilter && !mFilter->apply( *this, message ) )
		return;

	if( mLoggingTime ) {
		time_t clockTime;
		time( &clockTime );
		const tm* t = localtime( &clockTime );
		char buf[100];
		sprintf( buf, "%i:%i:%i ", t->tm_hour, t->tm_min, t->tm_sec );
		mRenderingCtx->write( buf );
	}
	if( mWritingChannel ) {
		mRenderingCtx->write( mName + "> " );
	}
	mRenderingCtx->write( message );
	mRenderingCtx->write( "\n" );
}
