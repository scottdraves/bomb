# Microsoft Developer Studio Project File - Name="proj" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=proj - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "proj.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "proj.mak" CFG="proj - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "proj - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "proj - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "proj - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib dxguid.lib dxerr8.lib ddraw.lib dsound.lib /nologo /subsystem:windows /machine:I386 /out:"Release/bomb.exe"

!ELSEIF  "$(CFG)" == "proj - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\src" /I "..\proj" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dsound.lib dxguid.lib dxerr8.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/bomb.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "proj - Win32 Release"
# Name "proj - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\acidlife.c
# End Source File
# Begin Source File

SOURCE=..\src\anneal.c
# End Source File
# Begin Source File

SOURCE=..\src\bomb.c
# End Source File
# Begin Source File

SOURCE=..\src\brain.c
# End Source File
# Begin Source File

SOURCE=..\src\cmap.c
# End Source File
# Begin Source File

SOURCE=.\ddutil.cpp
# End Source File
# Begin Source File

SOURCE=.\dxutil.cpp
# End Source File
# Begin Source File

SOURCE=..\src\fast.c
# End Source File
# Begin Source File

SOURCE=..\src\fuse.c
# End Source File
# Begin Source File

SOURCE=..\src\gif.c
# End Source File
# Begin Source File

SOURCE=..\src\image.c
# End Source File
# Begin Source File

SOURCE=..\src\image_rule.c
# End Source File
# Begin Source File

SOURCE=..\src\image_seq.c
# End Source File
# Begin Source File

SOURCE=..\src\libifs.c
# End Source File
# Begin Source File

SOURCE=..\src\match.c
# End Source File
# Begin Source File

SOURCE=..\src\quad.c
# End Source File
# Begin Source File

SOURCE=..\src\rd.c
# End Source File
# Begin Source File

SOURCE=..\src\rotor.c
# End Source File
# Begin Source File

SOURCE=..\src\rug.c
# End Source File
# Begin Source File

SOURCE=..\src\rug_multi.c
# End Source File
# Begin Source File

SOURCE=..\src\rug_rug.c
# End Source File
# Begin Source File

SOURCE=..\src\shade.c
# End Source File
# Begin Source File

SOURCE=..\src\slip.c
# End Source File
# Begin Source File

SOURCE=..\src\sound.c
# End Source File
# Begin Source File

SOURCE=..\src\static.c
# End Source File
# Begin Source File

SOURCE=..\src\wave.c
# End Source File
# Begin Source File

SOURCE=.\win32.cpp
# End Source File
# Begin Source File

SOURCE=..\src\xshm.c
# End Source File
# Begin Source File

SOURCE=..\src\zio.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\bomb.h
# End Source File
# Begin Source File

SOURCE=..\src\cmap.h
# End Source File
# Begin Source File

SOURCE=.\ddutil.h
# End Source File
# Begin Source File

SOURCE=..\src\defs.h
# End Source File
# Begin Source File

SOURCE=.\dxutil.h
# End Source File
# Begin Source File

SOURCE=..\src\gif.h
# End Source File
# Begin Source File

SOURCE=..\src\image.h
# End Source File
# Begin Source File

SOURCE=..\src\image_db.h
# End Source File
# Begin Source File

SOURCE=..\src\libifs.h
# End Source File
# Begin Source File

SOURCE=..\src\match.h
# End Source File
# Begin Source File

SOURCE=..\src\sound.h
# End Source File
# Begin Source File

SOURCE=..\src\utils.h
# End Source File
# Begin Source File

SOURCE=..\src\vroot.h
# End Source File
# Begin Source File

SOURCE="..\src\xli-image.h"
# End Source File
# Begin Source File

SOURCE=..\src\xshm.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bomb.ico
# End Source File
# Begin Source File

SOURCE=.\bomb.rc
# End Source File
# End Group
# End Target
# End Project
