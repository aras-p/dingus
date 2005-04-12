// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "IndexBufferBundle.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"


using namespace dingus;



void CIndexBufferBundle::registerIB( CResourceId const& id,
		int elements, D3DFORMAT format, IIndexBufferFiller& filler )
{
	assert( format == D3DFMT_INDEX16 || format == D3DFMT_INDEX32 );
	SIndexBufferDesc desc;
	desc.elements = elements;
	desc.format = format;
	desc.filler = &filler;
	IDirect3DIndexBuffer9* ib = createIB( desc );
	registerResource( id, *(new CD3DIndexBuffer(ib)), desc );

	filler.fillIB( *getResourceById(id), desc.elements, desc.format );
}


IDirect3DIndexBuffer9* CIndexBufferBundle::createIB( const SIndexBufferDesc& d ) const
{
	HRESULT hres;
	IDirect3DIndexBuffer9* ib = NULL;
	
	hres = CD3DDevice::getInstance().getDevice().CreateIndexBuffer(
		d.elements * ((d.format==D3DFMT_INDEX16) ? 2 : 4),
		D3DUSAGE_WRITEONLY,
		d.format,
		D3DPOOL_MANAGED,
		&ib, NULL );
	if( FAILED( hres ) ) {
		std::string msg = "failed to create IB";
		CConsole::CON_ERROR.write(msg);
		THROW_DXERROR( hres, msg );
	}
	assert( ib );
	return ib;
}

void CIndexBufferBundle::createResource()
{
	// recreate all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		SIndexBufferDesc& desc = it->second.first;
		CD3DIndexBuffer& res = *it->second.second;
		res.setObject( createIB( desc ) );
		// refill
		desc.filler->fillIB( res, desc.elements, desc.format );
		assert( !res.isNull() );
	}
}

void CIndexBufferBundle::activateResource()
{
}

void CIndexBufferBundle::passivateResource()
{
}

void CIndexBufferBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DIndexBuffer& res = *it->second.second;
		assert( !res.isNull() );
		res.getObject()->Release();
		res.setObject( NULL );
	}
}
