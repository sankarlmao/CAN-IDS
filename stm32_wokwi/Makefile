CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -I. -I./edge-impulse-sdk/classifier

all: run_sim

run_sim: sketch.ino edge-impulse-sdk/classifier/ei_run_classifier.h test_buffers.h
	$(CC) $(CFLAGS) -x c++ sketch.ino -o run_sim

clean:
	rm -f run_sim
