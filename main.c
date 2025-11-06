#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

GtkBuilder *builder;
GtkWidget *window;
GtkStack *stack;

char *logged_username;
char *selected_client_cnpj;
char *selected_client_search_main_view;
char *selected_client_search_destiny_view;

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

// Funções para criptografia simples (XOR com chave + conversão para hex)
void simple_encrypt_decrypt(char *data, const char *key)
{
    int key_len = strlen(key);
    int data_len = strlen(data);

    // Buffer temporário para armazenar dados XOR
    char temp_data[100];
    strcpy(temp_data, data);

    // Aplica XOR
    for (int i = 0; i < data_len; i++)
    {
        temp_data[i] ^= key[i % key_len];
    }

    // Converte para representação hexadecimal para evitar caracteres de controle
    char hex_result[200];
    hex_result[0] = '\0';

    for (int i = 0; i < data_len; i++)
    {
        char hex_byte[3];
        sprintf(hex_byte, "%02X", (unsigned char)temp_data[i]);
        strcat(hex_result, hex_byte);
    }

    // Copia resultado de volta
    strcpy(data, hex_result);
}

// Função para descriptografar dados que estão em formato hexadecimal
void simple_decrypt_from_hex(char *hex_data, const char *key)
{
    int hex_len = strlen(hex_data);
    int key_len = strlen(key);

    // Verifica se o comprimento é par (cada byte = 2 caracteres hex)
    if (hex_len % 2 != 0)
    {
        return; // Dados inválidos
    }

    int byte_count = hex_len / 2;
    char temp_data[100];

    // Converte de hex para bytes
    for (int i = 0; i < byte_count; i++)
    {
        char hex_byte[3];
        hex_byte[0] = hex_data[i * 2];
        hex_byte[1] = hex_data[i * 2 + 1];
        hex_byte[2] = '\0';

        temp_data[i] = (char)strtol(hex_byte, NULL, 16);
    }

    // Aplica XOR para descriptografar
    for (int i = 0; i < byte_count; i++)
    {
        temp_data[i] ^= key[i % key_len];
    }

    // Termina a string
    temp_data[byte_count] = '\0';

    // Copia resultado de volta
    strcpy(hex_data, temp_data);
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

    printf("DEBUG: client list store load\n");

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
        return;
    }

    printf("DEBUG: client file opened\n");

    char line[1024];

    while (fgets(line, sizeof(line), file))
    {
        // Remove quebra de linha
        line[strcspn(line, "\n")] = 0;

        // Parse CSV:
        // ID,
        // name,
        // responsible,
        // company_name,
        // encrypted_cnpj,
        // corporate_reason,
        // fantasy,
        // encrypted_phone,
        // mail,
        // opening_date,
        // street,number,
        // neighborhood,
        // city,
        // state,
        // postal_code

        char *tokens[16];
        int token_count = 0;
        char *token = strtok(line, ",");

        while (token != NULL && token_count < 16)
        {
            tokens[token_count++] = token;
            token = strtok(NULL, ",");
        }

        // if (token_count >= 14)
        // {
        char *name = tokens[1];
        char *fantasy = tokens[6];
        char *city = tokens[12];
        char *state = tokens[13];
        char *encrypted_cnpj = tokens[4];

        // Descriptografa CNPJ para exibição
        char decrypted_cnpj[50];
        strcpy(decrypted_cnpj, encrypted_cnpj);
        simple_decrypt_from_hex(decrypted_cnpj, "clientkey123");

        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, name,           // Coluna 1: Nome
                           1, decrypted_cnpj, // Coluna 2: CNPJ (descriptografado)
                           2, fantasy,        // Coluna 3: Fantasia
                           3, city,           // Coluna 4: Cidade
                           4, state,          // Coluna 5: Estado
                           -1);
        // }
    }

    printf("DEBUG: client file closed\n");

    fclose(file);
}

bool search_client_by_cnpj(const char *cnpj)
{
    printf("DEBUG: Procurando cliente com CNPJ: %s\n", cnpj);

    FILE *file = fopen("clients.txt", "r");
    if (file == NULL)
    {
        printf("DEBUG: Arquivo clients.txt não encontrado\n");
        return false;
    }

    char line[1024];
    char encrypted_cnpj[50];
    strcpy(encrypted_cnpj, cnpj);
    simple_encrypt_decrypt(encrypted_cnpj, "clientkey123"); // Criptografa para comparar

    // Debug: mostra CNPJ criptografado
    printf("DEBUG: CNPJ original: %s\n", cnpj);
    printf("DEBUG: CNPJ criptografado: %s\n", encrypted_cnpj);
    while (fgets(line, sizeof(line), file))
    {
        // Remove quebra de linha
        line[strcspn(line, "\n")] = 0;

        // Pula linhas vazias
        if (strlen(line) == 0)
        {
            continue;
        }

        // Parse CSV para obter o CNPJ criptografado (posição 4)
        char temp_line[1024];
        strcpy(temp_line, line); // Preserva linha original para debug

        char *tokens[16];
        int token_count = 0;
        char *token = strtok(line, ",");

        while (token != NULL && token_count < 16)
        {
            tokens[token_count++] = token;
            token = strtok(NULL, ",");
        }

        // Verifica se temos tokens suficientes (pelo menos ID, name, responsible, company_name, cnpj)
        if (token_count >= 5)
        {
            char *stored_encrypted_cnpj = tokens[4];
            printf("DEBUG: Comparando com registro - tokens: %d, CNPJ armazenado: [dados criptografados]\n", token_count);

            // Debug: mostra CNPJ armazenado
            printf("DEBUG: CNPJ armazenado: %s\n", stored_encrypted_cnpj);

            if (stored_encrypted_cnpj != NULL && g_strcmp0(encrypted_cnpj, stored_encrypted_cnpj) == 0)
            {
                printf("DEBUG: Cliente encontrado! CNPJs criptografados são iguais\n");

                // Armazena o CNPJ na variável global (descriptografado)
                if (selected_client_cnpj)
                {
                    g_free(selected_client_cnpj);
                }
                selected_client_cnpj = g_strdup(cnpj);

                fclose(file);
                return true;
            }
            else
            {
                printf("DEBUG: CNPJs criptografados são diferentes\n");
            }
        }
        else
        {
            printf("DEBUG: Linha com tokens insuficientes (%d): %s\n", token_count, temp_line);
        }
    }

    fclose(file);
    printf("DEBUG: Cliente não encontrado após verificar todo o arquivo\n");
    return false;
}

// Funções para gerenciamento de resíduos
int get_next_waste_id()
{
    FILE *file = fopen("wastes.txt", "r");
    int last_id = 0;
    char line[1024];

    if (file == NULL)
    {
        return 1; // Primeiro resíduo
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

void add_waste_record(const char *client_cnpj, const char *waste_quantity, const char *logged_user)
{
    int new_id = get_next_waste_id();

    FILE *file = fopen("wastes.txt", "a");
    if (file == NULL)
    {
        printf("DEBUG: Erro ao abrir arquivo wastes.txt para escrita\n");
        return;
    }

    // Criptografa o CNPJ para armazenar (mesma chave usada para clientes)
    char encrypted_cnpj[50];
    strcpy(encrypted_cnpj, client_cnpj);
    simple_encrypt_decrypt(encrypted_cnpj, "clientkey123");

    // Obtém data e hora atual
    time_t rawtime;
    struct tm *timeinfo;
    char timestamp[100];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Formato CSV: ID,encrypted_cnpj,waste_quantity,logged_user,timestamp
    fprintf(file, "%d,%s,%s,%s,%s\n", new_id, encrypted_cnpj, waste_quantity, logged_user, timestamp);

    fclose(file);
    printf("DEBUG: Registro de resíduo salvo - ID: %d, CNPJ: %s, Quantidade: %s\n", new_id, client_cnpj, waste_quantity);
}

double get_client_waste_quantity(const char *client_cnpj)
{
    printf("DEBUG: Buscando quantidade de resíduos para CNPJ: %s\n", client_cnpj);

    FILE *file = fopen("wastes.txt", "r");
    if (file == NULL)
    {
        printf("DEBUG: Arquivo wastes.txt não encontrado\n");
        return 0.0;
    }

    char line[1024];
    char encrypted_cnpj[50];
    strcpy(encrypted_cnpj, client_cnpj);
    simple_encrypt_decrypt(encrypted_cnpj, "clientkey123"); // Criptografa para comparar

    double total_quantity = 0.0;

    while (fgets(line, sizeof(line), file))
    {
        // Remove quebra de linha
        line[strcspn(line, "\n")] = 0;

        // Parse CSV: ID,encrypted_cnpj,waste_quantity,logged_user,timestamp
        char *tokens[5];
        int token_count = 0;
        char *token = strtok(line, ",");

        while (token != NULL && token_count < 5)
        {
            tokens[token_count++] = token;
            token = strtok(NULL, ",");
        }

        if (token_count >= 3)
        {
            char *stored_encrypted_cnpj = tokens[1];
            char *quantity_str = tokens[2];

            if (g_strcmp0(encrypted_cnpj, stored_encrypted_cnpj) == 0)
            {
                double quantity = strtod(quantity_str, NULL);
                total_quantity += quantity;
                printf("DEBUG: Encontrado registro com quantidade: %s (%.2f)\n", quantity_str, quantity);
            }
        }
    }

    fclose(file);
    printf("DEBUG: Quantidade total de resíduos: %.2f\n", total_quantity);
    return total_quantity;
}

bool update_client_waste_quantity(const char *client_cnpj, const char *new_quantity, const char *logged_user)
{
    printf("DEBUG: Atualizando quantidade de resíduos para CNPJ: %s\n", client_cnpj);

    // Primeiro, lemos todos os registros
    FILE *file = fopen("wastes.txt", "r");
    if (file == NULL)
    {
        printf("DEBUG: Arquivo wastes.txt não encontrado\n");
        return false;
    }

    char lines[1000][1024]; // Assume máximo 1000 registros
    int line_count = 0;
    bool found = false;

    char encrypted_cnpj[50];
    strcpy(encrypted_cnpj, client_cnpj);
    simple_encrypt_decrypt(encrypted_cnpj, "clientkey123");

    // Lê TODAS as linhas do arquivo primeiro
    while (fgets(lines[line_count], sizeof(lines[line_count]), file) && line_count < 1000)
    {
        // Remove quebra de linha
        lines[line_count][strcspn(lines[line_count], "\n")] = 0;
        line_count++;
    }

    fclose(file);
    printf("DEBUG: Total de linhas lidas: %d\n", line_count);

    // Agora processa as linhas em memória
    for (int i = 0; i < line_count; i++)
    {
        // Verifica se esta linha é do cliente
        char temp_line[1024];
        strcpy(temp_line, lines[i]);

        char *tokens[5];
        int token_count = 0;
        char *token = strtok(temp_line, ",");

        while (token != NULL && token_count < 5)
        {
            tokens[token_count++] = token;
            token = strtok(NULL, ",");
        }

        if (token_count >= 2 && g_strcmp0(encrypted_cnpj, tokens[1]) == 0)
        {
            // Atualiza esta linha
            time_t rawtime;
            struct tm *timeinfo;
            char timestamp[100];

            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

            snprintf(lines[i], sizeof(lines[i]),
                     "%s,%s,%s,%s,%s", tokens[0], tokens[1], new_quantity, logged_user, timestamp);
            found = true;
            printf("DEBUG: Registro na linha %d atualizado\n", i + 1);
            break; // Para na primeira ocorrência encontrada
        }
    }

    if (!found)
    {
        printf("DEBUG: Nenhum registro encontrado para atualizar\n");
        return false;
    }

    // Reescreve o arquivo com TODAS as linhas
    file = fopen("wastes.txt", "w");
    if (file == NULL)
    {
        printf("DEBUG: Erro ao abrir arquivo para escrita\n");
        return false;
    }

    for (int i = 0; i < line_count; i++)
    {
        fprintf(file, "%s\n", lines[i]);
    }

    fclose(file);
    printf("DEBUG: Arquivo atualizado com sucesso - %d linhas escritas\n", line_count);
    return true;
}

void populate_client_waste_cnpj()
{
    printf("DEBUG: Preenchendo CNPJ e quantidade do cliente selecionado\n");

    GtkWidget *client_waste_entry_readonly = GTK_WIDGET(gtk_builder_get_object(builder, "client_waste_entry_readonly"));
    GtkWidget *client_waste_quantity_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_waste_quantity_entry"));

    if (!client_waste_entry_readonly)
    {
        printf("DEBUG: Widget client_waste_entry_readonly não encontrado\n");
        return;
    }

    if (!client_waste_quantity_entry)
    {
        printf("DEBUG: Widget client_waste_quantity_entry não encontrado\n");
        return;
    }

    if (selected_client_cnpj && strlen(selected_client_cnpj) > 0)
    {
        // Preenche o CNPJ
        gtk_entry_set_text(GTK_ENTRY(client_waste_entry_readonly), selected_client_cnpj);
        printf("DEBUG: CNPJ preenchido: %s\n", selected_client_cnpj);

        // Busca e preenche a quantidade existente
        double existing_quantity = get_client_waste_quantity(selected_client_cnpj);

        if (existing_quantity > 0.0)
        {
            char quantity_str[50];
            snprintf(quantity_str, sizeof(quantity_str), "%.2f", existing_quantity);
            gtk_entry_set_text(GTK_ENTRY(client_waste_quantity_entry), quantity_str);
            printf("DEBUG: Quantidade existente preenchida: %s\n", quantity_str);
        }
        else
        {
            gtk_entry_set_text(GTK_ENTRY(client_waste_quantity_entry), "");
            printf("DEBUG: Nenhuma quantidade existente encontrada\n");
        }
    }
    else
    {
        gtk_entry_set_text(GTK_ENTRY(client_waste_entry_readonly), "");
        gtk_entry_set_text(GTK_ENTRY(client_waste_quantity_entry), "");
        printf("DEBUG: Nenhum cliente selecionado\n");
    }
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
    load_users_to_treeview();
}

void on_client_view_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_client_menu");
}

void on_waste_view_btn_clicked(GtkWidget *widget, gpointer data)
{
    if (selected_client_search_main_view)
    {
        g_free(selected_client_search_main_view);
    }
    selected_client_search_main_view = g_strdup("view_common_user");

    if (selected_client_search_destiny_view)
    {
        g_free(selected_client_search_destiny_view);
    }
    selected_client_search_destiny_view = g_strdup("view_waste_create_edit");

    gtk_stack_set_visible_child_name(stack, "view_search_client");
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

    // Validação de campos obrigatórios
    if (strlen(name) == 0 || strlen(responsible) == 0 || strlen(company_name) == 0 ||
        strlen(cnpj) == 0 || strlen(phone) == 0 || strlen(mail) == 0)
    {
        advice("Erro", "Por favor, preencha todos os campos obrigatórios:\nNome, Responsável, Razão Social, CNPJ, Telefone e Email.", "dialog-error");
        return;
    }

    // Validação básica do CNPJ (apenas comprimento)
    if (strlen(cnpj) != 14)
    {
        advice("Erro", "CNPJ deve ter exatamente 14 dígitos.", "dialog-error");
        return;
    }

    // Verifica se o cliente já existe pelo CNPJ
    if (client_exists_by_cnpj(cnpj))
    {
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
    gtk_stack_set_visible_child_name(stack, "view_client_list");
}

void on_client_view_back_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_common_user");
}

void on_client_list_back_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_client_menu");
}

void on_client_list_bt_clicked(GtkWidget *widget, gpointer data)
{
    load_clients_to_treeview();
}

void on_client_search_back_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, "view_common_user");
}

void on_client_search_confirm_btn_clicked(GtkWidget *widget, gpointer data)
{
    printf("DEBUG: Botão buscar cliente clicado\n");

    // Obtém o widget de entrada do CNPJ
    GtkWidget *client_search_cnpj_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_search_entry"));

    if (!client_search_cnpj_entry)
    {
        printf("DEBUG: Erro ao obter widget client_search_cnpj_entry\n");
        advice("Erro", "Erro interno da aplicação.", "dialog-error");
        return;
    }

    // Obtém o CNPJ digitado
    const char *cnpj_text = gtk_entry_get_text(GTK_ENTRY(client_search_cnpj_entry));

    printf("DEBUG: CNPJ a ser procurado: '%s'\n", cnpj_text);

    // Validação de campo vazio
    if (strlen(cnpj_text) == 0)
    {
        printf("DEBUG: CNPJ não informado\n");
        advice("Erro", "Por favor, digite o CNPJ do cliente.", "dialog-error");
        return;
    }

    // Validação do CNPJ (comprimento exato e apenas números)
    if (strlen(cnpj_text) != 14)
    {
        printf("DEBUG: CNPJ inválido - deve ter exatamente 14 dígitos\n");
        advice("Erro", "CNPJ deve ter exatamente 14 dígitos.", "dialog-error");
        return;
    }

    // Verifica se contém apenas dígitos
    for (int i = 0; i < strlen(cnpj_text); i++)
    {
        if (cnpj_text[i] < '0' || cnpj_text[i] > '9')
        {
            printf("DEBUG: CNPJ inválido - contém caracteres não numéricos\n");
            advice("Erro", "CNPJ deve conter apenas números.", "dialog-error");
            return;
        }
    }

    // Busca o cliente pelo CNPJ
    if (search_client_by_cnpj(cnpj_text))
    {
        printf("DEBUG: Cliente encontrado e CNPJ armazenado: %s\n", selected_client_cnpj);

        // Navega para a próxima tela ou carrega dados específicos do cliente
        gtk_stack_set_visible_child_name(stack, selected_client_search_destiny_view);

        // Preenche o CNPJ no campo readonly se for a tela de waste
        if (g_strcmp0(selected_client_search_destiny_view, "view_waste_create_edit") == 0)
        {
            populate_client_waste_cnpj();
        }
    }
    else
    {
        printf("DEBUG: Cliente não encontrado\n");
        advice("Erro", "Cliente com este CNPJ não foi encontrado. Tente novamente ou cadastre o cliente.", "dialog-error");

        // Limpa a variável global se não encontrou
        if (selected_client_cnpj)
        {
            g_free(selected_client_cnpj);
            selected_client_cnpj = NULL;
        }
    }
}

void on_client_waste_save_back_btn_clicked(GtkWidget *widget, gpointer data)
{
    gtk_stack_set_visible_child_name(stack, selected_client_search_main_view);
}

void on_client_waste_save_btn_clicked(GtkWidget *widget, gpointer data)
{
    printf("DEBUG: Botão salvar resíduo clicado\n");

    // Obtém os widgets de entrada
    GtkWidget *client_waste_entry_readonly = GTK_WIDGET(gtk_builder_get_object(builder, "client_waste_entry_readonly"));
    GtkWidget *client_waste_quantity_entry = GTK_WIDGET(gtk_builder_get_object(builder, "client_waste_quantity_entry"));

    if (!client_waste_entry_readonly || !client_waste_quantity_entry)
    {
        printf("DEBUG: Erro ao obter widgets de entrada\n");
        advice("Erro", "Erro interno da aplicação.", "dialog-error");
        return;
    }

    // Obtém os valores dos campos
    const char *cnpj_text = gtk_entry_get_text(GTK_ENTRY(client_waste_entry_readonly));
    const char *quantity_text = gtk_entry_get_text(GTK_ENTRY(client_waste_quantity_entry));

    printf("DEBUG: CNPJ: '%s', Quantidade: '%s'\n", cnpj_text, quantity_text);

    // Validação de campos vazios
    if (strlen(cnpj_text) == 0)
    {
        printf("DEBUG: CNPJ não informado\n");
        advice("Erro", "CNPJ do cliente não encontrado. Realize uma nova busca.", "dialog-error");
        return;
    }

    if (strlen(quantity_text) == 0)
    {
        printf("DEBUG: Quantidade não informada\n");
        advice("Erro", "Por favor, informe a quantidade de resíduos.", "dialog-error");
        return;
    }

    // Validação da quantidade (deve ser um número)
    char *endptr;
    double quantity = strtod(quantity_text, &endptr);
    if (*endptr != '\0' || quantity <= 0)
    {
        printf("DEBUG: Quantidade inválida: %s\n", quantity_text);
        advice("Erro", "A quantidade deve ser um número válido maior que zero.", "dialog-error");
        return;
    }

    // Verifica se existe um usuário logado
    if (!logged_username || strlen(logged_username) == 0)
    {
        printf("DEBUG: Usuário não logado\n");
        advice("Erro", "Usuário não identificado. Faça login novamente.", "dialog-error");
        return;
    }

    // Verifica se o CNPJ corresponde ao cliente selecionado
    if (!selected_client_cnpj || g_strcmp0(cnpj_text, selected_client_cnpj) != 0)
    {
        printf("DEBUG: CNPJ não corresponde ao cliente selecionado\n");
        advice("Erro", "CNPJ não corresponde ao cliente selecionado. Realize uma nova busca.", "dialog-error");
        return;
    }

    // Verifica se já existe registro para este cliente
    double existing_quantity = get_client_waste_quantity(cnpj_text);

    if (existing_quantity > 0.0)
    {
        printf("DEBUG: Atualizando registro existente...\n");
        // Atualiza registro existente
        if (update_client_waste_quantity(cnpj_text, quantity_text, logged_username))
        {
            printf("DEBUG: Registro atualizado com sucesso\n");
            advice("Sucesso", "Registro de resíduo atualizado com sucesso!", "emblem-default");
        }
        else
        {
            printf("DEBUG: Erro ao atualizar registro\n");
            advice("Erro", "Erro ao atualizar o registro. Tente novamente.", "dialog-error");
            return;
        }
    }
    else
    {
        printf("DEBUG: Criando novo registro...\n");
        // Cria novo registro
        add_waste_record(cnpj_text, quantity_text, logged_username);
        printf("DEBUG: Novo registro criado com sucesso\n");
        advice("Sucesso", "Registro de resíduo salvo com sucesso!", "emblem-default");
    }

    // Limpa o campo de quantidade
    gtk_entry_set_text(GTK_ENTRY(client_waste_quantity_entry), "");

    // Volta para a tela anterior
    gtk_stack_set_visible_child_name(stack, selected_client_search_main_view);
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
        "on_client_list_back_btn_clicked", G_CALLBACK(on_client_list_back_btn_clicked),
        "on_client_list_bt_clicked", G_CALLBACK(on_client_list_bt_clicked),
        "on_client_search_back_btn_clicked", G_CALLBACK(on_client_search_back_btn_clicked),
        "on_client_search_confirm_btn_clicked", G_CALLBACK(on_client_search_confirm_btn_clicked),
        "on_client_waste_save_back_btn_clicked", G_CALLBACK(on_client_waste_save_back_btn_clicked),
        "on_client_waste_save_btn_clicked", G_CALLBACK(on_client_waste_save_btn_clicked),
        NULL);

    gtk_builder_connect_signals(builder, NULL);

    stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}