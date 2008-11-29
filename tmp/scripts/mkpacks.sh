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
	export SHELL=busybox-$SHELL_VER
	export SSL=openssl-$OPENSSL_VER$OPENSSL_BUILD
	export BDB=db-$BDB_VER
	export LDAP=openldap-$OPENLDAP_VER
	export SNMP=net-snmp-$NETSNMP_VER
	export KRB5=krb5-$KRB5_VER
	export RADIUS=freeradius-$FREERADIUS_VER
	export HTTP=httpd-$APACHE_VER
	export DOT1X=dot1x-$DOT1X_VER

	export SSL_CFG=$SSL_INSTALL/openssl.cnf
}

install_file()
{
	local prefix=$2
	local from=$1
	local to=$prefix/$1

	if [ -x $to ]; then
		return 0
	fi

	dir=`dirname $to`
	dot1x_mkdir $dir
	link=`readlink $from`

	if [ "x$link" != "x" ]; then
		dot1x_log "Linking `basename $from`"
		(
			cd $dir
			ln -s $link `basename $to` 1>/dev/null 2>&1
			res=`get_link $from`
			install_file $res $prefix
		)
	else
		dot1x_log "Installing `basename $from`"
		if [ -d $from ]; then
			cp -rf $from $to
		elif [ -x $from ]; then
			libs=`ldd $from | sort | uniq | \
			      awk -F\= '{print $2}' | \
			      awk '{print $2}' | uniq`
			for lib in $libs ; do
				install_file $lib $prefix
			done
			cp -f $from $to
			strip $to 1>/dev/null 2>&1
		else
			cp -f $from $to
			strip $to 1>/dev/null 2>&1
		fi
	fi
}

make_package()
{
	local package=$1
	local files=`$DOT1X_ROOT/scripts/$package.sh package`

	# install files
	if [ "x$files" == "x" ]; then
		return 1
	fi
	rm -rf $DOT1X_TEMP/$package
	for from in $files ; do
		install_file $from $DOT1X_TEMP/$package
	done

	dot1x_log "Packaging $package"
	# packaging
	(
	cd $DOT1X_TEMP/$package
	tar -zcf $package.tgz --exclude=$package.tgz .
	)
	cp $DOT1X_TEMP/$package/$package.tgz $DOT1X_BINARY/$package.tgz
}

make_dot1x()
{
	for package in $PACKAGES ; do
		dot1x_log "Making $package"
		make_package $package
	done
}

get_root $0
source $DOT1X_ROOT/scripts/make.cfg
set_environment
if [ "x$1" = "x" ]; then
	make_dot1x
else
	make_package $1
fi

