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

#include <gtk/gtk.h>

#include "common.h"
#include "client/logic.h"
#include "client/view.h"


int main(int argc, char **argv) {
    gtk_init(&argc, &argv); //initializtion of gtk module

    //show window with input for player info
    player p = show_new_player();
    g_message("Registering new user with nickname %s", p.nickname);

    //get login message
    login_msg m = generate_login_msg(p);
    //and send it
    connect_to_server(m);
    
    //main window widget
    GtkWidget *window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Draughts");
    g_signal_connect(window, "destroy", 
        G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 200);
    //show widgets

    gtk_widget_show(window);

    //main loop
    gtk_main();

    return 0;
}