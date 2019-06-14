GCC += gcc
CFLAGS+= -g -Wall -mwindows
INCLUDES+= c:/users/tien\ duc/desktop/sigil-mingw32/include/
LIBS+= c:/users/tien\ duc/desktop/sigil-mingw32/lib
LDFLAGS+= sigil
SRC= sourcecode/spacewar.c
EXE = SpaceWar.exe

all: build

build:
	$(GCC) -o $(EXE) $(SRC) -I. -I$(INCLUDES) $(CFLAGS) -L$(LIBS) -l$(LDFLAGS)

debug: CFLAGS+=-DDEBUG_ON
debug: build

clean:
	rm $(EXE)