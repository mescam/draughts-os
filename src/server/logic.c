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
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"


void debug(char *msg, ...) {
    //get and format time
    char time_buff[20];
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buff, 20, "[%H:%M:%S]", timeinfo);
    //arguments for printf
    va_list args;
    char buff[1024 * 16];
    va_start(args, msg);
    //print time
    printf("%s ", time_buff);
    //print data
    vsprintf(buff, msg, args);
    //print new line
    printf("%s\n", buff);
    va_end(args);
}

void sigint_cleanup(int signum) {
    debug("SIGINT caught: doing cleanup...");
    exit(0);
}