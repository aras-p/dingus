// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "VideoPlayer.h"
#include "Mutex.h"
#include "../../kernel/D3DDevice.h"
#include <streams.h>
#include <errors.h>

using namespace dingus;


// --------------------------------------------------------------------------
//  DShow Texture Renderer
// --------------------------------------------------------------------------

// GUID for Texture Renderer
// {3317CBA4-A022-458c-B5AF-97A6B4B8C138}
struct __declspec(uuid("{3317CBA4-A022-458c-B5AF-97A6B4B8C138}")) CLSID_DShowTextureRenderer;


// ------------------------------------

namespace dingus {

class CDShowTextureRenderer : public CBaseVideoRenderer {
public:
	CDShowTextureRenderer( LPUNKNOWN pUnk,HRESULT *phr );
	~CDShowTextureRenderer();
	
public:
	HRESULT CheckMediaType( const CMediaType *pmt );		// Format acceptable?
	HRESULT SetMediaType( const CMediaType *pmt );			// Video format notification
	HRESULT DoRenderSample( IMediaSample *pMediaSample );	// New video sample
	
	// no interface, just corresponds to IDeviceResource
	void createResource();
	void activateResource();
	void passivateResource();
	void deleteResource();

	void stop();

	CD3DTexture& getTexture() { return *mTexture; }
	int getVideoX() const { return mVideoX; }
	int getVideoY() const { return mVideoY; }
	int getTextureX() const { return mTextureX; }
	int getTextureY() const { return mTextureY; }

private:
	HRESULT initTexture(); // creates texture

public:
	CMutex	mMutex;
private:
	int 			mVideoX, mVideoY, mVideoPitch;
	int				mTextureX, mTextureY;
	IDirect3DSurface9*	mSurface;
	CD3DTexture*		mTexture;
	D3DFORMAT		mTextureFormat;
};

}; // namespace

// ------------------------------------

CDShowTextureRenderer::CDShowTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr )
:	CBaseVideoRenderer(__uuidof(CLSID_DShowTextureRenderer), NAME("Texture Renderer"), pUnk, phr),
	mVideoX(-1), mVideoY(-1),
	mSurface(NULL), mTexture(NULL)
{
	assert( phr );
	if( phr )
		*phr = S_OK;
	assert( !mTexture );
	mTexture = new CD3DTexture();
}

CDShowTextureRenderer::~CDShowTextureRenderer()
{
	stop();
	delete mTexture;
}

// ------------------------------------

HRESULT CDShowTextureRenderer::CheckMediaType( const CMediaType *pmt )
{
	CScopedLock lock( mMutex );

	// Force R8G8B8 video type so we can copy to texture easily

	HRESULT   hr = E_FAIL;
	CheckPointer( pmt, E_POINTER );

	// Reject the connection if this is not a video type
	if( *pmt->FormatType() != FORMAT_VideoInfo )
		return E_INVALIDARG;

	// Only accept RGB24 video
	VIDEOINFO* pvi = (VIDEOINFO*)pmt->Format();
	if( IsEqualGUID( *pmt->Type(), MEDIATYPE_Video) && IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24 ) )
		hr = S_OK;
	return hr;
}

HRESULT CDShowTextureRenderer::initTexture()
{
	CScopedLock lock( mMutex );

	assert( !mSurface );
	assert( mTexture );
	assert( mTexture->isNull() );
	HRESULT hr = S_OK;

	CD3DDevice& dx = CD3DDevice::getInstance();
	const D3DCAPS9& caps = dx.getCaps();

	// calc texture size
	mTextureX = 16;
	mTextureY = 16;
	if( caps.TextureCaps & D3DPTEXTURECAPS_POW2 ) {
		while( mTextureX < mVideoX )
			mTextureX = mTextureX << 1;
		while( mTextureY < mVideoY )
			mTextureY = mTextureY << 1;
	} else {
		mTextureX = mVideoX;
		mTextureY = mVideoY;
	}
	
	// create texture
	IDirect3DTexture9* texture = NULL;
	hr = dx.getDevice().CreateTexture( mTextureX, mTextureY, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R5G6B5, D3DPOOL_DEFAULT, &texture, NULL );
	if( FAILED(hr) ) {
		ASSERT_FAIL_MSG( "Can't create d3d texture" );
		return hr;
	}

	hr = dx.getDevice().CreateOffscreenPlainSurface( mTextureX, mTextureY, D3DFMT_R5G6B5, D3DPOOL_DEFAULT, &mSurface, NULL );
	if( FAILED(hr) ) {
		ASSERT_FAIL_MSG( "Can't create d3d surface" );
		return hr;
	}
	
	// d3d can silently change the parameters on us, so check it
	D3DSURFACE_DESC ddsd;
	ZeroMemory( &ddsd, sizeof(ddsd) );
	if( FAILED( hr = mSurface->GetDesc( &ddsd ) ) ) {
		ASSERT_FAIL_MSG( "Can't get texture desc" );
		texture->Release();
		return hr;
	}
	mTextureFormat = ddsd.Format;
	if( mTextureFormat != D3DFMT_X8R8G8B8 && mTextureFormat != D3DFMT_R5G6B5 ) {
		ASSERT_FAIL_MSG( "Can't handle such texture format!" );
		texture->Release();
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// clear texture to white TBD: supply color?
	IDirect3DSurface9* texSurf = 0;
	texture->GetSurfaceLevel( 0, &texSurf );
	dx.getDevice().ColorFill( texSurf, NULL, 0xFFffffff );
	texSurf->Release();

	mTexture->setObject( texture );
	return S_OK;
}

HRESULT CDShowTextureRenderer::SetMediaType( const CMediaType *pmt )
{
	VIDEOINFO *pviBmp = (VIDEOINFO*)pmt->Format();
	mVideoX  = pviBmp->bmiHeader.biWidth;
	mVideoY = abs(pviBmp->bmiHeader.biHeight);
	mVideoPitch  = (mVideoX * 3 + 3) & ~(3); // We are forcing RGB24

	return initTexture();
}

HRESULT CDShowTextureRenderer::DoRenderSample( IMediaSample* sample )
{
	BYTE  *ptrBmp, *ptrTex;
	LONG  pitchTex;
	
	CScopedLock lock( mMutex );
	if( !mTexture->getObject() )
		return S_OK;
	if( !mSurface )
		return S_OK;

	CheckPointer(sample,E_POINTER);

	// get the video bitmap buffer
	sample->GetPointer( &ptrBmp );
	
	// lock the surface
	D3DLOCKED_RECT lr;
	if( FAILED( mSurface->LockRect( &lr, 0, 0 ) ) )
		return E_FAIL;
	ptrTex = static_cast<byte*>( lr.pBits );
	pitchTex = lr.Pitch;
	
	// Copy the bits	
	if( mTextureFormat == D3DFMT_X8R8G8B8 ) {
		// Instead of copying data bytewise, we use DWORD alignment here.
		// We also unroll loop by copying 4 pixels at once.
		// 
		// original BYTE array is [b0][g0][r0][b1][g1][r1][b2][g2][r2][b3][g3][r3]
		// aligned DWORD array is [b1 r0 g0 b0][g2 b2 r1 g1][r3 g3 b3 r2]
		// We want to transform it to [ff r0 g0 b0][ff r1 g1 b1][ff r2 g2 b2][ff r3 b3 g3]
		
		int dwordWidth = mVideoX / 4; // aligned width of the row, in DWORDS (pixel by 3 bytes over sizeof(DWORD))
		for( int row = 0; row < mVideoY; ++row ) {
			int col;
			DWORD* pdwS = (DWORD*)ptrBmp;
			DWORD* pdwD = (DWORD*)ptrTex;
			for( col = 0; col < dwordWidth; ++col ) {
				pdwD[0] =  pdwS[0] | 0xFF000000;
				pdwD[1] = ((pdwS[1]<<8)  | 0xFF000000) | (pdwS[0]>>24);
				pdwD[2] = ((pdwS[2]<<16) | 0xFF000000) | (pdwS[1]>>16);
				pdwD[3] = 0xFF000000 | (pdwS[2]>>8);
				pdwD +=4;
				pdwS +=3;
			}
			// we might have remaining (misaligned) bytes here
			BYTE* pbS = (BYTE*) pdwS;
			for( col = 0; col < mVideoX%4; ++col ) {
				*pdwD = 0xFF000000 |
					(pbS[2] << 16) |
					(pbS[1] <<	8) |
					(pbS[0]);
				pdwD++;
				pbS += 3;			
			}
			ptrBmp += mVideoPitch;
			ptrTex += pitchTex;
		}
	} else if( mTextureFormat == D3DFMT_R5G6B5 ) {
		for( int y = 0; y < mVideoY; ++y ) {
			BYTE *oldBmp = ptrBmp;
			BYTE *oldTex = ptrTex;	
			for( int x = 0; x < mVideoX; ++x ) {
				*(WORD*)ptrTex = (WORD)
					(((ptrBmp[2] & 0xF8) << 8) +
					((ptrBmp[1] & 0xFC) << 3) +
					(ptrBmp[0] >> 3));
				ptrTex += 2;
				ptrBmp += 3;
			}
			ptrBmp = oldBmp + mVideoPitch;
			ptrTex = oldTex + pitchTex;
		}
	}
	
	// unlock the surface
	if( FAILED( mSurface->UnlockRect() ) )
		return E_FAIL;

	// copy into texture
	IDirect3DSurface9* texSurf = NULL;
	HRESULT hr = mTexture->getObject()->GetSurfaceLevel( 0, &texSurf );
	if( FAILED(hr) ) {
		return E_FAIL;
	}
	CD3DDevice& dx = CD3DDevice::getInstance();
	hr = dx.getDevice().StretchRect( mSurface, NULL, texSurf, NULL, D3DTEXF_NONE );
	texSurf->Release();
	
	return S_OK;
}

void CDShowTextureRenderer::stop()
{
	passivateResource();
	mVideoX = -1; mVideoY = -1;
}

// ------------------------------------

void CDShowTextureRenderer::createResource()
{
	assert( mTexture->isNull() );
}

void CDShowTextureRenderer::activateResource()
{
	// kind of HACK: if we don't have video size - then we aren't playing yet
	if( mVideoX < 0 )
		return;
	assert( mTexture->isNull() );
	HRESULT hr = initTexture();
	assert( SUCCEEDED(hr) );
}

void CDShowTextureRenderer::passivateResource()
{
	CScopedLock lock( mMutex );
	if( !mTexture->isNull() ) {
		mTexture->getObject()->Release();
		mTexture->setObject( NULL );
	}
	safeRelease( mSurface );
}

void CDShowTextureRenderer::deleteResource()
{
	assert( mTexture->isNull() );
}

// --------------------------------------------------------------------------
//  Video Player
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
/*
static HRESULT gGetUnconnectedPin( IBaseFilter *filter, PIN_DIRECTION pinDir, IPin **resPin )
{
	*resPin = 0;
	IPin *pin = 0;
	IEnumPins *pEnum = 0;
	HRESULT hr = filter->EnumPins(&pEnum);
	if( FAILED(hr) )
		return hr;
	while( pEnum->Next( 1, &pin, NULL ) == S_OK ) {
		PIN_DIRECTION thisPinDir;
		pin->QueryDirection( &thisPinDir );
		if( thisPinDir == pinDir ) {
			IPin *tmp = 0;
			hr = pin->ConnectedTo( &tmp );
			if( SUCCEEDED(hr) )	{ // Already connected
				tmp->Release();
			} else { // Unconnected, this is the pin we want.
				pEnum->Release();
				*resPin = pin;
				return S_OK;
			}
		}
		pin->Release();
	}
	pEnum->Release();
	// Did not find a matching pin.
	return E_FAIL;
}

static HRESULT gConnectFilters( IGraphBuilder *graph, IPin *pinOut, IBaseFilter *dest )
{
	if( (graph == NULL) || (pinOut == NULL) || (dest == NULL) )
		return E_POINTER;

#ifdef _DEBUG
	PIN_DIRECTION PinDir;
	pinOut->QueryDirection(&PinDir);
	assert( PinDir == PINDIR_OUTPUT );
#endif
	
	// Find an input pin on the downstream filter.
	IPin *pIn = 0;
	HRESULT hr = gGetUnconnectedPin( dest, PINDIR_INPUT, &pIn );
	if( FAILED(hr) ) {
		return hr;
	}
	// Try to connect them.
	hr = graph->Connect( pinOut, pIn );
	pIn->Release();
	return hr;
}

static HRESULT gConnectFilters( IGraphBuilder *graph, IBaseFilter *src, IBaseFilter *dest )
{
	if( (graph == NULL) || (src == NULL) || (dest == NULL) )
		return E_POINTER;
	
	// Find an output pin on the first filter.
	IPin *pinOut = 0;
	HRESULT hr = gGetUnconnectedPin( src, PINDIR_OUTPUT, &pinOut );
	if( FAILED(hr) ) 
		return hr;
	hr = gConnectFilters( graph, pinOut, dest );
	pinOut->Release();
	return hr;
}
*/

// --------------------------------------------------------------------------

CVideoPlayer::CVideoPlayer()
:	mGraph(0), mRenderer(0), mMediaControl(0), mMediaEvent(0)
{
	HRESULT hr;
	mRenderer = new CDShowTextureRenderer( NULL, &hr );
	assert( SUCCEEDED(hr) && mRenderer );
	mRenderer->AddRef();
}

CVideoPlayer::~CVideoPlayer()
{
	stop();
	assert( mRenderer );
	mRenderer->Release();
}

bool CVideoPlayer::play( const char* file )
{
	stop();
	
	assert( file );
	WCHAR wfile[1000];
	int flen = strlen( file );
	MultiByteToWideChar( CP_ACP, 0, file, -1, wfile, flen );
	wfile[flen] = 0;
	
	HRESULT hr;
	
	// create filter graph
	hr = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_ALL, IID_IFilterGraph, (void**)&mGraph );
	if( FAILED(hr) ) {
		stop();
		return false;
	}

	// create texture renderer, add to graph
	if( FAILED(hr = mGraph->AddFilter( mRenderer, L"TexRenderer") ) ) {
		stop();
		return false;
	}

	// source filter, render
	IBaseFilter *source;
	hr = mGraph->AddSourceFilter( wfile, L"Source", &source );
	if( FAILED(hr) ) {
		char buf[1000];
		AMGetErrorText( hr, buf, 1000 );
		// TBD
		return false;
	}
	IPin* pinOut = NULL;
	if( FAILED(hr = source->FindPin( L"Output", &pinOut ) ) ) {
		// TBD
		return false;
	}
	if( FAILED(hr = mGraph->Render(pinOut) ) ) {
		// TBD
		return false;
	}

	// run
	hr = mGraph->QueryInterface( IID_IMediaControl, (void**)&mMediaControl );
	if( FAILED(hr) )
		return false;
	hr = mGraph->QueryInterface( IID_IMediaEvent, (void**)&mMediaEvent );
	if( FAILED(hr) )
		return false;
	hr = mMediaControl->Run();
	if( FAILED(hr) )
		return false;
	
	return true;
}

void CVideoPlayer::stop()
{
	if( mMediaControl != NULL ) {
		OAFilterState state;
		do {
			mMediaControl->Stop();
			mMediaControl->GetState( 0, &state );
		} while( state != State_Stopped );
	}
	mRenderer->stop();
	safeRelease( mMediaControl );
	safeRelease( mMediaEvent );
	safeRelease( mGraph );
}

bool CVideoPlayer::isPlaying()
{
    HRESULT hr;

    if( !mMediaEvent )
        return false;
        
    // Check for completion events
	bool playing = true;
    long lEventCode;
    long lParam1;
    long lParam2;
    hr = mMediaEvent->GetEvent( &lEventCode, (LONG_PTR*)&lParam1, (LONG_PTR*) &lParam2, 0 );
    if( SUCCEEDED(hr) ) {
        if( EC_COMPLETE == lEventCode )
			playing = false;
        // Free any memory associated with this event
        hr = mMediaEvent->FreeEventParams( lEventCode, lParam1, lParam2 );
    }
	return playing;
}


void CVideoPlayer::createResource()
{
	mRenderer->createResource();
}

void CVideoPlayer::activateResource()
{
	mRenderer->activateResource();
}

void CVideoPlayer::passivateResource()
{
	mRenderer->passivateResource();
}

void CVideoPlayer::deleteResource()
{
	mRenderer->deleteResource();
}

CD3DTexture& CVideoPlayer::getTexture()
{
	return mRenderer->getTexture();
}

int CVideoPlayer::getVideoX() const { return mRenderer->getVideoX(); }
int CVideoPlayer::getVideoY() const { return mRenderer->getVideoY(); }
int CVideoPlayer::getTextureX() const { return mRenderer->getTextureX(); }
int CVideoPlayer::getTextureY() const { return mRenderer->getTextureY(); }

