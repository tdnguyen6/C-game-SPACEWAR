GCC = gcc
CFLAGS = -g -Wall -mwindows
SL_H = sigil-mingw32/include/
VECTOR_H = sourcecode/
LIBS = sigil-mingw32/lib
LDFLAGS = sigil
SRC = sourcecode/spacewar.c
EXE = SpaceWar.exe

all: build

build:
	$(GCC) -o $(EXE) $(CFLAGS) $(SRC) -I$(VECTOR_H) -I$(SL_H) -L$(LIBS) -l$(LDFLAGS)

debug: CFLAGS+=-DDEBUG_ON
debug: build

clean:
	rm $(EXE)