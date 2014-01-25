/*
    Copyright (C) 2013 Jakub Woźniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef COMMON_H
#define COMMON_H
#define _BSD_SOURCE
#include <stddef.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define ANSI_COLOR_RED          "\x1b[31m"
#define ANSI_COLOR_GREEN        "\x1b[32m"
#define ANSI_COLOR_YELLOW       "\x1b[33m"
#define ANSI_COLOR_BLUE         "\x1b[34m"
#define ANSI_COLOR_MAGENTA      "\x1b[35m"
#define ANSI_COLOR_CYAN         "\x1b[36m"
#define ANSI_COLOR_RESET        "\x1b[0m"

#define ANSI_BG_COLOR_BLACK     "\x1b[40m"
#define ANSI_BG_COLOR_WHITE     "\x1b[47m"
    

#define WHITE_PIECE             "\u26C0"
#define BLACK_PIECE             "\u26C2"

#define GLOBAL_QUEUE            42
#define LOGIN_MSG_TYPE          1
#define STATUS_MSG_TYPE         1
#define CMD_MSG_TYPE            2
#define GAMES_MSG_TYPE          3
#define CLIENT_MOVE_MSG_TYPE    4
#define GAME_START_MSG_TYPE     5
#define GAME_JOIN_MSG_TYPE      6
#define MOVE_MADE_MSG_TYPE      6
#define OBSERVER_JOIN_MSG_TYPE  7
#define OBSERVER_LEFT_MSG_TYPE  8
#define GAME_END_MSG_TYPE       9
#define GAME_CREATED_MSG_TYPE   10
#define login_msg_size  sizeof(login_msg) - sizeof(long)
#define MSGSIZE(X)  sizeof(X)-sizeof(long)

typedef struct _login_msg {
    long mtype;
    char nickname[32];
    int queue_id;
    int shm_pref;
} login_msg;

typedef struct _game_end_msg {
    long mtype;
    int win;
} game_end_msg;

typedef struct _observer_left_msg {
    long mtype;
    char nickname[32];
    int queue_id;
} observer_left_msg;

typedef struct _observer_join_msg {
    long mtype;
    int board[8][8];
} observer_join_msg;

typedef struct _move_made_msg {
    long mtype;
    int from_y;
    int from_x;
    int to_y;
    int to_x;
    int pawn_removed_count;
    int pawn_removed[16][2];
} move_made_msg;

typedef struct _game_start_msg {
    long mtype;
    int first;
} game_start_msg;

typedef struct _game_created_msg {
    long mtype;
    int queue;
} game_created_msg;

typedef struct _preferences {
    int level;
    int color;
} preferences;

typedef struct _player {
    char nickname[32];
    preferences *pref;
    int queue_id;
    int queue_key;
} player;

typedef struct _game_state {
    int board[8][8];
    int status;
    player *player1;
    player *player2;
    player *observers[32]; 
    int queue_id;
} game_state;

typedef struct _game {
    char player1[32];
    char player2[32];
    int game_id;
    int queue_id;
} game;

typedef struct _games_msg {
    long mtype;
    game games[32];
} games_msg;

typedef struct _status_msg {
    long mtype;
    int status;
} status_msg;

typedef struct _cmd_msg {
    long mtype;
    int command;
} cmd_msg;

typedef struct _game_join_msg {
    long mtype;
    char nickname[32];
    int queue_id;
} game_join_msg;

#endif