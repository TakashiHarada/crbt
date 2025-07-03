VPATH = include
CFLAGS = -I include
CC = gcc -g3 -Wall -Wextra -lrt

crbt: crbt.o
	$(CC) -g3 -Wall -Wextra crbt.o -o crbt -lm -lrt

crbt.o : crbt.c crbt.h header.h rule.h partition.h c1p.h circ1p.h graph.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o *.dSYM
