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

int main(int argc, char **argv) {

    //show window with input for player info
    player p;
    p.pref = malloc(sizeof(preferences));
    printf("Nickname: ");
    scanf("%32s", p.nickname);
    printf("Level (0-3): ");
    scanf("%d", &(p.pref->level));
    printf("Color (0-WHITE, 1-BLACK): ");
    scanf("%d", &(p.pref->color));

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
        if(cmd > 0 && cmd < 3) {
            cmd_msg c;
            c.mtype = CMD_MSG_TYPE;
            c.command = cmd;
            msgsnd(p.queue_id, &c, MSGSIZE(cmd_msg), 0);
            switch(cmd) {
                case 0: { //listen for games
                    break;
                }
                case 1: { //listen for game id
                    game_created_msg gc;
                    msgrcv(p.queue_id, &gc, MSGSIZE(game_created_msg), GAME_CREATED_MSG_TYPE, 0);
                    printf("Game created on queue_key %d!\n", gc.queue);
                    break;
                }
                case 2: { //logout!
                    return 0;
                }
            }
        }else if(cmd == 3) {
            printf("Game id: ");
            int gid;
            scanf("%d", &gid);
            printf("not implemented yet\n");
        }
    }

    return 0;
}