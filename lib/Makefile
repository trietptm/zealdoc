all: dll

dll:
	@gcc -fPIC -shared -o libzc.so zc.c
install:
	@ln -sf `pwd`/libzc.so /usr/local/lib/libzc.so
clean:
	@rm -rf libzc.so
	@rm -rf /usr/local/lib/libzc.so
