# Microsoft Developer Studio Project File - Name="vtss" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=vtss - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vtss.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vtss.mak" CFG="vtss - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vtss - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "vtss - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vtss - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\bin\release"
# PROP BASE Intermediate_Dir "..\..\obj\release\vtss"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\bin\release"
# PROP Intermediate_Dir "..\..\obj\release\vtss"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../include" /I "../../lib/missing" /I "../../" /I "." /D "NDEBUG" /D "WIN32" /D "BOARD_GROCX_REF" /D BOARD_NAME="grox_ref" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "vtss - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\bin\debug"
# PROP BASE Intermediate_Dir "..\..\obj\debug\vtss"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\bin\debug"
# PROP Intermediate_Dir "..\..\obj\debug\vtss"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../lib/missing" /I "../../" /I "." /D "_DEBUG" /D "WIN32" /D "BOARD_GROCX_REF" /D BOARD_NAME="grox_ref" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "vtss - Win32 Release"
# Name "vtss - Win32 Debug"
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\lib\missing\autoconf.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\missing\linux\star_switch_api.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\missing\linux\vitgenio.h
# End Source File
# Begin Source File

SOURCE=..\..\include\vtss.h
# End Source File
# Begin Source File

SOURCE=..\..\include\vtss_acl.h
# End Source File
# Begin Source File

SOURCE=..\..\include\vtss_board.h
# End Source File
# Begin Source File

SOURCE=.\vtss_cil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\vtss_conf.h
# End Source File
# Begin Source File

SOURCE=..\..\include\vtss_cpu.h
# End Source File
# Begin Source File

SOURCE=.\vtss_grocx.h
# End Source File
# Begin Source File

SOURCE=.\vtss_heathrow.h
# End Source File
# Begin Source File

SOURCE=.\vtss_phy.h
# End Source File
# Begin Source File

SOURCE=.\vtss_priv.h
# End Source File
# Begin Source File

SOURCE=.\vtss_sparx_reg.h
# End Source File
# Begin Source File

SOURCE=.\vtss_switch.h
# End Source File
# Begin Source File

SOURCE=..\..\include\vtss_veriphy.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\vtss_acl.c

!IF  "$(CFG)" == "vtss - Win32 Release"

!ELSEIF  "$(CFG)" == "vtss - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vtss_board.c
# End Source File
# Begin Source File

SOURCE=.\vtss_core.c
# End Source File
# Begin Source File

SOURCE=.\vtss_cpu.c

!IF  "$(CFG)" == "vtss - Win32 Release"

!ELSEIF  "$(CFG)" == "vtss - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vtss_grocx.c
# End Source File
# Begin Source File

SOURCE=.\vtss_io.c
# End Source File
# Begin Source File

SOURCE=.\vtss_mactab.c
# End Source File
# Begin Source File

SOURCE=.\vtss_phy.c
# End Source File
# Begin Source File

SOURCE=.\vtss_pvlan.c
# End Source File
# Begin Source File

SOURCE=.\vtss_sparx.c
# End Source File
# Begin Source File

SOURCE=.\vtss_switch.c
# End Source File
# Begin Source File

SOURCE=.\vtss_veriphy.c

!IF  "$(CFG)" == "vtss - Win32 Release"

!ELSEIF  "$(CFG)" == "vtss - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\Kbuild
# End Source File
# Begin Source File

SOURCE=.\Kconfig
# End Source File
# End Target
# End Project
