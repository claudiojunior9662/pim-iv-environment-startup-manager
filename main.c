#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>

GtkBuilder *builder;
GtkWidget *window;

void on_main_window_destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void advice(char text[100], char secondaryText[50], char iconName[50])
{
    GtkMessageDialog *dialog = gtk_builder_get_object(builder, "advices_message_dialog");

    g_object_set(dialog, "text", text, NULL);
    g_object_set(dialog, "secondary_text", secondaryText, NULL);
    g_object_set(dialog, "icon_name", iconName, NULL);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(dialog);
    gtk_widget_hide(dialog);
}

void login(char *username, char *password)
{
    if (g_strcmp0(username, "admin") == 0 && g_strcmp0(password, "admin") == 0)
    {
        advice("Bem vindo!", "Bem vindo, admin.", "emblem-default");
    }
    else
    {
        // Login failed
    }
}

void on_login_btn_clicked(GtkWidget *widget, gpointer data)
{
    GObject *username = gtk_builder_get_object(builder, "user_login_entry");
    GObject *password = gtk_builder_get_object(builder, "user_password_entry");
    login(gtk_entry_get_text(GTK_ENTRY(username)), gtk_entry_get_text(GTK_ENTRY(password)));
}

void on_user_create_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_user_list_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_logout_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_user_create_back_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_user_create_confirm_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_user_list_back_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_user_list_bt_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("glade-project.glade");

    gtk_builder_add_callback_symbols(
        builder,
        "on_main_window_destroy", G_CALLBACK(on_main_window_destroy),
        "on_login_btn_clicked", G_CALLBACK(on_login_btn_clicked),
        "on_user_create_btn_clicked", G_CALLBACK(on_user_create_btn_clicked),
        "on_user_list_btn_clicked", G_CALLBACK(on_user_list_btn_clicked),
        "on_logout_btn_clicked", G_CALLBACK(on_logout_btn_clicked),
        "on_user_create_back_btn_clicked", G_CALLBACK(on_user_create_back_btn_clicked),
        "on_user_create_confirm_btn_clicked", G_CALLBACK(on_user_create_confirm_btn_clicked),
        "on_user_list_back_btn_clicked", G_CALLBACK(on_user_list_back_btn_clicked),
        "on_user_list_bt_clicked", G_CALLBACK(on_user_list_bt_clicked),
        NULL);

    gtk_builder_connect_signals(builder, NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));

    // g_signal_connect(window, "destroy", G_CALLBACK(on_main_window_destroy), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}