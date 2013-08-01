rem Environmental variables for GRASS stand-alone installer

set GRASS_WISH=%GISBASE%\extrabin\wish.exe
set GRASS_SH=%GISBASE%\msys\bin\sh.exe
set GRASS_PAGER=more

set GRASS_HTML_BROWSER=explorer

set GRASS_PYTHON=%GISBASE%\extrabin\python.exe
set PYTHONHOME=%GISBASE%\Python27

set GRASS_PROJSHARE=%GISBASE%\share\proj
set GDAL_DATA=%GISBASE%\share\gdal
set GEOTIFF_CSV=%GISBASE%\share\epsg_csv

set PATH=%GISBASE%\msys\bin;%PATH%
set PATH=%GISBASE%\extrabin;%GISBASE%\extralib;%PATH%
set PATH=%GISBASE%\bin;%PATH%
