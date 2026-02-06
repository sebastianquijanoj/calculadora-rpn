// rpn.c — Calculadora RPN por consola (stack)
// Compilar: gcc rpn.c -o rpn -lm
// Ejecutar:  ./rpn

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define STACK_MAX 1024
#define LINE_MAX  2048

typedef struct {
    double data[STACK_MAX];
    int top;   // número de elementos (0 = vacía)
} Stack;

/* ====== Funciones de pila ====== */

static void stack_init(Stack *s) { s->top = 0; }

static int stack_push(Stack *s, double x) {
    if (s->top >= STACK_MAX) return 0;
    s->data[s->top++] = x;
    return 1;
}

static int stack_pop(Stack *s, double *out) {
    if (s->top <= 0) return 0;
    *out = s->data[--s->top];
    return 1;
}

static int stack_peek(const Stack *s, double *out) {
    if (s->top <= 0) return 0;
    *out = s->data[s->top - 1];
    return 1;
}

static void stack_clear(Stack *s) { s->top = 0; }

// Imprime la pila vertical y numerada: [8] arriba, [1] abajo
static void stack_print(const Stack *s) {
    const int DISPLAY = 8; // mostrar 8 posiciones
    printf("Pila:\n");
    for (int pos = DISPLAY; pos >= 1; pos--) {
        double val = 0.0;
        if (pos <= s->top) val = s->data[s->top - pos];
        printf("%d. %.6f\n", pos, val);
    }
}

/* ====== Utilidades ====== */

static int parse_number(const char *token, double *out) {
    char *endptr = NULL;
    double val = strtod(token, &endptr);
    if (endptr == token) return 0;      // no leyó nada
    if (*endptr != '\0') return 0;      // sobró texto
    *out = val;
    return 1;
}

/* ====== Operadores ====== */

// Operadores binarios + - * /
static int apply_operator(Stack *s, char op) {
    double b, a;

    if (!stack_pop(s, &b) || !stack_pop(s, &a)) {
        printf("Error: pila insuficiente para '%c'\n", op);
        return 0;
    }

    double res = 0.0;

    switch (op) {
        case '+': res = a + b; break;
        case '-': res = a - b; break;
        case '*': res = a * b; break;
        case '/':
            if (b == 0.0) {
                printf("Error: división por cero\n");
                // restaurar operandos como estaban
                stack_push(s, a);
                stack_push(s, b);
                return 0;
            }
            res = a / b;
            break;
        default:
            printf("Error: operador inválido '%c'\n", op);
            // restaurar
            stack_push(s, a);
            stack_push(s, b);
            return 0;
    }

    if (!stack_push(s, res)) {
        printf("Error: pila llena (no se pudo guardar el resultado)\n");
        // restaurar operandos
        stack_push(s, a);
        stack_push(s, b);
        return 0;
    }

    printf("= %g\n", res);
    return 1;
}

// Funciones unarias (usan GRADOS)
static int apply_unary(Stack *s, const char *op) {
    double a;

    if (!stack_pop(s, &a)) {
        printf("Error: pila insuficiente para '%s'\n", op);
        return 0;
    }

    double res = 0.0;

    if (strcmp(op, "sqrt") == 0) {
        if (a < 0) {
            printf("Error: raíz de número negativo\n");
            stack_push(s, a);
            return 0;
        }
        res = sqrt(a);
    } else if (strcmp(op, "sin") == 0) {
        res = sin(a * M_PI / 180.0);
    } else if (strcmp(op, "cos") == 0) {
        res = cos(a * M_PI / 180.0);
    } else if (strcmp(op, "tan") == 0) {
        res = tan(a * M_PI / 180.0);
    } else {
        printf("Operador desconocido: %s\n", op);
        stack_push(s, a);
        return 0;
    }

    if (!stack_push(s, res)) {
        printf("Error: pila llena (no se pudo guardar el resultado)\n");
        stack_push(s, a);
        return 0;
    }

    printf("= %g\n", res);
    return 1;
}

// Potencia binaria
static int apply_pow(Stack *s) {
    double exp, base;

    if (!stack_pop(s, &exp) || !stack_pop(s, &base)) {
        printf("Error: pila insuficiente para 'pow'\n");
        return 0;
    }

    double res = pow(base, exp);

    if (!stack_push(s, res)) {
        printf("Error: pila llena (no se pudo guardar el resultado)\n");
        stack_push(s, base);
        stack_push(s, exp);
        return 0;
    }

    printf("= %g\n", res);
    return 1;
}

/* ====== Ayuda ====== */

static void print_help(void) {
    printf("Calculadora RPN (Notación Polaca Inversa)\n");
    printf("Uso: tokens separados por espacios. Ej: 3 4 +\n");
    printf("Operadores: +  -  *  /\n");
    printf("Funciones: sqrt  sin  cos  tan  pow\n");
    printf("  - sin/cos/tan usan GRADOS\n");
    printf("Comandos:\n");
    printf("  p  -> ver tope\n");
    printf("  s  -> ver pila completa\n");
    printf("  c  -> limpiar pila\n");
    printf("  q  -> salir\n");
    printf("  h  -> ayuda\n");
}

/* ====== main ====== */

int main(void) {
    Stack st;
    stack_init(&st);

    print_help();

    char line[LINE_MAX];

    while (1) {
        printf("rpn> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;

        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;

        char *saveptr = NULL;
        char *token = strtok_r(line, " \t", &saveptr);

        while (token) {

            if (strcmp(token, "q") == 0) return 0;
            else if (strcmp(token, "h") == 0) print_help();
            else if (strcmp(token, "c") == 0) {
                stack_clear(&st);
                printf("[pila limpia]\n");
            }
            else if (strcmp(token, "p") == 0) {
                double t;
                if (stack_peek(&st, &t)) printf("tope: %g\n", t);
                else printf("[pila vacía]\n");
            }
            else if (strcmp(token, "s") == 0) {
                stack_print(&st);
            }
            else if (
                strcmp(token, "sqrt") == 0 ||
                strcmp(token, "sin")  == 0 ||
                strcmp(token, "cos")  == 0 ||
                strcmp(token, "tan")  == 0
            ) {
                apply_unary(&st, token);
            }
            else if (strcmp(token, "pow") == 0) {
                apply_pow(&st);
            }
            else if (strlen(token) == 1 && strchr("+-*/", token[0])) {
                apply_operator(&st, token[0]);
            }
            else {
                double num;
                if (parse_number(token, &num)) {
                    if (!stack_push(&st, num)) {
                        printf("Error: pila llena (no se pudo apilar %g)\n", num);
                    }
                } else {
                    printf("Token inválido: '%s'\n", token);
                }
            }

            token = strtok_r(NULL, " \t", &saveptr);
        }
    }
    return 0;
}
