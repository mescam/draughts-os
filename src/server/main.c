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

player *players[32] = { NULL };
game *games[32] = { NULL };
game_state game_states[32];
int players_count = 0;

int get_game_id_by_player(player *p) {
    int i;
    for(i = 0; i < 32; i++) {
        if(games[i] == NULL)
            continue;
        if(game_states[i].player1 == p || game_states[i].player2 == p)
            return i;
    }
    return -1;
}

player *get_player(int queue_key) {
    int i;
    for(i = 0; i < 32; i++) {
        if(players[i] != NULL && players[i]->queue_key == queue_key)
            return players[i];
    }
    return NULL;
}

void add_observer(int i, player *p) {
    int j;
    for(j = 0; j < 32; j++) {
        if(game_states[i].observers[j] != NULL)
            break;
    }
    game_states[i].observers[j] = p;

    //send message about current board state
    observer_join_msg oj;
    oj.mtype = OBSERVER_JOIN_MSG_TYPE;
    memcpy(oj.board, game_states[i].board, sizeof(int)*64);
    msgsnd(p->queue_id, &oj, MSGSIZE(observer_join_msg), 0);
}

void init_game(int i) {
    game_start_msg gsm1, gsm2;
    gsm1.mtype = gsm2.mtype = GAME_START_MSG_TYPE;
    gsm1.first = 1; gsm2.first = 0;
    msgsnd(game_states[i].player1->queue_id, &gsm1, MSGSIZE(game_start_msg), 0);
    msgsnd(game_states[i].player2->queue_id, &gsm2, MSGSIZE(game_start_msg), 0);
}

void listen_game(int i) {
    int status;
    game_join_msg gjm;
    status = msgrcv(game_states[i].queue_id, &gjm, MSGSIZE(game_join_msg), GAME_JOIN_MSG_TYPE, IPC_NOWAIT);
    if(status > 0) {
        player *p = get_player(gjm.queue_id);
        debug("New player %s in game %d", p->nickname, i);
        debug("Game status is: %d", game_states[i].status);
        if(game_states[i].status == 0 && strlen(games[i]->player1) == 0) {
            strcpy(games[i]->player1, gjm.nickname);
            game_states[i].status = 1;
            game_states[i].player1 = p;
            init_game(i);
            return;
        }
        if(game_states[i].status == 0 && strlen(games[i]->player2) == 0) {
            strcpy(games[i]->player2, gjm.nickname);
            game_states[i].status = 1;
            game_states[i].player2 = p;
            init_game(i);
            return;
        }
        if(game_states[i].status == 1) {
            add_observer(i, p);
        }
    }
}

int main(int argc, char **argv) {
    //printf("%d", MSGSIZE(move_made_msg));
    srand(time(0));
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
            int queue_key = temp_login.queue_id;
            temp_login.queue_id = msgget(temp_login.queue_id, 0777);
            if(temp_login.queue_id < 0) {
                debug("Error in queue, connection refused");
                continue;
            }
            add_new_player(players, &players_count, temp_login, queue_key);
        }//end of new player connected

        int i;
        for(i = 0; i < 32; i++) { //listen commands from players
            if(players[i] != NULL) {
                int cmd = listen_commands(players[i]);
                switch(cmd) {
                    case 0: { //list games
                        games_msg gm;
                        gm.mtype = GAMES_MSG_TYPE;
                        int k, j = 0;
                        for(k = 0; k < 32; k++) {
                            if(games[k] != NULL) {
                                if(strlen(games[k]->player1) == 0 && players[i]->pref->color == 0 
                                    && players[i]->pref->level == game_states[k].player2->pref->level) {
                                    memcpy(gm.games+j, games[k], sizeof(game));
                                    j++;
                                }
                                if(strlen(games[k]->player2) == 0 && players[i]->pref->color == 1
                                    && players[i]->pref->level == game_states[k].player1->pref->level) {
                                    memcpy(gm.games+j, games[k], sizeof(game));
                                    j++;
                                }
                                if(strlen(games[k]->player1) != 0 && strlen(games[k]->player2) != 0) {
                                    memcpy(gm.games+j, games[k], sizeof(game));
                                    j++;
                                }
                            }
                        }
                        for(; j < 32; j++) {
                            gm.games[j].game_id = -1;
                            gm.games[j].queue_id = -1;
                            strcpy(gm.games[j].player1, "\0");
                            strcpy(gm.games[j].player2, "\0");
                        }
                        msgsnd(players[i]->queue_id, &gm, MSGSIZE(games_msg), 0);
                        break;
                    } //end of list games

                    case 1: { //new game
                        add_new_game(players[i], games, game_states);
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
            listen_game(i);
        } //end of listen games
        
        for(i = 0; i < 32; i++) { //listen for moves
            if(players[i] == NULL)
                continue;
            move_made_msg mm;
            int status;
            status = msgrcv(players[i]->queue_id, &mm, MSGSIZE(move_made_msg), CLIENT_MOVE_MSG_TYPE, IPC_NOWAIT);
            if(status <= 0)
                continue;
            int g_id = get_game_id_by_player(players[i]);
            mm.mtype = MOVE_MADE_MSG_TYPE;
            debug("Got move from game %d", g_id);
            if(game_states[g_id].player1 == players[i])
                msgsnd(game_states[g_id].player2->queue_id, &mm, MSGSIZE(move_made_msg), 0);
            else
                msgsnd(game_states[g_id].player1->queue_id, &mm, MSGSIZE(move_made_msg), 0);
            int j;
            for(j = 0; j < 32; j++) {
                if(game_states[g_id].observers[j] == NULL)
                    continue;
                msgsnd(game_states[g_id].observers[j]->queue_id, &mm, MSGSIZE(move_made_msg), 0);
            }
            //update our own map
            int piece = game_states[g_id].board[mm.from_x][mm.from_y];
            game_states[g_id].board[mm.from_x][mm.from_y] = 0;
            game_states[g_id].board[mm.to_x][mm.to_y] = piece;
            for(i = 0; i < mm.pawn_removed_count; i++) {
                game_states[g_id].board[mm.pawn_removed[i][1]][mm.pawn_removed[i][0]] = 0;
            } 
        }

        sleep(1);
    }//end of main event loop
    return 0;
}