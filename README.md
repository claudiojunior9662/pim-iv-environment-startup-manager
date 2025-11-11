# Sistema de Gerenciamento Ambiental - PIM IV

Um sistema desktop completo para gerenciamento de startups ambientais, desenvolvido em C com interface gráfica GTK 3.0. O projeto oferece controle total sobre usuários, clientes e registros de resíduos com criptografia de dados sensíveis.

## 📋 Visão Geral

Este sistema foi desenvolvido como parte do Projeto Integrado Multidisciplinar (PIM IV) e oferece uma solução completa para:

- **Autenticação de usuários** com diferentes níveis de acesso
- **Gerenciamento de clientes** com dados empresariais completos
- **Controle de resíduos** com rastreamento por cliente
- **Segurança de dados** através de criptografia XOR+Hex
- **Interface gráfica intuitiva** baseada em GTK

## 🚀 Funcionalidades Principais

### 🔐 Sistema de Autenticação

- **Login administrativo**: Usuário padrão `admin/admin` com acesso total
- **Criação de usuários**: Cadastro de novos usuários do sistema
- **Validação de credenciais**: Verificação segura de login/senha
- **Listagem de usuários**: Visualização de todos os usuários cadastrados

### 👥 Gerenciamento de Clientes

- **Cadastro completo**: Dados pessoais, empresariais e endereço
- **Criptografia de dados sensíveis**: CNPJ e telefone protegidos
- **Busca por CNPJ**: Localização rápida de clientes
- **Listagem em TreeView**: Interface organizada para visualização
- **Validação de dados**: Verificação de campos obrigatórios e formato

### ♻️ Controle de Resíduos

- **Registro de quantidades**: Vinculação de resíduos por cliente
- **Atualização de registros**: Modificação de quantidades existentes
- **Histórico com timestamp**: Rastreamento temporal das operações
- **Soma automática**: Cálculo total de resíduos por cliente
- **Auditoria**: Registro do usuário responsável pelas operações

### 📊 Sistema de Relatórios

- **Relatórios semestrais**: Total de insumos tratados por semestre
- **Relatórios mensais**: Estimativa de gastos mensais baseada em resíduos
- **Múltiplos formatos**: Suporte para TXT, CSV e XLS
- **Geração automática**: Criação e abertura automática dos arquivos
- **Análises estatísticas**: Totais, médias e resumos por período

### 🔒 Segurança de Dados

- **Criptografia XOR**: Proteção de dados sensíveis
- **Encoding hexadecimal**: Prevenção de caracteres de controle
- **Chaves específicas**: Diferentes chaves para diferentes tipos de dados
- **Descriptografia transparente**: Exibição normal na interface

## 🛠️ Tecnologias Utilizadas

### **Linguagem de Programação**

- **C (C99/C11)**: Linguagem principal do projeto
- **Bibliotecas padrão**: stdio.h, stdlib.h, string.h, time.h, stdbool.h

### **Interface Gráfica**

- **GTK 3.0**: Framework principal para GUI
- **Glade 3.40.0**: Designer visual para interfaces
- **GtkBuilder**: Carregamento dinâmico de interfaces
- **GtkStack**: Navegação entre telas
- **GtkTreeView/ListStore**: Exibição de dados tabulares

### **Armazenamento de Dados**

- **Arquivos CSV**: Formato simples para persistência
- **users.txt**: Dados de usuários (ID, username, password)
- **clients.txt**: Informações completas de clientes
- **wastes.txt**: Registros de resíduos por cliente

### **Criptografia**

- **XOR Cipher**: Algoritmo de criptografia simétrica
- **Hex Encoding**: Conversão para formato seguro
- **Chaves estáticas**: "clientkey123" para dados sensíveis

## 📦 Dependências

### **Bibliotecas de Sistema (Windows)**

```
gtk+-3.0
glib-2.0
gobject-2.0
gdk-3.0
```

### **Ferramentas de Desenvolvimento**

- **Compilador C**: GCC ou MinGW (Windows)
- **pkg-config**: Gerenciamento de dependências
- **Glade**: Editor de interface (opcional, para modificações)

### **Instalação das Dependências (Windows)**

1. **MSYS2** (Recomendado):

```bash
# Instalar MSYS2 de https://www.msys2.org/
pacman -S mingw-w64-x86_64-gtk3
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-pkg-config
```

2. **GTK Windows Runtime**:
   - Baixar de: https://github.com/tschoonj/GTK-for-Windows-Runtime-Environment-Installer

## 🔧 Compilação e Execução

### **Compilação**

```bash
# Usando GCC com pkg-config
gcc -o main main.c `pkg-config --cflags --libs gtk+-3.0`

# Ou especificando bibliotecas manualmente (Windows)
gcc -o main.exe main.c -lgtk-3 -lgdk-3 -lglib-2.0 -lgobject-2.0
```

### **Execução**

```bash
# Linux/Unix
./main

# Windows
main.exe
```

### **Requisitos de Arquivos**

Certifique-se de que os seguintes arquivos estejam no mesmo diretório do executável:

- `glade-project.glade` - Interface gráfica
- `users.txt` - Dados de usuários (criado automaticamente)
- `clients.txt` - Dados de clientes (criado automaticamente)
- `wastes.txt` - Registros de resíduos (criado automaticamente)

## 📁 Estrutura do Projeto

```
pim-iv-environment-startup-manager/
├── main.c                    # Código fonte principal
├── glade-project.glade       # Interface gráfica (Glade)
├── glade-project.glade~      # Backup da interface
├── users.txt                 # Base de dados de usuários
├── clients.txt               # Base de dados de clientes
├── wastes.txt                # Registros de resíduos
├── reports/                  # Diretório de relatórios gerados
├── main.exe                  # Executável compilado
├── README.md                 # Esta documentação
├── .gitignore                # Arquivos ignorados pelo Git
└── LICENSE                   # Licença do projeto
```

## 💡 Como Usar

### **1. Primeiro Acesso**

- Execute o programa
- Use as credenciais padrão: `admin` / `admin`
- Acesse o painel administrativo

### **2. Gerenciamento de Usuários**

- Clique em "Gerenciar Usuários"
- Crie novos usuários ou visualize existentes
- Usuários criados podem fazer login no sistema

### **3. Cadastro de Clientes**

- Acesse "Gerenciar Clientes"
- Preencha todos os campos obrigatórios
- CNPJ deve ter exatamente 14 dígitos
- Dados sensíveis são automaticamente criptografados

### **4. Registro de Resíduos**

- Clique em "Registrar Resíduos"
- Informe o CNPJ do cliente
- Digite a quantidade de resíduos
- Sistema atualiza automaticamente registros existentes

### **5. Geração de Relatórios**

- Acesse "Gerar Relatórios"
- Selecione o tipo de relatório:
  - **Total de insumos tratados semestralmente**: Análise por semestre
  - **Total de gastos mensais**: Estimativa de custos mensais
- Escolha o formato de saída:
  - **TXT**: Relatório formatado para leitura
  - **CSV**: Dados estruturados para importação
  - **XLS**: Formato compatível com Excel
- Clique em "Gerar Relatório"
- O arquivo será criado no diretório `/reports` e aberto automaticamente
- **Organização**: Todos os relatórios são salvos em `/reports/` para melhor organização

### **6. Consultas**

- Use a listagem de clientes para visualizar todos os cadastros
- Busque clientes específicos pelo CNPJ
- Visualize quantidades totais de resíduos

## 🔧 Configurações Avançadas

### **Modificação da Criptografia**

Para alterar a chave de criptografia, modifique a constante no código:

```c
// Linha ~200 e outras ocorrências
simple_encrypt_decrypt(encrypted_cnpj, "NOVA_CHAVE_AQUI");
```

### **Customização da Interface**

1. Abra `glade-project.glade` no Glade
2. Modifique os elementos visuais
3. Salve o arquivo
4. Recompile o projeto

### **Gerenciamento de Relatórios**

- **Localização**: Todos os relatórios são salvos em `/reports/`
- **Limpeza**: Para limpar relatórios antigos, delete o conteúdo da pasta `/reports/`
- **Backup**: Faça backup dos relatórios importantes antes da limpeza
- **Git**: O diretório `/reports/` está no `.gitignore` para não versionar arquivos gerados

### **Formato dos Arquivos de Dados**

**users.txt**: `ID,username,password`

```
1,admin,admin
2,usuario1,senha123
```

**clients.txt**: `ID,name,responsible,company_name,encrypted_cnpj,corporate_reason,fantasy,encrypted_phone,mail,opening_date,street,number,neighborhood,city,state,postal_code`

**wastes.txt**: `ID,encrypted_cnpj,waste_quantity,logged_user,timestamp`

## 🐛 Solução de Problemas

### **Erro: "Failed to load glade file"**

- Verifique se `glade-project.glade` está no mesmo diretório
- Confirme as permissões de leitura do arquivo

### **Erro: "Cannot connect to display"**

- No Windows: Instale o GTK Runtime Environment
- No Linux: Execute `export DISPLAY=:0.0`

### **Compilação falha**

- Verifique se as bibliotecas GTK estão instaladas
- Use `pkg-config --cflags --libs gtk+-3.0` para verificar

### **Dados não persistem**

- Confirme permissões de escrita na pasta
- Verifique se os arquivos .txt não estão sendo bloqueados

## 🤝 Contribuição

Este projeto é parte de um trabalho acadêmico (PIM IV). Para contribuições:

1. Faça um fork do projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## 📄 Licença

Este projeto está sob a licença especificada no arquivo `LICENSE`.

## 👨‍💻 Desenvolvedor

Desenvolvido como parte do **Projeto Integrado Multidisciplinar IV (PIM IV)** - Sistema de Gerenciamento para Startups Ambientais.

---

### 🎯 Status do Projeto

- ✅ **Sistema de autenticação funcional**
- ✅ **CRUD completo de usuários**
- ✅ **CRUD completo de clientes com criptografia**
- ✅ **Sistema de resíduos com create/update**
- ✅ **Sistema de relatórios com múltiplos formatos**
- ✅ **Interface gráfica completa**
- ✅ **Validações e tratamento de erros**
- ✅ **Persistência em arquivos CSV**

**Versão**: 1.1.0 - Sistema completo com relatórios
