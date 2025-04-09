#include <stdio.h>
#include <string.h>

typedef enum {
    TAC_ASSIGN,
    TAC_BINARY,
    TAC_IF_FALSE,
    TAC_CALL,
    TAC_LABEL,
    TAC_GOTO
} TACOpType;

typedef struct {
    TACOpType type;
    char result[20];
    char arg1[50];
    char arg2[50];
    char op[10];
    char label[20];
} TACInstruction;

TACInstruction tac_list[100];
int tac_count = 0;

void parse_tac_line(char *line) {
    TACInstruction instr = {0};

    if (strstr(line, "ifFalse") == line) {
        instr.type = TAC_IF_FALSE;
        sscanf(line, "ifFalse %s goto %s", instr.arg1, instr.label);
    } else if (strstr(line, "goto") == line) {
        instr.type = TAC_GOTO;
        sscanf(line, "goto %s", instr.label);
    } else if (strchr(line, '=') && strstr(line, "===")) {
        instr.type = TAC_BINARY;
        sscanf(line, "%s = %s === %s", instr.result, instr.arg1, instr.arg2);
        strcpy(instr.op, "===");
    } else if (strchr(line, '=')) {
        instr.type = TAC_ASSIGN;
        sscanf(line, "%s = %[^\n]", instr.result, instr.arg1);
    } else if (strchr(line, ':')) {
        instr.type = TAC_LABEL;
        sscanf(line, "%s", instr.label);
        instr.label[strlen(instr.label) - 1] = '\0'; // remove ':'
    } else if (strstr(line, "call")) {
        instr.type = TAC_CALL;
        sscanf(line, "call %[^,] , %[^\n]", instr.arg1, instr.arg2);
    }

    tac_list[tac_count++] = instr;
}

void generate_llvm_header(FILE *f) {
    fprintf(f, "@.str = private unnamed_addr constant [13 x i8] c\"Hello World\\00\", align 1\n\n");
    fprintf(f, "declare i32 @printf(i8*, ...)\n\n");
    fprintf(f, "define i32 @main() {\nentry:\n");
}

void generate_llvm_footer(FILE *f) {
    fprintf(f, "  ret i32 0\n}\n");
}

void generate_instruction_llvm(FILE *f, TACInstruction instr) {
    static int temp = 0;

    switch (instr.type) {
        case TAC_ASSIGN:
            fprintf(f, "  %%%s = alloca i32\n", instr.result);
            fprintf(f, "  store i32 %s, i32* %%%s\n", instr.arg1, instr.result);
            break;

        case TAC_BINARY:
            fprintf(f, "  %%val%d = load i32, i32* %%%s\n", temp, instr.arg1);
            fprintf(f, "  %%cmp%d = icmp eq i32 %%val%d, %s\n", temp, temp, instr.arg2);
            fprintf(f, "  %%%s = alloca i1\n", instr.result);
            fprintf(f, "  store i1 %%cmp%d, i1* %%%s\n", temp, instr.result);
            temp++;
            break;

        case TAC_IF_FALSE:
            fprintf(f, "  %%cond%d = load i1, i1* %%%s\n", temp, instr.arg1);
            fprintf(f, "  br i1 %%cond%d, label %%after%s, label %%%s\n", temp, instr.label, instr.label);
            break;

        case TAC_CALL:
            fprintf(f, "  %%msg = getelementptr [13 x i8], [13 x i8]* @.str, i32 0, i32 0\n");
            fprintf(f, "  call i32 (i8*, ...) @printf(i8* %%msg)\n");
            break;

        case TAC_LABEL:
            fprintf(f, "%s:\n", instr.label);
            break;

        case TAC_GOTO:
            fprintf(f, "  br label %%%s\n", instr.label);
            break;
    }
}

int main() {
    const char *tac_lines[] = {
        "num = 42",
        "hex = 0x1F",
        "bin = 0b01010",
        "str = Hello World",
        "bool = true",
        "t0 = num === 42",
        "ifFalse t0 goto L0",
        "call console.log , 0",
        "L0:",
        NULL
    };

    FILE *f = fopen("program.ll", "w");
    if (!f) {
        perror("Failed to open file");
        return 1;
    }

    for (int i = 0; tac_lines[i] != NULL; i++) {
        parse_tac_line((char *)tac_lines[i]);
    }

    generate_llvm_header(f);
    for (int i = 0; i < tac_count; i++) {
        generate_instruction_llvm(f, tac_list[i]);
    }
    fprintf(f, "afterL0:\n");
    generate_llvm_footer(f);

    fclose(f);
    printf("LLVM IR generated in 'program.ll'\n");
    return 0;
}
