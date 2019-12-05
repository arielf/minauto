SHELL = /bin/sh
CC = gcc
CFLAGS = -g
# CFLAGS = -B -v -n -t # GNX/CTP compiler flags...
# CFLAGS = -pg # profiling w/ gprof
# line-profiling w/ gcov
GCOVCFLAGS = --coverage
# was -fprofile-arcs -ftest-coverage in older gcc versions

OBJS = main.o  inout.o  dead.o  partit.o  ufind.o
GCFILES = *.gcno *.gcda *.gcov

all : minauto

minauto : $(OBJS)
	$(CC) $(CFLAGS) -o minauto  $(OBJS)

clean clobber:
	-rm -f *.o minauto $(GCFILES)

prof gcov:
	make clean
	make CFLAGS="$(GCOVCFLAGS)"

gcovtest:
	make gcov
	make test
	for src in *.c; do gcov $$src; done
	echo === X.gcov instrumented files created

run test check: minauto
	./all.t

