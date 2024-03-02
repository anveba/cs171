CC = g++

INCLUDE = -I /usr/include/GL -I /usr/include -I vendor -I src
LNK = -lm -lGLEW -lGL -lGLU -lglut
SRC = $(shell find src/ -name *.cpp)
PRGNAME = renderer
OUT = -o $(PRGNAME)
DBG = -g
RLS = -O3 -flto
FLG = -std=c++17

all:
	$(CC) $(OUT) $(SRC) $(FLG) $(INCLUDE) $(LNK) $(DBG)

release:
	$(CC) $(OUT) $(SRC) $(FLG) $(INCLUDE) $(LNK) $(RLS)

clean:
	rm -f *.o $(PRGNAME)

.PHONY: all clean
