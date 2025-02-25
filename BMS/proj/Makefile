CC := gcc
CFLAGS := -std=c++14 -Wall -Wextra -g
LDFLAGS := -lstdc++
SRC_DIR := src

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRC_FILES:.cpp=.o)

.PHONY: all clean

all: rds_encoder rds_decoder

rds_encoder: rds_encoder.cpp rds_encoder.hpp $(OBJS)
	$(CC) $(CFLAGS) -o $@ rds_encoder.cpp $(OBJS) $(LDFLAGS)

rds_decoder: rds_decoder.cpp rds_decoder.hpp $(OBJS)
	$(CC) $(CFLAGS) -o $@ rds_decoder.cpp $(OBJS) $(LDFLAGS)

%.o: %.cpp %.hpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f rds_encoder rds_decoder $(OBJS)