@echo off
rem #########################################################################
rem
rem  MODULE:   	GRASS Initialization
rem  AUTHOR(S):	Paul Kelly
rem
rem  PURPOSE:  	The source file for this batch script is lib/init/init.bat.
rem             It sets up some environment variables, default GISRC file
rem             if necessary, etc. prior to starting GRASS proper.
rem             It is intended to be a native Windows replacement for Init.bat,
rem             but does not (yet) contain all the same functionality.
rem
rem             In particular also, GUI mode prints nothing to the terminal
rem             and does not expect or provide an interactive terminal
rem             running in addition to the GUI display.
rem 
rem  COPYRIGHT: (C) 2006 by the GRASS Development Team
rem
rem             This program is free software under the GNU General Public
rem   	    	License (>=v2). Read the file COPYING that comes with GRASS
rem   	    	for details.
rem
rem #########################################################################

set SAVEPATH=%PATH%
rem DON'T include scripts directory in PATH - .bat files in bin directory
rem are used to run scripts on Windows
if "%GRASS_ADDON_PATH%"=="" set PATH=%WINGISBASE%\bin;%WINGISBASE%\lib;%PATH%
if not "%GRASS_ADDON_PATH%"=="" set PATH=%WINGISBASE%\bin;%WINGISBASE%\lib;%GRASS_ADDON_PATH%;%PATH%

set GRASS_VERSION=GRASS_VERSION_NUMBER
if "%HOME%"=="" set HOME=%USERPROFILE%
set WINGISRC=%HOME%\.grassrc6
rem Make sure %GISRC% is set so g.dirseps will work
rem (not actually used)
set GISRC=junk

rem Generate GISBASE by converting dirsep characters from \ to /
FOR /F "usebackq delims==" %%i IN (`g.dirseps -g "%WINGISBASE%"`) DO @set GISBASE=%%i

set GRASS_PAGER=more
if "%GRASS_WISH%"=="" set GRASS_WISH=wish.exe
if "%GRASS_SH%"=="" set GRASS_SH=c:\msys\1.0\bin\sh.exe
rem Should do something with "assoc .html" and ftype here but would require
rem a new g.manual.bat too so leaving it like this for now...
if "%GRASS_HTML_BROWSER%"=="" set GRASS_HTML_BROWSER=%SYSTEMDRIVE%/PROGRA~1/INTERN~1/IEXPLORE.EXE
if "%GRASS_PROJSHARE%"=="" set GRASS_PROJSHARE=CONFIG_PROJSHARE

rem Add python scripts to the PATHEXT variable
set PATHEXT=%PATHEXT%;.PY

if "%1" == "-version" goto displaylicence
if "%1" == "-v" goto displaylicence

if "%1" == "-text" goto settextmode
:aftertextcheck

if "%1" == "-tcltk" goto setguimode
if "%1" == "-wxpython" goto setwxmode
if "%1" == "-wx" goto setwxmode
if "%1" == "-gui" goto setguimode

:afterguicheck

if exist "%WINGISRC%" (
   set HAVE_GISRC=true
   goto aftercreategisrc
)

set HAVE_GISRC=false
rem Create an initial GISRC file based on current directory
"%WINGISBASE%\etc\echo" "GISDBASE: %USERPROFILE%" | g.dirseps -g > "%WINGISRC%"
"%WINGISBASE%\etc\echo" "LOCATION_NAME: <UNKNOWN>" >> "%WINGISRC%"
"%WINGISBASE%\etc\echo" "MAPSET: <UNKNOWN>" >> "%WINGISRC%"

:aftercreategisrc

rem Now set the real GISRC
FOR /F "usebackq delims==" %%i IN (`g.dirseps -g "%WINGISRC%"`) DO @set GISRC=%%i

rem Set GRASS_GUI

if "%GRASS_GUI%" == "" (
  FOR /F "usebackq delims==" %%i IN (`g.gisenv "get=GRASS_GUI"`) DO @set GRASS_GUI=%%i
) else (
  g.gisenv "set=GRASS_GUI=%GRASS_GUI%"
)

rem Set tcltk as default if not specified elsewhere
if "%GRASS_GUI%"=="" set GRASS_GUI=tcltk

rem Clean out old .tmp files from the mapset
if "%HAVE_GISRC%"=="true" (
  "%WINGISBASE%\etc\clean_temp" > NUL:
)
set HAVE_GISRC=

if "%GRASS_GUI%"=="text" goto text
if "%GRASS_GUI%"=="wxpython" goto wxpython

if not "%GRASS_WISH%"=="" (
	"%GRASS_WISH%" "%WINGISBASE%\etc\gis_set.tcl"
) else (
	"%WINGISBASE%\etc\gis_set.tcl"
)

rem This doesn't seem to work; don't understand return codes from gis_set.tcl PK
rem if return ok, gis.m start:
if %errorlevel% == 2 goto exitinit

rem Does line 42 above mean that GRASS_WISH will always be set?
if not "%GRASS_WISH%"=="" (
	start /b "GRASS Tcl/Tk" "%GRASS_WISH%" "%WINGISBASE%\etc\gm\gm.tcl"
) else (
	start /b "GRASS Tcl/Tk" "%WINGISBASE%\etc\gm\gm.tcl"
)

rem Will redirecting output to NUL hide legitamite error messages, harming debugging?
"%WINGISBASE%\etc\clean_temp" > NUL:

goto exitinit

:wxpython

set PYTHONPATH=%PYTHONPATH%;%WINGISBASE%\etc\python;%WINGISBASE%\etc\wxpython

python "%GISBASE%/etc/wxpython/gis_set.py"
if %errorlevel% == 2 goto exitinit
python "%GISBASE%/etc/wxpython/wxgui.py"

goto exitinit

:text

"%WINGISBASE%\etc\set_data"

if %errorlevel% == 1 goto exitinit

rem Get LOCATION_NAME to use in prompt
FOR /F "usebackq delims==" %%i IN (`g.gisenv "get=LOCATION_NAME"`) DO @set LOCATION_NAME=%%i

type "%WINGISBASE%\etc\welcome"

"%WINGISBASE%\etc\echo" ""
"%WINGISBASE%\etc\echo" "GRASS homepage:                          http://grass.osgeo.org/"
"%WINGISBASE%\etc\echo" "This version running thru:               Windows Command Shell (cmd.exe)"
"%WINGISBASE%\etc\echo" "When ready to quit enter:                exit"
"%WINGISBASE%\etc\echo" "Help is available with the command:      g.manual -i"
"%WINGISBASE%\etc\echo" "See the licence terms with:              g.version -c"
"%WINGISBASE%\etc\echo" ""

prompt GRASS %GRASS_VERSION% $C%LOCATION_NAME%$F$G 

cmd.exe

prompt
goto exitinit

:displaylicence

type "%WINGISBASE%\etc\license"
goto exitinit

:settextmode

set GRASS_GUI=text
shift

goto aftertextcheck

:setguimode
set GRASS_GUI=tcltk
shift

goto afterguicheck

:setwxmode
set GRASS_GUI=wxpython
shift

goto afterguicheck

:exitinit

rem Clean out old .tmp files from the mapset
"%WINGISBASE%\etc\clean_temp" > NUL:

set PATH=%SAVEPATH%
set SAVEPATH=
exit /b
