#!/bin/sh

dot1x_log()
{
	local time=`date +"%D %T"`
	echo $time "dot1x:" $@
}

# Make partition
dot1x_mkpart()
{
	local part=$1

	# create partition
	if mke2fs -F -m0 -q /dev/$part 1>/dev/null 2>&1; then
		tune2fs -c 0 -i 0 /dev/$part 1>/dev/null 2>&1
		dot1x_log "Making partition $part success"
		return 0
	fi
	dot1x_log "Making partition $part failure"
	return 1
}

# Make ramdisk
dot1x_mkram()
{
	local image=$1
	local base=`basename $1`
	local ksize=$2

	dot1x_log "Creating image $base of ${ksize}KB"
	if dd if=/dev/zero of=$image bs=1024 count=$ksize 1>/dev/null 2>&1
	then
		if mke2fs -F -m0 -q $image 1>/dev/null 2>&1; then
			tune2fs -i 0 -c 0 $image 1>/dev/null 2>&1
			dot1x_log "Creating image $base success"
			return 0
		fi
	fi
	dot1x_log "Creating image $base failure"
	return 1
}

dot1x_mkdir()
{
	if [ ! -d $1 ]; then
		mkdir -p $1
	fi
}

dot1x_init()
{
	dot1x_log DOT1X_ROOT=$DOT1X_ROOT
	source $DOT1X_ROOT/scripts/make.cfg
	export ARCH=$TARGET

	export DOT1X_BUILD=$DOT1X_ROOT/obj/$ARCH
	dot1x_mkdir $DOT1X_BUILD
	export DOT1X_BINARY=$DOT1X_ROOT/bin/$ARCH
	dot1x_mkdir $DOT1X_BINARY
	export DOT1X_TEMP=$DOT1X_ROOT/tmp/$ARCH
	dot1x_mkdir $DOT1X_TEMP
	
	export DOT1X_PREFIX=$PREFIX
	dot1x_mkdir $DOT1X_PREFIX

	export SYSCONF_DIR=$SYSCONF
	export LOCALSTATE_DIR=$LOCALSTATE
	export OPENSSL_DIR=$SSL_INSTALL
	export CERTS_DIR=$OPENSSL_DIR/certs
	export DOT1X_CGIDIR=$DOT1X_CGIDIR
}

stop_slapd()
{
	local pid=/var/run/slapd.pid
	local bin=$PREFIX/sbin/slapd
	if [ -f $pid ]; then
		kill -15 `cat $pid` 1>/dev/null 2>&1
		killall -KILL slapd 1>/dev/null 2>&1
		rm -f $pid
		dot1x_log "Stopping slapd success"
	fi
}

start_slapd()
{
	local cfg=/etc/openldap/slapd.conf
	local bin=$PREFIX/sbin/slapd
	stop_slapd
	if [ -f $cfg -a -f $bin ]; then
		if $bin -f $cfg 1>/dev/null 2>&1; then
			dot1x_log "Starting slapd success"
			return 0
		fi
	fi
	dot1x_log "Starting slapd failure"
}

stop_radiusd()
{
	local pid=/var/run/radiusd.pid
	local bin=$PREFIX/sbin/radiusd
	if [ -f $pid ]; then
		kill -15 `cat $pid` 1>/dev/null 2>&1
		killall -KILL radiusd 1>/dev/null 2>&1
		rm -f $pid
		dot1x_log "Stopping radiusd success"
	fi
}

start_radiusd()
{
	local cfg=/etc/raddb
	local bin=$PREFIX/sbin/radiusd
	stop_radiusd
	if [ -d $cfg -a -f $bin ]; then
		if $bin -d $cfg -l syslog 1>/dev/null 2>&1; then
			dot1x_log "Starting radiusd success"
			return 0
		fi
	fi
	dot1x_log "Starting radiusd failure"
}
