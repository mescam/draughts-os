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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "client/view.h"

void show_perror_and_exit() {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL,
                                    GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_OK,
                                    "%s", 
                                    strerror(errno));
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    exit(1);
}

player show_new_player() {
    GtkBuilder *builder;
    GtkDialog *window;
    GtkEntry *nickname;
    player p;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "assets/new_player.glade", NULL);
    window = GTK_DIALOG(gtk_builder_get_object(builder, "window"));
    nickname = GTK_ENTRY(gtk_builder_get_object(builder, "entry1"));
    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(G_OBJECT(builder));
    gtk_dialog_run(window);
    strcpy(p.nickname, gtk_entry_get_text(nickname));

    //TODO: get preferences from window

    return p;
}
