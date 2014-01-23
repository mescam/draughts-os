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
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"


void debug(char *msg, ...) {
    //get and format time
    char time_buff[20];
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buff, 20, "[%H:%M:%S]", timeinfo);
    //arguments for printf
    va_list args;
    char buff[1024 * 16];
    va_start(args, msg);
    //print time
    printf("%s ", time_buff);
    //print data
    vsprintf(buff, msg, args);
    //print new line
    printf("%s\n", buff);
    va_end(args);
}

void sigint_cleanup(int signum) {
    debug("SIGINT caught: doing cleanup...");
    exit(0);
}

void add_new_player(player *players[32], int *pcount, login_msg login, int queue_key) {
    status_msg status;
    status.mtype = STATUS_MSG_TYPE;
    //first empty space
    int id = -1;
    for(int i = 0; i < 32; i++) {
        if(players[i] == NULL) {
            id = i;
            break;
        }
    }
    if(id == -1) { //server is full
        status.status = 2; //SERVERFULL
        msgsnd(login.queue_id, &status, MSGSIZE(status_msg), 0);
        debug("Server is full");
        return;
    }
    players[id] = (player*) malloc(sizeof(player));
    (*pcount)++;
    strcpy(players[id]->nickname, login.nickname);
    int shmid = shmget(login.shm_pref, sizeof(preferences), 0777);
    players[id]->pref = shmat(shmid, NULL, 0);
    players[id]->queue_id = login.queue_id;
    players[id]->queue_key = queue_key;
    debug("Preferences: %d %d", players[id]->pref->level, players[id]->pref->color);
    status.status = 0; //OK
    msgsnd(login.queue_id, &status, MSGSIZE(status_msg), 0);

    debug("Player registered on position %d", id);
    return;
}

int is_black_field(int i, int j) {
    return (i + j) % 2;
}

void add_new_game(player *player, game *games[32], game_state *states) {
    int id = -1;
    int i, j;
    for(i = 0; i < 32; i++) {
        if(games[i] == NULL) {
            id = i;
            break;
        }
    }
    int queue_key, queue_id = -1;
    while(queue_id < 0) {
        queue_key = rand() ^ time(0);
        queue_id = msgget(queue_key, 0777 | IPC_CREAT | IPC_EXCL);
    }
    states[id].queue_id = queue_id;
    games[id] = malloc(sizeof(game));
    strcpy(games[id]->player1, "\0");
    strcpy(games[id]->player2, "\0");

    if(player->pref->color == 0) {
        strcpy(games[id]->player1, player->nickname);
        states[id].player1 = player;
    }else{
        strcpy(games[id]->player2, player->nickname);
        states[id].player2 = player;
    }

    states[id].status = 0;

    // for(i = 0; i < 8; i++) {
    //     for(j = 0; j < 8; j++) {
    //         states[id].board[i][j] = 0;
    //     }
    // }
    for(i = 0; i < 8; i++) {
        for(j = 0; j < 8; j++) {
            if(i < 3 && is_black_field(i, j))
                states[id].board[i][j] = 1;
            else if(i > 4 && is_black_field(i, j))
                states[id].board[i][j] = -1;
            else
                states[id].board[i][j] = 0;
        }
    }

    for(i = 0; i < 32; i++) {
        states[id].observers[i] = NULL;
    }
    
    games[id]->game_id = id;
    games[id]->queue_id = queue_key;
    debug("Created new game by %s with gid %d and qid %d", 
        player->nickname, games[id]->game_id, games[id]->queue_id);
    game_created_msg msg;
    msg.mtype = GAME_CREATED_MSG_TYPE;
    msg.queue = queue_key;
    msgsnd(player->queue_id, &msg, MSGSIZE(game_created_msg), 0);
}

int listen_commands(player *player) {
    int status;
    cmd_msg cmd;
    status = msgrcv(player->queue_id, &cmd, MSGSIZE(cmd_msg), CMD_MSG_TYPE, IPC_NOWAIT);
    if(status > 0) {
        return cmd.command;
    }
    return -1;
}