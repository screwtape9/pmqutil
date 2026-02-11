CFLAGS=-c -g -Wall
LIBS=-lrt

%.o: %.c
	$(CC) $(CFLAGS) $<

EXE=pmqu
OBJ=main.o

.PHONY: all clean rebuild

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ) $(EXE)

rebuild: clean all
