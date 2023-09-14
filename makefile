CC=g++
CFLAGS=-c -Wall -Werror -Wpedantic -std=c++14 -O3
LDFLAGS=
SOURCES=experiments.cc treap.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=treap.exe

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
