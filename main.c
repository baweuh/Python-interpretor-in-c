#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VARS 100
#define MAX_VAR_NAME_LENGTH 50
#define MAX_STACK_SIZE 100
#define MAX_STRING_LENGTH 256

typedef enum {
    TOKEN_INVALID = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_ASSIGN,
    TOKEN_PRINT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_TIMES,
    TOKEN_DIVIDE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_EOF,
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_COLON // Ajout de TOKEN_COLON pour le `:`
} TokenType;

// Structure pour les variables
typedef struct {
    char name[MAX_VAR_NAME_LENGTH];
    int var_type;  // 0 = int, 1 = float, 2 = string
    union {
        int i_value;
        float f_value;
        char s_value[MAX_STRING_LENGTH];
    };
} Variable;

Variable variables[MAX_VARS];
int num_vars = 0;

// Structure pour les tokens
typedef struct {
    TokenType type;
    char value[100];
} Token;

// Structure pour la pile
typedef struct {
    int top;
    float data[MAX_STACK_SIZE];
} Stack;

// Structure pour les tables de symboles et la gestion de la portée
typedef struct SymbolTable {
    Variable* variables[MAX_VARS];
    int num_vars;
    struct SymbolTable* parent;  // Pointeur vers la table parente
} SymbolTable;

SymbolTable* symbol_stack = NULL;

// Fonction pour entrer dans une nouvelle portée (scope)
void enter_scope() {
    SymbolTable* new_scope = malloc(sizeof(SymbolTable)); // Allouer une nouvelle table de symboles
    new_scope->num_vars = 0;  // Initialiser le nombre de variables
    new_scope->parent = symbol_stack; // Le parent est la table de symboles précédente
    symbol_stack = new_scope; // Mettre à jour la pile des tables de symboles
}

// Fonction pour sortir de la portée (dépiler)
void exit_scope() {
    SymbolTable* temp = symbol_stack;  // Sauvegarder la table actuelle
    symbol_stack = symbol_stack->parent; // Passer à la table parente
    free(temp); // Libérer la mémoire de la table de symboles actuelle
}

// Déclarations des fonctions manquantes
void push(Stack* stack, float value);
float pop(Stack* stack);
int precedence(TokenType op);
float apply_operator(TokenType op, float a, float b);
Token get_next_token(const char* line, int* pos);
float evaluate_expression(Token* tokens, int start, int end);
Variable* find_variable(const char* name);
void add_variable(const char* name, Variable* var); // Déclaration ajoutée
void print_variable(Variable* var); // Déclaration ajoutée
void enter_scope();
void exit_scope();

// Fonction pour ajouter une variable à la table des symboles
void add_variable(const char* name, Variable* var) {
    if (symbol_stack->num_vars < MAX_VARS) {
        symbol_stack->variables[symbol_stack->num_vars++] = var;
    } else {
        printf("Erreur : Limite de variables atteinte.\n");
        exit(1);
    }
}

// Fonction pour afficher une variable
void print_variable(Variable* var) {
    if (var != NULL) {
        if (var->var_type == TOKEN_NUMBER) {
            printf("%d\n", var->i_value);
        } else if (var->var_type == TOKEN_FLOAT) {
            printf("%f\n", var->f_value);
        } else if (var->var_type == TOKEN_STRING) {
            printf("%s\n", var->s_value);
        }
    } else {
        printf("Variable non définie.\n");
    }
}

// Fonction pour trouver une variable dans la table des symboles
Variable* find_variable(const char* name) {
    SymbolTable* scope = symbol_stack;
    while (scope != NULL) {
        for (int i = 0; i < scope->num_vars; i++) {
            if (strcmp(scope->variables[i]->name, name) == 0) {
                return scope->variables[i];
            }
        }
        scope = scope->parent;
    }
    return NULL;
}

// Fonction sécurisée pour pousser une valeur dans la pile
void push(Stack* stack, float value) {
    if (stack->top < MAX_STACK_SIZE - 1) {
        stack->data[++stack->top] = value;
    } else {
        printf("Erreur : Pile pleine\n");
        exit(1);
    }
}

// Fonction sécurisée pour retirer une valeur de la pile
float pop(Stack* stack) {
    if (stack->top == -1) {
        printf("Erreur : Pile vide\n");
        exit(1);
    } else {
        return stack->data[stack->top--];
    }
}

// Fonction pour obtenir la priorité d'un opérateur
int precedence(TokenType op) {
    switch (op) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 1;
        case TOKEN_TIMES:
        case TOKEN_DIVIDE:
            return 2;
        default:
            return 0;
    }
}

// Fonction pour appliquer un opérateur sur deux valeurs
float apply_operator(TokenType op, float a, float b) {
    switch (op) {
        case TOKEN_PLUS: return a + b;
        case TOKEN_MINUS: return a - b;
        case TOKEN_TIMES: return a * b;
        case TOKEN_DIVIDE:
            if (b == 0.0) {
                printf("Erreur : Division par zéro.\n");
                exit(1);
            }
            return a / b;
        default:
            printf("Erreur : Opérateur non reconnu.\n");
            exit(1);
    }
}

Token get_next_token(const char* line, int* pos) {
    Token token = {TOKEN_INVALID, ""};
    while (line[*pos] != '\0' && isspace(line[*pos])) {
        (*pos)++;
    }

    if (line[*pos] == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }

    if (isalpha(line[*pos])) {
        int start_pos = *pos;
        while (isalnum(line[*pos]) || line[*pos] == '_') {
            (*pos)++;
        }
        strncpy(token.value, line + start_pos, *pos - start_pos);
        token.value[*pos - start_pos] = '\0';

        if (strcmp(token.value, "print") == 0) {
            token.type = TOKEN_PRINT;
        } else if (strcmp(token.value, "while") == 0) {
            token.type = TOKEN_WHILE;
        } else if (strcmp(token.value, "for") == 0) {
            token.type = TOKEN_FOR;
        } else if (strcmp(token.value, "if") == 0) {
            token.type = TOKEN_IF;
        } else if (strcmp(token.value, "else") == 0) {
            token.type = TOKEN_ELSE;
        } else {
            token.type = TOKEN_IDENTIFIER;
        }
    } else if (isdigit(line[*pos]) || line[*pos] == '.') {
        int is_float = 0;
        int start_pos = *pos;
        while (isdigit(line[*pos]) || (line[*pos] == '.' && !is_float)) {
            if (line[*pos] == '.') {
                is_float = 1;
            }
            (*pos)++;
        }
        strncpy(token.value, line + start_pos, *pos - start_pos);
        token.value[*pos - start_pos] = '\0';

        if (is_float) {
            token.type = TOKEN_FLOAT;
        } else {
            token.type = TOKEN_NUMBER;
        }
    } else if (line[*pos] == '"') {
        int start_pos = ++(*pos);
        while (line[*pos] != '"' && line[*pos] != '\0') {
            (*pos)++;
        }
        strncpy(token.value, line + start_pos, *pos - start_pos);
        token.value[*pos - start_pos] = '\0';
        token.type = TOKEN_STRING;
        if (line[*pos] == '"') {
            (*pos)++;
        }
    } else {
        switch (line[*pos]) {
            case '=': token.type = TOKEN_ASSIGN; break;
            case '+': token.type = TOKEN_PLUS; break;
            case '-': token.type = TOKEN_MINUS; break;
            case '*': token.type = TOKEN_TIMES; break;
            case '/': token.type = TOKEN_DIVIDE; break;
            case '(': token.type = TOKEN_LPAREN; break;
            case ')': token.type = TOKEN_RPAREN; break;
            case ':': token.type = TOKEN_COLON; break; // Ajout pour gérer le `:`
        }
        if (token.type != TOKEN_INVALID) {
            (*pos)++;
        } else {
            printf("Token non reconnu : %c\n", line[*pos]);
        }
    }

    return token;
}

// Fonction pour exécuter une instruction 'if-else'
void execute_if_else(Token* tokens, int start, int end) {
    // La condition est entre tokens[start+1] et tokens[end-2]
    float condition_result = evaluate_expression(tokens, start + 1, end - 2);
    if (condition_result != 0) {
        printf("Condition 'if' vraie\n");
    } else {
        printf("Condition 'if' fausse\n");
    }
}

// Fonction pour évaluer une expression (par exemple, 4 > 5)
float evaluate_expression(Token* tokens, int start, int end) {
    Stack values = { .top = -1 };
    Stack ops = { .top = -1 };

    for (int i = start; i <= end; i++) {
        if (tokens[i].type == TOKEN_NUMBER || tokens[i].type == TOKEN_FLOAT) {
            float value = tokens[i].type == TOKEN_NUMBER ? atof(tokens[i].value) : strtof(tokens[i].value, NULL);
            push(&values, value);
        } else if (tokens[i].type == TOKEN_PLUS || tokens[i].type == TOKEN_MINUS ||
                   tokens[i].type == TOKEN_TIMES || tokens[i].type == TOKEN_DIVIDE ||
                   tokens[i].type == TOKEN_GT || tokens[i].type == TOKEN_LT) {
            while (ops.top != -1 && precedence(ops.data[ops.top]) >= precedence(tokens[i].type)) {
                float val2 = pop(&values);
                float val1 = pop(&values);
                TokenType op = ops.data[ops.top--];
                push(&values, apply_operator(op, val1, val2));
            }
            push(&ops, tokens[i].type);
        }
    }

    while (ops.top != -1) {
        float val2 = pop(&values);
        float val1 = pop(&values);
        TokenType op = ops.data[ops.top--];
        push(&values, apply_operator(op, val1, val2));
    }

    return values.data[0];
}

void process_line(const char* line) {
    int pos = 0;
    Token token = get_next_token(line, &pos);

    if (token.type == TOKEN_IDENTIFIER) {
        char var_name[MAX_VAR_NAME_LENGTH];
        strcpy(var_name, token.value);
        token = get_next_token(line, &pos);

        if (token.type == TOKEN_ASSIGN) {
            token = get_next_token(line, &pos);
            if (token.type == TOKEN_NUMBER || token.type == TOKEN_FLOAT || token.type == TOKEN_STRING) {
                Variable var;
                strcpy(var.name, var_name);
                if (token.type == TOKEN_NUMBER) {
                    var.var_type = TOKEN_NUMBER;
                    var.i_value = atoi(token.value);
                } else if (token.type == TOKEN_FLOAT) {
                    var.var_type = TOKEN_FLOAT;
                    var.f_value = atof(token.value);
                } else if (token.type == TOKEN_STRING) {
                    var.var_type = TOKEN_STRING;
                    strncpy(var.s_value, token.value, MAX_STRING_LENGTH);
                }

                add_variable(var_name, &var);
            }
        }
    } else if (token.type == TOKEN_PRINT) {
        token = get_next_token(line, &pos);
        if (token.type == TOKEN_IDENTIFIER) {
            Variable* var = find_variable(token.value);
            print_variable(var);
        }
    } else if (token.type == TOKEN_IF) {
        // Gérer la condition if (ici, on s'attend à un `:` après la condition)
        token = get_next_token(line, &pos);
        if (token.type == TOKEN_NUMBER || token.type == TOKEN_IDENTIFIER) {
            // Exemple : `if 4 > 5 :` => On peut traiter l'expression avant le `:`
            printf("Condition 'if' vraie\n");
            // Ajouter la logique pour traiter l'expression conditionnelle
            token = get_next_token(line, &pos);
            if (token.type == TOKEN_COLON) {
                // Traitez ici le bloc `if` (par exemple, entrer dans une nouvelle portée)
                enter_scope();
                // Traitez les instructions à l'intérieur de `if`
                exit_scope();
            }
        }
    } else if (token.type == TOKEN_ELSE) {
        // Gérer la partie else
        token = get_next_token(line, &pos);
        if (token.type == TOKEN_COLON) {
            // Traitez le bloc `else` de la même manière
        }
    }
}

// Mode interactif
void interactive_mode() {
    char input_buffer[1024];
    while (1) {
        printf(">> ");
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            break;
        }
        process_line(input_buffer);
    }
}

// Mode fichier
void file_mode(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Erreur : Impossible d'ouvrir le fichier '%s'.\n", filename);
        return;
    }

    char input_buffer[1024];
    while (fgets(input_buffer, sizeof(input_buffer), file)) {
        process_line(input_buffer);
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        file_mode(argv[1]);
    } else {
        interactive_mode();
    }
    return 0;
}
