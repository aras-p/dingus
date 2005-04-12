// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "SkinMesh.h"
#include "../../kernel/D3DDevice.h"


using namespace dingus;

CSkinMesh::CSkinMesh()
:	mMesh(0), mSkeleton(0),
	mBonesPerVertex(0)
{
	mMesh = new CMesh();
}

CSkinMesh::~CSkinMesh()
{
	ASSERT_MSG( !isCreated(), "skin mesh not cleaned up" );
	delete mMesh;
}


void CSkinMesh::createResource( const CMesh& srcMesh, const CSkeletonInfo& skelInfo )
{
	ASSERT_MSG( !isCreated(), "skin mesh not cleaned up" );

	mSkeleton = &skelInfo;

	//
	// calculate bone palette size

	mPaletteSize = 25; // TBD: better calculation of this, based on VS constant count
	if( skelInfo.getBoneCount() < mPaletteSize )
		mPaletteSize = skelInfo.getBoneCount();

	HRESULT hr;

	int ngroups = srcMesh.getGroupCount();

	//
	// create ID3DXMesh from our mesh

	ID3DXMesh* dxSrcMesh = 0;
	DWORD meshOpts = D3DXMESH_SYSTEMMEM;
	if( srcMesh.getIndexStride() == 4 )
		meshOpts |= D3DXMESH_32BIT;
	// get declaration
	D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
	UINT numElements;
	srcMesh.getVertexDecl().getObject()->GetDeclaration( decl, &numElements );
	// re-cast d3dcolor in decl into ubyte4
    D3DVERTEXELEMENT9* declP = decl;
    while( declP->Stream != 0xFF ) {
		if( declP->Usage == D3DDECLUSAGE_BLENDINDICES && declP->UsageIndex == 0 )
			declP->Type = D3DDECLTYPE_UBYTE4;
		++declP;
    }
	// create mesh
	hr = D3DXCreateMesh(
		srcMesh.getIndexCount()/3, srcMesh.getVertexCount(), meshOpts,
		decl, &CD3DDevice::getInstance().getDevice(), &dxSrcMesh );
	// copy VB
	{
		const void* srcVB = srcMesh.lockVBRead();
		void* dxVB = 0;
		hr = dxSrcMesh->LockVertexBuffer( 0, &dxVB );
		memcpy( dxVB, srcVB, srcMesh.getVertexCount() * srcMesh.getVertexStride() );
		hr = dxSrcMesh->UnlockVertexBuffer();
		srcMesh.unlockVBRead();
	}
	// copy IB
	{
		const void* srcIB = srcMesh.lockIBRead();
		void* dxIB = 0;
		hr = dxSrcMesh->LockIndexBuffer( 0, &dxIB );
		memcpy( dxIB, srcIB, srcMesh.getIndexCount() * srcMesh.getIndexStride() );
		hr = dxSrcMesh->UnlockIndexBuffer();
		srcMesh.unlockIBRead();
	}
	// copy groups and set up src bone combo table
	D3DXBONECOMBINATION* srcBoneCombs = new D3DXBONECOMBINATION[ngroups];
	DWORD* boneArray = new DWORD[skelInfo.getBoneCount()];
	for( int b = 0; b < skelInfo.getBoneCount(); ++b )
		boneArray[b] = b;
	{

		D3DXATTRIBUTERANGE* attrs = new D3DXATTRIBUTERANGE[ngroups];
		DWORD* attrBuf = 0;
		dxSrcMesh->LockAttributeBuffer( 0, &attrBuf );
		for( int g = 0; g < ngroups; ++g ) {
			attrs[g].AttribId = g;
			const CMesh::CGroup& group = srcMesh.getGroup(g);
			attrs[g].VertexStart = group.getFirstVertex();
			attrs[g].VertexCount = group.getVertexCount();
			attrs[g].FaceStart = group.getFirstPrim();
			attrs[g].FaceCount = group.getPrimCount();
			srcBoneCombs[g].AttribId = g;
			srcBoneCombs[g].FaceStart = group.getFirstPrim();
			srcBoneCombs[g].FaceCount = group.getPrimCount();
			srcBoneCombs[g].VertexStart = group.getFirstVertex();
			srcBoneCombs[g].VertexCount = group.getVertexCount();
			srcBoneCombs[g].BoneId = boneArray;
			for( int f = 0; f < group.getPrimCount(); ++f )
				*attrBuf++ = g;
		}
		dxSrcMesh->UnlockAttributeBuffer();
		hr = dxSrcMesh->SetAttributeTable( attrs, ngroups );
		delete[] attrs;
	}


	//
	// create ID3DXSkinInfo from the created mesh

	ID3DXSkinInfo* dxSkin;
	hr = D3DXCreateSkinInfoFromBlendedMesh( dxSrcMesh, skelInfo.getBoneCount(), srcBoneCombs, &dxSkin );
	delete[] boneArray;
	delete[] srcBoneCombs;


	//
	// convert to indexed blended mesh, with possibly multiple bone palettes

	DWORD* srcMeshAdjacency = new DWORD[srcMesh.getIndexCount()/3*3];
	hr = dxSrcMesh->GenerateAdjacency( 1.0e-6f, srcMeshAdjacency );
	DWORD* resMeshAdjacency = new DWORD[srcMesh.getIndexCount()/3*3];

	DWORD maxBonesPerVertUsed = 0;
	DWORD boneComboCount = 0;
	ID3DXBuffer* boneCombos = 0;
	ID3DXMesh* dxResMesh = 0;
	dxSkin->ConvertToIndexedBlendedMesh(
		dxSrcMesh, /*meshOpts*/0, mPaletteSize,
		srcMeshAdjacency, resMeshAdjacency, NULL, NULL,
		&maxBonesPerVertUsed, &boneComboCount, &boneCombos, &dxResMesh );
	mBonesPerVertex = maxBonesPerVertUsed;
	
	delete[] srcMeshAdjacency;
	// re-cast ubyte4 in decl into d3dcolor
	hr = dxResMesh->GetDeclaration( decl );
    declP = decl;
    while( declP->Stream != 0xFF ) {
		if( declP->Usage == D3DDECLUSAGE_BLENDINDICES && declP->UsageIndex == 0 )
			declP->Type = D3DDECLTYPE_D3DCOLOR;
		++declP;
    }
	hr = dxResMesh->UpdateSemantics( decl );

	
	// TEST
	DWORD strSrc = dxSrcMesh->GetNumBytesPerVertex();
	DWORD strRes = dxResMesh->GetNumBytesPerVertex();
	assert( dxSrcMesh->GetNumBytesPerVertex() == dxResMesh->GetNumBytesPerVertex() );


	//
	// optimize resulting dx mesh

	hr = dxResMesh->OptimizeInplace( D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE,
		resMeshAdjacency, NULL, NULL, NULL );
	delete[] resMeshAdjacency;
		

	//
	// create partitioned mesh

	mMesh->createResource( dxResMesh->GetNumVertices(), dxResMesh->GetNumFaces()*3,
		srcMesh.getVertexFormat(), srcMesh.getIndexStride(), srcMesh.getVertexDecl(), CMesh::BUF_STATIC );
	// copy VB
	{
		void* resVB = mMesh->lockVBWrite();
		void* dxVB = 0;
		hr = dxResMesh->LockVertexBuffer( D3DLOCK_READONLY, &dxVB );
		memcpy( resVB, dxVB, mMesh->getVertexCount() * mMesh->getVertexStride() );
		hr = dxResMesh->UnlockVertexBuffer();
		mMesh->unlockVBWrite();
	}
	// copy IB
	{
		void* resIB = mMesh->lockIBWrite();
		void* dxIB = 0;
		hr = dxResMesh->LockIndexBuffer( D3DLOCK_READONLY, &dxIB );
		memcpy( resIB, dxIB, mMesh->getIndexCount() * mMesh->getIndexStride() );
		hr = dxResMesh->UnlockIndexBuffer();
		mMesh->unlockIBWrite();
	}
	// copy groups
	{
		DWORD ngroups;
		hr = dxResMesh->GetAttributeTable( NULL, &ngroups );
		assert( ngroups == boneComboCount );
		D3DXATTRIBUTERANGE* attrs = new D3DXATTRIBUTERANGE[ngroups];
		hr = dxResMesh->GetAttributeTable( attrs, &ngroups );
		for( int g = 0; g < ngroups; ++g ) {
			const D3DXATTRIBUTERANGE& gr = attrs[g];
			mMesh->addGroup( CMesh::CGroup(gr.VertexStart, gr.VertexCount, gr.FaceStart, gr.FaceCount) );
		}
		delete[] attrs;
	}
	mMesh->computeAABBs();
	// copy palette infos
	{
		const D3DXBONECOMBINATION* boneComb = reinterpret_cast<const D3DXBONECOMBINATION*>( boneCombos->GetBufferPointer() );
		for( int bc = 0; bc < boneComboCount; ++bc ) {
			mPalettes.push_back( CSkinBonePalette() );
			CSkinBonePalette& bpal = mPalettes.back();
			bpal.beginPalette( boneComb->AttribId );
			for( int b = 0; b < mPaletteSize; ++b )
				bpal.setBone( b, boneComb->BoneId[b] );
			bpal.endPalette();
			++boneComb;
		}
	}

	//
	// release stuff

	ULONG rc;
	rc = dxSrcMesh->Release();
	rc = dxSkin->Release();
	rc = dxResMesh->Release();
	rc = boneCombos->Release();
}

void CSkinMesh::deleteResource()
{
	ASSERT_MSG( isCreated(), "skin mesh not created" );

	mMesh->deleteResource();
	mSkeleton = 0;

	mBonesPerVertex = 0;
	mPalettes.clear();
}
