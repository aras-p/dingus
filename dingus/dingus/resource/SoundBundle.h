// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SOUND_BUNDLE_H
#define __SOUND_BUNDLE_H

#include "ResourceBundle.h"
#include "../audio/SoundResource.h"
#include "../utils/Singleton.h"

namespace dingus {


// --------------------------------------------------------------------------

/**
 *  Vertex stream descriptor. Contains stream's CVertexFormat and starting UV
 *  index for a stream.
 */
class CSoundDesc {
public:
	CSoundDesc( const CResourceId& rid, bool ambient, int trackCount = 4, bool streaming = false )
		: mID(rid), mAmbient(ambient), mTrackCount(trackCount), mStreaming(streaming) { }
	CSoundDesc( const CSoundDesc& r )
		: mID(r.mID), mAmbient(r.mAmbient), mTrackCount(r.mTrackCount), mStreaming(r.mStreaming) { }
	void operator=( const CSoundDesc& r ) { mID=r.mID; mAmbient=r.mAmbient; mTrackCount=r.mTrackCount; mStreaming=r.mStreaming; }

	const CResourceId& getID() const { return mID; }
	bool	isAmbient() const { return mAmbient; }
	bool	isStreaming() const { return mStreaming; }
	int		getTrackCount() const { return mTrackCount; }

	bool operator< ( const CSoundDesc& rhs ) const {
		if( mID < rhs.mID )	return true;
		if( mID > rhs.mID )	return false;
		if( mAmbient < rhs.mAmbient ) return true;
		if( mAmbient > rhs.mAmbient ) return false;
		if( mStreaming < rhs.mStreaming ) return true;
		if( mStreaming > rhs.mStreaming ) return false;
		if( mTrackCount < rhs.mTrackCount ) return true;
		return false;
	}
	bool operator== ( const CSoundDesc& rhs ) const {
		return mID==rhs.mID && mAmbient==rhs.mAmbient && mStreaming==rhs.mStreaming && mTrackCount==rhs.mTrackCount;
	}
	bool operator!= ( const CSoundDesc& rhs ) const {
		return mID!=rhs.mID || mAmbient!=rhs.mAmbient || mStreaming!=rhs.mStreaming || mTrackCount!=rhs.mTrackCount;
	}
private:
	CResourceId		mID;
	bool	mAmbient;
	int		mTrackCount;
	bool	mStreaming;
};


// --------------------------------------------------------------------------

class CSoundBundle : public IResourceBundle, public CSingleton<CSoundBundle> {
public:
	typedef std::map<CSoundDesc, CSoundResource*> TResourceMap;
public:
	void	initialize( const std::string& dir ) {
		mDirectory = dir;
	}
	const std::string& getDirectory() const { return mDirectory; }

	CSoundResource* getResourceById( const CSoundDesc& id ) {
		CSoundResource* resource = findResource( id );
		if( !resource ) {
			resource = loadResourceById( id );
			assert( resource );
			mResourceMap.insert( std::make_pair( id, resource ) );
		}
		return resource;
	}

	void clear() {
		for( TResourceMap::iterator it = mResourceMap.begin(); it != mResourceMap.end(); ) {
			assert( it->second );
			deleteResource( *it->second );
			it = mResourceMap.erase( it );
		}
	}

	TResourceMap::iterator getSoundsBegin() { return mResourceMap.begin(); }
	TResourceMap::iterator getSoundsEnd() { return mResourceMap.end(); }

protected:
	CSoundResource* loadResourceById( const CSoundDesc& id );

	CSoundResource* findResource( const CSoundDesc& id ) {
		TResourceMap::const_iterator it = mResourceMap.find( id );
		return ( it != mResourceMap.end() ) ? it->second : NULL;
	}

	void deleteResource( CSoundResource& resource ) { resource.deleteResource(); delete &resource; }

private:
	IMPLEMENT_SIMPLE_SINGLETON(CSoundBundle);
	CSoundBundle() { };
	virtual ~CSoundBundle() { clear(); };
	
	/// @return false on error
	bool loadSound( const CSoundDesc& d, CSoundResource& sound ) const;

private:
	TResourceMap	mResourceMap;
	std::string		mDirectory;
};

}; // namespace


/// Shortcut macro
#define RGET_SOUND(rid) dingus::CSoundBundle::getInstance().getResourceById(rid)



#endif
