CC = gcc
CFLAGS = -Wall -pthread

all: voting_machine voting_client

voting_machine: voting_machine.c
	$(CC) $(CFLAGS) voting_machine.c -o voting_machine

voting_client: voting_client.c
	$(CC) $(CFLAGS) voting_client.c -o voting_client

clean:
	rm -f voting_machine voting_client