#ifndef __MESH_PROCESSOR_H
#define __MESH_PROCESSOR_H

#include <d3dx9math.h>
#include <d3dx9mesh.h>
#include <assert.h>

namespace mproc {


// --------------------------------------------------------------------------

typedef DWORD uint;

const int UV_COUNT = 4;

extern const D3DVERTEXELEMENT9 MESH_DECLARATION[];

extern IDirect3DDevice9*	d3dDevice;


// --------------------------------------------------------------------------

/// Vertex structure that mesh processor operates on.
struct SVertex {
	D3DXVECTOR3	p;			// position
	D3DXVECTOR3	n, t, b;	// normal, tangent, binormal
	D3DXVECTOR3 w;			// blend weights
	DWORD		i;			// bone indices
	D3DXVECTOR2 uv[UV_COUNT];	// UVs
};


// --------------------------------------------------------------------------

/// Mesh class
class CMesh {
public:
	CMesh();
	CMesh( const CMesh& r );
	void operator=( const CMesh& r );
	~CMesh();

	void	assignMesh( ID3DXMesh& mesh );
	void	assignMesh( ID3DXMesh& mesh, DWORD* adjacency );
	void	clearMesh();

	bool	isValid() const { return mMesh!=0; }
	ID3DXMesh& getMesh() { assert(mMesh); return *mMesh; }
	DWORD* getAdjacency() { assert(mAdjacency); return mAdjacency; }

	/// @return True on success.
	bool	calcAdjacency();

	/// @return True on success.
	bool	weldVertices();

private:
	ID3DXMesh*	mMesh;
	DWORD*		mAdjacency;
};


// --------------------------------------------------------------------------

HRESULT createMesh( uint ntris, uint nverts, CMesh& dest );
HRESULT copyAttrBuf( CMesh& src, CMesh& dst );
HRESULT cleanMesh( CMesh& src, CMesh& dst );
HRESULT optimizeMesh( CMesh& src, CMesh& dst );


};


#endif
