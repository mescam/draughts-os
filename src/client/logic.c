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

#include "common.h"
#include "client/logic.h"
#include "client/view.h"

void connect_to_server(login_msg m) {
    int msgid = msgget(GLOBAL_QUEUE, 0666);
    int res = msgsnd(msgid, &m, login_msg_size, 0);
    if(res < 0) {
            perror(NULL);
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

login_msg generate_login_msg(player p) {
    login_msg m;
    m.mtype = LOGIN_MSG_TYPE;
    //write nickname
    strcpy(m.nickname, p.nickname);
    //create queue
    m.queue_id = getpid();
    p.queue_id = msgget(m.queue_id, IPC_CREAT | 0777);
    if(m.queue_id < 0) {
        printf("%d\n", m.queue_id);
        perror(NULL);
        show_perror_and_exit();
    }
    //shared memory for preferences
    int shm_key = getpid();
    int shmid = shmget(shm_key, sizeof(preferences), IPC_CREAT | 0777);
    if(shmid < 0) {
        show_error_msg_and_exit("Error while creating shm");
    }
    preferences *pref = (preferences*) shmat(shmid, NULL, 0);
    memcpy(pref, &(p.pref), sizeof(preferences));
    m.shm_pref = shm_key;

    return m;
}