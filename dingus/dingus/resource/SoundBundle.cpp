// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "SoundBundle.h"
#include "../utils/Errors.h"
#include "../audio/AudioContext.h"


using namespace dingus;



bool CSoundBundle::loadSound( const CSoundDesc& d, CSoundResource& sound ) const
{
	assert( !mDirectory.empty() );
	if( !G_AUDIOCTX->isOpen() ) {
		CONSOLE.write( "sound unavailable, skipping loading of '" + d.getID().getUniqueName() + "'" );
		return true;
	}

	bool ok = sound.createResource( mDirectory + d.getID().getUniqueName() + ".wav", d.getTrackCount(), d.isAmbient(), d.isStreaming() );
	if( !ok ) {
		std::string msg = "failed to load sound '" + d.getID().getUniqueName() + "'";
		CConsole::CON_ERROR.write(msg);
		THROW_ERROR( msg );
	}
	CONSOLE.write( "sound loaded '" + d.getID().getUniqueName() + "'" );
	return true;
}

CSoundResource* CSoundBundle::loadResourceById( const CSoundDesc& id )
{
	assert( G_AUDIOCTX );
	CSoundResource* sound = new CSoundResource();
	bool ok = loadSound( id, *sound );
	if( !ok ) {
		delete sound;
		return NULL;
	}
	return sound;
}
