SRC = $(shell find . -type f -regex '.*\.cpp')
SRC_C = $(shell find . -type f -regex '.*\.c')
OBJ = $(SRC:%.cpp=%.o)
OBJ_C = $(SRC_C:%.c=%.o)
BIN = kiv-ups-agarserver
INCLUDEDIRS = -Isrc/Gameplay -Isrc/Network -Isrc/System -Idep/sha1 -Idep/sqlite
LIBS = -lm -lpthread -ldl

OUTDIR = bin
OUT = $(OUTDIR)/$(BIN)

all: $(BIN)

copy_config:
	if [ ! -f $(OUTDIR)/server.cfg ]; \
	then \
		cp config/server.cfg $(OUTDIR)/server.cfg ; \
	fi

mkoutdir:
	mkdir -p $(OUTDIR)

$(BIN): mkoutdir copy_config $(OBJ) $(OBJ_C)
	g++ $(LIBS) $(OBJ_C) $(OBJ) -o $(OUT)

%.o: %.c
	gcc $(INCLUDEDIRS) -c $< -o $@

%.o: %.cpp
	g++ -std=c++11 $(INCLUDEDIRS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(OBJ_C)
