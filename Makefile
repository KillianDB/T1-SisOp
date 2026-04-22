CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11

TARGETS = threads_no_sync threads_mutex processes_no_sync processes_semaphore

all: $(TARGETS)

threads_no_sync: threads_no_sync.c
	$(CC) $(CFLAGS) -o $@ $< -pthread

threads_mutex: threads_mutex.c
	$(CC) $(CFLAGS) -o $@ $< -pthread

processes_no_sync: processes_no_sync.c
	$(CC) $(CFLAGS) -o $@ $<

processes_semaphore: processes_semaphore.c
	$(CC) $(CFLAGS) -o $@ $< -pthread

clean:
	rm -f $(TARGETS) *.o *_output.txt

.PHONY: all clean
