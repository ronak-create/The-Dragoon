#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void gen_header() {
    printf("section .data\n");
    printf("    newline db 10, 0\n\n");
    printf("section .bss\n");
    printf("    t1 resd 1\n    t2 resd 1\n    t3 resd 1\n\n");

    printf("section .text\n");
    printf("    global _start\n\n");
    printf("_start:\n");
}

void gen_footer() {
    printf("    ; exit\n");
    printf("    mov eax, 1\n");
    printf("    xor ebx, ebx\n");
    printf("    int 0x80\n\n");

    printf("; print_int: prints EAX to stdout\n");
    printf("print_int:\n");
    printf("    mov ecx, 10\n");
    printf("    xor edi, edi\n");
    printf("    mov ebx, esp\n");
    printf(".loop:\n");
    printf("    xor edx, edx\n");
    printf("    div ecx\n");
    printf("    add dl, '0'\n");
    printf("    dec esp\n");
    printf("    mov [esp], dl\n");
    printf("    inc edi\n");
    printf("    test eax, eax\n");
    printf("    jnz .loop\n");
    printf(".print:\n");
    printf("    mov eax, 4\n");
    printf("    mov ebx, 1\n");
    printf("    mov ecx, esp\n");
    printf("    mov edx, edi\n");
    printf("    int 0x80\n");
    printf("    add esp, edi\n");
    printf("    ret\n");
}

void handle_assignment(char* result, char* left, char* op, char* right) {
    if (strcmp(op, "+") == 0) {
        printf("    mov eax, [%s]\n", left);
        printf("    add eax, [%s]\n", right);
        printf("    mov [%s], eax\n", result);
    } else if (strcmp(op, "-") == 0) {
        printf("    mov eax, [%s]\n", left);
        printf("    sub eax, [%s]\n", right);
        printf("    mov [%s], eax\n", result);
    } else if (strcmp(op, "*") == 0) {
        printf("    mov eax, [%s]\n", left);
        printf("    imul eax, [%s]\n", right);
        printf("    mov [%s], eax\n", result);
    } else if (strcmp(op, "/") == 0) {
        printf("    mov eax, [%s]\n", left);
        printf("    cdq\n");
        printf("    idiv dword [%s]\n", right);
        printf("    mov [%s], eax\n", result);
    }
}

void handle_simple_assign(char* var, char* val) {
    printf("    mov dword [%s], %s\n", var, val);
}

void handle_print(char* var) {
    printf("    mov eax, [%s]\n", var);
    printf("    call print_int\n");
    printf("    mov eax, 4\n");
    printf("    mov ebx, 1\n");
    printf("    mov ecx, newline\n");
    printf("    mov edx, 1\n");
    printf("    int 0x80\n");
}

int main() {
    char line[256];
    char var1[20], var2[20], var3[20], op[5];

    gen_header();

    while (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, "%s = %s %s %s", var1, var2, op, var3) == 4) {
            handle_assignment(var1, var2, op, var3);
        } else if (sscanf(line, "%s = %s", var1, var2) == 2) {
            handle_simple_assign(var1, var2);
        } else if (sscanf(line, "print %s", var1) == 1) {
            handle_print(var1);
        }
    }

    gen_footer();
    return 0;
}
