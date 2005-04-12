// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef D3D_UTIL_H__
#define D3D_UTIL_H__


namespace dingus {

/**
 *  Returns the string for the given D3DFORMAT.
 */
const TCHAR* convertD3DFormatToString( D3DFORMAT format );

/** Gives the D3D device a cursor with image and hotspot from hCursor. */
HRESULT setDeviceCursor( IDirect3DDevice9& device, HCURSOR hCursor );


}; // namespace dingus

#endif
