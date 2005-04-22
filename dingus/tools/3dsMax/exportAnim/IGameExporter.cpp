
#include "IGameExporter.h"


static IGameExporterClassDesc IGameExporterDesc;
ClassDesc2* GetIGameExporterDesc() { return &IGameExporterDesc; }

#include <d3dx9math.h>

#define CHECK_FROM(ctrl,member) CheckDlgButton( wnd, ctrl, (member) ? BST_CHECKED : BST_UNCHECKED )
#define IS_CHECK(ctrl) (IsDlgButtonChecked( wnd, ctrl ) ? 1 : 0)


static void gOptionsToDialog( HWND wnd, const IGameExporter& exp )
{
	const SExportOptions& o = exp.mOptions;
	ISpinnerControl *spin;

	spin = GetISpinner( GetDlgItem(wnd, IDC_SAMPLE_FRAME_SPIN) );
	spin->LinkToEdit( GetDlgItem(wnd,IDC_SAMPLE_FRAME), EDITTYPE_INT );
	spin->SetLimits( 1, 100, TRUE );
	spin->SetScale( 1 );
	spin->SetValue( o.mFramesPerSample, FALSE );
	ReleaseISpinner( spin );

	spin = GetISpinner( GetDlgItem(wnd, IDC_TOL_POS_SPIN) );
	spin->LinkToEdit( GetDlgItem(wnd,IDC_TOL_POS), EDITTYPE_FLOAT );
	spin->SetLimits( 0.000001f, 1000.0f, TRUE );
	spin->SetScale( o.mTolPos );
	spin->SetValue( o.mTolPos, FALSE );
	ReleaseISpinner( spin );

	spin = GetISpinner( GetDlgItem(wnd, IDC_TOL_ROT_SPIN) );
	spin->LinkToEdit( GetDlgItem(wnd,IDC_TOL_ROT), EDITTYPE_FLOAT );
	spin->SetLimits( 0.0000001f, 0.1f, TRUE );
	spin->SetScale( o.mTolRot );
	spin->SetValue( o.mTolRot, FALSE );
	ReleaseISpinner( spin );

	spin = GetISpinner( GetDlgItem(wnd, IDC_TOL_SCALE_SPIN) );
	spin->LinkToEdit( GetDlgItem(wnd,IDC_TOL_SCALE), EDITTYPE_FLOAT );
	spin->SetLimits( 0.000001f, 1.0f, TRUE );
	spin->SetScale( o.mTolScale );
	spin->SetValue( o.mTolScale, FALSE );
	ReleaseISpinner( spin );

	SetDlgItemText( wnd, IDC_EDIT_NAME_START, exp.mNameMustStart.data() );
	SetDlgItemText( wnd, IDC_EDIT_NAME_END, exp.mNameCantEnd.data() );

	int chkRadio = IDC_RAD_CLAMP;
	if( o.mLooping == 1 )
		chkRadio = IDC_RAD_LOOP;
	if( o.mLooping == 2 )
		chkRadio = IDC_RAD_LOOP2;
	CheckRadioButton( wnd, IDC_RAD_CLAMP, IDC_RAD_LOOP2, chkRadio );

	CHECK_FROM( IDC_CHK_DISCARDLAST, o.mDiscardLastSample );

	CHECK_FROM( IDC_CHK_POSITION, o.mDoPos );
	CHECK_FROM( IDC_CHK_ROTATION, o.mDoRot );
	CHECK_FROM( IDC_CHK_SCALE, o.mDoScale );
	CHECK_FROM( IDC_CHK_CAMERA, o.mDoCamera );
	CHECK_FROM( IDC_CHK_COLOR, o.mDoColor );

	CHECK_FROM( IDC_CHK_LEAFS, o.mDoLeafs );
	CHECK_FROM( IDC_CHK_ROOTS, o.mDoRoots );
	CHECK_FROM( IDC_CHK_STRIPBIP, o.mStripBipFromBones );

	CHECK_FROM( IDC_CHK_SCALE_1, o.mScaleScalar );
	CHECK_FROM( IDC_CHK_SCALE_F16, o.mFlt16Scale );
	CHECK_FROM( IDC_CHK_ROT_F16, o.mFlt16Rot );

	char buf[200];
	sprintf( buf, "%g", o.mUnitMultiplier );
	SetDlgItemText( wnd, IDC_EDIT_COORDMULT, buf );
}

static void gOptionsFromDialog( HWND wnd, IGameExporter& exp )
{
	SExportOptions& o = exp.mOptions;
	ISpinnerControl *spin;

	spin = GetISpinner( GetDlgItem(wnd, IDC_SAMPLE_FRAME_SPIN) );
	o.mFramesPerSample = spin->GetIVal(); 
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem(wnd, IDC_TOL_POS_SPIN) );
	o.mTolPos = spin->GetFVal(); 
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem(wnd, IDC_TOL_ROT_SPIN) );
	o.mTolRot = spin->GetFVal(); 
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem(wnd, IDC_TOL_SCALE_SPIN) );
	o.mTolScale = spin->GetFVal(); 
	ReleaseISpinner( spin );

	TCHAR buf[300];
	GetDlgItemText( wnd, IDC_EDIT_NAME_START, buf, 299 );
	exp.mNameMustStart = TSTR(buf);
	GetDlgItemText( wnd, IDC_EDIT_NAME_END, buf, 299 );
	exp.mNameCantEnd = TSTR(buf);

	if( IsDlgButtonChecked( wnd, IDC_RAD_CLAMP ) )
		o.mLooping = 0;
	if( IsDlgButtonChecked( wnd, IDC_RAD_LOOP ) )
		o.mLooping = 1;
	if( IsDlgButtonChecked( wnd, IDC_RAD_LOOP2 ) )
		o.mLooping = 2;
	o.mDiscardLastSample = IS_CHECK( IDC_CHK_DISCARDLAST );
	o.mDoPos = IS_CHECK( IDC_CHK_POSITION );
	o.mDoRot = IS_CHECK( IDC_CHK_ROTATION );
	o.mDoScale = IS_CHECK( IDC_CHK_SCALE );
	o.mDoCamera = IS_CHECK( IDC_CHK_CAMERA );
	o.mDoColor = IS_CHECK( IDC_CHK_COLOR );
	o.mDoLeafs = IS_CHECK( IDC_CHK_LEAFS );
	o.mDoRoots = IS_CHECK( IDC_CHK_ROOTS );
	o.mStripBipFromBones = IS_CHECK( IDC_CHK_STRIPBIP );

	o.mScaleScalar = IS_CHECK( IDC_CHK_SCALE_1 );
	o.mFlt16Scale = IS_CHECK( IDC_CHK_SCALE_F16 );
	o.mFlt16Rot = IS_CHECK( IDC_CHK_ROT_F16 );

	GetDlgItemText( wnd, IDC_EDIT_COORDMULT, buf, 200 );
	o.mUnitMultiplier = atof( buf );
}


BOOL CALLBACK IGameExporterOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	IGameExporter *exp = (IGameExporter*)GetWindowLongPtr( hWnd, GWLP_USERDATA );

	static prec = 0;
	
	switch(message) {
	case WM_INITDIALOG:
		prec = GetSpinnerPrecision();
		SetSpinnerPrecision( 8 );
		exp = (IGameExporter*)lParam;
		SetWindowLongPtr( hWnd,GWLP_USERDATA,lParam );
		CenterWindow( hWnd,GetParent(hWnd) );
		gOptionsToDialog( hWnd, *exp );
		return TRUE;
		
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			gOptionsFromDialog( hWnd, *exp );
			SetSpinnerPrecision( prec );
			EndDialog( hWnd, 1 );
			break;
		case IDCANCEL:
			SetSpinnerPrecision( prec );
			EndDialog(hWnd,0);
			break;
		}
		default:
			return FALSE;
	}
	return TRUE;
}

void stripWhiteSpace( TSTR* buf, TCHAR &newBuf )
{
	TCHAR newb[256]={""};
	strcpy( newb,buf->data() );
	int len = strlen(newb);
	int index = 0;
	for( int i=0;i<len;i++ ) {
		if((newb[i] != ' ') && (!ispunct(newb[i])))
			(&newBuf)[index++] = newb[i];
	}
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

// --------------------------------------------------------------------------
//  Construction/destruction
// --------------------------------------------------------------------------

IGameExporter::IGameExporter()
:	mFile(0),
	mGameScene(0),
	mNameMustStart( _T("Bip") ),
	mNameCantEnd( _T("Nub") )
{
}

IGameExporter::~IGameExporter() 
{
	if( mFile ) {
		fclose( mFile );
		mFile = 0;
	}
}


// --------------------------------------------------------------------------
//  SceneExport interface
// --------------------------------------------------------------------------

int IGameExporter::ExtCount() { return 1; } // one extension
const TCHAR *IGameExporter::Ext( int n ) { return _T("danim"); }
const TCHAR *IGameExporter::LongDesc() { return _T("IM animations exporter"); }
const TCHAR *IGameExporter::ShortDesc() { return _T("IMAnimExporter"); }
const TCHAR *IGameExporter::AuthorName() { return _T("Aras Pranckevicius"); }
const TCHAR *IGameExporter::CopyrightMessage() { return _T("(c)2003 InteraMotion LLC"); }
const TCHAR *IGameExporter::OtherMessage1() { return _T(""); }
const TCHAR *IGameExporter::OtherMessage2() { return _T(""); }
unsigned int IGameExporter::Version() {	return 100; } // version number * 100
void IGameExporter::ShowAbout(HWND hWnd) { /* Optional */ }

BOOL IGameExporter::SupportsOptions(int ext, DWORD options)
{
	// Decide which options to support.  Simply return
	// true for each option supported by each Extension 
	// the exporter supports.
	return TRUE;
}

bool IGameExporter::rejectName( const TSTR& name ) const
{
	TSTR t;
	TSTR substr;
	// test if starts correctly
	t = mNameMustStart;
	if( name.Length() < t.Length() )
		return true;
	t.toLower();
	substr = name.Substr( 0, t.Length() );
	substr.toLower();
	if( t != substr )
		return true;
	// test if ends correctly
	t = mNameCantEnd;
	if( name.Length() < t.Length() )
		return false;
	t.toLower();
	substr = name.Substr( name.Length()-t.Length(), t.Length() );
	substr.toLower();
	if( t == substr )
		return true;

	return false;
}

void IGameExporter::processNode( IGameNode* node )
{
	TSTR buf;
	TSTR nodeName( node->GetName() );
	++mCurrNodeProgress;
	buf = TSTR("Processing: ") + nodeName;
	GetCOREInterface()->ProgressUpdate( (int)((float)mCurrNodeProgress/mGameScene->GetTotalNodeCount()*100.0f), FALSE, buf.data() );

	// discard leafs?
	if( !mOptions.mDoLeafs && node->GetChildCount() == 0 )
		return;
	// discard roots?
	if( !mOptions.mDoRoots && node->GetNodeParent() == 0 )
		goto _rest;

	// cleanup name
	if( rejectName( nodeName ) )
		goto _rest;
	
	if( !node->GetMaxNode()->Selected() && mExportSelected ) {
		// skip unselected
	} else if( node->IsGroupOwner() ) {
		// group owner
		// seems that in plain biped there's no groups
	} else {

		++mCurrCurve;
		
		// stupid max's Tab doesn't use copy constructors!
		TSTR dummyName( _T("") );
		mAnimCurveNames.Append( 1, &dummyName ); 
		// hence we assign the just-appended dummy name
		mAnimCurveNames[mAnimCurveNames.Count()-1] = nodeName;

		int nodeID = node->GetNodeID();
		mAnimCurveIDs.Append( 1, &nodeID );

		int parentIdx = -1;
		int parentID = -1;
		IGameNode* p = node->GetNodeParent();
		if( p ) {
			parentID = p->GetNodeID();
			// search for parent index
			for( int j = 0; j < mCurrCurve; ++j ) {
				if( mAnimCurveIDs[j] == parentID ) {
					parentIdx = j;
					break;
				}
			}
		}
		
		mAnimCurveParents.Append( 1, &parentIdx );

		sampleAnim( node );
	}

_rest:
	// children
	for( int i = 0; i < node->GetChildCount(); ++i )
		processNode( node->GetNodeChild(i) );
}

void IGameExporter::dumpMatrix( const GMatrix& m )
{
	// pos
	D3DXVECTOR3 pos;
	pos.x = m[3][0]; pos.y = m[3][1]; pos.z = m[3][2];
	pos *= mOptions.mUnitMultiplier;

	// scale
	D3DXVECTOR3 axisX, axisY, axisZ;
	axisX.x = m[0][0]; axisX.y = m[0][1]; axisX.z = m[0][2];
	axisY.x = m[1][0]; axisY.y = m[1][1]; axisY.z = m[1][2];
	axisZ.x = m[2][0]; axisZ.y = m[2][1]; axisZ.z = m[2][2];

	D3DXVECTOR3 scale;
	scale.x = D3DXVec3Length( &axisX );
	scale.y = D3DXVec3Length( &axisY );
	scale.z = D3DXVec3Length( &axisZ );
	axisX /= scale.x;
	axisY /= scale.y;
	axisZ /= scale.z;

	// build rotation matrix without scaling
	// use D3DX to extract quat - Max extracts in a funky way. Maybe it's something
	// with RHvsLH systems?
	D3DXMATRIX matNoScale2;
	D3DXMatrixIdentity( &matNoScale2 );
	matNoScale2._11 = axisX.x; matNoScale2._12 = axisX.y; matNoScale2._13 = axisX.z;
	matNoScale2._21 = axisY.x; matNoScale2._22 = axisY.y; matNoScale2._23 = axisY.z;
	matNoScale2._31 = axisZ.x; matNoScale2._32 = axisZ.y; matNoScale2._33 = axisZ.z;
	D3DXQUATERNION rot;
	D3DXQuaternionRotationMatrix( &rot, &matNoScale2 );

	// fetch mirroring
	D3DXMATRIX matRot;
	D3DXMatrixRotationQuaternion( &matRot, &rot );
	D3DXVECTOR3 axis2X, axis2Y, axis2Z;
	axis2X.x = matRot._11; axis2X.y = matRot._12; axis2X.z = matRot._13;
	axis2Y.x = matRot._21; axis2Y.y = matRot._22; axis2Y.z = matRot._23;
	axis2Z.x = matRot._31; axis2Z.y = matRot._32; axis2Z.z = matRot._33;
	float dotX = D3DXVec3Dot( &axisX, &axis2X );
	float dotY = D3DXVec3Dot( &axisY, &axis2Y );
	float dotZ = D3DXVec3Dot( &axisZ, &axis2Z );
	scale.x *= dotX;
	scale.y *= dotY;
	scale.z *= dotZ;

	/*
	D3DXMATRIX mm;
	mm._11 = m[0][0]; mm._12 = m[0][1]; mm._13 = m[0][2]; mm._14 = 0.0f;
	mm._21 = m[1][0]; mm._22 = m[1][1]; mm._23 = m[1][2]; mm._24 = 0.0f;
	mm._31 = m[2][0]; mm._32 = m[2][1]; mm._33 = m[2][2]; mm._34 = 0.0f;
	mm._41 = m[3][0]; mm._42 = m[3][1]; mm._43 = m[3][2]; mm._44 = 1.0f;
	D3DXVECTOR3 pos, scale;
	D3DXQUATERNION rot;
	D3DXMatrixDecompose( &scale, &rot, &pos, &mm );
	*/

	// optionally, make scale a scalar (into x; yz are set to zero)
	// if scale is not uniform, it's user's fault :)
	if( mOptions.mScaleScalar ) {
		scale.x = (scale.x + scale.y + scale.z) / 3.0f;
		scale.y = scale.z = 0.0f;
	}

	// append to current anim curve
	mCurrCurvePos.Append( 1, &pos );
	mCurrCurveRot.Append( 1, &rot );
	mCurrCurveScale.Append( 1, &scale );
}

void IGameExporter::tryCollapseVec3( TVec3Tab& t, float tol )
{
	int i;
	int n = t.Count();
	if( n < 2 )
		return;

	tol = tol * tol;

	// find average
	D3DXVECTOR3 sum(0,0,0);
	for( i = 0; i < n; ++i ) {
		sum += t[i];
	}
	sum /= (float)n;
	// see if all close
	for( i = 0; i < n; ++i ) {
		D3DXVECTOR3 diff = t[i] - sum;
		if( D3DXVec3LengthSq( &diff ) > tol )
			return;
	}
	t.ZeroCount();
	t.Append( 1, &sum );
}

void IGameExporter::tryCollapseQuat( TQuatTab& t, float tol )
{
	int i;
	int n = t.Count();
	if( n < 2 )
		return;

	tol = tol * tol;

	// find average
	D3DXQUATERNION sum( 0, 0, 0, 0 );
	for( i = 0; i < n; ++i )
		sum += t[i];
	sum /= (float)n;
	// see if all close
	for( i = 0; i < n; ++i ) {
		D3DXQUATERNION diff = t[i] - sum;
		if( D3DXQuaternionLengthSq( &diff ) > tol )
			return;
	}
	t.ZeroCount();
	t.Append( 1, &sum );
}

void IGameExporter::tryCollapseFloat( TFloatTab& t, float tol )
{
	int i;
	int n = t.Count();
	if( n < 2 )
		return;

	tol = tol * tol;

	// find average
	float sum = 0;
	for( i = 0; i < n; ++i ) {
		sum += t[i];
	}
	sum /= (float)n;
	// see if all close
	for( i = 0; i < n; ++i ) {
		float diff = fabsf( t[i] - sum );
		if( diff > tol )
			return;
	}
	t.ZeroCount();
	t.Append( 1, &sum );
}

void IGameExporter::tryCollapseColor( TColorTab& t, float tol )
{
	int i;
	int n = t.Count();
	if( n < 2 )
		return;

	tol = tol * tol;

	// find average
	D3DXCOLOR sum(0,0,0,0);
	for( i = 0; i < n; ++i ) {
		sum += t[i];
	}
	sum /= (float)n;
	// see if all close
	for( i = 0; i < n; ++i ) {
		D3DXCOLOR d = t[i] - sum;
		if( d.r*d.r + d.g*d.g + d.b*d.b + d.a*d.a > tol )
			return;
	}
	t.ZeroCount();
	D3DCOLOR finalCol = sum;
	t.Append( 1, &finalCol );
}


static bool gNodeHasExportableMaterial( IGameNode* node )
{
	IGameMaterial* mat = node->GetNodeMaterial();
	// we can't handle: no materials and composite materials
	if( !mat  )
		return false;
	if( mat->IsMultiType() )
		return false;
	if( mat->IsSubObjType() )
		return false;
	// else we can possibly handle it!
	return true;
}

void IGameExporter::sampleAnim( IGameNode* node )
{
	IGameObject* obj = node->GetIGameObject();
	IGameControl* sc = node->GetIGameControl();

	IGameKeyTab keys;

	mCurrCurvePos.ZeroCount();
	mCurrCurveRot.ZeroCount();
	mCurrCurveScale.ZeroCount();
	mCurrCurveCamera.ZeroCount();

	if( !sc ) {
		node->ReleaseIGameObject();
		return;
	}
	
	// sample transforms...
	if( sc->GetFullSampledKeys( keys, mOptions.mFramesPerSample, IGAME_TM ) ) {
		int keyCount = keys.Count();
		if( keyCount > 1 && mOptions.mDiscardLastSample )
			--keyCount;
		for( int i = 0; i < keyCount; ++i ) {
			dumpMatrix( keys[i].sampleKey.gval );
		}
	} else {
	}

	//
	// if do camera and we are camera - do them as well

	if( mOptions.mDoCamera && obj && (obj->GetIGameType() == IGameObject::IGAME_CAMERA) ) {
		IGameCamera* camera = (IGameCamera*)obj;
		IGameKeyTab keysNear, keysFar, keysFov;
		IGameProperty* propNear = camera->GetCameraNearClip();
		IGameProperty* propFar = camera->GetCameraFarClip();
		IGameProperty* propFov = camera->GetCameraFOV();
		IGameControl* ctlNear = propNear->GetIGameControl();
		IGameControl* ctlFar = propFar->GetIGameControl();
		IGameControl* ctlFov = propFov->GetIGameControl();
		if( ctlNear ) {
			ctlNear->GetFullSampledKeys( keysNear, mOptions.mFramesPerSample, IGAME_FLOAT, false );
		}
		if( ctlFar ) {
			ctlFar->GetFullSampledKeys( keysFar, mOptions.mFramesPerSample, IGAME_FLOAT, false );
		}
		if( ctlFov ) {
			ctlFov->GetFullSampledKeys( keysFov, mOptions.mFramesPerSample, IGAME_FLOAT, false );
		}
		const int keyCount = mCurrCurvePos.Count();
		mCurrCurveCamera.Resize( keyCount );
		mCurrCurveCamera.SetCount( keyCount );
		assert( keysNear.Count() == 0 || keysNear.Count() >= keyCount );
		assert( keysFar.Count() == 0 || keysFar.Count() >= keyCount );
		assert( keysFov.Count() == 0 || keysFov.Count() >= keyCount );
		float vnear, vfar, vfov;
		propNear->GetPropertyValue( vnear );
		propFar->GetPropertyValue( vfar );
		propFov->GetPropertyValue( vfov );
		for( int i = 0; i < keyCount; ++i ) {
			D3DXVECTOR3& v = mCurrCurveCamera[i];
			v.x = (keysNear.Count()==0) ? vnear : keysNear[i].sampleKey.fval;
			v.y = (keysFar.Count()==0) ? vfar : keysFar[i].sampleKey.fval;
			v.z = (keysFov.Count()==0) ? vfov : keysFov[i].sampleKey.fval;
			v.x *= mOptions.mUnitMultiplier;
			v.y *= mOptions.mUnitMultiplier;
		}
	}
	// pretend that camera data is collapsed if we have none
	if( mCurrCurveCamera.Count() < 1 ) {
		D3DXVECTOR3 p(0,0,0);
		mCurrCurveCamera.Append( 1, &p );
	}

	//
	// if do color and we have suitable materal

	if( mOptions.mDoColor && obj && gNodeHasExportableMaterial(node) ) {
		IGameMaterial* mat = node->GetNodeMaterial();
		IGameKeyTab keysDiff, keysAlpha;
		IGameProperty* propDiff = mat->GetAmbientData(); // TBD: ambient just now, for demo purposes!
		//IGameProperty* propDiff = mat->GetDiffuseData();
		IGameProperty* propAlpha = mat->GetOpacityData();
		IGameControl* ctlDiff = propDiff->GetIGameControl();
		IGameControl* ctlAlpha = propAlpha->GetIGameControl();
		if( ctlDiff ) {
			ctlDiff->GetFullSampledKeys( keysDiff, mOptions.mFramesPerSample, IGAME_POINT3, false );
		}
		if( ctlAlpha ) {
			ctlAlpha->GetFullSampledKeys( keysAlpha, mOptions.mFramesPerSample, IGAME_FLOAT, false );
		}
		const int keyCount = mCurrCurvePos.Count();
		mCurrCurveColor.Resize( keyCount );
		mCurrCurveColor.SetCount( keyCount );
		assert( keysDiff.Count() == 0 || keysDiff.Count() >= keyCount );
		assert( keysAlpha.Count() == 0 || keysAlpha.Count() >= keyCount );
		Point3	vdiff;
		float	valpha;
		propDiff->GetPropertyValue( vdiff );
		propAlpha->GetPropertyValue( valpha );
		for( int i = 0; i < keyCount; ++i ) {
			D3DXCOLOR c;
			if( keysDiff.Count()==0 ) {
				c.r = vdiff.x;	c.g = vdiff.y;	c.b = vdiff.z;
			} else {
				c.r = keysDiff[i].sampleKey.pval.x;
				c.g = keysDiff[i].sampleKey.pval.y;
				c.b = keysDiff[i].sampleKey.pval.z;
			}
			c.a = (keysAlpha.Count()==0) ? valpha : keysAlpha[i].sampleKey.fval;
			mCurrCurveColor[i] = c;
		}
	}
	// pretend that color data is collapsed if we have none
	if( mCurrCurveColor.Count() < 1 ) {
		D3DCOLOR p = 0xFFffffff;
		mCurrCurveColor.Append( 1, &p );
	}

	mSampleCount = mCurrCurvePos.Count();

	// see if we can collapse the curves
	tryCollapseVec3( mCurrCurvePos, mOptions.mTolPos );
	tryCollapseQuat( mCurrCurveRot, mOptions.mTolRot );
	tryCollapseVec3( mCurrCurveScale, mOptions.mTolScale );
	tryCollapseVec3( mCurrCurveCamera, mOptions.mTolPos );
	tryCollapseColor( mCurrCurveColor, mOptions.mTolScale );

	// construct curves and add to anim groups
	bool posColl = (mCurrCurvePos.Count()==1);
	TVec3AnimGroup::TAnimCurve posCurve( !posColl ? D3DXVECTOR3(0.0f,0.0f,0.0f) : mCurrCurvePos[0], posColl ? 0 : mAnimPos.samples.Count(), posColl ? (TVec3AnimGroup::TAnimCurve::NONE) : (TVec3AnimGroup::TAnimCurve::LINEAR) );
	bool rotColl = (mCurrCurveRot.Count()==1);
	TQuatAnimGroup::TAnimCurve rotCurve( !rotColl ? D3DXQUATERNION(0.0f,0.0f,0.0f,0.0f) : mCurrCurveRot[0], rotColl ? 0 : mAnimRot.samples.Count(), rotColl ? (TQuatAnimGroup::TAnimCurve::NONE) : (TQuatAnimGroup::TAnimCurve::LINEAR) );
	bool scaleColl = (mCurrCurveScale.Count()==1);
	TVec3AnimGroup::TAnimCurve scaleCurve( !scaleColl ? D3DXVECTOR3(0.0f,0.0f,0.0f) : mCurrCurveScale[0], scaleColl ? 0 : mAnimScale.samples.Count(), scaleColl ? (TVec3AnimGroup::TAnimCurve::NONE) : (TVec3AnimGroup::TAnimCurve::LINEAR) );
	bool cameraColl = (mCurrCurveCamera.Count()==1);
	TVec3AnimGroup::TAnimCurve cameraCurve( !cameraColl ? D3DXVECTOR3(0.0f,0.0f,0.0f) : mCurrCurveCamera[0], cameraColl ? 0 : mAnimCamera.samples.Count(), cameraColl ? (TVec3AnimGroup::TAnimCurve::NONE) : (TVec3AnimGroup::TAnimCurve::LINEAR) );
	bool colorColl = (mCurrCurveColor.Count()==1);
	TColorAnimGroup::TAnimCurve colorCurve( !colorColl ? 0x00000000 : mCurrCurveColor[0], colorColl ? 0 : mAnimColor.samples.Count(), colorColl ? (TColorAnimGroup::TAnimCurve::NONE) : (TColorAnimGroup::TAnimCurve::LINEAR) );
	mAnimPos.curves.Append( 1, &posCurve );
	mAnimRot.curves.Append( 1, &rotCurve );
	mAnimScale.curves.Append( 1, &scaleCurve );
	mAnimCamera.curves.Append( 1, &cameraCurve );
	mAnimColor.curves.Append( 1, &colorCurve );

	// add samples to anim groups
	if( !posColl )
		mAnimPos.samples.Append( mCurrCurvePos.Count(), &mCurrCurvePos[0] );
	if( !rotColl )
		mAnimRot.samples.Append( mCurrCurveRot.Count(), &mCurrCurveRot[0] );
	if( !scaleColl )
		mAnimScale.samples.Append( mCurrCurveScale.Count(), &mCurrCurveScale[0] );
	if( !cameraColl )
		mAnimCamera.samples.Append( mCurrCurveCamera.Count(), &mCurrCurveCamera[0] );
	if( !colorColl )
		mAnimColor.samples.Append( mCurrCurveColor.Count(), &mCurrCurveColor[0] );
	
	node->ReleaseIGameObject();
}

#include "../../../dingus/animator/AnimDataType.h"

template<typename TDINANIM, typename TDATA, int animType>
struct SSampledAnimGroupWriter {
	void operator()( FILE* f, const char* name, int ncurves, const SAnimGroup<TDINANIM>& anim ) {
		int i;
		// anim data type
		i = animType;
		fwrite( &i, 1, 4, f );
		// sample data size
		i = sizeof(TDATA); fwrite( &i, 1, 4, f );
		// group name
		fputs( name, f ); fputc( 0, f );
		// curves
		for( i = 0; i < ncurves; ++i ) {
			const SAnimGroup<TDINANIM>::TAnimCurve& c = anim.curves[i];
			// ipol type
			int ipol = c.getIpol();
			fwrite( &ipol, 1, 4, f );
			// first sample index
			int frst = c.getFirstSampleIndex();
			fwrite( &frst, 1, 4, f );
			// collapsed value
			fwrite( &c.getCollapsedValue(), 1, sizeof(TDATA), f );
		}
		// samples
		int nsamples = anim.samples.Count();
		fwrite( &nsamples, 1, 4, f ); // sample count
		for( i = 0; i < nsamples; ++i ) {
			const TDATA& s = (const TDATA&)anim.samples[i];
			fwrite( &s, 1, sizeof(TDATA), f );
		}
	}
};


void IGameExporter::writeAllData()
{
	assert( mFile );
	int i;

	int ncurves = mAnimCurveNames.Count();

	// looping?
	fwrite( "DANI", 1, 4, mFile ); // magic
	i = 0;
	fwrite( &i, 1, 4, mFile ); // type (only sampled now)
	fwrite( &mOptions.mLooping, 1, 4, mFile ); // looping
	fwrite( &ncurves, 1, 4, mFile ); // curve count
	
	int ngroups = 0;
	if( mOptions.mDoPos )		++ngroups;
	if( mOptions.mDoRot )		++ngroups;
	if( mOptions.mDoScale )		++ngroups;
	if( mOptions.mDoCamera )	++ngroups;
	if( mOptions.mDoColor )		++ngroups;
	fwrite( &ngroups, 1, 4, mFile ); // group count

	// write curve names and parent indices
	for( i = 0; i < ncurves; ++i ) {
		char buf[200];
		gCopyStringStripBip( buf, mAnimCurveNames[i].data(), 200, mOptions.mStripBipFromBones ? true:false );
		fputs( buf, mFile ); // name
		fputc( 0, mFile ); // asciiz
		fwrite( &mAnimCurveParents[i], 1, 4, mFile );
	}

	fwrite( &mSampleCount, 1, 4, mFile ); // sample count in curves

	// position
	if( mOptions.mDoPos ) {
		SSampledAnimGroupWriter<D3DXVECTOR3,D3DXVECTOR3,dingus::ANIMTYPE_VEC3> writer;
		writer( mFile, "pos", ncurves, mAnimPos );
	}
	// rotation
	if( mOptions.mDoRot ) {
		SSampledAnimGroupWriter<D3DXQUATERNION,D3DXQUATERNION,dingus::ANIMTYPE_QUAT> writer;
		writer( mFile, "rot", ncurves, mAnimRot );
	}
	// scale
	if( mOptions.mDoScale ) {
		if( mOptions.mScaleScalar ) {
			SSampledAnimGroupWriter<D3DXVECTOR3,float,dingus::ANIMTYPE_FLT> writer;
			writer( mFile, "scale", ncurves, mAnimScale );
		} else {
			SSampledAnimGroupWriter<D3DXVECTOR3,D3DXVECTOR3,dingus::ANIMTYPE_VEC3> writer;
			writer( mFile, "scale3", ncurves, mAnimScale );
		}
	}
	// camera anim
	if( mOptions.mDoCamera ) {
		SSampledAnimGroupWriter<D3DXVECTOR3,D3DXVECTOR3,dingus::ANIMTYPE_VEC3> writer;
		writer( mFile, "cam", ncurves, mAnimCamera );
	}
	// color
	if( mOptions.mDoColor ) {
		SSampledAnimGroupWriter<D3DCOLOR,D3DCOLOR,dingus::ANIMTYPE_COLOR> writer;
		writer( mFile, "color", ncurves, mAnimColor );
	}
}

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
		if( buf[buflen-4] == 'A' ) buf[buflen-4] = 'a';
		if( buf[buflen-3] == 'N' ) buf[buflen-3] = 'n';
		if( buf[buflen-2] == 'I' ) buf[buflen-2] = 'i';
		if( buf[buflen-1] == 'M' ) buf[buflen-1] = 'm';
	}

	// open file
	assert( !mFile );
	mFile = fopen( buf, "wb" );
	if( !mFile )
		return FALSE;
	// TEST
	//char name2[1000];
	//strcpy( name2, name );
	//strcat( name2, ".txt" );
	//gFile = fopen( name2, "wt" );
	//if( !gFile )
	//	return FALSE;
	// /TEST
	
	// Set a global prompt display switch
	mShowPrompts = suppressPrompts ? false : true;
	mExportSelected = (options & SCENE_EXPORT_SELECTED) ? true : false;
	
	if( mShowPrompts ) {
		// Prompt the user with dialogbox, and get all the options.
		if( !DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PANEL), i->GetMAXHWnd(), IGameExporterOptionsDlgProc, (LPARAM)this) ) {
			return 1;
		}
	}
	
	mCurrNodeProgress = 0;
	ip->ProgressStart( _T("Exporting anims.."), TRUE, gProgressBarFn, NULL );

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
	mGameScene->InitialiseIGame( false );
	mGameScene->SetStaticFrame( 0 );

	// clear our data
	mAnimPos.clear();
	mAnimRot.clear();
	mAnimScale.clear();
	mAnimCamera.clear();
	mAnimCurveNames.ZeroCount();
	mCurrCurve = -1;
	mSampleCount = 1;
	
	for( int loop = 0; loop < mGameScene->GetTopLevelNodeCount(); ++loop ) {
		IGameNode* gameNode = mGameScene->GetTopLevelNode(loop);
		processNode( gameNode );
	}
	mGameScene->ReleaseIGame();

	// write data
	writeAllData();

	// close all
	fflush( mFile );
	fclose( mFile );
	mFile = NULL;

	// TEST
	//fflush( gFile );
	//fclose( gFile );

	mGameScene = NULL;
	
	ip->ProgressEnd();
	
	writeConfig();
	return TRUE;
}


// --------------------------------------------------------------------------
//  config stuff
// --------------------------------------------------------------------------


TSTR IGameExporter::getCfgFilename()
{
	TSTR filename;
	filename += GetCOREInterface()->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += "IMAnimExport.cfg";
	return filename;
}


BOOL IGameExporter::readConfig()
{
	TSTR filename = getCfgFilename();
	FILE* fcfg = fopen( filename, "rb" );
	if( !fcfg )
		return FALSE;

	int cfgVer;
	fread( &cfgVer, 1, 4, fcfg );
	if( cfgVer != SExportOptions::OPTIONS_VERSION )
		return FALSE;
	
	fread( &mOptions, 1, sizeof(mOptions), fcfg );

	TCHAR buf[300];
	int v;
	v = _getw(fcfg);
	_fgetts( buf, v+1, fcfg );
	mNameMustStart = TSTR(buf);
	v = _getw(fcfg);
	_fgetts( buf, v+1, fcfg );
	mNameCantEnd = TSTR(buf);

	fclose( fcfg );
	return TRUE;
}

void IGameExporter::writeConfig()
{
	TSTR filename = getCfgFilename();
	FILE* fcfg = fopen( filename, "wb" );
	if( !fcfg )
		return;

	const int cfgVer = SExportOptions::OPTIONS_VERSION;
	fwrite( &cfgVer, 1, 4, fcfg );

	fwrite( &mOptions, 1, sizeof(mOptions), fcfg );
	
	_putw( mNameMustStart.length(), fcfg );
	_fputts( mNameMustStart.data(), fcfg );
	_putw( mNameCantEnd.length(), fcfg );
	_fputts( mNameCantEnd.data(), fcfg );

	fclose( fcfg );
}
