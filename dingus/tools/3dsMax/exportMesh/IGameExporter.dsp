# Microsoft Developer Studio Project File - Name="IGameExporter" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=IGameExporter - Win32 R5 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IGameExporter.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IGameExporter.mak" CFG="IGameExporter - Win32 R5 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IGameExporter - Win32 R5 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IGameExporter - Win32 R5 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IGameExporter - Win32 R6 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IGameExporter - Win32 R6 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IGameExporter - Win32 R5 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "IGameExporter___Win32_R5_Release"
# PROP BASE Intermediate_Dir "IGameExporter___Win32_R5_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build/release5"
# PROP Intermediate_Dir "build/release5"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /O2 /I "f:\Dev\magma42\maxsdk\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /LD /c
# ADD CPP /nologo /G6 /MD /W3 /O2 /I "../IGame/3dsmax5/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /LD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib msxml2.lib /nologo /base:"0x1fd90000" /subsystem:windows /dll /machine:I386 /out:"F:\3dsmax42\plugins\IGameExporter.dle" /libpath:"F:\Dev\magma42\maxsdk\lib" /release
# ADD LINK32 ..\IGame\3dsmax5\IGame.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib d3d9.lib d3dx9.lib /nologo /base:"0x1fd90000" /subsystem:windows /dll /machine:I386 /out:"DingusMeshExport.dle" /libpath:"F:\Dev\magma42\maxsdk\lib" /release

!ELSEIF  "$(CFG)" == "IGameExporter - Win32 R5 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "IGameExporter___Win32_R5_Hybrid"
# PROP BASE Intermediate_Dir "IGameExporter___Win32_R5_Hybrid"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build/hybrid5"
# PROP Intermediate_Dir "build/hybrid5"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /LD /c
# ADD CPP /nologo /G6 /MD /W3 /Zi /Od /I "../IGame/3dsmax5/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /LD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\IGame\3dsmaxr5\lib\IGame.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib d3d9.lib d3dx9.lib /nologo /base:"0x1fd90000" /subsystem:windows /dll /machine:I386 /out:"IMMeshExport.dle" /libpath:"F:\Dev\magma42\maxsdk\lib" /release
# ADD LINK32 ..\IGame\3dsmax5\IGame.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib d3d9.lib d3dx9.lib /nologo /base:"0x1fd90000" /subsystem:windows /dll /debug /machine:I386 /out:"DingusMeshExport.dle" /libpath:"F:\Dev\magma42\maxsdk\lib" /release

!ELSEIF  "$(CFG)" == "IGameExporter - Win32 R6 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "IGameExporter___Win32_R6_Release"
# PROP BASE Intermediate_Dir "IGameExporter___Win32_R6_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build/release6"
# PROP Intermediate_Dir "build/release6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /O2 /I "../IGame/3dsmax5/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /LD /c
# ADD CPP /nologo /G6 /MD /W3 /O2 /I "../IGame/3dsmax6/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /LD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\IGame\3dsmax5\IGame.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib d3d9.lib d3dx9.lib /nologo /base:"0x1fd90000" /subsystem:windows /dll /machine:I386 /out:"IMMeshExport.dle" /libpath:"F:\Dev\magma42\maxsdk\lib" /release
# ADD LINK32 ..\IGame\3dsmax6\IGame.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib d3d9.lib d3dx9.lib /nologo /base:"0x1fd90000" /subsystem:windows /dll /machine:I386 /out:"DingusMeshExport.dle" /libpath:"F:\Dev\magma42\maxsdk\lib" /release

!ELSEIF  "$(CFG)" == "IGameExporter - Win32 R6 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "IGameExporter___Win32_R6_Hybrid"
# PROP BASE Intermediate_Dir "IGameExporter___Win32_R6_Hybrid"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build/hybrid6"
# PROP Intermediate_Dir "build/hybrid6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /Zi /Od /I "../IGame/3dsmax5/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /LD /c
# ADD CPP /nologo /G6 /MD /W3 /Zi /Od /I "../IGame/3dsmax6/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /LD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\IGame\3dsmax5\IGame.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib d3d9.lib d3dx9.lib /nologo /base:"0x1fd90000" /subsystem:windows /dll /debug /machine:I386 /out:"IMMeshExport.dle" /libpath:"F:\Dev\magma42\maxsdk\lib" /release
# ADD LINK32 ..\IGame\3dsmax6\IGame.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib paramblk2.lib d3d9.lib d3dx9dt.lib /nologo /base:"0x1fd90000" /subsystem:windows /dll /debug /machine:I386 /out:"DingusMeshExport.dle" /libpath:"F:\Dev\magma42\maxsdk\lib" /release

!ENDIF 

# Begin Target

# Name "IGameExporter - Win32 R5 Release"
# Name "IGameExporter - Win32 R5 Hybrid"
# Name "IGameExporter - Win32 R6 Release"
# Name "IGameExporter - Win32 R6 Hybrid"
# Begin Group "src"

# PROP Default_Filter "cpp;c;h"
# Begin Source File

SOURCE=.\DllEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\IGameExporter.cpp
# ADD CPP /FAcs
# End Source File
# Begin Source File

SOURCE=.\IGameExporter.h
# End Source File
# Begin Source File

SOURCE=.\MeshProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshProcessor.h
# End Source File
# Begin Source File

SOURCE=.\NVMeshMender.cpp
# End Source File
# Begin Source File

SOURCE=.\NVMeshMender.h
# End Source File
# Begin Source File

SOURCE=.\VertexFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\VertexFormat.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\changes.txt
# End Source File
# Begin Source File

SOURCE=.\IGameExporter.def
# End Source File
# Begin Source File

SOURCE=.\IGameExporter.rc
# End Source File
# Begin Source File

SOURCE=.\meshFileFormat.txt
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Target
# End Project
