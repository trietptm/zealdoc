ARM_FLAG:=ARCH_ARM

OTHER_SRC:=vtss/vtss_regop.c

SRC:=main.c time.c $(OTHER_SRC)

all:
	gcc -g $(SRC)
arm:
	arm-linux-gcc -g $(SRC) -D$(ARM_FLAG)
tar:
	tar --no-recursion -zcf  bench.tar.gz *.c Makefile *.h vtss/*.c vtss/*.h
clean:
	rm a.out
