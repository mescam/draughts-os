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
#ifndef LOGIC_H
#define LOGIC_H
void debug(char *msg, ...);
void sigint_cleanup(int signum);
void add_new_player(player *players[32], int *pcount, login_msg login, int queue_key);
int listen_commands(player *player);
void add_new_game(player *player, game *games[32], game_state *states);
#endif