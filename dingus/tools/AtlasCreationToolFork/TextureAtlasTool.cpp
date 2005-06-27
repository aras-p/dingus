//-----------------------------------------------------------------------------
// Copyright NVIDIA Corporation 2004
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED 
// *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS 
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL 
// NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR 
// CONSEQUENTIAL DAMAGES WHATSOEVER INCLUDING, WITHOUT LIMITATION, DAMAGES FOR 
// LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS 
// INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR 
// INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGES.
// 
// File: TextureAtlasTool.cpp
// Desc: DirectX window application created by the DirectX AppWizard
//-----------------------------------------------------------------------------

#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <stdio.h>
#include <d3dx9.h>
#include <algorithm>

#include "resource.h"
#include "DX9SDKSampleFramework\DX9SDKSampleFramework.h"
#include "TextureAtlasTool.h"

#include "TATypes.h"
#include "CmdLineOptions.h"
#include "TextureObject.h"
#include "AtlasContainer.h"


//-----------------------------------------------------------------------------
// Global access to the app (needed for the global WndProc())
//-----------------------------------------------------------------------------
CMyD3DApplication* g_pApp  = NULL;
HINSTANCE          g_hInst = NULL;


void PrintVersion()
{
    float const     kVersion = 2.0f;
	fprintf( stderr, "TextureAtlasTool.exe Version %5.2f\n", kVersion );
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point to the program. It parses all cmd-line options 
//       and fills a command structure w/ all this data.
//       Then creates a d3dapp oject that has reference to a d3d device 
//       and uses that device to create, load, and save textures,
//       particularly texture atlases.
//-----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    PrintVersion();

    CmdLineOptionCollection options(argc, argv);
    if (! options.IsValid())
        exit (-1);

    CMyD3DApplication d3dApp;
    g_pApp  = &d3dApp;

    InitCommonControls();
    if (FAILED(d3dApp.Create(NULL)))
        return 0;

    bool const kResult = d3dApp.CreateTextureAtlases(options);
    d3dApp.CleanShutdown();

    return kResult ? 0 : -1;
}


//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: Application constructor.   Paired with ~CMyD3DApplication()
//       Member variables should be initialized to a known state here.  
//       The application window has not yet been created and no Direct3D device 
//       has been created, so any initialization that depends on a window or 
//       Direct3D should be deferred to a later stage. 
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_dwCreationWidth                   = 400;
    m_dwCreationHeight                  = 300;
    m_strWindowTitle                    = TEXT( "TextureAtlasTool" );
    m_d3dEnumeration.AppUsesDepthBuffer = TRUE;
	m_bStartFullscreen			        = false;
	m_bShowCursorWhenFullscreen	        = false;
}


//-----------------------------------------------------------------------------
// Name: ~CMyD3DApplication()
// Desc: Application destructor.  Paired with CMyD3DApplication()
//-----------------------------------------------------------------------------
CMyD3DApplication::~CMyD3DApplication()
{
    ;
}

//-----------------------------------------------------------------------------
// Name: CleanShutdown()
// Desc: This function sends a message to the d3dapp simulating the to tell it
//       to cleanup and close the window it created.
//-----------------------------------------------------------------------------
void CMyD3DApplication::CleanShutdown()
{
    // Close the window, which shuts down the app
    SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the display device
//       for some minimum set of capabilities
//       Should ultimately be extended to only accept the reference device.
//       What about system where the reference device is not enumerated though?
//       In those case I really should go through and check for all the things 
//       the tool actually needs: 
//       texture formats, redner-targets formats etc.
//       For now cop out and accept any device :(
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format, D3DFORMAT Format2)
{
    UNREFERENCED_PARAMETER( Format2 );
    UNREFERENCED_PARAMETER( Format );
    UNREFERENCED_PARAMETER( dwBehavior );
    UNREFERENCED_PARAMETER( pCaps );
    
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    // Clear the viewport
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         0x707000ff, 1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
        m_pd3dDevice->EndScene();

    return S_OK;
}


// Override the following 2 D3DApplication functions so that the window remains
// hidden but a device is succesfully created:
//     CMyD3DApplication::Create()
//     CMyD3DApplicationInitialize3DEnvironment()

//-----------------------------------------------------------------------------
// Name: Initialize3DEnvironment()
// Desc: Usually this function is not overridden.  Here's what this function does:
//       - Sets the windowed flag to be either windowed or fullscreen
//       - Sets parameters for z-buffer depth and back buffer
//       - Creates the D3D device
//       - Sets the window position (if windowed, that is)
//       - Makes some determinations as to the abilites of the driver (HAL, etc)
//       - Sets up some cursor stuff
//       - Calls InitDeviceObjects()
//       - Calls RestoreDeviceObjects()
//       - If all goes well, m_bActive is set to TRUE, and the function returns
//       - Otherwise, initialization is reattempted using the reference device
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Initialize3DEnvironment()
{
    HRESULT hr;

    D3DAdapterInfo* pAdapterInfo = m_d3dSettings.PAdapterInfo();
    D3DDeviceInfo*  pDeviceInfo  = m_d3dSettings.PDeviceInfo();

    m_bWindowed = m_d3dSettings.IsWindowed;

    // Prepare window for possible windowed/fullscreen change
    AdjustWindowForChange();

    // Set up the presentation parameters
    BuildPresentParamsFromSettings();

    if( pDeviceInfo->Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE )
    {
        // Warn user about null ref device that can't render anything
        DisplayErrorMsg( D3DAPPERR_NULLREFDEVICE, 0 );
    }

    DWORD behaviorFlags;
    if (m_d3dSettings.GetVertexProcessingType() == SOFTWARE_VP)
        behaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    else if (m_d3dSettings.GetVertexProcessingType() == MIXED_VP)
        behaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
    else if (m_d3dSettings.GetVertexProcessingType() == HARDWARE_VP)
        behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else if (m_d3dSettings.GetVertexProcessingType() == PURE_HARDWARE_VP)
        behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
    else
        behaviorFlags = 0; // TODO: throw exception

    // Add multithreaded flag if requested by app
    if( m_bCreateMultithreadDevice )
        behaviorFlags |= D3DCREATE_MULTITHREADED;

    // Create the device
    hr = m_pD3D->CreateDevice( m_d3dSettings.AdapterOrdinal(), pDeviceInfo->DevType,
                               m_hWndFocus, behaviorFlags, &m_d3dpp,
                               &m_pd3dDevice );

    if( SUCCEEDED(hr) )
    {
        // When moving from fullscreen to windowed mode, it is important to
        // adjust the window size after recreating the device rather than
        // beforehand to ensure that you get the window size you want.  For
        // example, when switching from 640x480 fullscreen to windowed with
        // a 1000x600 window on a 1024x768 desktop, it is impossible to set
        // the window size to 1000x600 until after the display mode has
        // changed to 1024x768, because windows cannot be larger than the
        // desktop.
        if( m_bWindowed )
        {
            SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                          m_rcWindowBounds.left, m_rcWindowBounds.top,
                          ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                          ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                          SWP_HIDEWINDOW );
        }

        // Store device Caps
        m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
        m_dwCreateFlags = behaviorFlags;

        // Store device description
        if( pDeviceInfo->DevType == D3DDEVTYPE_REF )
            lstrcpy( m_strDeviceStats, TEXT("REF") );
        else if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
            lstrcpy( m_strDeviceStats, TEXT("HAL") );
        else if( pDeviceInfo->DevType == D3DDEVTYPE_SW )
            lstrcpy( m_strDeviceStats, TEXT("SW") );

        if( behaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING &&
            behaviorFlags & D3DCREATE_PUREDEVICE )
        {
            if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (pure hw vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated pure hw vp)") );
        }
        else if( behaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING )
        {
            if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (hw vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated hw vp)") );
        }
        else if( behaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING )
        {
            if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (mixed vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated mixed vp)") );
        }
        else if( behaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
        {
            lstrcat( m_strDeviceStats, TEXT(" (sw vp)") );
        }

        if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
        {
            // Be sure not to overflow m_strDeviceStats when appending the adapter 
            // description, since it can be long.  Note that the adapter description
            // is initially CHAR and must be converted to TCHAR.
            lstrcat( m_strDeviceStats, TEXT(": ") );
            const int cchDesc = sizeof(pAdapterInfo->AdapterIdentifier.Description);
            TCHAR szDescription[cchDesc];
            DXUtil_ConvertAnsiStringToGenericCch( szDescription, 
                pAdapterInfo->AdapterIdentifier.Description, cchDesc );
            int maxAppend = sizeof(m_strDeviceStats) / sizeof(TCHAR) -
                lstrlen( m_strDeviceStats ) - 1;
            _tcsncat( m_strDeviceStats, szDescription, maxAppend );
        }

        // Store render target surface desc
        LPDIRECT3DSURFACE9 pBackBuffer = NULL;
        m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
        pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
        pBackBuffer->Release();

        // Set up the fullscreen cursor
        if( m_bShowCursorWhenFullscreen && !m_bWindowed )
        {
            HCURSOR hCursor;
#ifdef _WIN64
            hCursor = (HCURSOR)GetClassLongPtr( m_hWnd, GCLP_HCURSOR );
#else
            hCursor = (HCURSOR)ULongToHandle( GetClassLong( m_hWnd, GCL_HCURSOR ) );
#endif
            D3DUtil_SetDeviceCursor( m_pd3dDevice, hCursor, true );
            m_pd3dDevice->ShowCursor( true );
        }

        // Confine cursor to fullscreen window
        if( m_bClipCursorWhenFullscreen )
        {
            if (!m_bWindowed )
            {
                RECT rcWindow;
                GetWindowRect( m_hWnd, &rcWindow );
                ClipCursor( &rcWindow );
            }
            else
            {
                ClipCursor( NULL );
            }
        }

        // Initialize the app's device-dependent objects
        hr = InitDeviceObjects();
        if( FAILED(hr) )
        {
            DeleteDeviceObjects();
        }
        else
        {
            m_bDeviceObjectsInited = true;
            hr = RestoreDeviceObjects();
            if( FAILED(hr) )
            {
                InvalidateDeviceObjects();
            }
            else
            {
                m_bDeviceObjectsRestored = true;
                return S_OK;
            }
        }

        // Cleanup before we try again
        Cleanup3DEnvironment();
    }

    // If that failed, fall back to the reference rasterizer
    if( hr != D3DAPPERR_MEDIANOTFOUND && 
        hr != HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) && 
        pDeviceInfo->DevType == D3DDEVTYPE_HAL )
    {
        if (FindBestWindowedMode(false, true))
        {
            m_bWindowed = true;
            AdjustWindowForChange();
            // Make sure main window isn't topmost, so error message is visible
            SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                          m_rcWindowBounds.left, m_rcWindowBounds.top,
                          ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                          ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                          SWP_SHOWWINDOW );

            // Let the user know we are switching from HAL to the reference rasterizer
            DisplayErrorMsg( hr, MSGWARN_SWITCHEDTOREF );

            hr = Initialize3DEnvironment();
        }
    }
    return hr;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
//-----------------------------------------------------------------------------
// Name: Create()
// Desc: Here's what this function does:
//       - Checks to make sure app is still active (if fullscreen, etc)
//       - Checks to see if it is time to draw with DXUtil_Timer, if not, it just returns S_OK
//       - Calls FrameMove() to recalculate new positions
//       - Calls Render() to draw the new frame
//       - Updates some frame count statistics
//       - Calls m_pd3dDevice->Present() to display the rendered frame.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Create( HINSTANCE hInstance)
{
    HRESULT hr;

    // Create the Direct3D object
    m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if( m_pD3D == NULL )
        return DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );

    // Build a list of Direct3D adapters, modes and devices. The
    // ConfirmDevice() callback is used to confirm that only devices that
    // meet the app's requirements are considered.
    m_d3dEnumeration.SetD3D( m_pD3D );
    m_d3dEnumeration.ConfirmDeviceCallback = ConfirmDeviceHelper;
    if( FAILED( hr = m_d3dEnumeration.Enumerate() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

    // Unless a substitute hWnd has been specified, create a window to
    // render into
    if( m_hWnd == NULL)
    {
        // Register the windows class
        WNDCLASS wndClass = { 0, WndProc, 0, 0, hInstance,
                              LoadIcon( hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON) ),
                              LoadCursor( NULL, IDC_ARROW ),
                              (HBRUSH)GetStockObject(WHITE_BRUSH),
                              NULL, _T("D3D Window") };
        RegisterClass( &wndClass );

        // Set the window's initial style
        m_dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | 
                          WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        
        HMENU hMenu = LoadMenu( hInstance, MAKEINTRESOURCE(IDR_MENU) );

        // Set the window's initial width
        RECT rc;
        SetRect( &rc, 0, 0, m_dwCreationWidth, m_dwCreationHeight );        
        AdjustWindowRect( &rc, m_dwWindowStyle, ( hMenu != NULL ) ? true : false );

        // Create the render window
        m_hWnd = CreateWindow( _T("D3D Window"), m_strWindowTitle, m_dwWindowStyle,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               (rc.right-rc.left), (rc.bottom-rc.top), 0,
                               hMenu, hInstance, 0 );
    }

    // The focus window can be a specified to be a different window than the
    // device window.  If not, use the device window as the focus window.
    if( m_hWndFocus == NULL )
        m_hWndFocus = m_hWnd;

    // Save window properties
    m_dwWindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
    GetWindowRect( m_hWnd, &m_rcWindowBounds );
    GetClientRect( m_hWnd, &m_rcWindowClient );

    if( FAILED( hr = ChooseInitialD3DSettings() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

    // Initialize the application timer
    DXUtil_Timer( TIMER_START );

    // Initialize the app's custom scene stuff
    if( FAILED( hr = OneTimeSceneInit() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

    // Initialize the 3D environment for the app
    if( FAILED( hr = Initialize3DEnvironment() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

    // The app is ready to go
    Pause( false );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CreateTextureAtlases()
// Desc: Creates atlases for the given textures.
//       All options/info is stored in the options parameter.
//       Returns false if errors occur.
//-----------------------------------------------------------------------------
bool CMyD3DApplication::CreateTextureAtlases(CmdLineOptionCollection const &options) const
{
    // Create an array of texture objects and load the filenames/textures into that array.
    int const   kNumTextures = options.GetNumFilenames();
    char const *pFilename = NULL;
    int         i;

    Texture2D * pSourceTex = new Texture2D[kNumTextures];

    for (i = 0; i < kNumTextures; ++i)
    {
        options.GetFilename(i, &pFilename);
        pSourceTex[i].Init(m_pd3dDevice, pFilename);
        if (FAILED(pSourceTex[i].LoadTexture(options)))
        {
            delete [] pSourceTex;
            return false;
        }
    }

    // Bin these textures into format groups (maps of vectors)
    TNewFormatMap   formatMap;
    for (i = 0; i < kNumTextures; ++i)
        formatMap[pSourceTex[i].GetFormat()].push_back( &(pSourceTex[i]) );

    // We do not do format conversions, so all these different formats 
    // require their own atlases.  Each format may have multiple atlases, e.g., 
    // there is not enough space in a single atlas for all textures of the 
    // same format.  An atlas container contains all these concepts.
    AtlasContainer   atlas(options, formatMap.size());

    // For each format-vector of textures, Sort textures by size (width*height, then height
    TNewFormatMap::iterator fmSort;
    for ( fmSort = formatMap.begin(); fmSort != formatMap.end(); ++fmSort)
        std::sort( (*fmSort).second.begin(), (*fmSort).second.end(), Texture2DGreater() );

    // For each format-vector of textures, insert them into their respective 
    // atlas vector: 
    TNewFormatMap::const_iterator fmIter;
    for (i = 0, fmIter = formatMap.begin(); fmIter != formatMap.end(); ++fmIter, ++i)
        atlas.Insert(i, fmIter->second);

    // Done inserting data: shrink all atlases to minimum size
    atlas.Shrink();

    // Write all atlases to disk w/ the filenames they have stored
    atlas.WriteToDisk();

    // Save the Texture Atlas Info (tai) file.
    // For each original texture read-out where it landed up
    // and write that into the tai file.
    bool const kSuccess = CreateTAIFile(options, formatMap);

    // free all memory: clear all vectors of everything
    delete [] pSourceTex;
    return kSuccess;
}

bool CMyD3DApplication::CreateTAIFile(CmdLineOptionCollection const &options, 
                                      TNewFormatMap const           &formatMap) const
{
    // write the tai header 
    char     fname[kFilenameLength];
    sprintf( fname, "%s.tai", options.GetArgument(CLO_OUTFILE, 0));

    FILE   *fp = fopen( fname, "w" );
    if (fp == NULL)
    {
        fprintf( stderr, "*** Error: Unable to open file \"%s\" for writing.\n", fname );
        return false;
    }
    fprintf( stderr, "Saving file: %s\n", fname );

    fprintf( fp, "# %s\n", fname );
    // echo the cmd line used to invoke this
    fprintf( fp, "# AtlasCreationTool.exe");
    for (int i = CLO_NOMIPMAP; i < CLO_NUM; ++i)
        if (options.IsSet(static_cast<eCmdLineOptionType>(i)))
        {
            fprintf( fp, " %s", kParseString[i]);
            for (int j = 0; j < kNumArguments[i]; ++j) 
                fprintf( fp, " %s", options.GetArgument(static_cast<eCmdLineOptionType>(i), j));    
        }
    fprintf( fp, "\n#\n");
    fprintf( fp, "# <filename>\t\t<atlas filename>, <atlas idx>, <atlas type>, <woffset>, <hoffset>, <depth offset>, <width>, <height>\n#\n" );
    fprintf( fp, "# Texture <filename> can be found in texture atlas <atlas filename>, i.e., \n");
    fprintf( fp, "# %s<idx>.dds of <atlas type> type with texture coordinates boundary given by:\n", options.GetArgument(CLO_OUTFILE, 0));
    fprintf( fp, "#   A = ( <woffset>, <hoffset> )\n" );
    fprintf( fp, "#   B = ( <woffset> + <width>, <hoffset> + <height> )\n#\n" );
    fprintf( fp, "# where coordinates (0,0) and (1,1) of the original texture map correspond\n" );
    fprintf( fp, "# to coordinates A and B, respectively, in the texture atlas.\n" );
    fprintf( fp, "# If the atlas is a volume texture then <depth offset> is the w-coordinate\n" );
    fprintf( fp, "# to use the access the appropriate slice in the volume atlas.\n" );
    fprintf( fp, "\n" );

    // go through each texture and convert coordinates and write out the data
    TNewFormatMap::const_iterator           fmIter;
    TTexture2DPtrVector::const_iterator     texIter;
    for (fmIter = formatMap.begin(); fmIter != formatMap.end(); ++fmIter)
        for (texIter = (fmIter->second).begin(); texIter != (fmIter->second).end(); ++texIter)  
            (*texIter)->WriteTAILine(options, fp);

    fclose( fp );
    return true;
}

