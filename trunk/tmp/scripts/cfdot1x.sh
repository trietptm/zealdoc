#!/bin/sh

get_root()
{
	local bindir=`dirname $1`
	export DOT1X_ROOT=`(cd $bindir; cd ..; pwd)`
	source $DOT1X_ROOT/scripts/libdot1x.sh
	dot1x_init
}

set_envs()
{
	if [ -z $1 ]; then
		export DOT1X_CONFIG=
	else
		export DOT1X_CONFIG=$1
	fi
}

usage()
{
	echo "Usage:"
	echo "  $1 command"
}

clean_sysconf()
{
	if [ "x$DOT1X_ROOT" != "x" ]; then
		dot1x_log "Cleaning sysconf"
		cp -f $DOT1X_ROOT/conf/init.d/* $DOT1X_CONFIG$SYSCONF_DIR/init.d/
		cp -f $DOT1X_ROOT/conf/cron.d/* $DOT1X_CONFIG$SYSCONF_DIR/cron.d/
		cp -f $DOT1X_ROOT/conf/usrbins/* $DOT1X_CONFIG/usr/bin/
	fi
}

clean_slapd()
{
	dot1x_log "Cleaning slapd"
	rm -rf $DOT1X_CONFIG$LOCALSTATE_DIR/openldap-data/*
	cp -f $DOT1X_ROOT/conf/epslib.conf $DOT1X_CONFIG$SYSCONF_DIR/eps/epslib.conf
	cp -f $DOT1X_ROOT/conf/epsldap.conf $DOT1X_CONFIG$SYSCONF_DIR/eps/epsldap.conf
	cp -f $DOT1X_ROOT/conf/slapd.conf $DOT1X_CONFIG$SYSCONF_DIR/openldap/slapd.conf
	cp -f $DOT1X_ROOT/conf/pkix.schema $DOT1X_CONFIG$SYSCONF_DIR/openldap/schema/pkix.schema
	cp -f $DOT1X_ROOT/conf/radius.schema $DOT1X_CONFIG$SYSCONF_DIR/openldap/schema/radius.schema
	cp -f $DOT1X_ROOT/conf/DB_CONFIG $DOT1X_CONFIG$LOCALSTATE_DIR/openldap-data/DB_CONFIG
}

clean_openssl()
{
	dot1x_log "Cleaning openssl"
	
	cp -f $DOT1X_ROOT/conf/openssl.cnf $DOT1X_CONFIG$SYSCONF_DIR/openssl/openssl.cnf
}

clean_httpd()
{
	dot1x_log "Cleaning httpd"
	
	mkdir -p $DOT1X_CONF/$SYSCONF_DIR/apache
	cp -f $DOT1X_ROOT/conf/httpd.conf $DOT1X_CONFIG$SYSCONF_DIR/apache/httpd.conf
	cp -f $DOT1X_ROOT/conf/httpd-ssl.conf $DOT1X_CONFIG$SYSCONF_DIR/apache/httpd-ssl.conf
	cp -f $DOT1X_ROOT/conf/mime.types $DOT1X_CONFIG$SYSCONF_DIR/apache/mime.types
	cp -f $DOT1X_ROOT/conf/httpd-server.conf $DOT1X_CONFIG$SYSCONF_DIR/apache/httpd-server.conf
	touch $DOT1X_CONFIG$SYSCONF_DIR/apache/httpd-extra.conf
}

clean_eps()
{
	dot1x_log "Cleaning eps"
	
	cp -f $DOT1X_ROOT/conf/epslib.conf $DOT1X_CONFIG$SYSCONF_DIR/eps/epslib.conf
	cp -f $DOT1X_ROOT/conf/epsldap.conf $DOT1X_CONFIG$SYSCONF_DIR/eps/epsldap.conf
	cp -f $DOT1X_ROOT/conf/syslog.conf $DOT1X_CONFIG$SYSCONF_DIR/syslog.conf
}

config_slapd()
{
	stop_slapd
	clean_slapd
	start_slapd
	sleep 1
	$PREFIX/bin/ldapadd \
		-h localhost \
		-D "cn=root,dc=soliton,dc=com" \
		-w secret \
		-f $DOT1X_ROOT/conf/core.ldif \
		1>/dev/null 2>&1 \
	|| dot1x_log "Ldap core uploading failure"
	$PREFIX/bin/ldapadd \
		-h localhost \
		-D "cn=root,dc=soliton,dc=com" \
		-w secret \
		-f $DOT1X_ROOT/conf/radius.ldif \
		1>/dev/null 2>&1 \
	|| dot1x_log "Radius data uploading failure"
}

clean_radiusd()
{
	local randfile=$DOT1X_CONFIG$SYSCONF_DIR/raddb/certs/random
	local dhfile=$DOT1X_CONFIG$SYSCONF_DIR/raddb/certs/dh

	dot1x_log "Cleaning radiusd"
	cp -f $DOT1X_ROOT/conf/snmp.conf $DOT1X_CONFIG$SYSCONF_DIR/raddb/snmp.conf
	cp -f $DOT1X_ROOT/conf/radiusd.conf $DOT1X_CONFIG$SYSCONF_DIR/raddb/radiusd.conf
	cp -f $DOT1X_ROOT/conf/clients.conf $DOT1X_CONFIG$SYSCONF_DIR/raddb/clients.conf
	dot1x_log "Generating random files"
	dot1x_log "Please typing anyting if stopped"
	if [ -f $randfile ]; then
		rm -f $randfile
	fi
	dd if=/dev/urandom of=$randfile count=4 bs=128 \
		1>/dev/null 2>&1 \
	|| dot1x_log "Radius radmon generation failure"
	if [ -f $dhfile ]; then
		rm -f $dhfile
	fi
	dd if=/dev/urandom of=$dhfile count=4 bs=128 \
		1>/dev/null 2>&1 \
	|| dot1x_log "Radius dh generation failure"
}

clean_snmpd()
{
	mkdir $DOT1X_CONFIG$SYSCONF_DIR/snmp
	cp -f $DOT1X_ROOT/conf/snmpd.conf $DOT1X_CONFIG$SYSCONF_DIR/snmp/snmpd.conf
}

config_radiusd()
{
	stop_radiusd
	clean_radiusd
	start_radiusd
	if $PREFIX/bin/radeapclient \
		-f $DOT1X_ROOT/conf/radclient.conf \
		-d $SYSCONF_DIR/raddb localhost auth secret; then
		dot1x_log "Radius client test success"
	else
		dot1x_log "Radius client test failure"
	fi
}

clean_rootfs()
{
	if [ "x$DOT1X_DISK" == "x" ]; then
		disk=`cat $DOT1X_ROOT/scripts/disk.cfg`
	else
		disk=$DOT1X_DISK
	fi
	sed "s/disk/$disk/g" $DOT1X_ROOT/conf/fstab >$DOT1X_CONFIG$SYSCONF_DIR/fstab
	echo $disk > $DOT1X_CONFIG$SYSCONF_DIR/eps/epside.conf
	cp -fp $PREFIX/bin/epsled $DOT1X_CONFIG/bin/epsled
}

export_configs()
{
	clean_sysconf
	clean_slapd
	clean_radiusd
	clean_snmpd
	clean_httpd
	clean_openssl
	clean_rootfs
	clean_eps
}

make_config()
{
	case "$1" in
	export)
		set_envs $2
		export_configs || exit 1
		;;
	ldap)
		set_envs
		config_slapd || exit 1
		;;
	radius)
		set_envs
		config_radiusd || exit 1
		;;
	*)
		set_envs
		config_slapd || exit 1
		config_radiusd || exit 1
		;;
	esac
}

get_root $0
make_config $@ || exit 1

#stop_radiusd
#stop_slapd

exit 0

