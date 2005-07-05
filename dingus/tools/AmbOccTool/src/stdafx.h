#ifndef __DINGUS_STDAFX_H
#define __DINGUS_STDAFX_H

// precompiled headers stuff

#pragma warning(disable:4786)
#pragma warning(disable:4018)
#pragma warning(disable:4511) // copy constructor can't be generated
#pragma warning(disable:4512) // assignment operator can't be generated
#pragma warning(disable:4100) // unreferenced formal parameter

// synch unicode settings
#ifdef _UNICODE
#define UNICODE
#endif


// windows
#define _WIN32_WINNT 0x0500

#ifdef _WINDLL

#define VC_EXTRALEAN
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif

#else

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#endif


// STL
#include <list>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <string>
#include <stdexcept>

// C libs
#include <cassert>
#include <cmath>
#include <cstring>
#include <malloc.h>

// D3D
#include <d3d9.h>
#include <d3dx9.h>

// smart pointers
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>

#endif
