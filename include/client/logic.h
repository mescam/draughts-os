#ifndef LOGIC_H
#define LOGIC_H
#include "common.h"

void connect_to_server(login_msg m);
int get_shm_key();
login_msg generate_login_msg(player *p);
#endif