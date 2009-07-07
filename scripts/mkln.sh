#!/bin/sh

bins=`ls`

#str=arm-linux
#echo ${str#arm}
	#echo ${bin#arm}

for bin in ${bins}
do
	tmp="${bin#arm-unknown-linux-gnu-}"
	ln -s ./${bin} arm-linux-${tmp}
done
