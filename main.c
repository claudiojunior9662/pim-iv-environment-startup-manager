#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

GtkBuilder *builder;
GtkWidget *window;
GtkStack *stack;

char *logged_username;

// Funções para gerenciamento de usuários
int get_next_user_id()
{
    FILE *file = fopen("users.txt", "r");
    int last_id = 0;
    int id;
    char username[50], password[50];

    if (file == NULL)
    {
        return 1; // Primeiro usuário
    }

    while (fscanf(file, "%d,%49[^,],%49s\n", &id, username, password) == 3)
    {
        if (id > last_id)
        {
            last_id = id;
        }
    }

    fclose(file);
    return last_id + 1;
}

bool user_exists(const char *username)
{
    FILE *file = fopen("users.txt", "r");
    if (file == NULL)
    {
        return false;
    }

    int id;
    char stored_username[50], stored_password[50];

    while (fscanf(file, "%d,%49[^,],%49s\n", &id, stored_username, stored_password) == 3)
    {
        if (g_strcmp0(username, stored_username) == 0)
        {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

void add_user(const char *username, const char *password)
{
    int new_id = get_next_user_id();

    FILE *file = fopen("users.txt", "a");
    if (file == NULL)
    {
        return;
    }

    fprintf(file, "%d,%s,%s\n", new_id, username, password);
    fclose(file);
}

bool validate_user(const char *username, const char *password)
{
    FILE *file = fopen("users.txt", "r");
    if (file == NULL)
    {
        return false;
    }

    int id;
    char stored_username[50], stored_password[50];

    while (fscanf(file, "%d,%49[^,],%49s\n", &id, stored_username, stored_password) == 3)
    {
        if (g_strcmp0(username, stored_username) == 0 && g_strcmp0(password, stored_password) == 0)
        {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

void load_users_to_treeview()
{
    GtkListStore *store = GTK_LIST_STORE(gtk_builder_get_object(builder, "user_list_store"));
    GtkTreeIter iter;

    if (!store)
    {
        printf("DEBUG: Erro ao obter user_list_store\n");
        return;
    }

    // Limpa o store antes de adicionar novos dados
    gtk_list_store_clear(store);

    FILE *file = fopen("users.txt", "r");
    if (file == NULL)
    {
        printf("DEBUG: Arquivo users.txt não encontrado\n");
        return;
    }

    int id;
    char username[50], password[50];

    printf("DEBUG: Carregando usuários...\n");
    while (fscanf(file, "%d,%49[^,],%49s\n", &id, username, password) == 3)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, id,       // Coluna 0: ID
                           1, username, // Coluna 1: Username
                           -1);
        printf("DEBUG: Usuário carregado - ID: %d, Username: %s\n", id, username);
    }

    fclose(file);
    printf("DEBUG: Carregamento de usuários concluído\n");
}

// Funções para criptografia simples (XOR com chave)
void simple_encrypt_decrypt(char *data, const char *key)
{
    int key_len = strlen(key);
    int data_len = strlen(data);

    for (int i = 0; i < data_len; i++)
    {
        data[i] ^= key[i % key_len];
    }
}

// Funções para gerenciamento de clientes
int get_next_client_id()
{
    FILE *file = fopen("clients.txt", "r");
    int last_id = 0;
    char line[1024];

    if (file == NULL)
    {
        return 1; // Primeiro cliente
    }

    while (fgets(line, sizeof(line), file))
    {
        int id;
        if (sscanf(line, "%d,", &id) == 1)
        {
            if (id > last_id)
            {
                last_id = id;
            }
        }
    }

    fclose(file);
    return last_id + 1;
}

bool client_exists_by_cnpj(const char *cnpj)
{
    FILE *file = fopen("clients.txt", "r");
    if (file == NULL)
    {
        return false;
    }

    char line[1024];
    char encrypted_cnpj[50];
    strcpy(encrypted_cnpj, cnpj);
    simple_encrypt_decrypt(encrypted_cnpj, "clientkey123"); // Criptografa para comparar

    while (fgets(line, sizeof(line), file))
    {
        // Pula o ID e vai para o CNPJ (posição 4 no CSV)
        char *token = strtok(line, ",");
        for (int i = 0; i < 3 && token != NULL; i++)
        {
            token = strtok(NULL, ",");
        }

        if (token != NULL && g_strcmp0(encrypted_cnpj, token) == 0)
        {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

void add_client(const char *name, const char *responsible, const char *company_name,
                const char *cnpj, const char *corporate_reason, const char *fantasy,
                const char *phone, const char *mail, const char *opening_date,
                const char *street, const char *number, const char *neighborhood,
                const char *city, const char *state, const char *postal_code)
{

    int new_id = get_next_client_id();

    FILE *file = fopen("clients.txt", "a");
    if (file == NULL)
    {
        return;
    }

    // Criptografa dados sensíveis
    char encrypted_cnpj[50], encrypted_phone[50];
    strcpy(encrypted_cnpj, cnpj);
    strcpy(encrypted_phone, phone);
    simple_encrypt_decrypt(encrypted_cnpj, "clientkey123");
    simple_encrypt_decrypt(encrypted_phone, "clientkey123");

    // Formato CSV: ID,name,responsible,company_name,encrypted_cnpj,corporate_reason,fantasy,encrypted_phone,mail,opening_date,street,number,neighborhood,city,state,postal_code
    fprintf(file, "%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            new_id, name, responsible, company_name, encrypted_cnpj, corporate_reason,
            fantasy, encrypted_phone, mail, opening_date, street, number,
            neighborhood, city, state, postal_code);

    fclose(file);
}

void load_clients_to_treeview()
{
    GtkListStore *store = GTK_LIST_STORE(gtk_builder_get_object(builder, "client_list_store"));
    GtkTreeIter iter;

    if (!store)
    {
        printf("DEBUG: Erro ao obter client_list_store\n");
        return;
    }

    // Limpa o store antes de adicionar novos dados
    gtk_list_store_clear(store);

    FILE *file = fopen("clients.txt", "r");
    if (file == NULL)
    {
        printf("DEBUG: Arquivo clients.txt não encontrado\n");
        return;
    }

    char line[1024];
    printf("DEBUG: Carregando clientes...\n");

    while (fgets(line, sizeof(line), file))
    {
        // Remove quebra de linha
        line[strcspn(line, "\n")] = 0;

        // Parse CSV: ID,name,responsible,company_name,encrypted_cnpj,corporate_reason,fantasy,encrypted_phone,mail,opening_date,street,number,neighborhood,city,state,postal_code
        char *tokens[16];
        int token_count = 0;
        char *token = strtok(line, ",");

        while (token != NULL && token_count < 16)
        {
            tokens[token_count++] = token;
            token = strtok(NULL, ",");
        }

        if (token_count >= 9)
        { // Pelo menos até o email
            int id = atoi(tokens[0]);
            char *name = tokens[1];
            char *responsible = tokens[2];
            char *company_name = tokens[3];
            char *encrypted_cnpj = tokens[4];
            char *mail = tokens[8];

            // Descriptografa CNPJ para exibição
            char decrypted_cnpj[50];
            strcpy(decrypted_cnpj, encrypted_cnpj);
            simple_encrypt_decrypt(decrypted_cnpj, "clientkey123");

            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                               0, id,             // Coluna 0: ID
                               1, name,           // Coluna 1: Nome
                               2, responsible,    // Coluna 2: Responsável
                               3, company_name,   // Coluna 3: Razão Social
                               4, decrypted_cnpj, // Coluna 4: CNPJ (descriptografado)
                               5, mail,           // Coluna 5: Email
                               -1);
            printf("DEBUG: Cliente carregado - ID: %d, Nome: %s, CNPJ: %s\n", id, name, decrypted_cnpj);
        }
    }

    fclose(file);
    printf("DEBUG: Carregamento de clientes concluído\n");
}

void on_main_window_destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void advice(char text[100], char secondaryText[50], char iconName[50])
{
    GtkMessageDialog *dialog = GTK_MESSAGE_DIALOG(gtk_builder_get_object(builder, "advices_message_dialog"));

    g_object_set(dialog, "text", text, NULL);
    g_object_set(dialog, "secondary_text", secondaryText, NULL);
    g_object_set(dialog, "icon_name", iconName, NULL);

    gtk_widget_show_all(GTK_WIDGET(dialog)); // CORREÇÃO: Cast
    gtk_dialog_run(GTK_DIALOG(dialog));      // CORREÇÃO: Cast
    gtk_widget_hide(GTK_WIDGET(dialog));     // CORREÇÃO: Cast
}

void login(const char *username, const char *password)
{
    if (g_strcmp0(username, "admin") == 0 && g_strcmp0(password, "admin") == 0)
    {
        advice("Bem vindo!", "Bem vindo, admin.", "emblem-default");
        gtk_stack_set_visible_child_name(stack, "view_admin");
    }
    else if (validate_user(username, password))
    {
        char welcome_msg[100];
        snprintf(welcome_msg, sizeof(welcome_msg), "Bem vindo, %s.", username);
        advice("Bem vindo!", welcome_msg, "emblem-default");
        gtk_stack_set_visible_child_name(stack, "view_common_user");
    }
    else
    {
        advice("Erro de Login", "Usuário ou senha incorretos.", "dialog-error");
    }
}

void on_login_btn_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *username_entry = GTK_WIDGET(gtk_builder_get_object(builder, "user_login_entry"));
    GtkWidget *password_entry = GTK_WIDGET(gtk_builder_get_object(builder, "user_password_entry"));

    const char *username_text = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const char *password_text = gtk_entry_get_text(GTK_ENTRY(password_entry));

    // Armazena o username na variável global
    if (logged_username)
    {
        g_free(logged_username); // Libera memória anterior se existir
    }
    logged_username = g_strdup(username_text); // Faz uma cópia do texto

    login(username_text, password_text);
}

void on_user_create_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_user_create");
}

void on_user_list_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_user_list");
}

void on_logout_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_login");
}

void on_user_create_back_btn_clicked(GtkWidget *widget, gpointer data)
{
    if (g_strcmp0(logged_username, "admin") == 0)
    {
        gtk_stack_set_visible_child_name(stack, "view_admin");
    }
    else
    {
        // implementar
        gtk_main_quit();
    }
}

void on_user_create_confirm_btn_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *username_create_entry = GTK_WIDGET(gtk_builder_get_object(builder, "user_login_create_entry"));
    GtkWidget *password_create_entry = GTK_WIDGET(gtk_builder_get_object(builder, "user_password_create_entry"));

    if (!username_create_entry || !password_create_entry)
    {
        advice("Erro", "Erro interno da aplicação.", "dialog-error");
        return;
    }

    const char *username_text = gtk_entry_get_text(GTK_ENTRY(username_create_entry));
    const char *password_text = gtk_entry_get_text(GTK_ENTRY(password_create_entry));

    // Validação de campos vazios
    if (strlen(username_text) == 0 || strlen(password_text) == 0)
    {
        advice("Erro", "Por favor, preencha todos os campos.", "dialog-error");
        return;
    }

    // Verifica se o usuário já existe
    if (user_exists(username_text))
    {
        advice("Erro", "Usuário já existe. Escolha outro nome.", "dialog-error");
        return;
    }

    // Adiciona o novo usuário
    add_user(username_text, password_text);

    // Limpa os campos
    gtk_entry_set_text(GTK_ENTRY(username_create_entry), "");
    gtk_entry_set_text(GTK_ENTRY(password_create_entry), "");

    advice("Sucesso", "Usuário criado com sucesso!", "emblem-default");
}

void on_user_list_back_btn_clicked(GtkWidget *widget, gpointer data)
{
    if (g_strcmp0(logged_username, "admin") == 0)
    {
        gtk_stack_set_visible_child_name(stack, "view_admin");
    }
    else
    {
        // implementar
        gtk_main_quit();
    }
}

void on_user_list_bt_clicked(GtkWidget *widget, gpointer data)
{
    printf("DEBUG: Botão listar usuários clicado\n");
    load_users_to_treeview();
    printf("DEBUG: Usuários carregados no TreeView\n");
}

void on_client_view_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_client_menu");
}

void on_waste_view_btn_clicked(GtkWidget *widget, gpointer data)
{
    // implementar
    gtk_main_quit();
}

void on_client_create_back_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_client_menu");
}

void on_client_create_confirm_btn_clicked(GtkWidget *widget, gpointer data)
{
    printf("DEBUG: Botão criar cliente clicado\n");

    // Obtém todos os widgets de entrada
    GtkWidget *client_create_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_name_entry"));
    GtkWidget *client_create_responsible_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_responsible_entry"));
    GtkWidget *client_create_company_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_company_name_entry"));
    GtkWidget *client_create_cnpj_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_cnpj_entry"));
    GtkWidget *client_create_corporate_reason_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_corporate_reason_entry"));
    GtkWidget *client_create_fantasy_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_fantasy_entry"));
    GtkWidget *client_create_phone_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_phone_entry"));
    GtkWidget *client_create_mail_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_mail_entry"));
    GtkWidget *client_create_opening_date_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_opening_date_entry"));
    GtkWidget *client_create_address_street_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_address_street_entry"));
    GtkWidget *client_create_address_number_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_address_number_entry"));
    GtkWidget *client_create_address_neighborhood_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_address_neighborhood_entry"));
    GtkWidget *client_create_address_city_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_address_city_entry"));
    GtkWidget *client_create_address_state_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_address_state_entry"));
    GtkWidget *client_create_address_postal_code_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_create_address_postal_code_entry"));

    // Verifica se todos os widgets foram encontrados
    if (!client_create_name_entry || !client_create_responsible_entry || !client_create_company_name_entry ||
        !client_create_cnpj_entry || !client_create_corporate_reason_entry || !client_create_fantasy_entry ||
        !client_create_phone_entry || !client_create_mail_entry || !client_create_opening_date_entry ||
        !client_create_address_street_entry || !client_create_address_number_entry || !client_create_address_neighborhood_entry ||
        !client_create_address_city_entry || !client_create_address_state_entry || !client_create_address_postal_code_entry)
    {
        printf("DEBUG: Erro ao obter widgets de entrada\n");
        advice("Erro", "Erro interno da aplicação.", "dialog-error");
        return;
    }

    // Obtém os valores dos campos
    const char *name = gtk_entry_get_text(GTK_ENTRY(client_create_name_entry));
    const char *responsible = gtk_entry_get_text(GTK_ENTRY(client_create_responsible_entry));
    const char *company_name = gtk_entry_get_text(GTK_ENTRY(client_create_company_name_entry));
    const char *cnpj = gtk_entry_get_text(GTK_ENTRY(client_create_cnpj_entry));
    const char *corporate_reason = gtk_entry_get_text(GTK_ENTRY(client_create_corporate_reason_entry));
    const char *fantasy = gtk_entry_get_text(GTK_ENTRY(client_create_fantasy_entry));
    const char *phone = gtk_entry_get_text(GTK_ENTRY(client_create_phone_entry));
    const char *mail = gtk_entry_get_text(GTK_ENTRY(client_create_mail_entry));
    const char *opening_date = gtk_entry_get_text(GTK_ENTRY(client_create_opening_date_entry));
    const char *street = gtk_entry_get_text(GTK_ENTRY(client_create_address_street_entry));
    const char *number = gtk_entry_get_text(GTK_ENTRY(client_create_address_number_entry));
    const char *neighborhood = gtk_entry_get_text(GTK_ENTRY(client_create_address_neighborhood_entry));
    const char *city = gtk_entry_get_text(GTK_ENTRY(client_create_address_city_entry));
    const char *state = gtk_entry_get_text(GTK_ENTRY(client_create_address_state_entry));
    const char *postal_code = gtk_entry_get_text(GTK_ENTRY(client_create_address_postal_code_entry));

    printf("DEBUG: Dados obtidos - Nome: %s, CNPJ: %s\n", name, cnpj);

    // Validação de campos obrigatórios
    if (strlen(name) == 0 || strlen(responsible) == 0 || strlen(company_name) == 0 ||
        strlen(cnpj) == 0 || strlen(phone) == 0 || strlen(mail) == 0)
    {
        printf("DEBUG: Campos obrigatórios não preenchidos\n");
        advice("Erro", "Por favor, preencha todos os campos obrigatórios:\nNome, Responsável, Razão Social, CNPJ, Telefone e Email.", "dialog-error");
        return;
    }

    // Validação básica do CNPJ (apenas comprimento)
    if (strlen(cnpj) < 14)
    {
        printf("DEBUG: CNPJ inválido\n");
        advice("Erro", "CNPJ deve ter pelo menos 14 dígitos.", "dialog-error");
        return;
    }

    // Verifica se o cliente já existe pelo CNPJ
    if (client_exists_by_cnpj(cnpj))
    {
        printf("DEBUG: Cliente já existe\n");
        advice("Erro", "Cliente com este CNPJ já está cadastrado.", "dialog-error");
        return;
    }

    printf("DEBUG: Adicionando cliente...\n");
    // Adiciona o novo cliente
    add_client(name, responsible, company_name, cnpj, corporate_reason, fantasy,
               phone, mail, opening_date, street, number, neighborhood, city, state, postal_code);

    printf("DEBUG: Limpando campos...\n");
    // Limpa todos os campos
    gtk_entry_set_text(GTK_ENTRY(client_create_name_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_responsible_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_company_name_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_cnpj_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_corporate_reason_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_fantasy_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_phone_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_mail_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_opening_date_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_address_street_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_address_number_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_address_neighborhood_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_address_city_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_address_state_entry), "");
    gtk_entry_set_text(GTK_ENTRY(client_create_address_postal_code_entry), "");

    printf("DEBUG: Cliente criado com sucesso\n");
    advice("Sucesso", "Cliente cadastrado com sucesso!", "emblem-default");
}

void on_client_create_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_client_create");
}

void on_client_list_btn_clicked(GtkWidget *widget, gpointer data)
{
    // gtk_stack_set_visible_child_name(stack, "view_common_user");

    // printf("DEBUG: Botão listar clientes clicado\n");
    // gtk_stack_set_visible_child_name(stack, "view_client_list");
    // load_clients_to_treeview();
    // printf("DEBUG: Clientes carregados no TreeView\n");
}

void on_client_view_back_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_common_user");
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
        "on_client_view_btn_clicked", G_CALLBACK(on_client_view_btn_clicked),
        "on_waste_view_btn_clicked", G_CALLBACK(on_waste_view_btn_clicked),
        "on_client_create_back_btn_clicked", G_CALLBACK(on_client_create_back_btn_clicked),
        "on_client_create_confirm_btn_clicked", G_CALLBACK(on_client_create_confirm_btn_clicked),
        "on_client_create_btn_clicked", G_CALLBACK(on_client_create_btn_clicked),
        "on_client_list_btn_clicked", G_CALLBACK(on_client_list_btn_clicked),
        "on_client_view_back_btn_clicked", G_CALLBACK(on_client_view_back_btn_clicked),
        NULL);

    gtk_builder_connect_signals(builder, NULL);

    stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}