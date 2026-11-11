CC     = gcc
CFLAGS = -O2 -Wall -Wextra
SRC    = main.c sim.c bench.c leftist.c minmax.c sortedarr.c

hospital: $(SRC) hospital.h pq.h patient.h util.h
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	rm -f hospital hospital.exe
