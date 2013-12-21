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
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "string.h"

#include "client/logic.h"
#include "common.h"
#include "client/view.h"

void connect_to_server() {
    int res;
    login_msg login;
    login.mtype = 0;
    strcpy(login.nickname, "mescam");
    res = msgsnd(GLOBAL_QUEUE, &login, sizeof(login_msg) - sizeof(long), 0);
    if(res < 0) {
            show_error_msg_and_exit("Error while connecting to server");
    }
}

int get_shm_key() {
    int k = shmget(IPC_PRIVATE, sizeof(preferences), 0666);
    if (k < 0) {
        show_perror_and_exit();
    }
    return k;
}