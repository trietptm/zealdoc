all: dll

zcsrc := zclib.c

dll:
	@gcc -fPIC -shared -o libzc.so $(zcsrc)
install:
	@ln -sf `pwd`/libzc.so /usr/lib/libzc.so
	@ln -sf `pwd`/zclib.h /usr/include/zclib.h
	@ln -sf `pwd`/zc_sysdep.h /usr/include/zc_sysdep.h
clean:
	@rm -rf libzc.so
	@rm -rf /usr/lib/libzc.so
	@rm -rf /usr/include/zclib.h
	@rm -rf /usr/include/zc_sysdep.h

