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

#include <signal.h>
#include <stdio.h>

#include "common.h"
#include "server/logic.h"



int main(int argc, char **argv) {
    srand(time(0));
    player *players[32] = { NULL };
    game *games[32] = { NULL };
    int internal_queues[32];
    int board[8][8][32]; //oh wait, wtf?

    //int i;
    int players_count = 0;
    signal(SIGINT, sigint_cleanup); //we have to be prepared for ^C

    //create messages queue
    int msgid = msgget(GLOBAL_QUEUE, IPC_CREAT | 0777);
    if(msgid < 0) {
        debug("Fatal error while creating msg queue %d: %s", msgid, strerror(errno));
        exit(0);
    }else{
        debug("Listening on msg queue (key: %d, id: %d)", GLOBAL_QUEUE, msgid);
    }
    login_msg temp_login;
    while(1) { //main event loop
        int status;

        //new player register
        if((status = msgrcv(msgid, &temp_login, login_msg_size, LOGIN_MSG_TYPE, IPC_NOWAIT)) >= 0) {
            debug("Player with nickname %s registered.", temp_login.nickname);
            temp_login.queue_id = msgget(temp_login.queue_id, 0777);
            if(temp_login.queue_id < 0) {
                debug("Error in queue, connection refused");
                continue;
            }
            add_new_player(players, &players_count, temp_login);
        }//end of new player connected

        int i;
        for(i = 0; i < 32; i++) {
            if(players[i] != NULL) {
                int cmd = listen_commands(players[i]);
                switch(cmd) {
                    case 0: { //list games
                        games_msg gm;
                        gm.mtype = GAMES_MSG_TYPE;
                        memcpy(&(gm.games), &games, sizeof(games));
                        msgsnd(players[i]->queue_id, &gm, MSGSIZE(games_msg), 0);
                        break;
                    }

                    case 1: { //new game
                        add_new_game(players[i], games, internal_queues);
                        break;
                    }

                    case 2: { //logout
                        shmdt(players[i]->pref);
                        debug("Player %s logged off", players[i]->nickname);
                        players[i] = NULL;
                        players_count--;
                        break;
                    }

                    default: {

                    }
                }
            }
        } //end of for listening player commands

        for(i = 0 ; i < 32; i++) { //listen on games queues
            if(games[i] == NULL)
                continue;

        }
    }//end of main event loop
    return 0;
}