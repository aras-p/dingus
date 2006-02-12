// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __STORAGE_RESOURCE_BUNDLE_H
#define __STORAGE_RESOURCE_BUNDLE_H

#include "ResourceBundle.h"
#include "ResourceId.h"
#include "../console/Console.h"
#include "../utils/Errors.h"


namespace dingus {

template<class T>
class CStorageResourceBundle : public IResourceBundle {
private:
	typedef std::map<CResourceId, T*>	TResourceMap;
	
public:
	virtual ~CStorageResourceBundle() = 0 {}

	/**
	 *  Gets or loads resource given it's name.
	 *  Name is without pre-path and without file extension.
	 */
	T* getResourceById( const CResourceId& id ) {
		T* resource = trygetResourceById( id );
		if( resource )
			return resource;
		// error
		std::string msg = "Can't find resource '" + id.getUniqueName() + "'";
		CConsole::CON_ERROR.write( msg );
		THROW_ERROR( msg );
	}

	/**
	 *  Gets or loads resource given it's name, doesn't throw.
	 *  Name is without pre-path and without file extension.
	 */
	T* trygetResourceById( const CResourceId& id ) {
		// find if already loaded
		T* resource = findResource( id );
		if( resource )
			return resource;

		// try load
		resource = tryLoadResourceById( id );
		if( resource ) {
			mResourceMap.insert( std::make_pair( id, resource ) );
			return resource;
		}
		return NULL;
	}

	// NOTE: deletes the resource, so make sure no one references it!
	void clearResourceById( CResourceId const& id ) {
		TResourceMap::iterator it = mResourceMap.find( id );
		if( it != mResourceMap.end() ) {
			assert( (*it).second );
			// error
			std::string msg = "Clearing resource '" + id.getUniqueName() + "'";
			CONSOLE.write( msg );

			deleteResource( *it->second );
			mResourceMap.erase( it );
		}
	}

	void clear() {
		for( TResourceMap::iterator it = mResourceMap.begin(); it != mResourceMap.end(); ) {
			assert( (*it).second );
			deleteResource( *it->second );
			it = mResourceMap.erase( it );
		}
	}

	void addDirectory( const std::string& d ) { mDirectories.push_back(d); }
	
protected:
	CStorageResourceBundle() { }

	T* findResource( CResourceId const& id ) {
		TResourceMap::const_iterator it = mResourceMap.find( id );
		return ( ( it != mResourceMap.end() ) ? it->second : NULL );
	}

	/**
	 *  Try to load resource. Default implementation tries
	 *  all extensions. Simpler ones can just append pre-dir and extension
	 *  to id and use loadResourceById(), or some other bundle to load.
	 */
	virtual T* tryLoadResourceById( const CResourceId& id ) {
		size_t nd = mDirectories.size();
		size_t ne = mExtensions.size();
		// try all directories
		for( size_t d = 0; d < nd; ++d ) {
			// try all extensions
			for( size_t e = 0; e < ne; ++e ) {
				CResourceId fullid( mDirectories[d] + id.getUniqueName() + mExtensions[e] );
				T* resource = loadResourceById( id, fullid );
				if( resource )
					return resource;
			}
		}
		// If all that failed, maybe ID already contains full path and extension
		// (maybe useful in tools). Try loading it.
		return loadResourceById( id, id );
	}

	/**
	 *  Performs actual loading of resource.
	 *  On failure to find resource, should silently return NULL - storage
	 *  bundle will attempt another extension. On other failure
	 *  (eg. format mismatch) can assert/throw etc.
	 *
	 *  @param id Resource ID.
	 *  @param fullName Full path to resource (with pre-dir(s) and extension).
	 */
	virtual T* loadResourceById( const CResourceId& id, const CResourceId& fullName ) = 0;
	virtual void deleteResource( T& resource ) = 0;	

protected:
	typedef std::vector<std::string>	TStringVector;
	void addExtension( const char* e ) { mExtensions.push_back(e); }
	const TStringVector& getExtensions() const { return mExtensions; }
	const TStringVector& getDirectories() const { return mDirectories; }

protected:
	TResourceMap	mResourceMap;

private:
	TStringVector	mDirectories;
	TStringVector	mExtensions;
};

}; // namespace


#endif
