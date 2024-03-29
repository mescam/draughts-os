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
#include "common.h"
#include "client/logic.h"

player p;
int myColor;
volatile int inGame;
int _gid;

void endgame_as_player() {
    printf("You've decided to end the game.\n");
    printf("Please type who is the winner (1 - white, -1 - black): ");
    game_end_msg ge;
    ge.mtype = GAME_END_MSG_TYPE;
    scanf("%d", &(ge.win));
    msgsnd(_gid, &ge, MSGSIZE(game_end_msg), 0);
}

void signal_left_game_as_observer(int signum) {
    inGame = 0;
}

void wait_for_opponent() {
    game_start_msg gsm;
    printf("Waiting for the opponent...\n");
    msgrcv(p.queue_id, &gsm, MSGSIZE(game_start_msg), GAME_START_MSG_TYPE, 0);
    printf("Opponent joined. My color is %s\n", (gsm.first) ? "white" : "black");
    myColor = !gsm.first;
}

int is_black_field(int i, int j) {
    return ((i + j) % 2 == 0);
}

void print_board(int board[][8]) {
    int i, j;
    printf("    0   1   2   3   4   5   6   7 \n");
    printf("  +---+---+---+---+---+---+---+---+\n");
    for(i = 0; i < 8; i++) {
        printf("%d ", i);
        for(j = 0; j < 8; j++) {
            printf("|");
            if(is_black_field(i,j)) 
                printf(ANSI_BG_COLOR_BLACK);
            else
                printf(ANSI_BG_COLOR_WHITE);
            if(board[i][j] == 1) printf(" " WHITE_PIECE " ");
            else if(board[i][j] == -1) printf(" " BLACK_PIECE " ");
            else printf("   ");
            printf(ANSI_COLOR_RESET);
        }
        printf("|\n");
        printf("  +---+---+---+---+---+---+---+---+\n");
    }
    printf("Note, that colors may vary depending on your settings\n");
    printf("White piece - " WHITE_PIECE ", black piece - " BLACK_PIECE "\t");
}

void game_loop_player(int gid) {
    _gid = gid;
    inGame = 1;
    //signal(SIGINT, signal_endgame_as_player);
    int board[8][8];
    int i, j;
    int myTurn = (myColor == 0) ? 1 : 0;

    //fill board with initial piece state
    for(i = 0; i < 8; i++) {
        for(j = 0; j < 8; j++) {
            if(i < 3 && is_black_field(i, j))
                board[i][j] = 1;
            else if(i > 4 && is_black_field(i, j))
                board[i][j] = -1;
            else
                board[i][j] = 0;
        }
    }

    while(inGame) {
        system("clear");
        print_board(board);
        printf("Your color: %s\n",
            (myColor == 0) ? "white" : "black");

        move_made_msg mm;
        if(myTurn) {
            mm.mtype = CLIENT_MOVE_MSG_TYPE;
            printf("It is your turn now. What is your command (move, end): ");
            char cmd[8];
            scanf("%s", cmd);
            if(strcmp(cmd, "end") == 0) {
                endgame_as_player();
                game_end_msg ge;
                if(msgrcv(p.queue_id, &ge, MSGSIZE(game_end_msg), GAME_END_MSG_TYPE, 0) > 0) {
                    printf("Game has ended! The winner is %s!\n", (ge.win == 1) ? "white" : "black");
                    inGame = 0;
                    break;
                }
                break;
            }
            printf("Give x (horizontal), y (vertical) of piece to move (eg. 0 1): ");
            scanf("%d %d", &(mm.from_y), &(mm.from_x));
            printf("Give x, y of destination point: ");
            scanf("%d %d", &(mm.to_y), &(mm.to_x));
            printf("How many pieces have you beaten? ");
            scanf("%d", &(mm.pawn_removed_count));
            for(i = 0; i < mm.pawn_removed_count; i++) {
                printf("- x, y of %d. pawn: ", i);
                scanf("%d %d", &(mm.pawn_removed[i][0]), &(mm.pawn_removed[i][1]));
            }
            msgsnd(p.queue_id, &mm, MSGSIZE(move_made_msg), 0);
        }else{
            printf("Enemy's turn, wait...\n");
            while(1) {
                if(msgrcv(p.queue_id, &mm, MSGSIZE(move_made_msg), MOVE_MADE_MSG_TYPE, IPC_NOWAIT) > 0) {
                    break;
                }
                game_end_msg ge;
                if(msgrcv(p.queue_id, &ge, MSGSIZE(game_end_msg), GAME_END_MSG_TYPE, IPC_NOWAIT) > 0) {
                    printf("Game has ended! The winner is %s!\n", (ge.win == 1) ? "white" : "black");
                    inGame = 0;
                    break;
                }
            }
        }
        if(!inGame)
            break;
        int piece = board[mm.from_x][mm.from_y];
        board[mm.from_x][mm.from_y] = 0;
        board[mm.to_x][mm.to_y] = piece;
        for(i = 0; i < mm.pawn_removed_count; i++) {
            board[mm.pawn_removed[i][1]][mm.pawn_removed[i][0]] = 0;
        }
        myTurn = !myTurn;
    }
    inGame = 0;
}

void game_loop_observer(int gid, int board[][8]) {
    int i;
    inGame = 1;
    _gid = gid;
    signal(SIGINT, signal_left_game_as_observer);
    while(inGame) {
        system("clear");
        printf("OBSERVER MODE\n");
        print_board(board);
        printf("\nGame is in progress...\tPress ^C to quit\n");
        move_made_msg mm;
        while(1) {
            if(msgrcv(p.queue_id, &mm, MSGSIZE(move_made_msg), MOVE_MADE_MSG_TYPE, IPC_NOWAIT) > 0) {
                break;
            }
            game_end_msg ge;
            if(msgrcv(p.queue_id, &ge, MSGSIZE(game_end_msg), GAME_END_MSG_TYPE, IPC_NOWAIT) > 0) {
                printf("Game has ended! The winner is %s!\n", (ge.win == 1) ? "white" : "black");
                inGame = 0;
                break;
            }
            if(!inGame) {
                printf("\n\n*** Leaving observer mode (gid: %d)\n\n", gid);
                observer_left_msg ol;
                ol.mtype = OBSERVER_LEFT_MSG_TYPE;
                strcpy(ol.nickname, p.nickname);
                ol.queue_id = p.queue_key;
                printf("-----> %lu %d\n", MSGSIZE(observer_left_msg), OBSERVER_LEFT_MSG_TYPE);
                if(msgsnd(gid, &ol, MSGSIZE(observer_left_msg), 0) < 0) {
                    perror("Observer left error");    
                }
                break;
            }
        }
        if(!inGame)
            break;
        int piece = board[mm.from_x][mm.from_y];
        board[mm.from_x][mm.from_y] = 0;
        board[mm.to_x][mm.to_y] = piece;
        for(i = 0; i < mm.pawn_removed_count; i++) {
            board[mm.pawn_removed[i][1]][mm.pawn_removed[i][0]] = 0;
        }
    }
    signal(SIGINT, SIG_DFL);
}

void join_game(int g_queue) {
    int gid = msgget(g_queue, 0777);
    if(gid < 0) {
        printf("It is not a queue\n");
        return;
    }
    //send join msg
    game_join_msg gj;
    gj.mtype = GAME_JOIN_MSG_TYPE;
    strcpy(gj.nickname, p.nickname);
    gj.queue_id = p.queue_key;
    msgsnd(gid, &gj, MSGSIZE(game_join_msg), 0);

    while(1) { //wait for answer
        game_start_msg gs; gs.mtype = GAME_START_MSG_TYPE;
        observer_join_msg oj; oj.mtype = OBSERVER_JOIN_MSG_TYPE;
        int status;
        status = msgrcv(p.queue_id, &gs, MSGSIZE(game_start_msg), GAME_START_MSG_TYPE, IPC_NOWAIT);
        if(status > 0) {
            printf("Game started!\n");
            myColor = !gs.first;
            game_loop_player(gid);
            break;
        }
        status = msgrcv(p.queue_id, &oj, MSGSIZE(observer_join_msg), OBSERVER_JOIN_MSG_TYPE, IPC_NOWAIT);
        if(status > 0) {
            printf("Joined as observer\n");
            game_loop_observer(gid, oj.board);
            break;
        }
    }

    //return gid;
}

int main(int argc, char **argv) {
    p.pref = malloc(sizeof(preferences));
    printf("Nickname: ");
    scanf("%32s", p.nickname);
    do {
        printf("Level (0-3): ");
        scanf("%d", &(p.pref->level));    
    } while(p.pref->level > 3 && p.pref->level < 0);
    do {
        printf("Color (0-WHITE, 1-BLACK): ");
        scanf("%d", &(p.pref->color));
    } while(p.pref->color < 0 && p.pref->color > 1);
    
    printf("Registering new user with nickname %s\n", p.nickname);

    //get login message
    login_msg m = generate_login_msg(&p);
    //and send it
    connect_to_server(m);

    status_msg status;
    msgrcv(p.queue_id, &status, MSGSIZE(status_msg), STATUS_MSG_TYPE, 0);
    switch(status.status) {
        case 0: {
            printf("Logged in!\n");
            break;
        }
        case 1: {
            printf("Nickname is used by someone else\n");
            exit(0);
            break;
        }
        case 2: {
            printf("Server is full\n");
            exit(0);
            break;
        }
    }

    while(1) {
        int cmd;
        printf("What is your command?\n(0 - list games, 1 - new game, 2 - logout, 3 - join game)\nCommand: ");
        scanf("%d", &cmd);
        if(cmd >= 0 && cmd < 3) {
            cmd_msg c;
            c.mtype = CMD_MSG_TYPE;
            c.command = cmd;
            msgsnd(p.queue_id, &c, MSGSIZE(cmd_msg), 0);
            switch(cmd) {
                case 0: { //listen for games
                    games_msg gm;
                    msgrcv(p.queue_id, &gm, MSGSIZE(games_msg), GAMES_MSG_TYPE, 0);
                    int i;
                    printf("Available games:\n");
                    for(i = 0; i < 32; i++) {
                        if(gm.games[i].game_id == -1)
                            break;
                        printf("%d | %s %s | queue id: %d\n", 
                            gm.games[i].game_id, gm.games[i].player1, 
                            gm.games[i].player2, gm.games[i].queue_id);
                    }
                    printf("----------------\n");
                    break;
                }
                case 1: { //listen for game id
                    game_created_msg gc;
                    msgrcv(p.queue_id, &gc, MSGSIZE(game_created_msg), GAME_CREATED_MSG_TYPE, 0);
                    printf("Game created on queue_key %d!\n", gc.queue);
                    wait_for_opponent();
                    int gid = msgget(gc.queue, 0777);
                    game_loop_player(gid);
                    break;
                }
                case 2: { //logout!
                    sleep(2);
                    shmdt(p.pref);
                    shmctl(getpid(), IPC_RMID, 0);
                    msgctl(getpid(), IPC_RMID, 0);
                    return 0;
                }
            }
        }else if(cmd == 3) {
            printf("Please type game queue: ");
            int gid;
            scanf("%d", &gid);
            join_game(gid);
        }
    }

    return 0;
}