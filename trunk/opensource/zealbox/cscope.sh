
#!/bin/sh

# This shell script will add `csrb`
#
#'cscope -Rb and mkdir cscope directory'

TARGET=~/.bashrc
CMD_START="csrb start"
CMD_STOP="csrb stop"
CMD="alias csrb='echo "${CMD_START}"; cscope -Rb; mkdir -p cscope; \
	mv cscope.out cscope; echo "${CMD_STOP}"'"

echo "" >> ${TARGET}
echo "# This is alias for cscope" >> ${TARGET}
echo $CMD >> ~/.bashrc
echo "" >> ${TARGET}

source ${TARGET}
