#!/bin/sh

get_root()
{
	local bindir=`dirname $1`
	export DOT1X_ROOT=`(cd $bindir; cd ..; pwd)`
	source $DOT1X_ROOT/scripts/libdot1x.sh
	dot1x_init
}

get_device()
{
	# mkimage specific stuff
	export DOT1X_DEVICE=$1
	export DOT1X_DISK=$2
	export DOT1X_CARD=$DOT1X_BUILD/dot1x-card
	export DOT1X_INITRD=$DOT1X_CARD/rootfs
	rm -rf $DOT1X_INITRD
	dot1x_mkdir $DOT1X_INITRD
	export DOT1X_VARDIR=$DOT1X_CARD/vardir
	rm -rf $DOT1X_VARDIR
	dot1x_mkdir $DOT1X_VARDIR
	export DOT1X_PART1=$DOT1X_CARD/part1
	rm -rf $DOT1X_PART1
	dot1x_mkdir $DOT1X_PART1
	export DOT1X_PART2=$DOT1X_CARD/part2
	rm -rf $DOT1X_PART2
	dot1x_mkdir $DOT1X_PART2
	export DOT1X_PART3=$DOT1X_CARD/part3
	rm -rf $DOT1X_PART3
	dot1x_mkdir $DOT1X_PART3
	export DOT1X_IMAGE=$DOT1X_CARD/initrd
	export DOT1X_VARIMG=$DOT1X_CARD/var.img
}

make_part1()
{
	# create partition 1
	dot1x_mkpart ${DOT1X_DEVICE}1 || return 1

	umount $DOT1X_PART1 1>/dev/null 2>&1
	mount -t ext2 /dev/${DOT1X_DEVICE}1 $DOT1X_PART1 \
		1>/dev/null 2>&1 \
	|| dot1x_log "Mount partition1 failure"
	
	# install grub stuff
	dot1x_mkdir $DOT1X_PART1/boot/grub
	grub-install \
		--root-directory=$DOT1X_PART1 \
		--no-floppy \
		/dev/${DOT1X_DEVICE} \
		1>/dev/null 2>&1 \
	|| dot1x_log "Install grub failure"
	cp -f $DOT1X_ROOT/conf/grub.conf $DOT1X_PART1/boot/grub/menu.lst
	cp -f $DOT1X_BINARY/vmlinuz $DOT1X_PART1/boot/vmlinuz
	cp -f $DOT1X_BINARY/initrd $DOT1X_PART1/boot/initrd
	sync
	umount $DOT1X_PART1 1>/dev/null 2>&1
}

make_part2()
{
	# create partition 2
	dot1x_mkpart ${DOT1X_DEVICE}2 || return 1

	umount $DOT1X_PART2 1>/dev/null 2>&1
	mount -t ext2 /dev/${DOT1X_DEVICE}2 $DOT1X_PART2 \
		1>/dev/null 2>&1 \
	|| dot1x_log "Mount partition2 failure"
	(
	cd $DOT1X_PART2
	cp -f $DOT1X_BINARY/var.img $DOT1X_PART2
	)
	sync
	umount $DOT1X_PART2 1>/dev/null 2>&1
}

make_part3()
{
	# create partition 3
	dot1x_mkpart ${DOT1X_DEVICE}3 || return 1

	umount $DOT1X_PART3 1>/dev/null 2>&1
	mount -t ext2 /dev/${DOT1X_DEVICE}3 $DOT1X_PART3 \
		1>/dev/null 2>&1 \
	|| dot1x_log "Mount partition3 failure"
	(
	cd $DOT1X_PART3
	cp -f $DOT1X_BINARY/init.tgz $DOT1X_PART3
	)
	sync
	umount $DOT1X_PART3 1>/dev/null 2>&1
}

# Initialize CF card
#   1. Create partition table
#   2. Create EXT2 filesystem into partition1
#   3. Install grub into MBR and partition1
make_disk()
{
	dot1x_log "Making CF card"

	# create partition table
	sfdisk --force /dev/$DOT1X_DEVICE \
		< $DOT1X_ROOT/scripts/cf.dump \
		1>/dev/null 2>&1 \
	|| dot1x_log "Create partition table failure"

	make_part1 || exit 1
	make_part2 || exit 1
	make_part3 || exit 1
}

dot1x_cpfile()
{
	local from=$1
	local to=$2

	# fixup to (note to must not be the parent directory for actual to
	if [ ! -d `dirname $to` ]; then
		dot1x_mkdir `dirname $to`
	fi
	cp -rf $from $to
}

make_varimg()
{
	# make sysconfig var files
	local sysconf_dir=$DOT1X_INITRD$LOCALSTATE_DIR/sysconf

	dot1x_log "Making var image $sysconf_dir"
	rm -rf $sysconf_dir
	dot1x_mkdir $sysconf_dir
	
	local sysconf_files=`cat $DOT1X_ROOT/scripts/sysconf.cfg`
	for file in $sysconf_files; do
		local sysconf_file=$DOT1X_INITRD/$SYSCONF_DIR/$file
		dot1x_log "Backing up $file"
		dot1x_cpfile $sysconf_file $sysconf_dir/$file
		rm -rf $sysconf_file
		ln -s $LOCALSTATE_DIR/sysconf/$file $sysconf_file
		# TODO: make it writable for EGSCGI in a better way.
		# original EPS use -DBIG_SECURITY_HOLE for root apache
	done
	
	# archive var data
	dot1x_log "Archiving var image to $DOT1X_BINARY/var.tgz"
	(
	cd $DOT1X_INITRD$LOCALSTATE_DIR
	# build data storage package
	tar -zcf var.tgz --exclude=var.tgz .
	mv var.tgz $DOT1X_BINARY/var.tgz
	# build factory initialization package
	cd sysconf
	tar -zcf init.tgz --exclude=init.tgz .
	mv init.tgz $DOT1X_BINARY/init.tgz
	rm -rf ./*
	)

	# create var image
	umount $DOT1X_VARDIR 1>/dev/null 2>&1
	dot1x_mkram $DOT1X_VARIMG 16384 || return 1
	mount $DOT1X_VARIMG $DOT1X_VARDIR -o loop 1>/dev/null 2>&1
	(
	cd $DOT1X_VARDIR
	tar -zxf $DOT1X_BINARY/var.tgz
	)
	umount $DOT1X_VARDIR 1>/dev/null 2>&1

	# backup the var image file
	cp -f $DOT1X_VARIMG $DOT1X_BINARY/var.img
}

make_rootfs()
{
	# copy all needed files
	(
		cd $DOT1X_INITRD
		rm -rf ./*
		tar -zxvf $DOT1X_ROOT/pkgs/rootfs.tgz . \
			1>/dev/null 2>&1 \
		|| dot1x_log "Extracting root filesystem failure"
		for package in $PACKAGES; do
			if [ -f $DOT1X_BINARY/$package.tgz ]; then
				dot1x_log "Installing $package"
				tar -zxvf $DOT1X_BINARY/$package.tgz . \
					1>/dev/null 2>&1 \
				|| dot1x_log "Extracting $package failure"
			fi
		done
	)

	# export all initialized configuration files
	$DOT1X_ROOT/scripts/cfdot1x.sh export $DOT1X_INITRD
	
	# create ld.so.cache
	ldconfig -r $DOT1X_INITRD -v 1>/dev/null 2>&1

	# uncomment following lines to edit your root
	# do not forget to unmount the root filesystem
	#chroot $DOT1X_INITRD /bin/ash
	#exit 1

	# create /var image here
	make_varimg
}

# Create ROOT filesystem image
#   1. Restore ROOT filesystem contents
#   2. Create ROOT filesystem image
make_initrd()
{
	dot1x_log "Creating root filesystem"
	
	# create image
	umount $DOT1X_INITRD 1>/dev/null 2>&1
	dot1x_mkram $DOT1X_IMAGE 16384 || return 1
	mount $DOT1X_IMAGE $DOT1X_INITRD -o loop 1>/dev/null 2>&1

	make_rootfs

	# compress image file
	umount $DOT1X_INITRD
	if [ -f ${DOT1X_IMAGE}.gz ]; then
		rm -f ${DOT1X_IMAGE}.gz
	fi
	gzip -f $DOT1X_IMAGE

	# backup the image file
	cp -f ${DOT1X_IMAGE}.gz $DOT1X_BINARY/initrd
}

make_initramfs()
{
	dot1x_log "Creating root initramfs"
	
	[ ! -d "$DOT1X_INITRD" ] && mkdir -p $DOT1X_INITRD

	make_rootfs

	(
		cd $DOT1X_BINARY/cpio
		sh gen_initramfs_list.sh -o ${DOT1X_IMAGE}.gz $DOT1X_INITRD
	)

	# backup the image file
	cp -f ${DOT1X_IMAGE}.gz $DOT1X_BINARY/initrd
}

usage()
{
	echo "Usage:"
	echo "  $1 cf_device hd_device"
	echo "Where \"cf_device\" is your CF card device file to be written"
	echo "Where \"hd_device\" is your target system bootable devic"
	echo "Samples:"
	echo "  $1 sda hda"
	echo "  $1 hdd hdc"
}

get_root $0
source $DOT1X_ROOT/scripts/make.cfg
if [ "x$1" == "x" ]; then
	usage `basename $0`
	exit 1
fi
if [ "x$2" == "x" ]; then
	usage `basename $0`
	exit 1
fi
get_device $1 || exit 1

#make_initrd || exit 1
make_initramfs || exit 1
make_disk || exit 1

