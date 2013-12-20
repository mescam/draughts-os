SHELL		:=	/bin/bash
CC			=	gcc

CFLAGS_CL	=	-Wall `pkg-config --cflags gtk+-3.0` -Iinclude/ -c
LDFLAGS_CL	=	`pkg-config --libs gtk+-3.0`

CFLAGS_SE	=	-Wall -Iinclude/ -c
LDFLAGS_SE	=	

CLIENT_SRC 	=	$(wildcard src/client/*.c)
SERVER_SRC	=	$(wildcard src/server/*.c)

CLIENT_OBJ	=	$(CLIENT_SRC:src/client/%.c=obj/client/%.o)
SERVER_OBJ	=	$(SERVER_SRC:src/server/%.c=obj/server/%.o)

all: client server

client: $(CLIENT_OBJ)
	$(CC) $(CLIENT_OBJ) -o bin/draughts-client $(LDFLAGS_CL)

$(CLIENT_OBJ): obj/client/%.o : src/client/%.c
	@$(CC) $(CFLAGS_CL) -c $< -o $@ 

server: $(SERVER_OBJ)
	$(CC) $(SERVER_OBJ) -o bin/draughts-server $(LDFLAGS_SE)

$(SERVER_OBJ): obj/server/%.o : src/server/%.c
	@$(CC) $(CFLAGS_SE) -c $< -o $@ 

clean:
	rm -f obj/client/*
	rm -f obj/server/*
	rm -f bin/draughts-client
	rm -f bin/draughts-server

.PHONY: all clean