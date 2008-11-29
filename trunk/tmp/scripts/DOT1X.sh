#!/bin/sh

build_dot1x()
{
	cd $DOT1X_DIR || exit 1
	# export CFLAGS="-O2"
	cp -rf $DOT1X_ROOT/conf ./
	cp -rf $DOT1X_ROOT/htdocs ./src/epscgi
	cp -rf $DOT1X_ROOT/images ./src/epscgi
	./configure \
		--prefix=$DOT1X_PREFIX \
		--localstatedir=$LOCALSTATE_DIR \
		--sysconfdir=$SYSCONF_DIR \
		--libexecdir=$DOT1X_PREFIX/sbin \
        	--disable-debug \
        	--with-cgibin=$DOT1X_CGIDIR \
        	--with-openldap=$DOT1X_PREFIX \
        	--with-openssl=$DOT1X_PREFIX \
	|| exit
	make clean
	make
	make license
	echo ===========================================
	echo $DOT1X_DIR $DOT1X_PREFIX
	cp -f $DOT1X_DIR/src/scripts/epssave $DOT1X_PREFIX/sbin/epssave
	cp -f $DOT1X_DIR/src/scripts/epsinit $DOT1X_PREFIX/sbin/epsinit
	cp -f $DOT1X_DIR/src/scripts/epspass $DOT1X_PREFIX/sbin/epspass
	chmod +x $DOT1X_PREFIX/sbin/epspass
	# make DESTDIR=$DOT1X_ROOTFS install
}

package_dot1x()
{
	files="\
		$SYSCONF_DIR/eps/epslib.conf \
		$SYSCONF_DIR/eps/epsldap.conf \
		$DOT1X_CGIDIR/index.cgi \
		$DOT1X_CGIDIR/images \
		$DOT1X_PREFIX/cgi/epscgi/htdocs \
		$DOT1X_PREFIX/sbin/epsinit \
		$DOT1X_PREFIX/sbin/epspass \
		$DOT1X_PREFIX/sbin/epssave \
		$DOT1X_PREFIX/sbin/syslogd \
		$DOT1X_PREFIX/sbin/epscmd \
		$DOT1X_PREFIX/bin/epsled
	"
	echo $files
}

clean_dot1x()
{
	echo
}

trap "exit -1" INT
case "$1" in
	build)
		build_dot1x
		;;
	package)
		package_dot1x
		;;
	clean)
		clean_dot1x
		;;
	*)
		;;
esac

exit 0
