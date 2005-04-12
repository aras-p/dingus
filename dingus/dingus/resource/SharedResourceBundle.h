// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SHARED_RESOURCE_BUNDLE_H
#define __SHARED_RESOURCE_BUNDLE_H

#include "ResourceBundle.h"
#include "ResourceId.h"

namespace dingus {

template<typename T, typename RESDATA>
class CSharedResourceBundle : public IResourceBundle {
private:
	typedef std::map<CResourceId, std::pair<RESDATA,T*> >	TResourceMap;
	
public:
	CSharedResourceBundle() {}
	virtual ~CSharedResourceBundle() { }
	
	T* getResourceById( CResourceId const& id )	{
		T* resource = findResource( id );
		assert( resource );
		return resource;
	}

	// NOTE: deletes the resource, so make sure no one references it!
	void unregisterResource( const CResourceId& id ) {
		T* resource = findResource( id );
		if( resource ) {
			deleteResource( *resource );
			mResourceMap.erase( id );
		}
	}

	void clear() {
		for( TResourceMap::iterator it = mResourceMap.begin(); it != mResourceMap.end(); ) {
			assert( it->second.second );
			deleteResource( *it->second.second );
			it = mResourceMap.erase( it );
		}
	}
			
protected:
	void registerResource( CResourceId const& id, T& resource, RESDATA data ) {
		// construct pair object separately, or else VC7.1 produces bad code!
		std::pair<RESDATA,T*> pr( data, &resource );

		mResourceMap.insert( std::make_pair( id, pr ) );
	}

	T* findResource( CResourceId const& id ) {
		TResourceMap::const_iterator it = mResourceMap.find( id );
		return ( ( it != mResourceMap.end() ) ? it->second.second : NULL );
	}

	virtual void deleteResource( T& resource ) = 0;	
		
protected:
	TResourceMap	mResourceMap;
};

}; // namespace

#endif
