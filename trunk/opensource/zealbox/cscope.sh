
#!/bin/sh

# This shell script will add `csrb`
#COMMAND = "alias csrb='echo "cscoping ..."; cscope -Rb; mkdir -p cscope;
#	    mv cscope.out cscope; echo "cscoping finish..."'"


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
