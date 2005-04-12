// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "MeshSerializer.h"
#include "VertexDeclBundle.h"

using namespace dingus;

#define READ_4BYTE(var) fread(&var,1,4,f)
#define READ_2BYTE(var) fread(&var,1,2,f)

static inline std::string gReadString( FILE* f )
{
	std::string str;
	for(;;) {
		int c = fgetc( f );
		if( c==0 || c==EOF )
			break;
		str += (char)c;
	}
	return str;
}

bool CMeshSerializer::loadMeshFromFile( const char* fileName, CMesh& mesh )
{
	// open file
	FILE* f = fopen( fileName, "rb" );
	if( !f ) return false;

	//
	// read header

	// magic
	char magic[4];
	READ_4BYTE(magic);
	if( magic[0]!='D' || magic[1]!='M' || magic[2]!='S' || magic[3]!='H' ) {
		fclose( f );
		return false;
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

	// init mesh
	assert( !mesh.isCreated() );
	CVertexFormat format(vformat);
	assert( format.calcVertexSize() == vstride );
	CD3DVertexDecl* vertDecl = RGET_VDECL( CVertexDesc( format ) );
	mesh.createResource( nverts, ntris*3, format, istride, *vertDecl, CMesh::BUF_STATIC );

	// read vertices
	void* vb = mesh.lockVBWrite();
	fread( vb, vstride, nverts, f );
	mesh.unlockVBWrite();

	// read indices
	void* ib = mesh.lockIBWrite();
	fread( ib, istride, ntris*3, f );
	mesh.unlockIBWrite();

	// read groups
	for( int i = 0; i < ngroups; ++i ) {
		int vstart, vcount, fstart, fcount;
		READ_4BYTE(vstart);
		READ_4BYTE(vcount);
		READ_4BYTE(fstart);
		READ_4BYTE(fcount);
		// HACK: TBD: for null groups
		if( ngroups==1 && vcount==0 && fcount==0 ) {
			vstart = 0;
			vcount = nverts;
			fstart = 0;
			fcount = ntris;
		}
		mesh.addGroup( CMesh::CGroup(vstart,vcount,fstart,fcount) );
	}

	// close file
	fclose( f );
	return true;
}


CSkeletonInfo* CMeshSerializer::loadSkelInfoFromFile( const char* fileName )
{
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

	CVertexFormat format(vformat);
	if( format.getSkinDataMode() == CVertexFormat::FLT3_NONE )
		return NULL;

	// skip mesh geometry data
	int skelDataOffset = 0;
	skelDataOffset += vstride * nverts; // vertices
	skelDataOffset += istride * ntris * 3; // indices
	skelDataOffset += 4*4 * ngroups; // groups
	fseek( f, skelDataOffset, SEEK_CUR );

	// read skel info
	int nbones;
	READ_4BYTE(nbones);
	assert( nbones > 0 );
	CSkeletonInfo* skelInfo = new CSkeletonInfo( nbones );
	for( int b = 0; b < nbones; ++b ) {
		std::string name = gReadString( f );
		int parentIdx;
		SMatrix4x4 boneMat;
		boneMat.identify();
		READ_4BYTE(parentIdx);
		READ_4BYTE(boneMat._11); READ_4BYTE(boneMat._12); READ_4BYTE(boneMat._13);
		READ_4BYTE(boneMat._21); READ_4BYTE(boneMat._22); READ_4BYTE(boneMat._23);
		READ_4BYTE(boneMat._31); READ_4BYTE(boneMat._32); READ_4BYTE(boneMat._33);
		READ_4BYTE(boneMat._41); READ_4BYTE(boneMat._42); READ_4BYTE(boneMat._43);
		skelInfo->addBoneInfo( parentIdx, boneMat, name );
	}

	// close file
	fclose( f );
	return skelInfo;
}

