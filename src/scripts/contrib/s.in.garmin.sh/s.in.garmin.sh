#!/bin/sh

# s.in.garmin.sh -- import gps data from garmin receiver 
#                   into GRASS sites file
# 
# c) Andreas Lange, andreas.lange@rhein-main.de
# $Id$
# 
# requirements: GRASS 4.x or GRASS 5.0 with s.in.ascii
#      -  gpstrans from Carsten Tschach et al. 
#           get gpstrans from one of:
#             http://gpstrans.sourceforge.net
#             http://www.metalab.unc.edu/pub/Linux/science/cartography/
#             ftp://www.mayko.com/pub/gpstrans
#      -  bourne shell
#      -  unix tools: grep, cat, cut, paste, sed
#

#%Module
#%  description: Upload Waypoints, Routes and Tracks from a Garmin GPS reciever into a GRASS sites file, transformed into the current projection.
#%End
#%flag
#%  key: v
#%  description: verbose mode (version info)
#%end
#%flag
#%  key: w
#%  description: upload Waypoints
#%end
#%flag
#%  key: r
#%  description: upload Routes
#%end
#%flag
#%  key: t
#%  description: upload Track
#%end
##%flag
##%  key: s
##%  description: swap easting/northing (for tmerc projection)
##%end
#%flag
#%  key: k
#%  description: do not attempt projection transform from WGS84
#%end
#%option
#% key: sites
#% type: string
#% gisprompt: new,site_lists,sites
#% description: name for new sites file (omit for display to stdout)
#% required : no
#%end
#%option
#% key: port
#% type: string
#% description: port Garmin receiver is connected to
#% answer: /dev/gps
#% required : no
#%end

if [ "$1" != "@ARGS_PARSED@" ] ; then
  exec $GISBASE/etc/bin/cmd/g.parser "$0" "$@"
fi


PROG=`basename $0`
VERSION="$PROG c) 2000 Andreas Lange, andreas.lange@rhein-main.de"
GRASS4="#"
GRASS5="@"
DELIM=$GRASS5
OPTS=""


#### test if GRASS is running
if [ "$GISRC" = "" ] ; then
    echo "$PROG: You must be running GRASS to execute $PROG" 1>&2
    exit 1
fi


#### check for gpstrans 
GPSTRANS=`which gpstrans`

if [ "$GPSTRANS" = "" ] ; then
    echo "$PROG: gpstrans program not found, install it first" 1>&2
    exit 1
fi


#### check that GRASS variables are set
eval `g.gisenv`

: $GISDBASE $LOCATION $MAPSET


#### set temporary files
TMP=/tmp/tmp__$$
# better:
# TMP1=`g.tempfile pid=$$`


#### trap ctrl-c so that we can clean up tmp
trap 'rm -f ${TMP}*' 2 3 15


#### process command line arguments 
WPT=0 ; RTE=0 ; TRK=0 ; SWP=0 ; KEEP_WGS84=0

if [ "$GIS_OPT_sites" != "(null)" ] ; then
    NAME="$GIS_OPT_sites"
    echo "sites=$NAME" 1>&2
fi
if [ "$GIS_OPT_port" != "(null)" ] ; then
    GPSPORT="-p$GIS_OPT_port"
    echo "port=$GIS_OPT_port" 1>&2
fi
if [ $GIS_FLAG_v -eq 1 ] ; then
    echo $VERSION 1>&2
    echo " " 1>&2
fi
if [ $GIS_FLAG_w -eq 1 ] ; then
    WPT=1
fi
if [ $GIS_FLAG_r -eq 1 ] ; then
    RTE=1
fi
if [ $GIS_FLAG_t -eq 1 ] ; then
    TRK=1
fi
#if [ $GIS_FLAG_s -eq 1 ] ; then
#    SWP=1
#fi
if [ $GIS_FLAG_k -eq 1 ] ; then
    KEEP_WGS84=1
fi



#### check that receiver is responding on $GPSPORT
# sadly gpstrans 0.39 returns 0 after timeout.. hopefully fixed someday.
$GPSTRANS "$GPSPORT" -i 1> /dev/null
if [ $? -ne 0 ] ; then
    echo "$PROG: Receiver on $GPSPORT not responding, exiting" 1>&2
    rm -f ${TMP}*
    exit 1
fi


#### receive and pre-process data
if [ $WPT -eq 1 ] ; then
    echo "Uploading Waypoints" 1>&2
    $GPSTRANS "$GPSPORT" -dw >> $TMP 2>/dev/null
    if [ $? -ne 0 ] ; then
	echo "$PROG: Error uploading Waypoints" 1>&2
	rm -f ${TMP}*
	exit 1
    fi
fi

if [ $RTE -eq 1 ] ; then
    echo "Uploading Routes" 1>&2
    $GPSTRANS "$GPSPORT" -dr >> $TMP 2>/dev/null
    if [ $? -ne 0 ] ; then
	echo "$PROG: Error uploading Routes" 1>&2
	rm -f ${TMP}*
	exit 1
    fi
fi

if [ $TRK -eq 1 ] ; then
    echo "Uploading Tracks" 1>&2
    $GPSTRANS "$GPSPORT" -dt >> $TMP 2>/dev/null 
    if [ $? -ne 0 ] ; then
	echo "$PROG: Error uploading Tracks" 1>&2
	rm -f ${TMP}*
	exit 1
    fi
fi


#### swap easting and northing if necessary
if [ $SWP -eq 1 ] ; then
  WPTS="${TMP}_WPN ${TMP}_WPE"
else
  WPTS="${TMP}_WPE ${TMP}_WPN"
fi


#### check which projection we are working with
PROJ="`head -1 $TMP | sed -e 's/Format: //' | sed -e 's/  UTC.*//'`"
# echo ${PROJ}_
IS_WGS84="`head -1 $TMP | grep 'WGS 84'`"


#### process waypoint format
case "$PROJ" in 
    UTM)
	cat $TMP | grep "^W" | tr -s '[:space:]' | cut -f8     >> ${TMP}_WPN
	cat $TMP | grep "^W" | tr -s '[:space:]' | cut -f7     >> ${TMP}_WPE
	cat $TMP | grep "^W" | tr -s '[:space:]' | cut -f2,3,4 >> ${TMP}_WN
	;;
    DDD)
        cat $TMP | grep "^W" | tr -s '[:space:]' | cut -f5     >> ${TMP}_WPN
	cat $TMP | grep "^W" | tr -s '[:space:]' | cut -f6     >> ${TMP}_WPE
	cat $TMP | grep "^W" | tr -s '[:space:]' | cut -f2,3,4 >> ${TMP}_WN
	;;
    GKK)
        cat $TMP | grep "^W" | tr -s '[:space:]' | cut -f6     >> ${TMP}_WPN
	cat $TMP | grep "^W" | tr -s '[:space:]' | cut -f5     >> ${TMP}_WPE
	cat $TMP | grep "^W" | tr -s '[:space:]' | cut -f2,3,4 >> ${TMP}_WN
	;;
    *)
	echo "Unsupported format [$PROJ]" 1>&2
	exit 1
	;;
esac


#### process track format 
case "$PROJ" in
    UTM)
	cat $TMP | grep "^T" | tr -s '[:space:]' | cut -f6 >> ${TMP}_WPN
	cat $TMP | grep "^T" | tr -s '[:space:]' | cut -f5 >> ${TMP}_WPE
	cat $TMP | grep "^T" | tr -s '[:space:]' | cut -f2 >> ${TMP}_WN
	;;
    DDD)
	cat $TMP | grep "^T" | tr -s '[:space:]' | cut -f3 >> ${TMP}_WPN
	cat $TMP | grep "^T" | tr -s '[:space:]' | cut -f4 >> ${TMP}_WPE
	cat $TMP | grep "^T" | tr -s '[:space:]' | cut -f2 >> ${TMP}_WN
	;;
    GKK)
        cat $TMP | grep "^T" | tr -s '[:space:]' | cut -f4 >> ${TMP}_WPN
	cat $TMP | grep "^T" | tr -s '[:space:]' | cut -f3 >> ${TMP}_WPE
	cat $TMP | grep "^T" | tr -s '[:space:]' | cut -f2 >> ${TMP}_WN
	;;
    *)
	echo "Unsupported format [$PROJ]" 1>&2
	exit 1
	;;
esac


#### paste together import file
paste -d"\t$DELIM" $WPTS ${TMP}_WN | sed "s/$DELIM/ $DELIM/g" > ${TMP}_P_raw


#### convert from WGS84 to current projection
if [ -z "$IS_WGS84" ] || [ $KEEP_WGS84 -eq 1 ] ; then
    echo "No projection transformation performed" 1>&2
    mv -f ${TMP}_P_raw ${TMP}_P
else
    echo "Attempting projection transform with m.proj2" 1>&2
    m.proj2 -i input=${TMP}_P_raw output=${TMP}_P
    if [ $? -ne 0 ] ; then
        echo "Projection transform failed, retaining WGS84" 1>&2
        mv -f ${TMP}_P_raw ${TMP}_P
    fi
fi


#### if no name for sites file given, cat to stdout
if [ "$NAME" = "" ] ; then
    echo "Output to stdout" 1>&2
    cat ${TMP}_P 2>/dev/null
else 


    #### import to sites file 
    echo "Importing with s.in.ascii" 1>&2
    s.in.ascii sites=$NAME input=${TMP}_P $OPTS 1>&2 >/dev/null
    # fs=tab


    #### check success/failure 
    if [ $? -eq 0 ] ; then
	echo "$PROG: sites file \"$NAME@$MAPSET\" successfully created" 1>&2
    else
	echo "$PROG: An error occured on creating \"$NAME\" in mapset \"$MAPSET\"," 1>&2
	echo "please check!" 1>&2
    fi
fi


#### clean up the mess
rm -f ${TMP}* 1>&2 > /dev/null


#### end
exit 0

