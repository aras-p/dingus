#include "stdafx.h"

// Demo application.
// Most of stuff is just kept in global variables :)

#include "Demo.h"
#include "DemoResources.h"

#include <dingus/resource/IndexBufferFillers.h>
#include <dingus/gfx/gui/Gui.h>
#include <dingus/math/MathUtils.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>
#include <dingus/renderer/RenderableMesh.h>
#include "WinFileDlg.h"

#include "Entity.h"


// --------------------------------------------------------------------------
// Demo variables, constants, etc.


/// Possible render modes.
enum eRenderMode {
	RM_CHECKER = 0,
	RM_NORMALMAP,
	RM_MAPPED,
	RM_BAKED,
	RM_TEXBAKER,
	RMCOUNT
};


const char* RMODE_FX[RMCOUNT] = {
	"checker",
	"normalmap",
	"mapped",
	"baked",
	"texbaker", // not user selectable
};


class CMeshTexerEntity : public CAbstractEntity {
public:
	CMeshTexerEntity( const std::string& meshName );
	virtual ~CMeshTexerEntity();

	/// Gets AABB
	const CAABox& getAABB() const { return mMesh->getTotalAABB(); }
	/// Gets mesh
	const CMesh& getMesh() const { return *mMesh; }

	/// Updates WVP, adds to render context
	void	render( eRenderMode renderMode );

	CRenderableMesh* getRenderMesh( eRenderMode renderMode ) {
		return mRenderMeshes[renderMode];
	}

private:
	CMesh*				mMesh;
	CRenderableMesh*	mRenderMeshes[RMCOUNT];
};


CMeshTexerEntity::CMeshTexerEntity( const std::string& meshName )
:	mMesh(0)
{
	assert( !meshName.empty() );

	mMesh = RGET_MESH(meshName);
	for( int i = 0; i < RMCOUNT; ++i ) {
		mRenderMeshes[i] = new CRenderableMesh( *mMesh, CRenderableMesh::ALL_GROUPS, &mWorldMat.getOrigin(), 0 );
		CEffectParams& ep = mRenderMeshes[i]->getParams();
		ep.setEffect( *RGET_FX(RMODE_FX[i]) );
		addMatricesToParams( ep );
		ep.addTexture( "tChecker", *RGET_TEX("Checker") );
		ep.addTexture( "tBase", *RGET_TEX("Checker") );
	}
}

CMeshTexerEntity::~CMeshTexerEntity()
{
	for( int i = 0; i < RMCOUNT; ++i )
		delete mRenderMeshes[i];
}

void CMeshTexerEntity::render( eRenderMode renderMode )
{
	updateWVPMatrices();
	G_RENDERCTX->attach( *mRenderMeshes[renderMode] );
}


// --------------------------------------------------------------------------


IDingusAppContext*	gAppContext;
SD3DSettingsPref*	gD3DSettingsPref;

std::string gErrorMsg;
SVector4	gScreenFixUVs;
int			gGlobalFillMode = D3DFILL_SOLID;


bool	gFinished = false;
bool	gShowStats = false;


// --------------------------------------------------------------------------

CDemo::CDemo()
{
}

bool CDemo::checkDevice( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors )
{
	bool ok = true;

	// require VS1.1, or SW vertex processing
	if( caps.getVShaderVersion() < CD3DDeviceCaps::VS_1_1 && vproc < CD3DDeviceCaps::VP_SW ) {
		// don't add error - will turn on swvp and it will be ok
		ok = false;
	}

	// require PS2.0
	if( caps.getPShaderVersion() < CD3DDeviceCaps::PS_2_0 ) {
		errors.addError( "Pixel shader 2.0 required" );
		ok = false;
	}

	// require at least this amount of video memory
	const int videoMB = 48;
	if( caps.getVideoMemoryMB() < videoMB ) {
		char buf[200];
		sprintf( buf, "%iMB of video memory is required, found %iMB", videoMB, caps.getVideoMemoryMB() );
		errors.addError( buf );
		ok = false;
	}

	// return result
	return ok;
}




bool CDemo::shouldFinish()
{
	return gFinished;
}

bool CDemo::shouldShowStats()
{
	return gShowStats;
}



// --------------------------------------------------------------------------
// Demo initialization

CCameraEntity		gCamera;
float				gCameraYaw, gCameraPitch, gCameraDist;

CMeshTexerEntity*	gMesh; // Current mesh. May be NULL.

std::string			gCurMeshName;
std::string			gCurNormalMapName;
std::string			gCurDiffTexName;

float				gMeshRadius = 1.0f;
SVector3			gMeshCenter;


float	gTexerScale;



// mouse
float		gMouseX; // from -1 to 1
float		gMouseY; // from -1 to 1
SVector3	gMouseRay;


// --------------------------------------------------------------------------

void	gLoadNewMesh( const std::string& meshName )
{
	// destroy old mesh if there was one
	if( !gCurMeshName.empty() ) {
		CMeshBundle::getInstance().clearResourceById( gCurMeshName );
	}

	safeDelete( gMesh );
	gCurMeshName = meshName;
	gMesh = new CMeshTexerEntity( meshName );

	// estimate mesh size
	const CAABox& aabb = gMesh->getAABB();
	gMeshCenter = aabb.getCenter();
	SVector3 halfDiag = aabb.getMax() - gMeshCenter;
	gMeshRadius = halfDiag.length();

	// position mesh in the origin
	gMesh->mWorldMat.identify();
	gMesh->mWorldMat.getOrigin() -= gMeshCenter;

	// position camera looking at mesh
	gCameraDist = gMeshRadius*2;
	gCameraYaw = 0.0f;
	gCameraPitch = 0.0f;
}


void gLoadNormapMap( const std::string& fileName )
{
	// destroy old normal map if there was one
	if( !gCurNormalMapName.empty() ) {
		CTextureBundle::getInstance().clearResourceById( gCurNormalMapName );
	}

	gCurNormalMapName = fileName;
	CD3DTexture* tex = RGET_TEX(fileName);
	if( gMesh ) {
		for( int i = 0; i < RMCOUNT; ++i ) {
			CEffectParams& ep = gMesh->getRenderMesh( eRenderMode(i) )->getParams();
			ep.removeTexture( "tNormalAO" );
			ep.addTexture( "tNormalAO", *tex );
		}
	}
}


void gLoadDiffuseTex( const std::string& fileName )
{
	// destroy old texture if there was one
	if( !gCurDiffTexName.empty() ) {
		CTextureBundle::getInstance().clearResourceById( gCurDiffTexName );
	}

	gCurDiffTexName = fileName;
	CD3DTexture* tex = RGET_TEX(fileName);
	if( gMesh ) {
		for( int i = 0; i < RMCOUNT; ++i ) {
			CEffectParams& ep = gMesh->getRenderMesh( eRenderMode(i) )->getParams();
			ep.removeTexture( "tBase" );
			ep.addTexture( "tBase", *tex );
		}
	}
}


void gBakeTexture( int sizeX, int sizeY, D3DXIMAGE_FILEFORMAT format )
{
	if( !gMesh )
		return;

	CD3DDevice& dx = CD3DDevice::getInstance();

	// create RT
	IDirect3DSurface9* rt = 0;
	CD3DSurface dummyRT;
	dx.getDevice().CreateRenderTarget( sizeX, sizeY, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &rt, NULL );
	dummyRT.setObject( rt );
	dx.setRenderTarget( &dummyRT, 0 );
	dx.setZStencil( NULL );

	// render baked mesh into UV space
	CEffectParams& ep = gMesh->getRenderMesh( RM_TEXBAKER )->getParams();
	ep.addFloat( "fSizeX", sizeX );
	ep.addFloat( "fSizeY", sizeY );

	dx.clearTargets( true, false, false, 0xFF000000 );
	dx.sceneBegin();
	G_RENDERCTX->applyGlobalEffect();
	gMesh->render( RM_TEXBAKER );
	G_RENDERCTX->perform();
	dx.sceneEnd();

	ep.removeFloat( "fSizeX" );
	ep.removeFloat( "fSizeY" );

	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();

	// create managed texture and fetch back RT
	IDirect3DTexture9* texture = 0;
	dx.getDevice().CreateTexture( sizeX, sizeY, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &texture, NULL );
	IDirect3DSurface9* surface = 0;
	texture->GetSurfaceLevel( 0, &surface );
	dx.getDevice().GetRenderTargetData( rt, surface );
	surface->Release();
	rt->Release();

	// apply texture gutter
	ID3DXMesh* dxMesh = gMesh->getMesh().createD3DXMesh();
	ID3DXTextureGutterHelper* gutter = NULL;
	HRESULT hr;
	hr = D3DXCreateTextureGutterHelper( sizeX, sizeY, dxMesh, float(max(sizeX,sizeY))/1024*10, &gutter );
	hr = gutter->ApplyGuttersTex( texture );
	gutter->Release();
	dxMesh->Release();

	// save the texture
	const char* fileName = "TexerBaked.dds";
	switch( format ) {
	case D3DXIFF_DDS: fileName = "TexerBaked.dds"; break;
	case D3DXIFF_PNG: fileName = "TexerBaked.png"; break;
	}

	hr = D3DXSaveTextureToFile( fileName, format, texture, NULL );
	texture->Release();

	// remove it from our resource bundle and load again
	CResourceId rid( "../../TexerBaked" );
	CTextureBundle::getInstance().clearResourceById( rid );
	CD3DTexture* tex = RGET_TEX( rid );

	// set it in the effect
	gMesh->getRenderMesh( RM_BAKED )->getParams().removeTexture( "tBaked" );
	gMesh->getRenderMesh( RM_BAKED )->getParams().addTexture( "tBaked", *tex );
}



// --------------------------------------------------------------------------
//  GUI

enum eUICtrlIDs {
	GID_BTN_OPENMESH = 1000,
	GID_BTN_OPENNMAP,
	GID_BTN_OPENDIFFTEX,
	GID_CMB_RENDERMODE,
	GID_BTN_BAKE,
	GID_CMB_BAKESIZEX,
	GID_CMB_BAKESIZEY,
	GID_CMB_BAKEAA,
	GID_CMB_BAKEFORMAT,
	GID_CHK_WIREFRAME,
};

CUIDialog*		gUIDlg;

CUIStatic*		gUILabelFPS;
CUIButton*		gUIBtnOpenMesh;
CUIButton*		gUIBtnOpenNMap;
CUIButton*		gUIBtnOpenDiffTex;
CUIButton*		gUIBtnBake;
CUIComboBox*	gUICmbRenderMode;
CUIComboBox*	gUICmbBakeSizeX;
CUIComboBox*	gUICmbBakeSizeY;
CUIComboBox*	gUICmbBakeAA;
CUIComboBox*	gUICmbBakeFormat;
CUICheckBox*	gUIChkWireframe;

CUIStatic*		gUILabMesh;
CUIStatic*		gUILabNMap;
CUIStatic*		gUILabDiffTex;

CUISlider*		gUISldScale;
CUIStatic*		gUILabScale;


void CALLBACK gUICallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
	switch( evt ) {
	case UIEVENT_BUTTON_CLICKED:
		switch( ctrlID ) {
		case GID_BTN_OPENMESH:
			{
				std::string fileName = gGetOpenFileName( FILE_FILTER_MESH );
				if( !fileName.empty() ) {
					gUIDlg->getButton(GID_BTN_OPENNMAP)->setEnabled( true );
					gUIDlg->getButton(GID_BTN_OPENDIFFTEX)->setEnabled( true );
					gUIDlg->getButton(GID_BTN_BAKE)->setEnabled( true );
					gUICmbBakeSizeX->setEnabled( true );
					gUICmbBakeSizeY->setEnabled( true );
					gLoadNewMesh( fileName );
					gUICmbRenderMode->setSelectedByData( (const void*)RM_CHECKER );
				}
			}
			break;
		case GID_BTN_OPENNMAP:
			{
				std::string fileName = gGetOpenFileName( FILE_FILTER_TEX );
				if( !fileName.empty() ) {
					gLoadNormapMap( fileName );
					gUICmbRenderMode->setSelectedByData( (const void*)RM_NORMALMAP );
				}
			}
			break;
		case GID_BTN_OPENDIFFTEX:
			{
				std::string fileName = gGetOpenFileName( FILE_FILTER_TEX );
				if( !fileName.empty() ) {
					gLoadDiffuseTex( fileName );
					gUICmbRenderMode->setSelectedByData( (const void*)RM_MAPPED );
				}
			}
			break;
		case GID_BTN_BAKE:
			{
				gBakeTexture(
					(int)gUICmbBakeSizeX->getSelectedData(),
					(int)gUICmbBakeSizeY->getSelectedData(),
					(D3DXIMAGE_FILEFORMAT)(DWORD)gUICmbBakeFormat->getSelectedData()
				);
				gUICmbRenderMode->setSelectedByData( (const void*)RM_BAKED );
			}
			break;
		};
	}
}

const int UIHCTL = 16;
const int UIHLAB = 14;


static void	gSetupGUI()
{
	int i;
	int ctlY = 5 - UIHCTL;
	int BTNW = 110;

	gUIDlg->addButton( GID_BTN_OPENMESH, "Open Mesh (M)", 3, ctlY += UIHCTL, BTNW, UIHCTL, 'M', false, &gUIBtnOpenMesh );
	gUIDlg->addStatic( 0, "", 5+BTNW, ctlY, 300, UIHCTL, false, &gUILabMesh );
	gUIDlg->addButton( GID_BTN_OPENNMAP, "Open NMap (N)", 3, ctlY += UIHCTL, BTNW, UIHCTL, 'N', false, &gUIBtnOpenNMap );
	gUIDlg->addStatic( 0, "", 5+BTNW, ctlY, 300, UIHCTL, false, &gUILabNMap );
	gUIBtnOpenNMap->setEnabled( false );
	gUIDlg->addButton( GID_BTN_OPENDIFFTEX, "Open DiffTex (T)", 3, ctlY += UIHCTL, BTNW, UIHCTL, 'T', false, &gUIBtnOpenDiffTex );
	gUIDlg->addStatic( 0, "", 5+BTNW, ctlY, 300, UIHCTL, false, &gUILabDiffTex );
	gUIBtnOpenDiffTex->setEnabled( false );

	gUIDlg->addComboBox( GID_CMB_RENDERMODE, 3, ctlY += UIHCTL*2, BTNW, UIHCTL, 0, false, &gUICmbRenderMode );
	for( i = 0; i < RMCOUNT-1; ++i ) { // last is texture baker - not user selectable
		gUICmbRenderMode->addItem( RMODE_FX[i], (const void*)i, true );
	}
	gUIDlg->addCheckBox( GID_CHK_WIREFRAME, "Wireframe (W)", 3, ctlY += UIHCTL, BTNW, UIHCTL, false, 'W', false, &gUIChkWireframe );

	ctlY += 2*UIHCTL;

	gUIDlg->addButton( GID_BTN_BAKE, "Bake (B)", 3, ctlY, 60, UIHCTL, 'B', false, &gUIBtnBake );
	gUIBtnBake->setEnabled( false );
	gUIDlg->addComboBox( GID_CMB_BAKESIZEX, 65, ctlY, 44, UIHCTL, 0, false, &gUICmbBakeSizeX );
	gUIDlg->addComboBox( GID_CMB_BAKESIZEY, 110, ctlY, 44, UIHCTL, 0, false, &gUICmbBakeSizeY );
	gUIDlg->addComboBox( GID_CMB_BAKEAA, 3, ctlY+=UIHCTL, 60, UIHCTL, 0, false, &gUICmbBakeAA );
	gUIDlg->addComboBox( GID_CMB_BAKEFORMAT, 65, ctlY, 89, UIHCTL, 0, false, &gUICmbBakeFormat );

	const int BAKE_SIZES[] = { 128, 256, 512, 1024, 2048 };
	for( i = 0; i < sizeof(BAKE_SIZES)/sizeof(BAKE_SIZES[0]); ++i ) {
		char buf[100];
		itoa( BAKE_SIZES[i], buf, 10 );
		gUICmbBakeSizeX->addItem( buf, (const void*)BAKE_SIZES[i], true );
		gUICmbBakeSizeY->addItem( buf, (const void*)BAKE_SIZES[i], true );
	}
	// make 1024x1024 bake size the default
	gUICmbBakeSizeX->setSelectedByIndex( 3 );
	gUICmbBakeSizeY->setSelectedByIndex( 3 );
	gUICmbBakeSizeX->setEnabled( false );
	gUICmbBakeSizeY->setEnabled( false );


	const int BAKE_AAS[] = { 1, 2, 4 };
	for( i = 0; i < sizeof(BAKE_AAS)/sizeof(BAKE_AAS[0]); ++i ) {
		char buf[100];
		sprintf( buf, "%ixAA", BAKE_AAS[i] );
		gUICmbBakeAA->addItem( buf, (const void*)BAKE_AAS[i], true );
	}
	// make 2xAA selected by default
	gUICmbBakeAA->setSelectedByIndex( 1 );
	gUICmbBakeAA->setEnabled( false );


	const int BAKE_FMTS[] = { D3DXIFF_DDS, D3DXIFF_PNG };
	const char* BAKE_FMTTEXTS[] = { "DDS", "PNG" };
	for( i = 0; i < sizeof(BAKE_FMTS)/sizeof(BAKE_FMTS[0]); ++i ) {
		gUICmbBakeFormat->addItem( BAKE_FMTTEXTS[i], (const void*)BAKE_FMTS[i], true );
	}
	// make DDS selected by default
	gUICmbBakeFormat->setSelectedByIndex( 0 );

	const int SLD_X = 400;
	ctlY = 5 - UIHCTL;
	gUIDlg->addStatic( 0, "Tex scale", SLD_X, ctlY += UIHCTL, 50, UIHCTL, false, NULL );
	gUIDlg->addSlider( 0, SLD_X+50, ctlY, 140, UIHCTL, 1, 70, 10, false, &gUISldScale );
	gUIDlg->addStatic( 0, "", SLD_X+200, ctlY, 50, UIHCTL, false, &gUILabScale );

	gUIDlg->addStatic( 0, "", 3, 480-UIHLAB-1, 100, UIHLAB, false, &gUILabelFPS );
	gUIDlg->enableNonUserEvents( true );
}


void CDemo::initialize( IDingusAppContext& appContext )
{
	CONS << "Initializing..." << endl;

	gAppContext = &appContext;

	CSharedTextureBundle& stb = CSharedTextureBundle::getInstance();
	CSharedSurfaceBundle& ssb = CSharedSurfaceBundle::getInstance();

	CD3DDevice& dx = CD3DDevice::getInstance();
	CONS << "Video memory seems to be: " << dx.getCaps().getVideoMemoryMB() << endl;

	G_INPUTCTX->addListener( *this );

	// --------------------------------
	// shared resources

	CIndexBufferBundle::getInstance().registerIB( RID_IB_QUADS, 8000*6, D3DFMT_INDEX16, *(new CIBFillerQuads()) );

	// --------------------------------
	// common params

	//G_RENDERCTX->getGlobalParams().addFloatRef( "fTime", &gTimeParam );
	G_RENDERCTX->getGlobalParams().addIntRef( "iFill", &gGlobalFillMode );
	G_RENDERCTX->getGlobalParams().addFloatRef( "fTexerScale", &gTexerScale );

	// --------------------------------
	// game

	// GUI
	gUIDlg = new CUIDialog();
	//gUIDlg->enableKeyboardInput( true );
	gUIDlg->setCallback( gUICallback );
	gUIDlg->setFont( 1, "Arial", 22, 50 );

	gSetupGUI();

	CONS << "Starting..." << endl;
}



// --------------------------------------------------------------------------
// Perform code (main loop)

bool CDemo::msgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	if( !gUIDlg )
		return false;

	bool done = false;

	done = gUIDlg->msgProc( hwnd, msg, wparam, lparam );
	if( done )
		return true;

	// track mouse...
	if( msg == WM_LBUTTONDOWN ) {
		//CEntityManager& entities = CGameInfo::getInstance().getEntities();
		//if( &entities ) {
		//	entities.onMouseClick();
		//}
	}
	if( msg == WM_MOUSEMOVE ) {
		CD3DDevice& dx = CD3DDevice::getInstance();
		gMouseX = (float(LOWORD(lparam)) / dx.getBackBufferWidth()) * 2 - 1;
		gMouseY = (float(HIWORD(lparam)) / dx.getBackBufferHeight()) * 2 - 1;
	}
	return false;
}

const float	TIME_SPD_NORMAL = 1.5f; // 1.5f
const float TIME_SPD_FAST = 10.0f;


void CDemo::onInputEvent( const CInputEvent& event )
{
	float dt = CSystemTimer::getInstance().getDeltaTimeS();
	float dmove = 4.0f * dt;
	float dturn = 2.0f * dt;

	if( event.getType() == CKeyEvent::EVENT_TYPE ) {
		const CKeyEvent& ke = (const CKeyEvent&)event;
		SMatrix4x4 mr;
		switch( ke.getKeyCode() ) {
		case DIK_9:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED )
				gShowStats = !gShowStats;
			break;
		case DIK_LEFT:
			gCameraYaw -= dturn;
			break;
		case DIK_RIGHT:
			gCameraYaw += dturn;
			break;
		case DIK_UP:
			gCameraPitch += dturn;
			gCameraPitch = clamp( gCameraPitch, -D3DX_PI/2, D3DX_PI/2 );
			break;
		case DIK_DOWN:
			gCameraPitch -= dturn;
			gCameraPitch = clamp( gCameraPitch, -D3DX_PI/2, D3DX_PI/2 );
			break;
		case DIK_A:
			gCameraDist -= dmove;
			gCameraDist = clamp( gCameraDist, gMeshRadius*0.5f, gMeshRadius*4 );
			break;
		case DIK_Z:
			gCameraDist += dmove;
			gCameraDist = clamp( gCameraDist, gMeshRadius*0.5f, gMeshRadius*4 );
			break;
		}
	}
}

void CDemo::onInputStage()
{
}


// --------------------------------------------------------------------------

const char* gGetFileNamePart( const std::string& name )
{
	int nameStartIdx = name.find_last_of( "/\\", name.size()-1 );
	const char* s = name.c_str();
	if( nameStartIdx >= 0 )
		s += nameStartIdx+1;
	return s;
}

/**
 *  Main loop code.
 */
void CDemo::perform()
{
	char buf[200];

	double t = CSystemTimer::getInstance().getTimeS();
	float dt = CSystemTimer::getInstance().getDeltaTimeS();
	//gTimeParam = float(t);

	CDynamicVBManager::getInstance().discard();

	CD3DDevice& dx = CD3DDevice::getInstance();
	gScreenFixUVs.set( 0.5f/dx.getBackBufferWidth(), 0.5f/dx.getBackBufferHeight(), 0.0f, 0.0f );

	G_INPUTCTX->perform();

	D3DXMatrixRotationYawPitchRoll( &gCamera.mWorldMat, gCameraYaw, gCameraPitch, 0.0f );
	gCamera.mWorldMat.getOrigin() -= gCamera.mWorldMat.getAxisZ() * gCameraDist;
	gCamera.setProjectionParams( D3DX_PI/3, dx.getBackBufferAspect(), gCameraDist * 0.3f, gCameraDist*10.0f );
	gCamera.setOntoRenderContext();

	//
	// update UI

	gMouseRay = gCamera.getWorldRay( gMouseX, gMouseY );
	const SVector3& eyePos = gCamera.mWorldMat.getOrigin();
	//SLine3 mouseRay;
	//mouseRay.pos = eyePos;
	//mouseRay.vec = gMouseRay;

	// wireframe?
	gGlobalFillMode = gUIChkWireframe->isChecked() ? D3DFILL_WIREFRAME : D3DFILL_SOLID;

	// loaded mesh and textures
	if( gMesh ) {
		sprintf( buf, "%s, %i verts, %i tris", gGetFileNamePart(gCurMeshName), gMesh->getMesh().getVertexCount(), gMesh->getMesh().getIndexCount()/3 );
		gUILabMesh->setText( buf );
	}
	if( !gCurNormalMapName.empty() ) {
		gUILabNMap->setText( gGetFileNamePart(gCurNormalMapName) );
	}
	if( !gCurDiffTexName.empty() ) {
		gUILabDiffTex->setText( gGetFileNamePart(gCurDiffTexName) );
	}

	// scales
	gTexerScale = gUISldScale->getValue() * 0.1f;
	sprintf( buf, "%.1f", gTexerScale );
	gUILabScale->setText( buf );
	
	// FPS
	sprintf( buf, "fps: %.1f", dx.getStats().getFPS() );
	gUILabelFPS->setText( buf );


	dx.clearTargets( true, true, false, 0xFF8080a0, 1.0f, 0L );
	dx.sceneBegin();
	G_RENDERCTX->applyGlobalEffect();

	// render mesh if we have it
	if( gMesh )
		gMesh->render( eRenderMode( (int)gUICmbRenderMode->getSelectedData() ) );
	
	G_RENDERCTX->perform();

	// render GUI
	gUIDlg->onRender( dt );
	dx.sceneEnd();

	// sleep a bit in order not to burn everything :)
	Sleep( 1 );
}



// --------------------------------------------------------------------------
// Cleanup


void CDemo::shutdown()
{
	CONS << "Shutting down..." << endl;

	if( gMesh )
		delete gMesh;

	safeDelete( gUIDlg );
	CONS << "Done!" << endl;
}
