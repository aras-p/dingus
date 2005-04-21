# Microsoft Developer Studio Project File - Name="ode" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ode - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ode.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ode.mak" CFG="ode - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ode - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ode - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ode - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../build/ode/release"
# PROP Intermediate_Dir "../../../build/ode/release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "inc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x427 /d "NDEBUG"
# ADD RSC /l 0x427 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ode - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../build/ode/debug"
# PROP Intermediate_Dir "../../../build/ode/debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /O2 /I "inc" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x427 /d "_DEBUG"
# ADD RSC /l 0x427 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ode - Win32 Release"
# Name "ode - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter "c;cpp"
# Begin Group "opcode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\opcode\IceAxes.h
# End Source File
# Begin Source File

SOURCE=.\src\opcode\IceContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\opcode\IceContainer.h
# End Source File
# Begin Source File

SOURCE=.\src\opcode\IceFPU.h
# End Source File
# Begin Source File

SOURCE=.\src\opcode\IceMemoryMacros.h
# End Source File
# Begin Source File

SOURCE=.\src\opcode\IcePairs.h
# End Source File
# Begin Source File

SOURCE=.\src\opcode\IcePreprocessor.h
# End Source File
# Begin Source File

SOURCE=.\src\opcode\IceRevisitedRadix.cpp
# End Source File
# Begin Source File

SOURCE=.\src\opcode\IceRevisitedRadix.h
# End Source File
# Begin Source File

SOURCE=.\src\opcode\IceTypes.h
# End Source File
# Begin Source File

SOURCE=.\src\opcode\OPC_IceHook.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\array.cpp
# End Source File
# Begin Source File

SOURCE=.\src\array.h
# End Source File
# Begin Source File

SOURCE=.\src\collision_kernel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\collision_kernel.h
# End Source File
# Begin Source File

SOURCE=.\src\collision_quadtreespace.cpp
# End Source File
# Begin Source File

SOURCE=.\src\collision_sapspace.cpp
# End Source File
# Begin Source File

SOURCE=.\src\collision_space.cpp
# End Source File
# Begin Source File

SOURCE=.\src\collision_space_internal.h
# End Source File
# Begin Source File

SOURCE=.\src\collision_std.cpp
# End Source File
# Begin Source File

SOURCE=.\src\collision_std.h
# End Source File
# Begin Source File

SOURCE=.\src\collision_transform.cpp
# End Source File
# Begin Source File

SOURCE=.\src\collision_transform.h
# End Source File
# Begin Source File

SOURCE=.\src\collision_util.cpp
# End Source File
# Begin Source File

SOURCE=.\src\collision_util.h
# End Source File
# Begin Source File

SOURCE=.\src\error.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdot.c
# End Source File
# Begin Source File

SOURCE=.\src\fastldlt.c
# End Source File
# Begin Source File

SOURCE=.\src\fastlsolve.c
# End Source File
# Begin Source File

SOURCE=.\src\fastltsolve.c
# End Source File
# Begin Source File

SOURCE=.\src\joint.cpp
# End Source File
# Begin Source File

SOURCE=.\src\joint.h
# End Source File
# Begin Source File

SOURCE=.\src\lcp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lcp.h
# End Source File
# Begin Source File

SOURCE=.\src\mass.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mat.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mat.h
# End Source File
# Begin Source File

SOURCE=.\src\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\src\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\objects.h
# End Source File
# Begin Source File

SOURCE=.\src\obstack.cpp
# End Source File
# Begin Source File

SOURCE=.\src\obstack.h
# End Source File
# Begin Source File

SOURCE=.\src\ode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\odemath.cpp
# End Source File
# Begin Source File

SOURCE=.\src\quickstep.cpp
# End Source File
# Begin Source File

SOURCE=.\src\quickstep.h
# End Source File
# Begin Source File

SOURCE=.\src\rotation.cpp
# End Source File
# Begin Source File

SOURCE=.\src\step.cpp
# End Source File
# Begin Source File

SOURCE=.\src\step.h
# End Source File
# Begin Source File

SOURCE=.\src\stepfast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\util.cpp
# End Source File
# Begin Source File

SOURCE=.\src\util.h
# End Source File
# End Group
# Begin Group "inc"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\inc\ode\collision.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\collision_space.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\common.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\compatibility.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\contact.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\error.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\mass.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\matrix.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\memory.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\misc.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\objects.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\ode.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\odemath.h
# End Source File
# Begin Source File

SOURCE=.\inc\ode\rotation.h
# End Source File
# End Group
# End Target
# End Project
