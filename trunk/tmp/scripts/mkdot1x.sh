#!/bin/sh

get_root()
{
	local bindir=`dirname $1`
	export DOT1X_ROOT=`(cd $bindir; cd ..; pwd)`
	source $DOT1X_ROOT/scripts/libdot1x.sh
	dot1x_init
}

get_link()
{
	local dir=`dirname $1`
	local file=`readlink $1`
	(cd $dir
	cd `dirname $file`
	res=`pwd`/`basename $file`
	echo $res)
}

set_environment()
{
	export DOT1X_ROOTFS=$DOT1X_BUILD/rootfs
	dot1x_mkdir $DOT1X_ROOTFS

	# compilation environments
	export CPPFLAGS="-I$DOT1X_PREFIX/include"
	export CFLAGS="-I$DOT1X_PREFIX/include"
	export LDFLAGS="-L$DOT1X_PREFIX/lib"

	export LINUX=linux-$LINUX_VER
	export GLIBC=glibc-$LIBC_VER
	export SHELL=busybox-$SHELL_VER
	export SSL=openssl-$OPENSSL_VER$OPENSSL_BUILD
	export BDB=db-$BDB_VER
	export LDAP=openldap-$OPENLDAP_VER
	export SNMP=net-snmp-$NETSNMP_VER
	export KRB5=krb5-$KRB5_VER
	export RADIUS=freeradius-$FREERADIUS_VER
	export HTTP=httpd-$APACHE_VER
	export DOT1X=dot1x-$DOT1X_VER
	export MACH=$MACH

	export SSL_CFG=$SSL_INSTALL/openssl.cnf
}

make_binary()
{
	local pkgbzip=0
	local package=$1

	if [ -f $DOT1X_ROOT/scripts/$package.sh ]; then
		local pkgname=$DOT1X_ROOT/archives/`printenv $package`
		if [ -f $pkgname.tar.gz ]; then
			export DOT1X_PKG=$pkgname.tar.gz
		elif [ -f $pkgname.tar.bz2 ]; then
			export DOT1X_PKG=$pkgname.tar.bz2
			pkgbzip=1
		elif [ -f $pkgname.tgz ]; then
			export DOT1X_PKG=$pkgname.tgz
		fi
		export DOT1X_DIR=$DOT1X_BUILD/`printenv $package`
		dot1x_log DOT1X_PKG=$DOT1X_PKG
		dot1x_log DOT1X_DIR=$DOT1X_DIR
		dot1x_log DOT1X_BUILD=$DOT1X_BUILD
		rm -rf $DOT1X_DIR
		#if [ ! -d $DOT1X_DIR ]; then
			(
			dot1x_mkdir $DOT1X_DIR;
			cd $DOT1X_BUILD;
			if [ $pkgbzip -eq 1 ]; then
				tar -xjf $DOT1X_PKG
			else
				pwd
				echo $DOT1X_PKG
				tar -xzf $DOT1X_PKG
			fi
			)
		#fi
		(cd $DOT1X_DIR || exit 1; chown -R root.root .)
		echo $DOT1X_ROOT/scripts/$package.sh
		$DOT1X_ROOT/scripts/$package.sh build
		ldconfig
	fi
}

make_dot1x()
{
	for package in $PACKAGES ; do
		dot1x_log "Making $package"
		make_binary $package
	done
}

get_root $0
source $DOT1X_ROOT/scripts/make.cfg
set_environment
if [ "x$1" = "x" ]; then
	make_dot1x
else
	make_binary $1
fi

