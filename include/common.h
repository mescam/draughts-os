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
#include <stddef.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define GLOBAL_QUEUE    42
#define LOGIN_MSG_TYPE  1
#define login_msg_size  sizeof(login_msg) - sizeof(long)

typedef struct _login_msg {
    long mtype;
    char nickname[32];
    int queue_id;
    int shm_pref;
} login_msg;

typedef struct _preferences {
    int level;
    int color;
} preferences;

typedef struct _player {
    char nickname[32];
    preferences pref;
} player;



#endif