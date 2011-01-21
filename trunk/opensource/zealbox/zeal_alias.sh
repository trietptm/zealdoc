
#!/bin/sh

# This is for adding `csrb`
#
#'cscope -Rb and mkdir cscope directory'

TARGET=~/.bashrc
CSRB_START="csrb start"
CSRB_STOP="csrb stop"
CSRB_CMD="alias csrb='echo "${CSRB_START}"; cscope -Rb; mkdir -p cscope; \
	mv cscope.out cscope; echo "${CSRB_STOP}"'"

echo "" >> ${TARGET}
echo "# This is alias for cscope" >> ${TARGET}
echo $CSRB_CMD >> ${TARGET}
echo "" >> ${TARGET}

# This is for adding `fxg`
#
#'find . | xargs grep'

FXG_IGNOR="-name cscope -prune -o -name .svn -prune -o -name .pc -prune -o -name CVS -prune -o -print"
FXG_CMD="alias fxg='find . ${FXG_IGNOR} | xargs grep'"

echo "# This is alias for fxg" >> ${TARGET}
echo $FXG_CMD >> ${TARGET}
echo "" >> ${TARGET}

echo "# some more XXX alias: " >> ${TARGET}
echo alias rm='rm -i' >> ${TARGET}
echo alias mv='mv -i' >> ${TARGET}
echo "" >> ${TARGET}

. ${TARGET}
