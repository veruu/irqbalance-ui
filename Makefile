CC = gcc
DEPS = irqbalance-ui.h ui.h helpers.h
OBJ = irqbalance-ui.o ui.o helpers.o
CFLAGS = -Wall
LIBS = `pkg-config --cflags --libs glib-2.0` `pkg-config --cflags --libs ncurses`
TARGET = irqbalance-ui

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o
