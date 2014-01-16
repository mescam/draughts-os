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

void add_new_player(player *players[32], int *pcount, login_msg login) {
    status_msg status;
    status.mtype = 1;
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
    debug("Allocated player");
    strcpy(players[id]->nickname, login.nickname);
    int shmid = shmget(login.shm_pref, sizeof(preferences), 0777);
    debug("Attaching preferences");
    players[id]->pref = shmat(shmid, NULL, 0);
    players[id]->queue_id = login.queue_id;
    debug("Before read");
    debug("Preferences: %d %d", players[id]->pref->level, players[id]->pref->color);
    status.status = 0; //OK
    msgsnd(login.queue_id, &status, MSGSIZE(status_msg), 0);

    debug("Player registered on position %d", id);
    return;
}

void listen_commands(player *player) {
    int status;
    cmd_msg cmd;
    if((status = msgrcv(player->queue_id, &cmd, MSGSIZE(cmd_msg), CMD_MSG_TYPE,IPC_NOWAIT)) >= 0) {
        return cmd.command;
    }
    return -1;
}