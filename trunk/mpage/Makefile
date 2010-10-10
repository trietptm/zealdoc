TXT2TAGS=txt2tags

# source and dest
# each sub-dir should add 'SRC' and 'TEMP' variables (see src/Makefile)
#
SRC_DIR=src
SRC=${SRC_DIR}/*.t2t
DEST_DIR=output
TEMP=${SRC_DIR}/*.html
include ${SRC_DIR}/Makefile

# config
CONF_FILE=txt2blog.conf
CONF_DIR=conf
CONF_FLAGS=-C ${DEST_DIR}/${CONF_FILE}

# misc flags
#OTHER_FLAGS=--toc


# TODO:
# 1. rules for specfic targets
# 2. make it configurable
#
%.html:%.t2t
	${TXT2TAGS} -t html $@

all:
	$(shell mkdir -p ${DEST_DIR})
	@echo "Generating config file"
	$(shell sed -e 's#PWD#${PWD}#g' < ${CONF_DIR}/${CONF_FILE} > ${DEST_DIR}/${CONF_FILE})
	@echo
	@echo "Compiling......"
	@${TXT2TAGS} ${CONF_FLAGS} ${OTHER_FLAGS} ${OUTPUT} ${SRC}
	@echo
	@echo "Generating output files"
	@mv ${TEMP} ${DEST_DIR}
clean:
	@rm -rf ${DEST_DIR}
