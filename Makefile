CC=gcc
CFLAGS=-O3
DEPS=
OBJ=update-net-widget.o
LIBS=

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

update-net-widget: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o update-net-widget
