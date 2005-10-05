#include "IGameExporter.h"
#include "VertexFormat.h"
#include <windowsx.h>
#include "NVMeshMender.h"

static IGameExporterClassDesc IGameExporterDesc;
ClassDesc2* GetIGameExporterDesc() { return &IGameExporterDesc; }


const int CFG_OPTIONS_VERSION = 20051005;


// --------------------------------------------------------------------------
//  Debugging
// --------------------------------------------------------------------------


FILE* gDebugFile = 0;
void IGameExporter::debugMsg( const char* msg, ... ) const
{
	if( !mOptions.mDebugOutput )
		return;

	if( gDebugFile ) {
		va_list args;
		va_start( args, msg );
		vfprintf( gDebugFile, msg, args );
		va_end( args );
		fprintf( gDebugFile, "\n" );
		fflush( gDebugFile );
	}
}



// --------------------------------------------------------------------------
//  UI
// --------------------------------------------------------------------------



#define CHECK_FROM(ctrl,member) CheckDlgButton( wnd, ctrl, (member) ? BST_CHECKED : BST_UNCHECKED )
#define CHECK_TO(ctrl,member) member = (IsDlgButtonChecked( wnd, ctrl ) ? 1 : 0)

static void gOptionsToDialog( HWND wnd, const SExportOptions& o )
{
	char buf[200];
	sprintf( buf, "%g", o.mUnitMultiplier );
	SetDlgItemText( wnd, IDC_EDIT_UNITMULT, buf );

	//CHECK_FROM( IDC_CHK_LOCALSPACE, o.mUseLocalSpace );
	CHECK_FROM( IDC_CHK_POSITIONS, o.mDoPositions );
	CHECK_FROM( IDC_CHK_NORMALS, o.mDoNormals );
	CHECK_FROM( IDC_CHK_TANGENTS, o.mDoTangents );
	CHECK_FROM( IDC_CHK_BINORMALS, o.mDoBinormals );
	CHECK_FROM( IDC_CHK_NORMALSCOLOR, o.mColorEncodeNTB );
	CHECK_FROM( IDC_CHK_UV0, o.mDoUVs[0] );
	CHECK_FROM( IDC_CHK_UV1, o.mDoUVs[1] );
	CHECK_FROM( IDC_CHK_UV2, o.mDoUVs[2] );
	CHECK_FROM( IDC_CHK_UV3, o.mDoUVs[3] );
	CHECK_FROM( IDC_CHK_SKIN, o.mDoSkin );
	CHECK_FROM( IDC_CHK_1BONESKIN, o.mCreate1BoneSkin );
	CHECK_FROM( IDC_CHK_STRIP_BIP, o.mStripBipFromBones );
	CHECK_FROM( IDC_CHK_DEBUG, o.mDebugOutput );

	HWND comboUV = GetDlgItem( wnd, IDC_CMB_TANGENTUV );
	ComboBox_ResetContent( comboUV );
	ComboBox_AddString( comboUV, "Base (0)" );
	ComboBox_AddString( comboUV, "UV1" );
	ComboBox_AddString( comboUV, "UV2" );
	ComboBox_AddString( comboUV, "UV3" );
	ComboBox_SetCurSel( comboUV, o.mTangentsUseUV );

	HWND comboSkin = GetDlgItem( wnd, IDC_CMB_SKINBONES );
	ComboBox_ResetContent( comboSkin );
	ComboBox_AddString( comboSkin, "1" );
	ComboBox_AddString( comboSkin, "2" );
	ComboBox_AddString( comboSkin, "3" );
	ComboBox_AddString( comboSkin, "4" );
	ComboBox_SetCurSel( comboSkin, o.mSkinBones-1 ); // bones start at 1, combo indices at 0
}

static void gOptionsFromDialog( HWND wnd, SExportOptions& o )
{
	//CHECK_TO( IDC_CHK_LOCALSPACE, o.mUseLocalSpace );
	CHECK_TO( IDC_CHK_POSITIONS, o.mDoPositions );
	CHECK_TO( IDC_CHK_NORMALS, o.mDoNormals );
	CHECK_TO( IDC_CHK_TANGENTS, o.mDoTangents );
	CHECK_TO( IDC_CHK_BINORMALS, o.mDoBinormals );
	CHECK_TO( IDC_CHK_NORMALSCOLOR, o.mColorEncodeNTB );
	CHECK_TO( IDC_CHK_UV0, o.mDoUVs[0] );
	CHECK_TO( IDC_CHK_UV1, o.mDoUVs[1] );
	CHECK_TO( IDC_CHK_UV2, o.mDoUVs[2] );
	CHECK_TO( IDC_CHK_UV3, o.mDoUVs[3] );
	CHECK_TO( IDC_CHK_SKIN, o.mDoSkin );
	CHECK_TO( IDC_CHK_1BONESKIN, o.mCreate1BoneSkin );
	CHECK_TO( IDC_CHK_STRIP_BIP, o.mStripBipFromBones );
	CHECK_TO( IDC_CHK_DEBUG, o.mDebugOutput );

	HWND comboUV = GetDlgItem( wnd, IDC_CMB_TANGENTUV );
	o.mTangentsUseUV = ComboBox_GetCurSel( comboUV );
	
	HWND comboSkin = GetDlgItem( wnd, IDC_CMB_SKINBONES );
	o.mSkinBones = ComboBox_GetCurSel( comboSkin ) + 1;
	if( o.mSkinBones < 1 )
		o.mSkinBones = 1;

	char buf[200];
	GetDlgItemText( wnd, IDC_EDIT_UNITMULT, buf, 200 );
	o.mUnitMultiplier = atof( buf );
}

BOOL CALLBACK IGameExporterOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	IGameExporter *exp = (IGameExporter*)GetWindowLongPtr( hWnd, GWLP_USERDATA );

	switch(message) {
	case WM_INITDIALOG:
		exp = (IGameExporter*)lParam;
		SetWindowLongPtr( hWnd,GWLP_USERDATA,lParam );
		CenterWindow( hWnd,GetParent(hWnd) );
		gOptionsToDialog( hWnd, exp->mOptions );
		return TRUE;
		
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			gOptionsFromDialog( hWnd, exp->mOptions );
			EndDialog( hWnd, 1 );
			break;
		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		default:
			return FALSE;
	}
	return TRUE;
}


// --------------------------------------------------------------------------
//  Stuff
// --------------------------------------------------------------------------


IGameExporter::IGameExporter()
:	mFile(0),
	mGameScene(0)
{
}

IGameExporter::~IGameExporter() 
{
	if( mFile ) {
		fclose( mFile );
		mFile = 0;
	}
}


int IGameExporter::ExtCount() { return 1; } // one extension
const TCHAR *IGameExporter::Ext( int n ) { return _T("dmesh"); }
const TCHAR *IGameExporter::LongDesc() { return _T("IM mesh exporter"); }
const TCHAR *IGameExporter::ShortDesc() { return _T("IMMeshExporter"); }
const TCHAR *IGameExporter::AuthorName() { return _T("Aras Pranckevicius"); }
const TCHAR *IGameExporter::CopyrightMessage() { return _T("(c)2003-2004 InteraMotion LLC"); }
const TCHAR *IGameExporter::OtherMessage1() { return _T(""); }
const TCHAR *IGameExporter::OtherMessage2() { return _T(""); }
unsigned int IGameExporter::Version() {	return 100; } // version number * 100
void IGameExporter::ShowAbout(HWND hWnd) { /* Optional */ }

BOOL IGameExporter::SupportsOptions(int ext, DWORD options)
{
	return TRUE;
}


// --------------------------------------------------------------------------
//  Node traversal
// --------------------------------------------------------------------------

const char* IGameExporter::gatherNode( IGameNode* node )
{
	// skip unselected ones
	if( node->GetMaxNode()->Selected() ) {
		if( node->IsGroupOwner() ) {
			// nothing
			return "Node is a group owner";
		} else {
			const char* result = "Node is not a mesh";
			IGameObject* obj = node->GetIGameObject();
			if( obj->GetIGameType() == IGameObject::IGAME_MESH ) {
				IGameMesh* mesh = (IGameMesh*)obj;
				SNodeInfo* nodeInfo = new SNodeInfo( *node, *mesh );
				mNodes.Append( 1, &nodeInfo );
				result = gatherMesh( *nodeInfo );
			}
			//return result;
		}
	}

	// children
	const char* result = 0;
	for( int i = 0; i < node->GetChildCount(); ++i ) {
		const char* r = gatherNode( node->GetNodeChild(i) );
		if( r )
			result = r;
	}
	return result;
}

const char* IGameExporter::gatherMesh( SNodeInfo& info )
{
	debugMsg( "gather mesh %s...", info.node->GetName() );

	info.mesh->SetUseWeightedNormals();
	bool okInited = info.mesh->InitializeData();
	if( !okInited )
		return "Failed to initialize mesh";

	mTotalVerts += info.mesh->GetNumberOfFaces()*3;
	mTotalTris += info.mesh->GetNumberOfFaces();

	gatherSkin( info );
	if( info.maxBonesPerVert > mTotalMaxBonesPerVert )
		mTotalMaxBonesPerVert = info.maxBonesPerVert;

	return 0;
}



// --------------------------------------------------------------------------
//  Skinning processing
// --------------------------------------------------------------------------


struct SVertBoneInfo {
	float	weight;
	int		index;
};

int __cdecl gBoneInfoSortFn( const void *elem1, const void *elem2 )
{
	const SVertBoneInfo& a = *(const SVertBoneInfo*)elem1;
	const SVertBoneInfo& b = *(const SVertBoneInfo*)elem2;
	float ab = a.weight - b.weight;
	if( ab > 0.0f ) return -1;
	if( ab < 0.0f ) return 1;
	return 0;
}

void IGameExporter::reorderBones( TNodeTab& inBones, TNodeTab& outBones )
{
	// find root
	int n = inBones.Count();
	for( int i = 0; i < n; ++i ) {
		IGameNode* node = inBones[i];
		// has parent?
		bool root = true;
		for( int j = 0; j < n; ++j ) {
			if( node->GetNodeParent() == inBones[j] ) {
				root = false;
				break;
			}
		}
		if( root )
			recurseAddBones( inBones, i, outBones );
	}
}

void IGameExporter::recurseAddBones( TNodeTab& inBones, int boneIdx, TNodeTab& outBones )
{
	IGameNode* bone = inBones[boneIdx];
	outBones.Append( 1, &bone );
	int nchilds = bone->GetChildCount();
	int n = inBones.Count();
	for( int c = 0; c < nchilds; ++c ) {
		IGameNode* child = bone->GetNodeChild( c );
		for( int b = 0; b < n; ++b ) {
			if( inBones[b] == child )
				recurseAddBones( inBones, b, outBones );
		}
	}
}

void IGameExporter::gatherSkin( SNodeInfo& info )
{
	info.maxBonesPerVert = -1;
	int i;
	
	// not skinned?
	if( !info.mesh->IsObjectSkinned() )
		return;

	debugMsg( "node %s is skinned...", info.node->GetName() );
	
	IGameSkin* skin = info.mesh->GetIGameSkin();
	
	// create skin info now
	int nverts = info.mesh->GetNumberOfVerts();
	info.weights = new D3DXVECTOR3[nverts];
	memset( info.weights, 0, nverts*sizeof(info.weights[0]) );
	info.indices = new DWORD[nverts];
	memset( info.indices, 0, nverts*sizeof(info.indices[0]) );
	info.bones = new Tab<IGameNode*>();

	// go through all vertices and collect bone nodes
	TNodeTab tempBones;
	for( i = 0; i < nverts; ++i ) {
		int nb = skin->GetNumberOfBones( i );
		for( int b = 0; b < nb; ++b ) {
			IGameNode* vbone = skin->GetIGameBone( i, b );
			int bb;
			for( bb = 0; bb < tempBones.Count(); ++bb ) {
				if( tempBones[bb]->GetNodeID() == vbone->GetNodeID() )
					break;
			}
			if( bb == tempBones.Count() )
				tempBones.Append( 1, &vbone );
		}
	}

	// now, reorder bone nodes so that they are laid out in a depth-first
	// traversal order
	Tab<IGameNode*>& bones = *info.bones;
	reorderBones( tempBones, bones );
	assert( tempBones.Count() == bones.Count() );

	// go through all vertices and collect skinning info
	for( i = 0; i < nverts; ++i ) {
		int b;
		int nb = skin->GetNumberOfBones( i );
		// gather vertex bone weights and indices
		Tab<SVertBoneInfo> allwhts;
		//allwhts.Resize( nb );
		for( b = 0; b < nb; ++b ) {
			SVertBoneInfo vbi;
			vbi.weight = skin->GetWeight( i, b );
			//if( vbi.weight < 1.0e-5f ) // TBD: configurable
			//	continue;
			IGameNode* vbone = skin->GetIGameBone( i, b );
			int bb;
			for( bb = 0; bb < bones.Count(); ++bb ) {
				if( bones[bb]->GetNodeID() == vbone->GetNodeID() )
					break;
			}
			assert( bb < bones.Count() );
			vbi.index = bb;
			allwhts.Append( 1, &vbi );
		}
		nb = allwhts.Count();
		allwhts.Sort( gBoneInfoSortFn );
		// trim to max. possible
		const int MAX_BONES_POSSIBLE = 4;
		if( nb > MAX_BONES_POSSIBLE )
			nb = MAX_BONES_POSSIBLE;
		if( nb > info.maxBonesPerVert )
			info.maxBonesPerVert = nb;
		D3DXVECTOR4 whts;
		whts.x = whts.y = whts.z = whts.w = 0.0f;
		DWORD idx = 0x00000000;
		for( b = 0; b < nb; ++b ) {
			whts[b] = allwhts[b].weight;
			idx |= (allwhts[b].index & 0xFF) << (b*8); // first index in lowest byte
		}
		// normalize weights (in case of trimming)
		float wsum = whts.x + whts.y + whts.z + whts.w;
		if( wsum != 0.0f )
			whts /= wsum;
		// remember weights and indices
		info.weights[i] = D3DXVECTOR3(whts.x,whts.y,whts.z);
		info.indices[i] = idx;
	}
	debugMsg( "  skin: bone count %i, max bones per vert %i", info.bones->Count(), info.maxBonesPerVert );
}



// --------------------------------------------------------------------------
//  Mesh construction and processing
// --------------------------------------------------------------------------


const char* IGameExporter::meshCreate()
{
	debugMsg( "mesh create..." );

	mMeshBones.ZeroCount();
	mMeshMats.ZeroCount();
	mMesh.clearMesh();
	memset( mMeshHasUVs, 0, sizeof(mMeshHasUVs) );

	// empty?
	if( mTotalVerts < 1 || mTotalTris < 1 )
		return "Empty mesh";

	// create
	assert( mDxDevice );
	mproc::d3dDevice = mDxDevice;
	HRESULT hr;
	hr = mproc::createMesh( mTotalTris, mTotalTris*3, mMesh );
	if( FAILED(hr) )
		return "Failed to create mesh";
	assert( mMesh.isValid() );

	return 0;
}


const char* IGameExporter::meshAddNode( SNodeInfo& info, int& vertOffset, int& triOffset )
{
	debugMsg( "mesh add node %s...", info.node->GetName() );
	assert( mMesh.isValid() );

	//
	// common params

	IGameMesh& mesh = *info.mesh;
	int nverts = mesh.GetNumberOfVerts();
	int nfaces = mesh.GetNumberOfFaces();
	debugMsg( "    verts=%i faces=%i", nverts, nfaces );

	int f;

	bool hasuvs[mproc::UV_COUNT];
	Tab<int> uvnumbers = mesh.GetActiveMapChannelNum(); // DOES include base UV channel (though IGame docs seem to say otherwise)
	for( f = 0; f < mproc::UV_COUNT; ++f ) {
		hasuvs[f] = mOptions.mDoUVs[f] ? true : false;
		if( f >= uvnumbers.Count() ) // no UV channel - don't export
			hasuvs[f] = false;
		if( hasuvs[f] )
			mMeshHasUVs[f] = true;
	}
	debugMsg( "    hasuvs: 0=%i 1=%i 2=%i 3=%i", hasuvs[0], hasuvs[1], hasuvs[2], hasuvs[3] );
	for( f = 0; f < uvnumbers.Count(); ++f )
		debugMsg( "    uvch%i = %i", f, uvnumbers[f] );

	bool skinned = info.isSkinned();
	debugMsg( "    skinned: %i", skinned );

	//
	// just create 3 vertices for each face. will optimize later

	HRESULT hr;

	debugMsg( "  fill mesh portion..." );

	mproc::SVertex* v = 0;
	hr = mMesh.getMesh().LockVertexBuffer( D3DLOCK_NOOVERWRITE, (void**)&v );
	assert( SUCCEEDED(hr) );
	assert( v );
	v += vertOffset;

	unsigned int* ib = 0;
	hr = mMesh.getMesh().LockIndexBuffer( D3DLOCK_NOOVERWRITE, (void**)&ib );
	assert( SUCCEEDED(hr) );
	assert( ib );
	ib += triOffset*3;

	DWORD* attr = 0;
	hr = mMesh.getMesh().LockAttributeBuffer( D3DLOCK_NOOVERWRITE, &attr );
	assert( SUCCEEDED(hr) );
	assert( attr );
	attr += triOffset;

	DWORD mapfaceidx[mproc::UV_COUNT][3];
	for( f = 0; f < nfaces; ++f ) {
		int i;
		FaceEx* face = mesh.GetFace( f );
		assert( face );
		
		// other UV channels' vertex indices
		for( i = 0; i < mproc::UV_COUNT; ++i ) {
			if( hasuvs[i] )
				bool ok = mesh.GetMapFaceIndex( uvnumbers[i], f, mapfaceidx[i] );
		}

		// vertices
		Point3 p;
		Point3 uv3;
		for( i = 0; i < 3; ++i ) {
			int vidx = face->vert[i];
			mesh.GetVertex( vidx, p, false );
			v->p.x = p.x; v->p.y = p.y; v->p.z = p.z;
			v->p *= mOptions.mUnitMultiplier;
			if( skinned ) {
				v->w = info.weights[vidx];
				v->i = info.indices[vidx];
			} else {
				v->w.x = v->w.y = v->w.z = 0.0f;
				v->i = 0;
			}
			mesh.GetNormal( f, i, p );
			v->n.x = p.x; v->n.y = p.y; v->n.z = p.z;
			D3DXVec3Normalize( &v->n, &v->n );
			v->t.x = 1; v->t.y = 0; v->t.z = 0;
			v->b.x = 1; v->b.y = 0; v->b.z = 0;
			// UV channels
			for( int uvc = 0; uvc < mproc::UV_COUNT; ++uvc ) {
				if( hasuvs[uvc] ) {
					mesh.GetMapVertex( uvnumbers[uvc], mapfaceidx[uvc][i], uv3 );
					v->uv[uvc].x = uv3.x;
					// NOTE: need to output 1-V coordinate
					v->uv[uvc].y = 1.0f-uv3.y;
				} else {
					v->uv[uvc].x = v->uv[uvc].y = 0.0f;
				}
			}
			++v;
		}
		
		// indices
		ib[0] = vertOffset + f*3+0;
		ib[1] = vertOffset + f*3+1;
		ib[2] = vertOffset + f*3+2;
		ib += 3;

		// material
		IGameMaterial* m = mesh.GetMaterialFromFace( face );
		int nm = mMeshMats.Count();
		for( int midx = 0; midx < nm; ++midx ) {
			if( mMeshMats[midx] == m )
				break;
		}
		if( midx >= nm )
			mMeshMats.Append( 1, &m );
		attr[f] = midx;
	}
	mMesh.getMesh().UnlockVertexBuffer();
	mMesh.getMesh().UnlockIndexBuffer();
	mMesh.getMesh().UnlockAttributeBuffer();

	if( info.bones )
		mMeshBones.Append( info.bones->Count(), &(*info.bones)[0] );

	vertOffset += nfaces*3;
	triOffset += nfaces;

	return 0;
}


const char* IGameExporter::meshProcess()
{
	debugMsg( "mesh process..." );
	assert( mMesh.isValid() );

	HRESULT hr;

	//
	// tangent space

	if( mOptions.mDoTangents || mOptions.mDoBinormals ) {
		debugMsg( "  tangent space..." );

		int i, n;
		MeshMender mender;
		std::vector< MeshMender::Vertex > mendVerts;
		std::vector< unsigned int > mendInds;
		std::vector< unsigned int > mendNew2Old;

		// fill mender VB
		const mproc::SVertex* vb = 0;
		hr = mMesh.getMesh().LockVertexBuffer( D3DLOCK_READONLY, (void**)&vb );
		assert( SUCCEEDED(hr) );
		assert( vb );
		n = mMesh.getMesh().GetNumVertices();
		for( i = 0; i < n; ++i ) {
			MeshMender::Vertex mv;
			mv.pos = vb[i].p;
			mv.normal = vb[i].n;
			mv.s = vb[i].uv[mOptions.mTangentsUseUV].x;
			mv.t = vb[i].uv[mOptions.mTangentsUseUV].y;
			mendVerts.push_back( mv );
		}

		// fill mender IB
		const unsigned int* ib = 0;
		hr = mMesh.getMesh().LockIndexBuffer( D3DLOCK_READONLY, (void**)&ib );
		assert( SUCCEEDED(hr) );
		assert( ib );
		n = mMesh.getMesh().GetNumFaces() * 3;
		for( i = 0; i < n; ++i ) {
			mendInds.push_back( ib[i] );
		}
		mMesh.getMesh().UnlockIndexBuffer();

		// mend
		mender.Mend( mendVerts, mendInds, mendNew2Old,
			-0.1f, -0.1f, -0.1f, 1.0f,
			MeshMender::DONT_CALCULATE_NORMALS,
			MeshMender::DONT_RESPECT_SPLITS,
			MeshMender::DONT_FIX_CYLINDRICAL );
		debugMsg( "    mended verts %i", mendVerts.size() );

		//
		// replace mesh

		mproc::CMesh mendedMesh;
		hr = mproc::createMesh( mMesh.getMesh().GetNumFaces(), mendVerts.size(), mendedMesh );
		assert( SUCCEEDED(hr) );

		// fill mended VB
		mproc::SVertex* vbmend = 0;
		hr = mendedMesh.getMesh().LockVertexBuffer( 0, (void**)&vbmend );
		assert( SUCCEEDED(hr) );
		assert( vbmend );
		n = mendVerts.size();
		for( i = 0; i < n; ++i ) {
			vbmend[i] = vb[ mendNew2Old[i] ];
			vbmend[i].t = mendVerts[i].tangent;
			vbmend[i].b = mendVerts[i].binormal;
		}
		mendedMesh.getMesh().UnlockVertexBuffer();
		mMesh.getMesh().UnlockVertexBuffer();

		// fill mended IB
		unsigned int* ibmend = 0;
		hr = mendedMesh.getMesh().LockIndexBuffer( 0, (void**)&ibmend );
		assert( SUCCEEDED(hr) );
		assert( ibmend );
		n = mendInds.size();
		for( i = 0; i < n; ++i ) {
			ibmend[i] = mendInds[i];
		}
		mendedMesh.getMesh().UnlockIndexBuffer();

		// fill mended attr
		hr = mproc::copyAttrBuf( mMesh, mendedMesh );
		assert( SUCCEEDED(hr) );

		// replace old with mended mesh
		mMesh = mendedMesh;
	}

	// generate mesh adjacency
	{
		debugMsg( "  generate adjacency..." );
		mMesh.calcAdjacency();
	}

	// clean mesh
	{
		debugMsg( "  clean mesh..." );
		mproc::CMesh cleanedMesh;
		hr = mproc::cleanMesh( mMesh, cleanedMesh );
		if( FAILED(hr) )
			return "Failed to clean mesh";
		mMesh = cleanedMesh;
		debugMsg( "    cleaned: verts=%i faces=%i", mMesh.getMesh().GetNumVertices(), mMesh.getMesh().GetNumFaces() );
	}

	// weld vertices
	{
		debugMsg( "  weld vertices..." );
		hr = mMesh.weldVertices();
		if( FAILED(hr) )
			return "Failed to weld mesh";
		debugMsg( "    welded: verts=%i faces=%i", mMesh.getMesh().GetNumVertices(), mMesh.getMesh().GetNumFaces() );
	}

	// optimize the mesh
	{
		debugMsg( "  optimize mesh..." );
		mproc::CMesh optiMesh;
		hr = mproc::optimizeMesh( mMesh, optiMesh );
		if( FAILED(hr) )
			return "Failed to optimize mesh";
		mMesh = optiMesh;
		debugMsg( "    optimized: verts=%i faces=%i", mMesh.getMesh().GetNumVertices(), mMesh.getMesh().GetNumFaces() );
	}

	return 0;
}


const char* IGameExporter::meshWrite()
{
	debugMsg( "mesh write..." );
	assert( mMesh.isValid() );

	//
	// figure out vertex format

	//
	// setup vertex format

	debugMsg( "  figure vertex format..." );

	bool skinned = (mTotalMaxBonesPerVert > 0);
	debugMsg( "    max bones per vert: %i", mTotalMaxBonesPerVert );
	debugMsg( "    exporting bones per vert: %i", (mOptions.mDoSkin && skinned) ? mOptions.mSkinBones : 0 );

	DWORD formatBits = 0;
	if( mOptions.mDoPositions )
		formatBits |= CVertexFormat::V_POSITION;
	if( mOptions.mColorEncodeNTB ) {
		// color encoded normal/tangent/binormal
		if( mOptions.mDoNormals )
			formatBits |= CVertexFormat::V_NORMALCOL;
		if( mOptions.mDoTangents )
			formatBits |= CVertexFormat::V_TANGENTCOL;
		if( mOptions.mDoBinormals )
			formatBits |= CVertexFormat::V_BINORMCOL;
	} else {
		// plain float3 normal/tangent/binormal
		if( mOptions.mDoNormals )
			formatBits |= CVertexFormat::V_NORMAL;
		if( mOptions.mDoTangents )
			formatBits |= CVertexFormat::V_TANGENT;
		if( mOptions.mDoBinormals )
			formatBits |= CVertexFormat::V_BINORM;
	}
	if( mOptions.mDoSkin && skinned ) {
		formatBits |= CVertexFormat::V_SKIN_WHT; // TBD: other weight encodings
		DWORD bb[4] = { CVertexFormat::V_SKIN_1, CVertexFormat::V_SKIN_2, CVertexFormat::V_SKIN_3, CVertexFormat::V_SKIN_4 };
		formatBits |= bb[mOptions.mSkinBones-1];
	}
	for( int f = 0; f < mproc::UV_COUNT; ++f ) {
		if( mMeshHasUVs[f] )
			formatBits |= (CVertexFormat::UV_2D) << ( (CVertexFormat::UV_BITS) + (CVertexFormat::UV_SHIFT)*f );
	}

	//
	// write data
	
	debugMsg( "  write mesh data..." );

	const char* writeErr = writeMeshData( mMesh.getMesh(), formatBits );
	const char* writeSkinErr = 0;
	if( skinned )
		writeSkinErr = writeSkinData( mMeshBones, mOptions.mSkinBones );

	if( writeErr )
		return writeErr;
	if( writeSkinErr )
		return writeSkinErr;
	return 0;
}


const char* IGameExporter::cleanupMem()
{
	for( int i = 0; i < mNodes.Count(); ++i )
		delete mNodes[i];
	mNodes.ZeroCount();

	mMeshBones.ZeroCount();
	mMeshMats.ZeroCount();

	mMesh.clearMesh();

	return 0;
}



// --------------------------------------------------------------------------
//   File writing
// --------------------------------------------------------------------------

static DWORD gVectorToColor( const D3DXVECTOR3& v ) {
	float vx = (v.x + 1.0f) * 127.5f;
	float vy = (v.y + 1.0f) * 127.5f;
	float vz = (v.z + 1.0f) * 127.5f;
	int nx = int(vx) & 255;
	int ny = int(vy) & 255;
	int nz = int(vz) & 255;
	return (nx<<16) | (ny<<8) | (nz<<0);
}


const char* IGameExporter::writeMeshData( ID3DXMesh& mesh, DWORD formatBits )
{
	assert( mFile );

	int nverts = mesh.GetNumVertices();
	int nfaces = mesh.GetNumFaces();
	int nattrs;
	mproc::SVertex* vb;
	unsigned int* ib;

	bool ib32bit = (nfaces > 20000);
	CVertexFormat format(formatBits);
	int vstride = format.calcVertexSize();
	int istride = ib32bit ? 4 : 2;
	
	mesh.GetAttributeTable( 0, (DWORD*)&nattrs );
	D3DXATTRIBUTERANGE* attr = new D3DXATTRIBUTERANGE[nattrs];
	mesh.GetAttributeTable( attr, (DWORD*)&nattrs );
	mesh.LockVertexBuffer( D3DLOCK_READONLY, (void**)&vb );
	mesh.LockIndexBuffer( D3DLOCK_READONLY, (void**)&ib );

	// header
	fwrite( "DMSH", 1, 4, mFile ); // magic "DMSH"
	fwrite( &nverts, 1, 4, mFile ); // vert count
	fwrite( &nfaces, 1, 4, mFile ); // face count
	fwrite( &nattrs, 1, 4, mFile ); // group count
	fwrite( &vstride, 1, 4, mFile ); // vertex stride
	fwrite( &formatBits, 1, 4, mFile ); // vertex format flags
	fwrite( &istride, 1, 4, mFile ); // index stride
	
	// vertices
	int i;
	for( i = 0; i < nverts; ++i ) {
		const mproc::SVertex& v = vb[i];
		// position?
		if( format.hasPosition() ) {
			fwrite( &v.p, 1, 4*3, mFile );
		}
		// skin weights, indices
		if( format.getSkinDataMode() != CVertexFormat::FLT3_NONE ) {
			// TBD: other weight encodings
			// weights
			int ss = format.getSkinMode();
			if( ss )
				fwrite( &v.w, 1, 4*ss, mFile );
			fwrite( &v.i, 1, 4, mFile );
		}
		// normal/tangent/binormal
		CVertexFormat::eFloat3Mode modeN = format.getNormalMode();
		CVertexFormat::eFloat3Mode modeT = format.getTangentMode();
		CVertexFormat::eFloat3Mode modeB = format.getBinormMode();
		// plain float3 n/t/b?
		if( modeN == CVertexFormat::FLT3_FLOAT3 )
			fwrite( &v.n, 1, 4*3, mFile );
		if( modeT == CVertexFormat::FLT3_FLOAT3 )
			fwrite( &v.t, 1, 4*3, mFile );
		if( modeB == CVertexFormat::FLT3_FLOAT3 )
			fwrite( &v.b, 1, 4*3, mFile );
		// color encoded n/t/b?
		if( modeN == CVertexFormat::FLT3_COLOR ) {
			DWORD col = gVectorToColor( v.n );
			fwrite( &col, 1, 4, mFile );
		}
		if( modeT == CVertexFormat::FLT3_COLOR ) {
			DWORD col = gVectorToColor( v.t );
			fwrite( &col, 1, 4, mFile );
		}
		if( modeB == CVertexFormat::FLT3_COLOR ) {
			DWORD col = gVectorToColor( v.b );
			fwrite( &col, 1, 4, mFile );
		}
		// UV?
		for( int k = 0; k < mproc::UV_COUNT; ++k ) {
			if( format.getUVMode(k) == CVertexFormat::UV_2D )
				fwrite( &v.uv[k], 1, 4*2, mFile );
		}
	}

	// indices
	for( i = 0; i < nfaces; ++i ) {
		const unsigned int *idx = ib + i*3;
		if( ib32bit ) {
			fwrite( idx, 1, 3*4, mFile );
		} else {
			unsigned short ib0 = unsigned short( idx[0] );
			unsigned short ib1 = unsigned short( idx[1] );
			unsigned short ib2 = unsigned short( idx[2] );
			fwrite( &ib0, 2, 1, mFile );
			fwrite( &ib1, 2, 1, mFile );
			fwrite( &ib2, 2, 1, mFile );
		}
	}

	// groups
	for( i = 0; i < nattrs; ++i ) {
		const D3DXATTRIBUTERANGE& a = attr[i];
		fwrite( &a.VertexStart, 1, 4, mFile );
		fwrite( &a.VertexCount, 1, 4, mFile );
		fwrite( &a.FaceStart, 1, 4, mFile );
		fwrite( &a.FaceCount, 1, 4, mFile );
	}

	delete[] attr;
	mesh.UnlockIndexBuffer();
	mesh.UnlockVertexBuffer();

	return 0;
}

static void gCopyStringStripBip( char* dest, const char* src, int n, bool stripBip ) {
	if( stripBip ) {
		if( toupper(src[0]) == 'B' &&
			toupper(src[1]) == 'I' &&
			toupper(src[2]) == 'P' )
		{
			src += 3;
			while( isdigit(src[0]) )
				++src;
			while( isspace(src[0]) )
				++src;
		}
	}
	strncpy( dest, src, n );
	return;
}

const char* IGameExporter::writeSkinData( TNodeTab& bones, int maxBonesPerVert )
{
	int nbones = bones.Count();
	assert( mFile );

	// bone count
	fwrite( &nbones, 1, 4, mFile );

	// bone info
	for( int b = 0; b < nbones; ++b ) {
		IGameNode* bone = bones[b];
		// bone name
		char buf[200];
		gCopyStringStripBip( buf, bone->GetName(), 200, mOptions.mStripBipFromBones ? true:false );
		fputs( buf, mFile ); fputc( 0, mFile );
		// bone parent index
		int parentIdx = -1;
		IGameNode* p = bone->GetNodeParent();
		if( p ) {
			for( int j = 0; j < nbones; ++j ) {
				if( bones[j]->GetNodeID() == p->GetNodeID() ) {
					parentIdx = j;
					break;
				}
			}
		}
		fwrite( &parentIdx, 1, 4, mFile );
		
		// calc bone offset matrix
		// get initial bone matrix
		GMatrix gboneTM = bone->GetWorldTM();
		D3DXMATRIX boneTM;
		memcpy( &boneTM, &gboneTM, sizeof(D3DXMATRIX) );
		boneTM._41 *= mOptions.mUnitMultiplier;
		boneTM._42 *= mOptions.mUnitMultiplier;
		boneTM._43 *= mOptions.mUnitMultiplier;
		// invert it
		D3DXMatrixInverse( &boneTM, 0, &boneTM );
		// bone offset matrix = skinTM * boneTM
		D3DXMATRIX bmat = boneTM;
		// write
		fwrite( &bmat._11, 3, 4, mFile );
		fwrite( &bmat._21, 3, 4, mFile );
		fwrite( &bmat._31, 3, 4, mFile );
		fwrite( &bmat._41, 3, 4, mFile );
	}

	return 0;
}



// --------------------------------------------------------------------------
//  Main
// --------------------------------------------------------------------------



// Dummy function for progress bar
DWORD WINAPI gProgressBarFn( LPVOID arg ) { return(0); }


class MyErrorProc : public IGameErrorCallBack {
public:
	virtual void ErrorProc( IGameError error ) {
		TCHAR* buf = GetLastIGameErrorText();
		DebugPrint( "ErrorCode = %d ErrorText = %s\n", error,buf );
	}
};

int IGameExporter::DoExport( const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts, DWORD options )
{
	int loop;

	Interface* ip = GetCOREInterface();
	
	MyErrorProc errorProc;
	SetErrorCallBack(&errorProc);
	
	readConfig();

	// file name (lowercase extension)
	char buf[1000];
	strncpy( buf, name, 999 );
	int buflen = strlen( buf );
	if( buflen > 5 ) {
		if( buf[buflen-5] == 'D' ) buf[buflen-5] = 'd';
		if( buf[buflen-4] == 'M' ) buf[buflen-4] = 'm';
		if( buf[buflen-3] == 'E' ) buf[buflen-3] = 'e';
		if( buf[buflen-2] == 'S' ) buf[buflen-2] = 's';
		if( buf[buflen-1] == 'H' ) buf[buflen-1] = 'h';
	}

	// open file
	assert( !mFile );
	mFile = fopen( buf, "wb" );
	if( !mFile )
		return IMPEXP_FAIL;

	// debug output
	if( mOptions.mDebugOutput ) {
		if( gDebugFile )
			fclose( gDebugFile );
		char buf2[400];
		sprintf( buf2, "%s.txt", buf );
		gDebugFile = fopen( buf2, "wt" );
	}
	
	// Set a global prompt display switch
	mShowPrompts = suppressPrompts ? false : true;
	mExportSelected = (options & SCENE_EXPORT_SELECTED) ? true : false;
	
	if( mShowPrompts ) {
		// Prompt the user with dialogbox, and get all the options.
		if( !DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PANEL), i->GetMAXHWnd(), IGameExporterOptionsDlgProc, (LPARAM)this) ) {
			return IMPEXP_CANCEL;
		}
	}
	
	mCurrNodeProgress = 0;
	ip->ProgressStart( _T("Exporting meshes.."), TRUE, gProgressBarFn, NULL );

	assert( !mGameScene );
	mGameScene = GetIGameInterface();
	
	IGameConversionManager* cm = GetConversionManager();
	UserCoord dummyUserCoord = {
		1,	//Right Handed
		1,	//X axis goes right
		4,	//Y Axis goes in
		3,	//Z Axis goes down.
		0,	//U Tex axis is left
		1,	//V Tex axis is Down
	};
	cm->SetUserCoordSystem(dummyUserCoord);
	cm->SetCoordSystem( IGameConversionManager::IGAME_D3D );
	mGameScene->InitialiseIGame(); // initialize with all nodes, export selected only

	//int ticksPerFrame = GetTicksPerFrame();
	//int curFrame = i->GetTime() / ticksPerFrame;
	mGameScene->SetStaticFrame( 0 );

	// set up d3d
	HRESULT hr;
	mDx = Direct3DCreate9( D3D_SDK_VERSION );
	if( !mDx )
		return IMPEXP_FAIL;
	D3DPRESENT_PARAMETERS pp;
	memset( &pp, 0, sizeof(pp) );
	pp.Windowed = TRUE;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.BackBufferFormat = D3DFMT_UNKNOWN;
	hr = mDx->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, i->GetMAXHWnd(), D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &pp, &mDxDevice );
	if( FAILED(hr) ) {
		mDx->Release();
		MessageBox( i->GetMAXHWnd(), "Failed to create D3D device", "Error", MB_OK | MB_ICONERROR );
		return IMPEXP_FAIL;
	}
	assert( mDxDevice );

	bool ok = true;
	const char* r = NULL;

	//
	// gather nodes for exporting

	mNodes.ZeroCount();
	mTotalVerts = 0;
	mTotalTris = 0;
	mTotalMaxBonesPerVert = -1;
	for( loop = 0; loop < mGameScene->GetTopLevelNodeCount(); ++loop ) {
		IGameNode* gameNode = mGameScene->GetTopLevelNode(loop);
		r = gatherNode( gameNode );
		if( r ) {
			MessageBox( i->GetMAXHWnd(), r, "Error", MB_OK | MB_ICONERROR );
			ok = false;
		}
	}
	// if all not skinned: optionally convert into 1-bone skinned.
	if( ok && mTotalMaxBonesPerVert <= 0 && mOptions.mCreate1BoneSkin ) {
		debugMsg( "creating 1-bone skin..." );
		for( int j = 0; j < mNodes.Count(); ++j ) {
			mNodes[j]->createSelfSkin( j );
		}
		mTotalMaxBonesPerVert = 1;
	}

	//
	// construct the mesh and process the nodes

	// TBD: errors
	r = meshCreate();
	int vertOffset = 0;
	int triOffset = 0;
	for( loop = 0; loop < mNodes.Count(); ++loop ) {
		r = meshAddNode( *mNodes[loop], vertOffset, triOffset );
	}
	r = meshProcess();
	r = meshWrite();

	//
	// close all

	r = cleanupMem();
	mGameScene->ReleaseIGame();
	fflush( mFile );
	fclose( mFile );
	mFile = NULL;

	if( gDebugFile ) {
		fflush( gDebugFile );
		fclose( gDebugFile );
	}
	gDebugFile = NULL;

	mGameScene = NULL;

	mDxDevice->Release();
	mDx->Release();
	
	ip->ProgressEnd();
	
	writeConfig();
	return ok ? IMPEXP_SUCCESS : IMPEXP_FAIL;
}



// --------------------------------------------------------------------------
//  Config stuff
// --------------------------------------------------------------------------



TSTR IGameExporter::getCfgFilename()
{
	TSTR filename;
	filename += GetCOREInterface()->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += "IMMeshExport.cfg";
	return filename;
}

BOOL IGameExporter::readConfig()
{
	TSTR filename = getCfgFilename();
	FILE* fcfg = fcfg = fopen( filename, "rb" );
	if( !fcfg )
		return FALSE;

	int cfgVer;
	fread( &cfgVer, 1, 4, fcfg );
	if( cfgVer != CFG_OPTIONS_VERSION )
		return FALSE;

	fread( &mOptions, 1, sizeof(mOptions), fcfg );
	
	fclose(fcfg);
	return TRUE;
}

void IGameExporter::writeConfig()
{
	TSTR filename = getCfgFilename();
	FILE* fcfg = fopen( filename, "wb" );
	if( !fcfg )
		return;

	const int cfgVer = CFG_OPTIONS_VERSION;
	fwrite( &cfgVer, 1, 4, fcfg );

	fwrite( &mOptions, 1, sizeof(mOptions), fcfg );
	
	fclose(fcfg);
}
