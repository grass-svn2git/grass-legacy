set ICON=%OSGEO4W_ROOT%\apps\grass\grass-@VERSION@\etc\gui\icons\grass.ico
set BATCH=%OSGEO4W_ROOT%\bin\@GRASS_EXECUTABLE@.bat
textreplace -std -t "%OSGEO4W_ROOT%"\bin\@GRASS_EXECUTABLE@.bat
textreplace -std -t "%OSGEO4W_ROOT%"\bin\@GRASS_EXECUTABLE@
textreplace -std -t "%OSGEO4W_ROOT%"\apps\grass\grass-@VERSION@\etc\fontcap

mkdir "%OSGEO4W_STARTMENU%\GRASS GIS @VERSION@" 
xxmklink "%OSGEO4W_STARTMENU%\GRASS GIS @VERSION@\GRASS GUI.lnk" "%BATCH%" "-wx" \ "Launch GRASS GIS @VERSION@ with wxGUI" 1 "%ICON%" 
xxmklink "%OSGEO4W_STARTMENU%\GRASS GIS @VERSION@\GRASS Old TclTk GUI.lnk" "%BATCH%" "-tcltk" \ "Launch GRASS GIS @VERSION@ with the old TclTk GUI" 1 "%ICON%" 
xxmklink "%OSGEO4W_STARTMENU%\GRASS GIS @VERSION@\GRASS Command Line.lnk" "%BATCH%" "-text" \ "Launch GRASS GIS @VERSION@ in text mode" 1 "%ICON%" 

xxmklink "%ALLUSERSPROFILE%\Desktop\GRASS GIS @VERSION@.lnk" "%BATCH%" "-wx" \ "Launch GRASS GIS @VERSION@ with wxGUI" 1 "%ICON%" 

del "%OSGEO4W_ROOT%"\bin\@GRASS_EXECUTABLE@.bat.tmpl
del "%OSGEO4W_ROOT%"\bin\@GRASS_EXECUTABLE@.tmpl
