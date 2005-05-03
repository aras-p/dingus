# Microsoft Developer Studio Project File - Name="dingus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=dingus - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dingus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dingus.mak" CFG="dingus - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dingus - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "dingus - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dingus - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build/release"
# PROP Intermediate_Dir "build/release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "lib" /I "lib/BaseClasses" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x427 /d "NDEBUG"
# ADD RSC /l 0x427 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "dingus - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "build/debug"
# PROP Intermediate_Dir "build/debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "lib" /I "lib/BaseClasses" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
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

# Name "dingus - Win32 Release"
# Name "dingus - Win32 Debug"
# Begin Group "dingus"

# PROP Default_Filter ""
# Begin Group "animator"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\animator\Animation.h
# End Source File
# Begin Source File

SOURCE=.\dingus\animator\AnimationBunch.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\animator\AnimationBunch.h
# End Source File
# Begin Source File

SOURCE=.\dingus\animator\AnimCurve.h
# End Source File
# Begin Source File

SOURCE=.\dingus\animator\AnimDataType.h
# End Source File
# Begin Source File

SOURCE=.\dingus\animator\AnimImmediateMixer.h
# End Source File
# Begin Source File

SOURCE=.\dingus\animator\AnimStream.h
# End Source File
# Begin Source File

SOURCE=.\dingus\animator\SampledAnimation.h
# End Source File
# End Group
# Begin Group "audio"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\audio\AudioContext.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\audio\AudioContext.h
# End Source File
# Begin Source File

SOURCE=.\dingus\audio\AudioListener.h
# End Source File
# Begin Source File

SOURCE=.\dingus\audio\Sound.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\audio\Sound.h
# End Source File
# Begin Source File

SOURCE=.\dingus\audio\SoundResource.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\audio\SoundResource.h
# End Source File
# Begin Source File

SOURCE=.\dingus\audio\SoundWaveFmt.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\audio\SoundWaveFmt.h
# End Source File
# End Group
# Begin Group "collision"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\collision\Collidable.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\collision\Collidable.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\collision\CollisionContext.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\collision\CollisionContext.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\collision\CollisionListener.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\collision\CollisionMesh.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\collision\CollisionMesh.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "console"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\console\Console.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\console\Console.h
# End Source File
# Begin Source File

SOURCE=.\dingus\console\D3DConsoleRenderingContext.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\console\D3DConsoleRenderingContext.h
# End Source File
# Begin Source File

SOURCE=.\dingus\console\FileConsoleRenderingContext.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\console\FileConsoleRenderingContext.h
# End Source File
# Begin Source File

SOURCE=.\dingus\console\W32StdConsoleRenderingContext.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\console\W32StdConsoleRenderingContext.h
# End Source File
# Begin Source File

SOURCE=.\dingus\console\WDebugConsoleRenderingContext.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\console\WDebugConsoleRenderingContext.h
# End Source File
# End Group
# Begin Group "dxutils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\dxutils\D3DApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\D3DApplication.h
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\D3DEnumeration.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\D3DEnumeration.h
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\D3DFont.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\D3DFont.h
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\D3DSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\D3DSettings.h
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\D3Dutil.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\D3Dutil.h
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\DXConfigDB.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\DXConfigDB.h
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\DXConfigManager.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\DXConfigManager.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\DXUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\DXUtil.h
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\DXUtils.rc
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\resource.h
# End Source File
# End Group
# Begin Group "gfx"

# PROP Default_Filter ""
# Begin Group "geometry"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\gfx\geometry\BufferChunk.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\geometry\ChunkSource.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\geometry\DynamicIBManager.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\geometry\DynamicIBManager.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\geometry\DynamicVBManager.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\geometry\DynamicVBManager.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\geometry\ManagedBuffer.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\geometry\VBChunkHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\geometry\VBChunkHelper.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\geometry\VBChunkLock.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\geometry\VBManagerSource.h
# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\gfx\gui\Gui.cpp

!IF  "$(CFG)" == "dingus - Win32 Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "dingus - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\gui\Gui.h
# End Source File
# End Group
# Begin Group "particles"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\gfx\particles\Particle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\ParticleEmitter.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\ParticleEmitters.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\ParticlePhysics.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\ParticlePhysicses.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\ParticleRenderer.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\ParticleRenderHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\ParticleRenderHelper.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\Particles.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\ParticleSystem.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\TextureHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\TextureHelper.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\VBParticleRenderer.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\particles\VBParticleRenderer_impl.h
# End Source File
# End Group
# Begin Group "skeleton"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\gfx\skeleton\CharacterAnimator.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\skeleton\CharacterAnimator.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\skeleton\SkeletonInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\skeleton\SkeletonInfo.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\skeleton\SkinMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\skeleton\SkinMesh.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\skeleton\SkinUpdater.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\skeleton\SkinUpdater.h
# End Source File
# End Group
# Begin Group "video"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\gfx\video\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\video\Mutex.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\video\VideoPlayer.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\video\VideoPlayer.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\dingus\gfx\DebugRenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\DebugRenderer.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\GfxUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\GfxUtils.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\Mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\Mesh.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\ModelDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\ModelDesc.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\ModelDescSerializer.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\ModelDescSerializer.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\RenderableBillboards.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\RenderableBillboards.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\RenderableOrderedBillboards.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\RenderableOrderedBillboards.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\VertexFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\VertexFormat.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\Vertices.h
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\VSKeyframeMeshRenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\gfx\VSKeyframeMeshRenderer.h
# End Source File
# End Group
# Begin Group "input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\input\DIKeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\input\DIKeyboard.h
# End Source File
# Begin Source File

SOURCE=.\dingus\input\DIMouse.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\input\DIMouse.h
# End Source File
# Begin Source File

SOURCE=.\dingus\input\InputContext.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\input\InputContext.h
# End Source File
# Begin Source File

SOURCE=.\dingus\input\InputDevice.h
# End Source File
# Begin Source File

SOURCE=.\dingus\input\InputEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\input\InputEvent.h
# End Source File
# Begin Source File

SOURCE=.\dingus\input\InputListener.h
# End Source File
# End Group
# Begin Group "kernel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\kernel\D3DDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\kernel\D3DDevice.h
# End Source File
# Begin Source File

SOURCE=.\dingus\kernel\D3DDeviceCaps.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\kernel\D3DDeviceCaps.h
# End Source File
# Begin Source File

SOURCE=.\dingus\kernel\EffectStateManager.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\kernel\EffectStateManager.h
# End Source File
# Begin Source File

SOURCE=.\dingus\kernel\Proxies.h
# End Source File
# Begin Source File

SOURCE=.\dingus\kernel\ProxyEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\kernel\ProxyEffect.h
# End Source File
# Begin Source File

SOURCE=.\dingus\kernel\RenderStats.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\kernel\RenderStats.h
# End Source File
# End Group
# Begin Group "lua"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\lua\LuaHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\lua\LuaHelper.h
# End Source File
# Begin Source File

SOURCE=.\dingus\lua\LuaIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\lua\LuaIterator.h
# End Source File
# Begin Source File

SOURCE=.\dingus\lua\LuaSingleton.h
# End Source File
# Begin Source File

SOURCE=.\dingus\lua\LuaValue.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\lua\LuaValue.h
# End Source File
# Begin Source File

SOURCE=.\dingus\lua\LuaWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\lua\LuaWrapper.h
# End Source File
# End Group
# Begin Group "math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\math\AABox.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Angle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Constants.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\FPU.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Interpolation.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Line3.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\MathUtils.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Matrix4x4.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\ODEMarshal.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Plane.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Primitives.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Quaternion.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Transform.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Transform.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Triangle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Triangle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\TypeTraits.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Vector2.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Vector3.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Vector3.h
# End Source File
# Begin Source File

SOURCE=.\dingus\math\Vector4.h
# End Source File
# End Group
# Begin Group "physics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\physics\PhysBody.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\physics\PhysBody.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\physics\PhysContext.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\physics\PhysContext.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\physics\PhysJoint.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\physics\PhysJointGroup.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "renderer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\renderer\EffectParams.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\EffectParams.h
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\EffectParamsNotifier.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\EffectParamsNotifier.h
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\Renderable.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\Renderable.h
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\RenderableBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\RenderableBuffer.h
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\RenderableMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\RenderableMesh.h
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\RenderableSkin.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\RenderableSkin.h
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\RenderCamera.cpp

!IF  "$(CFG)" == "dingus - Win32 Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "dingus - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\RenderCamera.h
# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\RenderContext.cpp

!IF  "$(CFG)" == "dingus - Win32 Release"

# ADD CPP /O2 /Oy /Ob1

!ELSEIF  "$(CFG)" == "dingus - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dingus\renderer\RenderContext.h
# End Source File
# End Group
# Begin Group "resource"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\resource\AnimationBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\AnimationBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\CollisionMeshBundle.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\CollisionMeshBundle.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\CubeTextureBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\CubeTextureBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\CubeTextureCreator.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\CubeTextureCreator.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\DeviceResource.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\EffectBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\EffectBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\FontBundle.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\FontBundle.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\IndexBufferBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\IndexBufferBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\IndexBufferFillers.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\MeshBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\MeshBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\MeshCreator.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\MeshSerializer.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\MeshSerializer.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\ModelDescBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\ModelDescBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\ReloadableBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\ResourceBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\ResourceBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\ResourceId.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedCubeTextureBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedCubeTextureBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedMeshBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedMeshBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedResourceBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedSurfaceBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedSurfaceBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedTextureBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedTextureBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedVolumeBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SharedVolumeBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SkeletonInfoBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SkeletonInfoBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SkinMeshBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SkinMeshBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SoundBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SoundBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\StorageResourceBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SurfaceCreator.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\SurfaceCreator.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\TextureBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\TextureBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\TextureCreator.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\TextureCreator.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\VertexDeclBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\VertexDeclBundle.h
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\VolumeCreator.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\resource\VolumeCreator.h
# End Source File
# End Group
# Begin Group "utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\utils\AssertHelper.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Config.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Config.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\CpuTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\CpuTimer.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Errors.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Errors.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\fastvector.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\FileHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\FileHelper.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\FixedRateProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\FixedRateProcess.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\InputFilter.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\MemoryPool.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\MemoryPool.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\MemUtil.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\mmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\mmgr.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\nommgr.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Notifier.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Perlin.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Perlin.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Pool.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Random.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Random.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\RefCounted.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Registry.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\ringdeque.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Singleton.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\STLUtils.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\StringHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\StringHelper.h
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Timer.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\utils\Timer.h
# End Source File
# End Group
# Begin Group "visibility"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\visibility\VisibilityContext.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\visibility\VisibilityContext.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\visibility\VisibilityDB.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\visibility\VisibilityDB.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\visibility\VisibilityLeaf.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dingus\visibility\VisibilityLeaf.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "app"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dingus\app\DingusApplication.h
# End Source File
# Begin Source File

SOURCE=.\dingus\app\DingusSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\dingus\app\DingusSystem.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\dingus\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\dingus\stdafx.h
# End Source File
# End Group
# Begin Group "doc"

# PROP Default_Filter ""
# Begin Group "fileformats"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\doc\fileformats\ModelFileFormat.txt
# End Source File
# End Group
# Begin Source File

SOURCE=.\doc\Effects.txt
# End Source File
# End Group
# Begin Source File

SOURCE=.\changes.txt
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\DirectX.ico
# End Source File
# Begin Source File

SOURCE=.\dingus\dxutils\logo.bmp
# End Source File
# End Target
# End Project
