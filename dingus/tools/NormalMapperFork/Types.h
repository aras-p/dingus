//========================================================================================================================================================
// Types.h -- Variable prototypes
//========================================================================================================================================================
// $File: //depot/3darg/Demos/Sushi/RunTime/Types.h $ $Revision: #13 $ $Author: gosselin $
//========================================================================================================================================================
// (C) 2002 ATI Research, Inc., All rights reserved.
//========================================================================================================================================================

#ifndef __Types_h
#define __Types_h

static char sgCopyrightString[] = "\r\n\r\n(C) 2002 ATI Research, Inc.\r\n\r\n";

// DEFINES ===============================================================================================================================================
#ifdef TRUE
 #undef TRUE
#endif
#define TRUE  1

#ifdef FALSE
 #undef FALSE
#endif
#define FALSE 0

#define for if(0) ; else for

// TYPEDEFS ==============================================================================================================================================
#ifndef ATI_MAC_OS

   #ifndef ATI_LINUX
   //=========//
   // Windows //
   //=========//
   //Signed
   typedef char bool8;
   typedef char char8;

   typedef char    int8;
   typedef short   int16;
   typedef int     int32;
   typedef __int64 int64;

   typedef float       float32;
   typedef double      float64;
   //typedef long double float80; //Windows treats this the same as a double

   //Unsigned
   typedef unsigned char    uint8;
   typedef unsigned short   uint16;
   typedef unsigned int     uint32;
   typedef unsigned __int64 uint64;
   
   //String
   typedef const char cstr;
   
   #else //ATI_LINUX
   
   #include <GL/gl.h>
     
    //Signed
   typedef char bool8;
   typedef char char8;

   typedef char    int8;
   typedef short   int16;
   typedef int int32;
   typedef long long int64;

   typedef float       float32;
   typedef double      float64;
   //typedef long double float80; //Windows treats this the same as a double

   //Unsigned
   typedef unsigned char    uint8;
   typedef unsigned short   uint16;
   typedef unsigned int     uint32;
   typedef unsigned long long uint64;
   
   //String
   typedef const char cstr;
   
   #endif //ATI_LINUX
   
#else
   //=====//
   // MAC //
   //=====//
   //Signed
   typedef char bool8;
   typedef char char8;

   typedef char  int8;
   typedef short int16;
   typedef long  int32;

   typedef float       float32;
   typedef double      float64;
   //typedef long double float80; //Not sure what the Mac does with this

   //Unsigned
   typedef unsigned char  uint8;
   typedef unsigned short uint16;
   typedef unsigned long  uint32;
   typedef unsigned long  LARGE_INTEGER;

   //String
   typedef const char cstr;
#endif



#endif
