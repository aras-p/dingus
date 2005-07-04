#include "stdafx.h"
#include "d3d.h"
#include "args.h"
#include <ctime>
#include "crackdecl.h"


const char* ARG_MESH = "-m";
const char* ARG_NORMALMAP = "-n";
const char* ARG_X = "-x";
const char* ARG_Y = "-y";
const char* ARG_SWAPYZ = "-s";


const CCmdlineArgs* gArgs;


std::string stripExtension( const char* ss )
{
	std::string s(ss);
	int idx = s.find_last_of( '.' );
	if( idx >= 0 ) {
		s = s.substr( 0, idx );
	}
	return s;
}


// ---- loading code from DMESH files 

#include "VertexFormat.h"

#define READ_4BYTE(var) fread(&var,1,4,f)
#define READ_2BYTE(var) fread(&var,1,2,f)

ID3DXMesh* loadMeshFromDMESH( const char* fileName )
{
	int i;

	// open file
	FILE* f = fopen( fileName, "rb" );
	if( !f ) return NULL;

	//
	// read header

	// magic
	char magic[4];
	READ_4BYTE(magic);
	if( magic[0]!='D' || magic[1]!='M' || magic[2]!='S' || magic[3]!='H' ) {
		fclose( f );
		return NULL;
	}
	// header
	int nverts, ntris, ngroups, vstride, istride, vformat;
	READ_4BYTE(nverts);
	READ_4BYTE(ntris);
	READ_4BYTE(ngroups);
	READ_4BYTE(vstride);
	READ_4BYTE(vformat);
	READ_4BYTE(istride);
	assert( nverts > 0 );
	assert( ntris > 0 );
	assert( ngroups > 0 );
	assert( vstride > 0 );
	assert( istride==2 || istride==4 );
	assert( vformat != 0 );

	// figure out vertex declaration
	dingus::CVertexFormat format(vformat);
	assert( format.calcVertexSize() == vstride );

	D3DVERTEXELEMENT9 d3dVDecl[MAX_FVF_DECL_SIZE];
	int declEntries = format.calcComponentCount();
	format.calcVertexDecl( d3dVDecl, 0, 0 );
	D3DVERTEXELEMENT9 elEnd = D3DDECL_END();
	d3dVDecl[declEntries] = elEnd;

	// create mesh
	ID3DXMesh* mesh;
	HRESULT hr;
	hr = D3DXCreateMesh( ntris, nverts, D3DXMESH_MANAGED | (istride==4?D3DXMESH_32BIT:0), d3dVDecl, gD3DDevice.get(), &mesh );
	if( FAILED(hr) ) {
		return NULL;
	}

	// read verts
	void* vb = 0;
	hr = mesh->LockVertexBuffer( 0, &vb );
	fread( vb, vstride, nverts, f );
	// compute AABB and center the mesh
	D3DXVECTOR3 bmin, bmax;
	D3DXComputeBoundingBox( (const D3DXVECTOR3*)vb, nverts, vstride, &bmin, &bmax );
	D3DXVECTOR3 center = (bmax+bmin) * 0.5f;
	for( i = 0; i < nverts; ++i ) {
		D3DXVECTOR3* pos = (D3DXVECTOR3*)( ((char*)vb) + i*vstride );
		*pos -= center;
	}

	mesh->UnlockVertexBuffer();

	// read indices
	void* ib = 0;
	hr = mesh->LockIndexBuffer( 0, &ib );
	fread( ib, istride, ntris*3, f );
	// change cull mode
	/*
	if( istride == 2 ) {
		unsigned short* ib2 = (unsigned short*)ib;
		for( i = 0; i < ntris; ++i ) {
			unsigned short ia = ib2[0];
			unsigned short ib = ib2[1];
			unsigned short ic = ib2[2];
			ib2[0] = ia;
			ib2[1] = ic;
			ib2[2] = ib;
			ib2 += 3;
		}
	}
	*/
	mesh->UnlockIndexBuffer();

	// TODO: right now assume one group per whole mesh
	DWORD* attrbuf = 0;
	hr = mesh->LockAttributeBuffer( 0, &attrbuf );
	memset( attrbuf, 0, ntris*sizeof(DWORD) );
	mesh->UnlockAttributeBuffer();

	D3DXATTRIBUTERANGE attrTab;
	attrTab.AttribId = 0;
	attrTab.FaceStart = 0;
	attrTab.FaceCount = ntris;
	attrTab.VertexStart = 0;
	attrTab.VertexCount = nverts;
	mesh->SetAttributeTable( &attrTab, 1 );

	// close file
	fclose( f );
	return mesh;
}

HRESULT WINAPI gProgressCallback( float percentDone, void* user )
{
	printf( "  %3.1f%%...\n", percentDone*100.0f );
	Sleep( 1 );
	return S_OK;
}


static ID3DXMesh* maybeCreateUVs( ID3DXMesh* mesh, int texX, int texY )
{
	HRESULT hr;

	// TBD: compute atlas not always
	bool computeAtlas = true;
	//CD3DXCrackDecl declCrack;

	// get declaration of the mesh, check if has UVs
	D3DVERTEXELEMENT9 vdecl[MAX_FVF_DECL_SIZE];
	mesh->GetDeclaration( vdecl );
	UINT declLen = D3DXGetDeclLength( vdecl );

	if( NULL == GetDeclElement( vdecl, D3DDECLUSAGE_TEXCOORD, 0 ) ) {
		printf( "adding UV slot to the mesh\n" );
		// TBD: for now assume there's always space in decl available

		D3DVERTEXELEMENT9 declElem;
		declElem.Stream = 0;
		declElem.Type = D3DDECLTYPE_FLOAT2;
		declElem.Method = D3DDECLMETHOD_DEFAULT;
		declElem.Usage = D3DDECLUSAGE_TEXCOORD;
		declElem.UsageIndex = 0;

		AppendDeclElement( &declElem, vdecl );

		// clone the mesh into new declaration
		ID3DXMesh* newMesh = NULL;
		hr = mesh->CloneMesh( D3DXMESH_32BIT | D3DXMESH_SYSTEMMEM, vdecl, gD3DDevice.get(), &newMesh );
		mesh->Release();
		mesh = newMesh;

		computeAtlas = true; // didn't have UVs - force atlas creation
	}

	if( computeAtlas ) {
		printf( "computing UV atlas...\n" );
		ID3DXMesh* newMesh = NULL;
		float outMaxStretch;
		UINT outNumCharts;
		hr = D3DXUVAtlasCreate( mesh, 0, 0.6667f, texX, texY, 8.0f, 0, NULL, NULL, gProgressCallback, 0.05f, NULL, &newMesh, NULL, NULL, &outMaxStretch, &outNumCharts );
		mesh->Release();
		mesh = newMesh;
	}

	return mesh;
}


static IDirect3DTexture9*	gCreateSignedNormalMap( IDirect3DTexture9* inputNormalMap, bool swapYZ )
{
	printf( "creating signed normal map texture for PRT...\n" );

	D3DSURFACE_DESC desc;
	inputNormalMap->GetLevelDesc( 0, &desc );
	
	HRESULT hr;

	// create texture
	IDirect3DTexture9* signedNM = NULL;
	hr = D3DXCreateTexture( gD3DDevice.get(), desc.Width, desc.Height, 1,
		0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED, &signedNM );
	if( FAILED(hr) )
		throw std::runtime_error( "Failed to create signed texture" );

	// convert
	IDirect3DSurface9 *srcSurf, *dstSurf;
	inputNormalMap->GetSurfaceLevel( 0, &srcSurf );
	signedNM->GetSurfaceLevel( 0, &dstSurf );
	hr = D3DXLoadSurfaceFromSurface( dstSurf, NULL, NULL, srcSurf, NULL, NULL, D3DX_DEFAULT, 0 );
	if( FAILED(hr) )
		throw std::runtime_error( "Failed to convert to signed normal map" );

	// go through our signed normal map and convert from 0..1 to -1..1 range
	D3DLOCKED_RECT lr;
	dstSurf->LockRect( &lr, NULL, 0 );
	for( int y = 0; y < desc.Height; ++y )
	{
		float* row = (float*)( ((char*)lr.pBits) + y*lr.Pitch );
		for( int x = 0; x < desc.Width; ++x )
		{
			D3DXVECTOR3 n;
			n.x = row[0]*2-1;
			n.y = row[1]*2-1;
			n.z = row[2]*2-1;
			D3DXVec3Normalize( &n, &n );
			row[0] = n.x;
			if( swapYZ ) {
				row[1] = n.z;
				row[2] = n.y;
			} else {
				row[1] = n.y;
				row[2] = n.z;
			}

			row += 4;
		}
	}
	dstSurf->UnlockRect();

	srcSurf->Release();
	dstSurf->Release();

	return signedNM;
}



void WINAPI gFillBlack( D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize, LPVOID pData )
{
   *pOut = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
}

static const bool GPU_SIM = true;
static const int SIM_SH_ORDER = 2;
static const int SIM_SAMPLES = 1024;


void processMesh( const char* meshFileName, const char* nmapFileName, int outX, int outY, bool swapYZ )
{
	HRESULT hr;

	time_t time1 = clock();

	std::string fileNoExt = stripExtension( nmapFileName ? nmapFileName : meshFileName );
	std::string resFile = fileNoExt + "AO.dds";

	// load mesh
	printf( "loading mesh '%s'...\n", meshFileName );
	ID3DXMesh* mesh = NULL;
	hr = D3DXLoadMeshFromX( meshFileName, D3DXMESH_32BIT | D3DXMESH_MANAGED,
		gD3DDevice.get(), NULL, NULL, NULL, NULL, &mesh );
	if( FAILED(hr) ) {
		// then try loading from DMESH
		mesh = loadMeshFromDMESH( meshFileName );
		if( !mesh )
			throw std::runtime_error( "Failed to load mesh" );
	}

	// load normal map
	IDirect3DTexture9* normalMap = NULL;
	if( nmapFileName ) {
		printf( "loading normal map '%s'...\n", nmapFileName );
		hr = D3DXCreateTextureFromFileEx(
			gD3DDevice.get(), nmapFileName, D3DX_DEFAULT, D3DX_DEFAULT, 0, 0, D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &normalMap );
		if( FAILED(hr) )
			throw std::runtime_error( "Failed to load texture" );
		D3DSURFACE_DESC desc;
		normalMap->GetLevelDesc( 0, &desc );

		// input nmap size overrides output
		outX = desc.Width;
		outY = desc.Height;
	}

	// possibly create UV atlas
	mesh = maybeCreateUVs( mesh, outX, outY );

	// create PRT engine
	printf( "creating PRT engine...\n" );
	ID3DXPRTEngine* prtEngine = NULL;
	hr = D3DXCreatePRTEngine( mesh, NULL, TRUE, NULL, &prtEngine );
	if( FAILED(hr) )
		throw std::runtime_error( "Failed to create PRT engine" );

	prtEngine->SetCallBack( gProgressCallback, 1.0f / 20.0f, NULL );

	// set per-texel normals onto PRT engine
	if( normalMap ) {
		// construct signed normal map
		IDirect3DTexture9* signedNormalMap = gCreateSignedNormalMap( normalMap, swapYZ );
		hr = prtEngine->SetPerTexelNormal( signedNormalMap );
		if( FAILED(hr) )
			throw std::runtime_error( "Failed to set per texel normal map" );
		signedNormalMap->Release();
	}

	hr = prtEngine->SetSamplingInfo( SIM_SAMPLES, TRUE, FALSE, FALSE, 0.0f );
	if( FAILED(hr) )
		throw std::runtime_error( "Failed to set PRT sampling params" );

	// create PRT buffer
	printf( "creating PRT buffer...\n" );
	ID3DXPRTBuffer* prtBuffer = NULL;
	hr = D3DXCreatePRTBufferTex( outX, outY, SIM_SH_ORDER*SIM_SH_ORDER, 1, &prtBuffer );
	if( FAILED(hr) )
		throw std::runtime_error( "Failed to create PRT buffer" );

	// create texture gutter helper
	printf( "creating texture gutter...\n" );
	ID3DXTextureGutterHelper* gutter = NULL;
	hr = D3DXCreateTextureGutterHelper( outX, outY, mesh, float(outX)/1024*16, &gutter );
	if( FAILED(hr) )
		throw std::runtime_error( "Failed to create texture gutter" );

	// compute PRT
	printf( "computing PRT...\n" );
	if( GPU_SIM )
		hr = prtEngine->ComputeDirectLightingSHGPU( gD3DDevice.get(),
			D3DXSHGPUSIMOPT_SHADOWRES1024, SIM_SH_ORDER, 0.001f, 0.01f, prtBuffer );
	else
		hr = prtEngine->ComputeDirectLightingSH( SIM_SH_ORDER, prtBuffer );

	if( FAILED(hr) )
		throw std::runtime_error( "Failed to compute PRT" );

	// scale PRT for ambient occlusion
	prtBuffer->ScaleBuffer( 2 * sqrtf(D3DX_PI) );

	// apply texture gutter
	printf( "applying texture gutter...\n" );
	hr = gutter->ApplyGuttersPRT( prtBuffer );
	if( FAILED(hr) )
		throw std::runtime_error( "Failed to apply texture gutter" );

	// create texture to store results
	printf( "creating texture for results...\n" );
	IDirect3DTexture9* prtTexture = NULL;
	hr = D3DXCreateTexture( gD3DDevice.get(), outX, outY, 1,
		0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &prtTexture );
	if( FAILED(hr) )
		throw std::runtime_error( "Failed to create texture" );
	D3DXFillTexture( prtTexture, gFillBlack, NULL );

	// extract PRT into texture
	printf( "extracting data to texture...\n" );
	hr = prtBuffer->ExtractTexture( 0, 0, 4, prtTexture );
	if( FAILED(hr) )
		throw std::runtime_error( "Failed to extract to texture" );

	// possibly combine normal map and ambient occlusion
	D3DLOCKED_RECT lrPRT;
	prtTexture->LockRect( 0, &lrPRT, NULL, 0 );
	if( nmapFileName ) {
		printf( "combining input normal map and AO...\n" );
		D3DLOCKED_RECT lrNM;
		normalMap->LockRect( 0, &lrNM, NULL, D3DLOCK_READONLY );

		for( int y = 0; y < outY; ++y ) {
			const DWORD* rowNM = (const DWORD*)( ((char*)lrNM.pBits) + y*lrNM.Pitch );
			DWORD* rowPRT = (DWORD*)( ((char*)lrPRT.pBits) + y*lrPRT.Pitch );
			for( int x = 0; x < outX; ++x ) {
				DWORD n = *rowNM;
				*rowPRT = ((*rowPRT) << 8) & 0xFF000000;
				if( swapYZ ) {
					*rowPRT |= (n&0x00ff0000) | ((n&0x0000ff00)>>8) | ((n&0x000000ff)<<8);
				} else {
					*rowPRT |= n & 0x00ffffff;
				}
				++rowPRT;
				++rowNM;
			}
		}

		normalMap->UnlockRect( 0 );

	} else {
		printf( "filling AO texture...\n" );

		for( int y = 0; y < outY; ++y ) {
			DWORD* rowPRT = (DWORD*)( ((char*)lrPRT.pBits) + y*lrPRT.Pitch );
			for( int x = 0; x < outX; ++x ) {
				DWORD ao = (rowPRT[0] >> 16) & 0xFF;
				*rowPRT = 0xFF000000 | (ao<<16) | (ao<<8) | ao;
				++rowPRT;
			}
		}

	}

	prtTexture->UnlockRect( 0 );

	// save texture
	printf( "saving texture '%s'...\n", resFile.c_str() );
	hr = D3DXSaveTextureToFile( resFile.c_str(), D3DXIFF_DDS, prtTexture, NULL );
	if( FAILED(hr) )
		throw std::runtime_error( "Failed to save texture" );

	// cleanup
	gutter->Release();
	prtTexture->Release();
	prtBuffer->Release();
	prtEngine->Release();
	if( normalMap )
		normalMap->Release();
	mesh->Release();

	time_t time2 = clock();
	printf( "time: %.2fs\n", (float)(time2-time1)/CLOCKS_PER_SEC );
}


void gPrintUsage()
{
	printf( "Usage: executable <options>\n" );
	printf( "Options:\n" );
	printf( "  -m <filename> Input mesh file.\n" );
	printf( "  -n <filename> Input/output normal map.\n" );
	printf( "  -x <width>    Output texture height.\n" );
	printf( "  -y <width>    Output texture width.\n" );
	printf( "  -s            Swap Y/Z of the normal map.\n" );
}


int main( int argc, const char** argv )
{
	try {
		//
		// init

		CCmdlineArgs args( argc, argv );
		gArgs = &args;
		const char* meshFileName = gArgs->getString( ARG_MESH );
		const char* nmapFileName = gArgs->getString( ARG_NORMALMAP );
		int outputX = gArgs->getInt( -1, ARG_X );
		int outputY = gArgs->getInt( -1, ARG_Y );
		bool swapYZ = gArgs->contains( ARG_SWAPYZ );

		bool hadErrors = false;
		if( !meshFileName ) {
			printf( "ERROR: input mesh required\n" );
			hadErrors = true;
		}
		if( !nmapFileName && (outputX < 0 && outputY < 0) ) {
			printf( "ERROR: input normalmap or output width/height required\n" );
			hadErrors = true;
		}
		if( hadErrors ) {
			gPrintUsage();
			return 1;
		}

		initD3D();

		//
		// process

		processMesh( meshFileName, nmapFileName, outputX, outputY, swapYZ );

		//
		// close

		closeD3D();
	} catch( std::runtime_error& ex ) {
		printf( "ERROR: %s\n", ex.what() );
	}
	return 0;
}
