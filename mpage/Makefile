TXT2TAGS=txt2tags

# config
CONF_FILE=txt2blog.conf
CONF_FLAGS=-C ${CONF_FILE}

# misc flags
#OTHER_FLAGS=--toc

# source and dest
# each sub-dir should add 'SRC' and 'TEMP' variables (see src/Makefile)
#
SRC_DIR=src
SRC=${SRC_DIR}/*.t2t
DEST_DIR=output
TEMP=${SRC_DIR}/*.html
include ${SRC_DIR}/Makefile

# TODO:
# 1. rules for specfic targets
# 2. make it configurable
#
%.html:%.t2t
	${TXT2TAGS} -t html $@

all:
	${TXT2TAGS} ${CONF_FLAGS} ${OTHER_FLAGS} ${OUTPUT} ${SRC}
	mkdir -p ${DEST_DIR}
	mv ${TEMP} ${DEST_DIR}
clean:
	@rm -rf ${DEST_DIR} 
