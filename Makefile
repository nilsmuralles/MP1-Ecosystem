CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =
SRCS = main.c simulation.c plants_herbivores.c carnivores_interactions.c
TARGET = ecosim

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    CC = clang
    LIBOMP_PREFIX := $(shell brew --prefix libomp 2>/dev/null)
    CFLAGS += -Xpreprocessor -fopenmp -I$(LIBOMP_PREFIX)/include
    LDFLAGS += -L$(LIBOMP_PREFIX)/lib -lomp
else
    CFLAGS += -fopenmp
endif

$(TARGET): $(SRCS) ecosystem.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET) results.txt
