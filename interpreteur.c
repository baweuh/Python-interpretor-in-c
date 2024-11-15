#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VARS 100
#define MAX_VAR_NAME_LENGTH 50
#define MAX_STACK_SIZE 100

// Définir un enum pour TokenType
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
    TOKEN_LPAREN, 
    TOKEN_RPAREN, 
    TOKEN_EQUAL, 
    TOKEN_NEQUAL, 
    TOKEN_SUP, 
    TOKEN_INF, 
    TOKEN_SUPEQUAL, 
    TOKEN_INFEQUAL, 
    TOKEN_EOF
} TokenType;

// Structure pour les variables
typedef struct {
    char name[MAX_VAR_NAME_LENGTH];
    int var_type;  // 0 = int, 1 = float, 2 = string
    union {
        int i_value;
        float f_value;
        char s_value[256];
    };
} Variable;

Variable variables[MAX_VARS];
int num_vars = 0;

// Structure pour les tokens
typedef struct {
    TokenType type;
    char value[100];
} Token;

typedef struct {
    int top;
    float data[MAX_STACK_SIZE];
} Stack;

// Fonction pour trouver une variable par son nom
Variable* find_variable(const char* name) {
    for (int i = 0; i < num_vars; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}

// Fonction pour obtenir le prochain token
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
            case '==': token.type = TOKEN_EQUAL; break;
            case '!=': token.type = TOKEN_NEQUAL; break;
            case '<': token.type = TOKEN_INF; break;
            case '>': token.type = TOKEN_SUP; break;
            case '>=': token.type = TOKEN_SUPEQUAL; break;
            case '<=': token.type = TOKEN_INFEQUAL; break;
        }
        if (token.type != TOKEN_INVALID) {
            (*pos)++;
        } else {
            printf("Token non reconnu : %c\n", line[*pos]);
        }
    }

    return token;
}

void push(Stack* stack, float value) {
    if (stack->top >= MAX_STACK_SIZE - 1) {
        printf("Erreur : Pile pleine.\n");
        exit(1);
    }
    stack->data[++stack->top] = value;
}

float pop(Stack* stack) {
    if (stack->top < 0) {
        printf("Erreur : Pile vide.\n");
        exit(1);
    }
    return stack->data[stack->top--];
}

int precedence(TokenType op) {
    if (op == TOKEN_PLUS || op == TOKEN_MINUS)
        return 1;
    if (op == TOKEN_TIMES || op == TOKEN_DIVIDE)
        return 2;
    return 0;
}

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

float evaluate_expression(Token* tokens, int start, int end) {
    Stack values = { .top = -1 };
    Stack ops = { .top = -1 };

    for (int i = start; i <= end; i++) {
        if (tokens[i].type == TOKEN_NUMBER || tokens[i].type == TOKEN_FLOAT) {
            float value = tokens[i].type == TOKEN_NUMBER ? atof(tokens[i].value) : strtof(tokens[i].value, NULL);
            push(&values, value);
        } else if (tokens[i].type == TOKEN_PLUS || tokens[i].type == TOKEN_MINUS ||
                   tokens[i].type == TOKEN_TIMES || tokens[i].type == TOKEN_DIVIDE) {
            while (ops.top != -1 && precedence(ops.data[ops.top]) >= precedence(tokens[i].type)) {
                float val2 = pop(&values);
                float val1 = pop(&values);
                TokenType op = ops.data[ops.top--];
                push(&values, apply_operator(op, val1, val2));
            }
            push(&ops, tokens[i].type);
        } else if (tokens[i].type == TOKEN_LPAREN) {
            push(&ops, TOKEN_LPAREN);
        } else if (tokens[i].type == TOKEN_RPAREN) {
            while (ops.top != -1 && ops.data[ops.top] != TOKEN_LPAREN) {
                float val2 = pop(&values);
                float val1 = pop(&values);
                TokenType op = ops.data[ops.top--];
                push(&values, apply_operator(op, val1, val2));
            }
            pop(&ops);
        } else if (tokens[i].type == TOKEN_IDENTIFIER) {
            Variable* var = find_variable(tokens[i].value);
            if (var != NULL && (var->var_type == TOKEN_NUMBER || var->var_type == TOKEN_FLOAT)) {
                float value = var->var_type == TOKEN_NUMBER ? var->i_value : var->f_value;
                push(&values, value);
            } else {
                printf("Variable '%s' non définie ou de mauvais type.\n", tokens[i].value);
                exit(1);
            }
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

// Fonction pour traiter une ligne de code
void process_line(const char* line) {
    int pos = 0;
    Token token = get_next_token(line, &pos);

    if (token.type == TOKEN_IDENTIFIER) {
        char var_name[MAX_VAR_NAME_LENGTH];
        strcpy(var_name, token.value);
        token = get_next_token(line, &pos);

        if (token.type == TOKEN_ASSIGN) {
            token = get_next_token(line, &pos);

            if (token.type == TOKEN_STRING) {
                Variable* var = find_variable(var_name);
                if (var != NULL && var->var_type == TOKEN_STRING) {
                    strcpy(var->s_value, token.value);
                } else {
                    Variable new_var;
                    new_var.var_type = TOKEN_STRING;
                    strcpy(new_var.name, var_name);
                    strcpy(new_var.s_value, token.value);
                    variables[num_vars++] = new_var;
                }
            } else {
                Token tokens[100];
                int token_count = 0;
                tokens[token_count++] = token;

                while (token.type != TOKEN_EOF) {
                    token = get_next_token(line, &pos);
                    if (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_NUMBER || 
                        token.type == TOKEN_FLOAT || token.type == TOKEN_PLUS || 
                        token.type == TOKEN_MINUS || token.type == TOKEN_TIMES || 
                        token.type == TOKEN_DIVIDE || token.type == TOKEN_LPAREN || 
                        token.type == TOKEN_RPAREN) {
                        tokens[token_count++] = token;
                    }
                }

                float result = evaluate_expression(tokens, 0, token_count - 1);
                Variable* var = find_variable(var_name);
                if (var != NULL && var->var_type == TOKEN_FLOAT) {
                    var->f_value = result;
                } else if (var != NULL && var->var_type == TOKEN_NUMBER) {
                    var->i_value = (int)result;
                } else {
                    Variable new_var;
                    new_var.var_type = TOKEN_FLOAT;
                    strcpy(new_var.name, var_name);
                    new_var.f_value = result;
                    variables[num_vars++] = new_var;
                }
            }
        }
    } else if (token.type == TOKEN_PRINT) {
        token = get_next_token(line, &pos);
        if (token.type == TOKEN_IDENTIFIER) {
            Variable* var = find_variable(token.value);
            if (var != NULL) {
                if (var->var_type == TOKEN_NUMBER) {
                    printf("%d\n", var->i_value);
                } else if (var->var_type == TOKEN_FLOAT) {
                    printf("%f\n", var->f_value);
                } else if (var->var_type == TOKEN_STRING) {
                    printf("%s\n", var->s_value);
                }
            } else {
                printf("Variable '%s' non définie.\n", token.value);
            }
        }
    }
}

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
        file_mode(argv[1]); // Mode fichier
    } else {
        interactive_mode(); // Mode interactif
    }
    return 0;
}
