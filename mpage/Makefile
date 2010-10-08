TXT2TAGS=txt2tags
CONF_FILE=txt2blog.conf
CONF_FLAGS=-C ${CONFIG_FILE}
OTHER_FLAGS=--toc
SRC=src/*.t2t

all:
	${TXT2TAGS} ${CONF_FLAGS} ${OTHER_FLAGS} ${OUTPUT} ${SRC}
clean:
	rm src/*.html
