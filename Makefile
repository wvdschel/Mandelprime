CFLAGS=-std=gnu99 -Wall -Werror -O2 #-g -DVERBOSE=1

default: mandelprime

run: mandelprime
	./mandelprime > mandelprime.log
	tail -n10 mandelprime.log

mandelprime: mandelbrot.o primesieve.o main.o workqueue.o log.o refcount.o
	$(CC) -pthread -lrt -o $@ $^

valgrind: mandelprime
	valgrind ./mandelprime

clean: 
	rm -rf *.o mandelprime mandelprime.log

.PHONY: run clean valgrind default
