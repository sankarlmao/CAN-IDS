CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -I.

all: run_sim

run_sim: main.cpp edge-impulse-sdk/classifier/ei_run_classifier.h test_buffers.h
	$(CC) $(CFLAGS) main.cpp -o run_sim

clean:
	rm -f run_sim
