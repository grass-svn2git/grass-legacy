#!/bin/sh

#generates HTML man pages docs/html/index.html
# Markus Neteler, 2003, 2004, 2005

#exclude following list of modules from help index:
# escape it properly:
EXCLUDEHTML="v\.topo\.check\|i\.ask\|i\.find\|photo\.elev\|photo\.target"

############# nothing to configure below ############

#fetch the ARCH for store the files:
ARCH="`cat ../include/Make/Platform.make | grep '^ARCH'  | sed 's+ ++g' | cut -d'=' -f2`"
HTMLDIR="../dist.$ARCH/docs/html"
GRASSVERSION=`cat ../dist.$ARCH/etc/VERSIONNUMBER`


write_html_header()
{
# $1: filename
# $2: page title

echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">
<html>
<head>
 <title>$2</title>
 <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">
 <meta name=\"Author\" content=\"GRASS Development Team\">
</head>
<body bgcolor=\"#FFFFFF\">
<h2>GRASS GIS $GRASSVERSION Reference Manual</h2>

Geographic Resources Analysis Support System, commonly referred to as GRASS, 
is a Geographic Information System (GIS) used for geospatial data management
and analysis, image processing, graphics/maps production, spatial modeling,
and visualization. GRASS is currently used in academic and commercial settings
around the world, as well as by many governmental agencies and environmental
consulting companies.
<P>
This reference manual details the use of modules distributed with
Geographic Resources Analysis Support System (GRASS), an open source (GNU
GPL'ed), image processing and geographic information system (GIS).
<p>

<h3>Quick Introduction</h3>

<!-- the helptext.html file lives in lib/init/helptext.html -->
<a href="../start/helptext.html">How to start with GRASS</a>
<P>

" > $1
}

write_html_footer()
{
# $1: filename
# $2: help index url
echo "<hr>" >> $1
echo "<a href=\"$2\">Help Index</a> | <a href=\"full_index.html\">Full Index</a><br>" >> $1
echo "&copy; 2003-2005 <a href=\"http://grass.itc.it\">GRASS Development Team</a>" >> $1
echo "</body>" >> $1   
echo "</html>" >> $1
}

expand_module_class_name()
{
# $1: module class
 if [ "$1" = "d" ]  ; then 
    echo "display"  
  elif [ "$1" = "db" ] ; then 
    echo "database" 
  elif [ "$1" = "g" ]  ; then 
    echo "general" 
  elif [ "$1" = "i" ]  ; then 
    echo "imagery" 
  elif [ "$1" = "m" ]  ; then 
    echo "misc" 
  elif [ "$1" = "pg" ] ; then 
    echo "postGRASS" 
  elif [ "$1" = "ps" ] ; then 
    echo "postscript" 
  elif [ "$1" = "p" ]  ; then 
    echo "paint" 
  elif [ "$1" = "r" ]  ; then 
    echo "raster" 
  elif [ "$1" = "r3" ]  ; then 
    echo "raster3D" 
  elif [ "$1" = "s" ]  ; then 
    echo "sites" 
  elif [ "$1" = "v" ]  ; then 
    echo "vector" 
  else 
    echo "$1"
 fi
}

#are we in the tools/ dir?
ls build_html_index.sh 2> /dev/null
if [ $? -eq 1 ] ; then
 echo "ERROR: this script must be run from the tools/ directory"
 exit
fi

FULLINDEX=full_index.html

#hardcoded list of notes etc:
NOTESLIST="sql"

#hardcoded list of drivers etc:
DRIVERLIST="displaydrivers"

#hardcoded list of variables etc:
VARIABLES="variables"

################

cd $HTMLDIR

#get list of available GRASS modules:
CMDLIST=`ls -1 *.*.html | grep -v "$FULLINDEX" | grep -v index.html | grep -v "\($EXCLUDEHTML\)" | cut -d'.' -f1 | sort -u`
CMDLISTNO=`echo $CMDLIST | wc -w | awk '{print $1}'`

#write main index:
echo "Generating HTML manual pages index (help system)..."
write_html_header $FULLINDEX "GRASS GIS $GRASSVERSION Reference Manual"
echo "<h3>Full command index:</h3>" >> $FULLINDEX
echo "<table border=0>" >> $FULLINDEX
echo "<tr><td>d.*  </td><td>display commands</td></tr>" >> $FULLINDEX
echo "<tr><td>db.* </td><td>database commands</td></tr>" >> $FULLINDEX
echo "<tr><td>g.*  </td><td>general commands</td></tr>" >> $FULLINDEX
echo "<tr><td>g3.* </td><td>general3D commands</td></tr>" >> $FULLINDEX
echo "<tr><td>i.*  </td><td>imagery commands</td></tr>" >> $FULLINDEX
echo "<tr><td>p.* </td><td>paint commands</td></tr>" >> $FULLINDEX
echo "<tr><td>pg.* </td><td>postGRASS commands</td></tr>" >> $FULLINDEX
echo "<tr><td>ps.* </td><td>postscript commands</td></tr>" >> $FULLINDEX
echo "<tr><td>r.*  </td><td>raster commands</td></tr>" >> $FULLINDEX
echo "<tr><td>r3.* </td><td>raster3D commands</td></tr>" >> $FULLINDEX
echo "<tr><td>v.*  </td><td>vector commands</td></tr>" >> $FULLINDEX
echo "</table>" >> $FULLINDEX
 
#generate main index of all modules:
echo "[ " >> $FULLINDEX
k=0
for i in $CMDLIST
do
  k=`expr $k + 1`
  echo -n "<b><a href=\"#$i\">$i.*</a></b>" >> $FULLINDEX
  if [ $k -lt $CMDLISTNO ] ; then
     echo -n " | " >> $FULLINDEX
  fi
done

echo " ]
<p>
" >> $FULLINDEX

#for all module groups:
for i in $CMDLIST
do 
  echo "<a name=\"$i\"></a>" >> $FULLINDEX
  echo "<b>$i.* commands:</b>" >> $FULLINDEX
  echo "<table>" >> $FULLINDEX

  #for all modules:  
  for i in `ls -1 $i.*.html | grep -v "\($EXCLUDEHTML\)"`
  do
    BASENAME=`basename $i .html`
    SHORTDESC="`cat $i | awk '/NAME/,/SYNOPSIS/' | grep '<em>' | cut -d'-' -f2- | sed 's+^ ++g' | grep -vi 'SYNOPSIS' | head -1`"
    echo "<tr><td><a href=\"$i\">$BASENAME</a></td> <td>$SHORTDESC</td></tr>" >> $FULLINDEX
  done
  echo "</table>" >> $FULLINDEX
  echo "<p>" >> $FULLINDEX
done

write_html_footer $FULLINDEX index.html
# done full index

#next write separate module pages:
#for all module groups:
for k in $CMDLIST
do 
  MODCLASS=`expand_module_class_name $k`
  FILENAME=$MODCLASS.html

  write_html_header $FILENAME "GRASS GIS $GRASSVERSION Reference Manual"
  echo "<b>$MODCLASS commands:</b>" >> $FILENAME
  echo "<table>" >> $FILENAME
  #for all modules:
  for k in `ls -1 $k.*.html | grep -v "\($EXCLUDEHTML\)"`
  do
    BASENAME=`basename $k .html`
    SHORTDESC="`cat $k | awk '/NAME/,/SYNOPSIS/' | grep '<em>' | cut -d'-' -f2- | sed 's+^ ++g' | grep -vi 'SYNOPSIS' | head -1`"
    echo "<tr><td><a href=\"$k\">$BASENAME</a></td> <td>$SHORTDESC</td></tr>" >> $FILENAME
  done
  echo "</table>" >> $FILENAME
  echo "<p>"   >> $FILENAME

  write_html_footer $FILENAME index.html
done

#next write main page:
FILENAME=index.html
write_html_header $FILENAME "GRASS GIS $GRASSVERSION Reference Manual"

#modules:
echo "<h3>Manual sections:</h3>" >> $FILENAME
echo "<ul>" >> $FILENAME
#for all module groups:
for k in $CMDLIST
do 
  MODCLASS=`expand_module_class_name $k`
  echo "<li><a href=\"$MODCLASS.html\">$MODCLASS commands</a>" >> $FILENAME
done
echo "</ul>" >> $FILENAME
echo "<p>"   >> $FILENAME

#notes:
echo "<b>Notes sections:</b>" >> $FILENAME
echo "<ul>" >> $FILENAME
#for all notes:
for k in $NOTESLIST
do 
  echo "<li><a href=\"$k.html\">$k notes</a>" >> $FILENAME
done
echo "</ul>" >> $FILENAME
echo "<p>"   >> $FILENAME

#############
#drivers:

echo "<b>Drivers sections:</b>" >> $FILENAME
echo "<ul>" >> $FILENAME
#for all drivers:
for k in $DRIVERLIST
do 
  echo "<li><a href=\"$k.html\">$k notes</a>" >> $FILENAME
done
echo "</ul>" >> $FILENAME
echo "<p>"   >> $FILENAME

#############
#variables:

echo "<b>GRASS variables and environment variables:</b>" >> $FILENAME
echo "<ul>" >> $FILENAME
#for all drivers:
for k in $VARIABLES
do 
  echo "<li><a href=\"$k.html\">$k notes</a>" >> $FILENAME
done
echo "</ul>" >> $FILENAME
echo "<p>"   >> $FILENAME

#############
write_html_footer $FILENAME index.html

#############
echo "Generated HTML docs in $HTMLDIR/index.html"
echo "----------------------------------------------------------------------"
echo "Following modules are missing the 'description.html' file in src code:"
for i in `find . -name "*.*.html" | sort | grep -v "$FULLINDEX"  | grep -v 'index.html'`
do
  if grep 'DESCRIPTION' $i >/dev/null 2>&1 ; then 
    :
  else
    echo `echo $i | sed 's?./??' | sed 's/.html//'`
  fi
done
echo "----------------------------------------------------------------------"
