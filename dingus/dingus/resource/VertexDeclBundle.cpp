// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "VertexDeclBundle.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"


using namespace dingus;



IDirect3DVertexDeclaration9* CVertexDeclBundle::createDecl( const CVertexDesc& d ) const
{
	int i;
	int n = d.getStreams().size();

	// count elements
	int nelems = 0;
	for( i = 0; i < n; ++i )
		nelems += d.getStreams()[i].getFormat().calcComponentCount();

	// construct elements
	D3DVERTEXELEMENT9* els = new D3DVERTEXELEMENT9[nelems+1];
	assert( els );
	D3DVERTEXELEMENT9* e = els;
	for( i = 0; i < n; ++i ) {
		const CVertexStreamDesc& s = d.getStreams()[i];
		s.getFormat().calcVertexDecl( e, i, s.getUVIndex() );
		e += s.getFormat().calcComponentCount();
	}
	D3DVERTEXELEMENT9 elEnd = D3DDECL_END();
	*e = elEnd;

	// create vertex declaration
	HRESULT hres;
	IDirect3DVertexDeclaration9* decl = NULL;
	
	hres = CD3DDevice::getInstance().getDevice().CreateVertexDeclaration( els, &decl );
	if( FAILED( hres ) ) {
		std::string msg = "failed to create vertex decl";
		CConsole::CON_ERROR.write(msg);
		THROW_DXERROR( hres, msg );
	}
	assert( decl );

	delete[] els;

	return decl;
}

CD3DVertexDecl* CVertexDeclBundle::loadResourceById( const CVertexDesc& id )
{
	IDirect3DVertexDeclaration9* decl = createDecl( id );
	return new CD3DVertexDecl( decl );
}


void CVertexDeclBundle::createResource()
{
	// recreate all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		const CVertexDesc& fmt = it->first;
		CD3DVertexDecl& res = *it->second;
		res.setObject( createDecl( fmt ) );
		assert( !res.isNull() );
	}
}

void CVertexDeclBundle::activateResource()
{
}

void CVertexDeclBundle::passivateResource()
{
}

void CVertexDeclBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DVertexDecl& res = *it->second;
		assert( !res.isNull() );
		res.getObject()->Release();
		res.setObject( NULL );
	}
}
