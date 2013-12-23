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
    player *players[32] = { NULL };
    int players_count = 0;
    signal(SIGINT, sigint_cleanup); //we have to be prepared for ^C

    //create messages queue
    int msgid = msgget(GLOBAL_QUEUE, IPC_CREAT | 0666);
    if(msgid < 0) {
        debug("Fatal error while creating msg queue %d: %s", msgid, strerror(errno));
        exit(0);
    }else{
        debug("Listening on msg queue (key: %d, id: %d)", GLOBAL_QUEUE, msgid);
    }
    login_msg temp_login;
    while(1) { //main event loop
        int x = msgrcv(msgid, &temp_login, login_msg_size, LOGIN_MSG_TYPE, 0);
        debug("Player with nickname %s registered.", temp_login.nickname);
    }
    return 0;
}