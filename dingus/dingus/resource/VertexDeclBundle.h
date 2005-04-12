// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VERTEX_DECL_BUNDLE_H
#define __VERTEX_DECL_BUNDLE_H

#include "ResourceBundle.h"
#include "../kernel/Proxies.h"
#include "DeviceResource.h"
#include "../gfx/VertexFormat.h"
#include "../utils/Singleton.h"

namespace dingus {


// --------------------------------------------------------------------------

/**
 *  Vertex stream descriptor. Contains stream's CVertexFormat and starting UV
 *  index for a stream.
 */
class CVertexStreamDesc {
public:
	CVertexStreamDesc( const CVertexFormat& fmt, int uvIndex = 0 )
		: mFormat(fmt), mUVIndex(uvIndex) { }

	const CVertexFormat& getFormat() const { return mFormat; }
	int getUVIndex() const { return mUVIndex; }

	bool operator< ( const CVertexStreamDesc& rhs ) const {
		if( mFormat < rhs.mFormat )
			return true;
		if( rhs.mFormat < mFormat )
			return false;
		return mUVIndex < rhs.mUVIndex;
	}
	bool operator== ( const CVertexStreamDesc& rhs ) const {
		return mFormat==rhs.mFormat && mUVIndex==rhs.mUVIndex;
	}
	bool operator!= ( const CVertexStreamDesc& rhs ) const {
		return mFormat!=rhs.mFormat || mUVIndex!=rhs.mUVIndex;
	}
private:
	CVertexFormat	mFormat;
	int				mUVIndex;
};


// --------------------------------------------------------------------------

/**
 *  Complete vertex declatator. Essentially a vector of CVertexStreamDesc.
 */
class CVertexDesc {
public:
	typedef std::vector<CVertexStreamDesc> TStreamDescVector;

public:
	CVertexDesc() { }
	CVertexDesc( const CVertexFormat& fmt, int uvIndex = 0 ) { mStreams.push_back(CVertexStreamDesc(fmt,uvIndex)); }
	CVertexDesc( const TStreamDescVector& desc ) : mStreams(desc) { }
	CVertexDesc( const CVertexDesc& r ) : mStreams(r.mStreams) { }
	const CVertexDesc& operator=( const CVertexDesc& r ) { mStreams = r.mStreams; }

	bool operator< ( const CVertexDesc& r ) const { return mStreams < r.mStreams; }
	bool operator== ( const CVertexDesc& r ) const { return mStreams == r.mStreams; }
	bool operator!= ( const CVertexDesc& r ) const { return mStreams != r.mStreams; }

	const TStreamDescVector& getStreams() const { return mStreams; }
	TStreamDescVector& getStreams() { return mStreams; }
	
private:
	TStreamDescVector	mStreams;
};


// --------------------------------------------------------------------------

class CVertexDeclBundle :	public IResourceBundle,
							public CSingleton<CVertexDeclBundle>,
							public IDeviceResource
{
private:
	typedef std::map<CVertexDesc, CD3DVertexDecl*> TResourceMap;
public:
	CD3DVertexDecl* getResourceById( const CVertexDesc& id ) {
		CD3DVertexDecl* resource = findResource( id );
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

	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	CD3DVertexDecl* loadResourceById( const CVertexDesc& id );

	CD3DVertexDecl* findResource( const CVertexDesc& id ) {
		TResourceMap::const_iterator it = mResourceMap.find( id );
		return ( it != mResourceMap.end() ) ? it->second : NULL;
	}

	void deleteResource( CD3DVertexDecl& resource ) {
		if( resource.getObject() )
			resource.getObject()->Release();
		delete &resource;
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CVertexDeclBundle);
	CVertexDeclBundle() { };
	virtual ~CVertexDeclBundle() { clear(); };
	
	IDirect3DVertexDeclaration9* createDecl( const CVertexDesc& d ) const;

private:
	TResourceMap	mResourceMap;
};

}; // namespace


/// Shortcut macro
#define RGET_VDECL(rid) dingus::CVertexDeclBundle::getInstance().getResourceById(rid)



#endif
