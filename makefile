VPATH = include
CFLAGS = -I include
CC = gcc -g3 -Wall -Wextra -lrt

crbt: crbt.o
	gcc -g3 -Wall -Wextra crbt.o -o crbt -lm -lrt

crbt.o crbt.d : crbt.c crbt.h header.h rule.h partition.h c1p.h

clean:
	rm -rf *.o *.dSYM
