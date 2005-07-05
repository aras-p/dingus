#ifndef __D3D_H
#define __D3D_H

// windows
#define _WIN32_WINNT 0x0500
#include <windows.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <boost/intrusive_ptr.hpp>

#include <stdexcept>

namespace boost {
inline void intrusive_ptr_add_ref( IUnknown* p ) { p->AddRef(); }
inline void intrusive_ptr_release( IUnknown* p ) { p->Release(); }
}

typedef boost::intrusive_ptr<IDirect3D9>		TD3DPtr;
typedef boost::intrusive_ptr<IDirect3DDevice9>	TD3DDevicePtr;


extern TD3DPtr			gD3D;
extern TD3DDevicePtr	gD3DDevice;


void initD3D();
void closeD3D();




#endif
