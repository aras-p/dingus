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
#pragma warning(disable:4786)
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <stdio.h>
#include <d3dx9.h>
#include <algorithm>
#include <cassert>

#include "TATypes.h"
#include "CmdLineOptions.h"
#include "TextureObject.h"
#include "AtlasContainer.h"


void PrintVersion()
{
	const char* APP_VERSION = "2.0 fork 20050627";
	fprintf( stderr, "Atlas Creation Tool version %s\n", APP_VERSION );
}


// forwards
bool initialize();
bool createAtlases( const CmdLineOptionCollection& options );
void shutdown();

bool createTAIFile( const CmdLineOptionCollection& options, const TNewFormatMap& formatMap );


// --------------------------------------------------------------------------
//  application's entry point

int main( int argc, char **argv )
{
    PrintVersion();

    CmdLineOptionCollection options(argc, argv);
    if (! options.IsValid())
        exit (-1);

    //InitCommonControls();
	if( !initialize() )
		return 0;

	bool ok = createAtlases( options );
	shutdown();

    return ok ? 0 : -1;
}


// --------------------------------------------------------------------------
//  application


IDirect3D9*			gD3D;
IDirect3DDevice9*	gD3DDevice;


const char* d3dInit()
{
	gD3D = Direct3DCreate9( D3D_SDK_VERSION );
	if( !gD3D )
		return "Failed to create D3D";

	D3DDISPLAYMODE mode;
	if( FAILED( gD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &mode ) ) )
		return "Failed to get display mode";

	D3DPRESENT_PARAMETERS params;
	ZeroMemory( &params, sizeof(params) );
	params.BackBufferWidth = 400;
	params.BackBufferHeight = 300;
	params.BackBufferFormat = mode.Format;
	params.BackBufferCount = 1;
	params.Windowed = TRUE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;

	if( FAILED( gD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(),
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &params, &gD3DDevice ) ) )
		return "Failed to create D3D device";
	assert( gD3DDevice );

	return NULL;
}

void d3dShutdown()
{
	if( gD3DDevice ) {
		gD3DDevice->Release();
		gD3DDevice = NULL;
	}
	if( gD3D ) {
		gD3D->Release();
		gD3D = NULL;
	}
}


bool initialize()
{
	const char* msg = d3dInit();
	if( msg ) {
		fprintf( stderr, "ERROR: %s\n", msg );
		return false;
	}
	return true;
}

void shutdown()
{
	d3dShutdown();
}


// --------------------------------------------------------------------------

/**
 *  Creates atlases for the given textures.
 *  All options/info is stored in the options parameter.
 *  Returns false if errors occur.
 */
bool createAtlases( const CmdLineOptionCollection& options )
{
    // Create an array of texture objects and load the filenames/textures into that array.
    int const   kNumTextures = options.GetNumFilenames();
    char const *pFilename = NULL;
    int         i;

    Texture2D * pSourceTex = new Texture2D[kNumTextures];

    for (i = 0; i < kNumTextures; ++i)
    {
        options.GetFilename(i, &pFilename);
        pSourceTex[i].Init( gD3DDevice, pFilename );
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
    bool const kSuccess = createTAIFile( options, formatMap );

    // free all memory: clear all vectors of everything
    delete [] pSourceTex;
    return kSuccess;
}

bool createTAIFile( const CmdLineOptionCollection& options, const TNewFormatMap& formatMap )
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

