#!/c/Programme/OSGeo4W/apps/msys/bin/sh

set -e

if ! [ -d mswindows ]; then
	echo Start from GRASS toplevel dir
	exit 1
fi

export OSGEO4W_ROOT_MSYS=/c/Programme/OSGeo4W
export PATH=.:/c/mingw/bin:/usr/local/bin:/bin:$OSGEO4W_ROOT_MSYS/bin:/c/WINDOWS/system32:/c/WINDOWS:/c/WINDOWS/System32/Wbem

version() {
	(
		read MAJOR
		read MINOR
		read PATCH
		echo $MAJOR.$MINOR.$PATCH
	) <include/VERSION
}

export VERSION=$(version)
export PACKAGE=3


(
echo
echo
echo

if ! [ -f mswindows/osgeo4w/configure-stamp ]; then
	echo $(date): STARTING configure

	./configure \
		--with-libs="$OSGEO4W_ROOT_MSYS/apps/gdal-16/lib $OSGEO4W_ROOT_MSYS/lib" \
		--with-includes="$OSGEO4W_ROOT_MSYS/apps/gdal-16/include $OSGEO4W_ROOT_MSYS/include" \
		--exec-prefix=$OSGEO4W_ROOT_MSYS/bin \
		--libexecdir=$OSGEO4W_ROOT_MSYS/bin \
		--prefix=$OSGEO4W_ROOT_MSYS/apps/grass \
		--includedir=$OSGEO4W_ROOT_MSYS/include \
		--disable-x --without-x \
		--with-cxx \
		--enable-shared \
		--enable-largefile \
		--with-opengl=windows \
		--with-fftw \
		--with-freetype \
		--with-proj-share=$OSGEO4W_ROOT_MSYS/share/proj \
		--with-gdal=$OSGEO4W_ROOT_MSYS/bin/gdal-config \
		--with-tcltk \
		--with-sqlite \
		--with-postgres \
		--with-curses \
		--with-regex \
		--with-nls \
		--without-swig \
		--with-freetype-includes=$OSGEO4W_ROOT_MSYS/include/freetype2 \
		--with-odbc

	echo $(date): STARTING make clean
	make clean

	touch mswindows/osgeo4w/configure-stamp
fi

echo $(date): STARTING make
make -j4 || make -j4

echo $(date): STARTING make install
make install

echo $(date): STARTING cleanup
mv $OSGEO4W_ROOT_MSYS/apps/grass/grass-$VERSION/lib/*.$VERSION.dll $OSGEO4W_ROOT_MSYS/bin
rm $OSGEO4W_ROOT_MSYS/apps/grass/grass-$VERSION/lib/*.dll

mv $OSGEO4W_ROOT_MSYS/apps/grass/grass-$VERSION/include/grass/config.h $OSGEO4W_ROOT_MSYS/apps/grass/grass-$VERSION/include/grass/config.h.mingw
cp mswindows/osgeo4w/config.h.switch $OSGEO4W_ROOT_MSYS/apps/grass/grass-$VERSION/include/grass/config.h
cp mswindows/osgeo4w/config.h.vc $OSGEO4W_ROOT_MSYS/apps/grass/grass-$VERSION/include/grass
cp mswindows/osgeo4w/grass64.bat.tmpl $OSGEO4W_ROOT_MSYS/bin/grass64.bat.tmpl
cp mswindows/osgeo4w/ini.bat.tmpl $OSGEO4W_ROOT_MSYS/etc/ini/grass.bat.tmpl
cp mswindows/osgeo4w/postinstall.bat $OSGEO4W_ROOT_MSYS/etc/postinstall/grass.bat
cp mswindows/osgeo4w/preremove.bat $OSGEO4W_ROOT_MSYS/etc/preremove/grass.bat
cp /c/mingw/bin/libgnurx-0.dll $OSGEO_ROOT_MSYS/bin
cp /c/mingw/bin/libiconv-2.dll $OSGEO_ROOT_MSYS/bin
cp /c/mingw/bin/libintl-8.dll $OSGEO_ROOT_MSYS/bin

P=$(pwd -W)
P=${P//\//\\\\}\\\\dist.i686-pc-mingw32

sed -e "s#$P#@osgeo4w@#" $OSGEO4W_ROOT_MSYS/apps/grass/grass-$VERSION/etc/fontcap >$OSGEO4W_ROOT_MSYS/apps/grass/grass-$VERSION/etc/fontcap.tmpl
sed -e "s#$P#@osgeo4w_msys@#" $OSGEO4W_ROOT_MSYS/apps/grass/bin/grass64 >$OSGEO4W_ROOT_MSYS/apps/grass/bin/grass64.tmpl
rm $OSGEO4W_ROOT_MSYS/apps/grass/grass-$VERSION/etc/fontcap 

echo $(date): STARTING building vc libraries
sh mswindows/osgeo4w/mklibs.sh $OSGEO4W_ROOT_MSYS/bin/*.$VERSION.dll
mv mswindows/osgeo4w/vc/grass*.lib $OSGEO4W_ROOT_MSYS/apps/grass/grass-$VERSION/lib

set -x
echo $(date): BUILDING GDAL GRASS plugins
cmd /c 'mswindows\\osgeo4w\\gdalplugins.cmd $VERSION'

echo $(date): CREATING packages
mkdir -p package/grass-devel package/grass-devel-mingw package/grass-devel-vc package/grass

PDIR=$PWD/package
cd $OSGEO4W_ROOT_MSYS
tar -cjf $PDIR/grass-devel/grass-devel-$VERSION-$PACKAGE.tar.bz2 \
	apps/grass/grass-$VERSION/include

tar -cjf $PDIR/grass-devel-mingw/grass-devel-mingw-$VERSION-$PACKAGE.tar.bz2 \
	apps/grass/grass-$VERSION/lib/libgrass*.a

tar -cjf $PDIR/grass-devel-vc/grass-devel-vc-$VERSION-$PACKAGE.tar.bz2 \
	apps/grass/grass-$VERSION/lib/*.lib

tar -cjf $PDIR/grass/grass-$VERSION-$PACKAGE.tar.bz2 \
	apps/grass/bin/grass64.tmpl \
	apps/grass/grass-$VERSION/authors \
	apps/grass/grass-$VERSION/bin/ \
	apps/grass/grass-$VERSION/bwidget/ \
	apps/grass/grass-$VERSION/changes \
	apps/grass/grass-$VERSION/copying \
	apps/grass/grass-$VERSION/docs \
	apps/grass/grass-$VERSION/driver \
	apps/grass/grass-$VERSION/etc \
	apps/grass/grass-$VERSION/gpl.txt \
	apps/grass/grass-$VERSION/requirements.html \
	apps/grass/grass-$VERSION/scripts \
	bin/libgnurx-0.dll \
	bin/libiconv-2.dll \
	bin/libintl-8.dll \
	bin/grass64.bat.tmpl \
	bin/libgrass_*.$VERSION.dll \
	etc/ini/grass.bat \
	etc/postinstall/grass.bat \
	etc/preremove/grass.bat

echo $(date): END

) | tee -a mswindows/osgeo4w/package.log
