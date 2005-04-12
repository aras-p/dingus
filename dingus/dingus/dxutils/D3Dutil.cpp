// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "D3DUtil.h"
#include "DXUtil.h"



const TCHAR* dingus::convertD3DFormatToString( D3DFORMAT format )
{
	const TCHAR* pstr = NULL;
	switch( format ) {
	case D3DFMT_UNKNOWN:		 pstr = TEXT("Unknown"); break;
	case D3DFMT_R8G8B8: 		 pstr = TEXT("24bit (rgb8)"); break;
	case D3DFMT_A8R8G8B8:		 pstr = TEXT("32bit (argb8)"); break;
	case D3DFMT_X8R8G8B8:		 pstr = TEXT("32bit (xrgb8)"); break;
	case D3DFMT_R5G6B5: 		 pstr = TEXT("16bit (r5g6b5)"); break;
	case D3DFMT_X1R5G5B5:		 pstr = TEXT("16bit (x1rgb5)"); break;
	case D3DFMT_A1R5G5B5:		 pstr = TEXT("16bit (a1rgb5)"); break;
	case D3DFMT_A4R4G4B4:		 pstr = TEXT("16bit (argb4)"); break;
	case D3DFMT_R3G3B2: 		 pstr = TEXT("8bit (r3g3b2)"); break;
	case D3DFMT_A8: 			 pstr = TEXT("8bit (a8)"); break;
	case D3DFMT_A8R3G3B2:		 pstr = TEXT("16bit (a8r3g3b2)"); break;
	case D3DFMT_X4R4G4B4:		 pstr = TEXT("16bit (xrgb4)"); break;
	case D3DFMT_A2B10G10R10:	 pstr = TEXT("32bit (a2bgr10)"); break;
	case D3DFMT_A8B8G8R8:		 pstr = TEXT("32bit (abgr8)"); break;
	case D3DFMT_X8B8G8R8:		 pstr = TEXT("32bit (xbgr8)"); break;
	case D3DFMT_G16R16: 		 pstr = TEXT("G16R16"); break;
	case D3DFMT_A2R10G10B10:	 pstr = TEXT("32bit (a2rgb10)"); break;
	case D3DFMT_A16B16G16R16:	 pstr = TEXT("A16B16G16R16"); break;
	case D3DFMT_A8P8:			 pstr = TEXT("A8P8"); break;
	case D3DFMT_P8: 			 pstr = TEXT("P8"); break;
	case D3DFMT_L8: 			 pstr = TEXT("L8"); break;
	case D3DFMT_A8L8:			 pstr = TEXT("A8L8"); break;
	case D3DFMT_A4L4:			 pstr = TEXT("A4L4"); break;
	case D3DFMT_V8U8:			 pstr = TEXT("V8U8"); break;
	case D3DFMT_L6V5U5: 		 pstr = TEXT("L6V5U5"); break;
	case D3DFMT_X8L8V8U8:		 pstr = TEXT("X8L8V8U8"); break;
	case D3DFMT_Q8W8V8U8:		 pstr = TEXT("Q8W8V8U8"); break;
	case D3DFMT_V16U16: 		 pstr = TEXT("V16U16"); break;
	case D3DFMT_A2W10V10U10:	 pstr = TEXT("A2W10V10U10"); break;
	case D3DFMT_UYVY:			 pstr = TEXT("UYVY"); break;
	case D3DFMT_YUY2:			 pstr = TEXT("YUY2"); break;
	case D3DFMT_DXT1:			 pstr = TEXT("DXT1"); break;
	case D3DFMT_DXT2:			 pstr = TEXT("DXT2"); break;
	case D3DFMT_DXT3:			 pstr = TEXT("DXT3"); break;
	case D3DFMT_DXT4:			 pstr = TEXT("DXT4"); break;
	case D3DFMT_DXT5:			 pstr = TEXT("DXT5"); break;
	case D3DFMT_D16_LOCKABLE:	 pstr = TEXT("D16_LOCKABLE"); break;
	case D3DFMT_D32:			 pstr = TEXT("32 z"); break;
	case D3DFMT_D15S1:			 pstr = TEXT("15 z 1 stencil"); break;
	case D3DFMT_D24S8:			 pstr = TEXT("24 z 8 stencil"); break;
	case D3DFMT_D24X8:			 pstr = TEXT("24 z"); break;
	case D3DFMT_D24X4S4:		 pstr = TEXT("24 z 4 stencil"); break;
	case D3DFMT_D16:			 pstr = TEXT("16 z"); break;
	case D3DFMT_L16:			 pstr = TEXT("L16"); break;
	case D3DFMT_VERTEXDATA: 	 pstr = TEXT("VERTEXDATA"); break;
	case D3DFMT_INDEX16:		 pstr = TEXT("INDEX16"); break;
	case D3DFMT_INDEX32:		 pstr = TEXT("INDEX32"); break;
	case D3DFMT_Q16W16V16U16:	 pstr = TEXT("Q16W16V16U16"); break;
	case D3DFMT_MULTI2_ARGB8:	 pstr = TEXT("MULTI2_ARGB8"); break;
	case D3DFMT_R16F:			 pstr = TEXT("R16F"); break;
	case D3DFMT_G16R16F:		 pstr = TEXT("G16R16F"); break;
	case D3DFMT_A16B16G16R16F:	 pstr = TEXT("A16B16G16R16F"); break;
	case D3DFMT_R32F:			 pstr = TEXT("R32F"); break;
	case D3DFMT_G32R32F:		 pstr = TEXT("G32R32F"); break;
	case D3DFMT_A32B32G32R32F:	 pstr = TEXT("A32B32G32R32F"); break;
	case D3DFMT_CxV8U8: 		 pstr = TEXT("CxV8U8"); break;
	default:					 pstr = TEXT("Unknown format"); break;
	}
	return pstr;
}


HRESULT dingus::setDeviceCursor( IDirect3DDevice9& device, HCURSOR hCursor )
{
	HRESULT 	hr = E_FAIL;
	ICONINFO	iconinfo;
	bool		bwCursor;
	IDirect3DSurface9* cursorSurface = NULL;
	HDC 		hdcColor = NULL;
	HDC 		hdcMask = NULL;
	HDC 		hdcScreen = NULL;
	BITMAP		bm;
	DWORD		dwWidth;
	DWORD		dwHeightSrc;
	DWORD		dwHeightDest;
	COLORREF	crColor;
	COLORREF	crMask;
	int 		x;
	int 		y;
	BITMAPINFO	bmi;
	COLORREF*	pcrArrayColor = NULL;
	COLORREF*	pcrArrayMask = NULL;
	DWORD*		pBitmap;
	HGDIOBJ 	hgdiobjOld;
	
	ZeroMemory( &iconinfo, sizeof(iconinfo) );
	if( !GetIconInfo( hCursor, &iconinfo ) )
		goto _end;
	
	if( 0 == GetObject((HGDIOBJ)iconinfo.hbmMask, sizeof(BITMAP), (LPVOID)&bm) )
		goto _end;
	dwWidth = bm.bmWidth;
	dwHeightSrc = bm.bmHeight;
	
	if( iconinfo.hbmColor == NULL ) {
		bwCursor = TRUE;
		dwHeightDest = dwHeightSrc / 2;
	} else {
		bwCursor = FALSE;
		dwHeightDest = dwHeightSrc;
	}
	
	// Create a surface for the fullscreen cursor
	if( FAILED( hr = device.CreateOffscreenPlainSurface( dwWidth, dwHeightDest, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &cursorSurface, NULL ) ) ) {
		goto _end;
	}
	
	pcrArrayMask = new DWORD[dwWidth * dwHeightSrc];
	
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = dwWidth;
	bmi.bmiHeader.biHeight = dwHeightSrc;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	
	hdcScreen = GetDC( NULL );
	hdcMask = CreateCompatibleDC( hdcScreen );
	if( hdcMask == NULL ) {
		hr = E_FAIL;
		goto _end;
	}
	hgdiobjOld = SelectObject(hdcMask, iconinfo.hbmMask);
	GetDIBits( hdcMask, iconinfo.hbmMask, 0, dwHeightSrc, pcrArrayMask, &bmi, DIB_RGB_COLORS );
	SelectObject( hdcMask, hgdiobjOld );
	
	if( !bwCursor ) {
		pcrArrayColor = new DWORD[dwWidth * dwHeightDest];
		hdcColor = CreateCompatibleDC( hdcScreen );
		if( hdcColor == NULL ) {
			hr = E_FAIL;
			goto _end;
		}
		SelectObject( hdcColor, iconinfo.hbmColor );
		GetDIBits( hdcColor, iconinfo.hbmColor, 0, dwHeightDest, pcrArrayColor, &bmi, DIB_RGB_COLORS );
	}
	
	// Transfer cursor image into the surface
	D3DLOCKED_RECT lr;
	cursorSurface->LockRect( &lr, NULL, 0 );
	pBitmap = (DWORD*)lr.pBits;
	for( y = 0; y < dwHeightDest; y++ ) {
		for( x = 0; x < dwWidth; x++ ) {
			if( bwCursor ) {
				crColor = pcrArrayMask[dwWidth*(dwHeightDest-1-y) + x];
				crMask = pcrArrayMask[dwWidth*(dwHeightSrc-1-y) + x];
			} else {
				crColor = pcrArrayColor[dwWidth*(dwHeightDest-1-y) + x];
				crMask = pcrArrayMask[dwWidth*(dwHeightDest-1-y) + x];
			}
			if( crMask == 0 )
				pBitmap[dwWidth*y + x] = 0xff000000 | crColor;
			else
				pBitmap[dwWidth*y + x] = 0x00000000;
		}
	}
	cursorSurface->UnlockRect();
	
	// Set the device cursor
	if( FAILED( hr = device.SetCursorProperties( iconinfo.xHotspot, iconinfo.yHotspot, cursorSurface ) ) ) {
		goto _end;
	}
	
	hr = S_OK;
	
_end:
	if( iconinfo.hbmMask != NULL )
		DeleteObject( iconinfo.hbmMask );
	if( iconinfo.hbmColor != NULL )
		DeleteObject( iconinfo.hbmColor );
	if( hdcScreen != NULL )
		ReleaseDC( NULL, hdcScreen );
	if( hdcColor != NULL )
		DeleteDC( hdcColor );
	if( hdcMask != NULL )
		DeleteDC( hdcMask );
	safeDeleteArray( pcrArrayColor );
	safeDeleteArray( pcrArrayMask );
	safeRelease( cursorSurface );
	return hr;
};
