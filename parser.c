#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"

// typedef enum
// {
//     TOKEN_IDENTIFIER,
//     TOKEN_KEYWORD,
//     TOKEN_NUMBER,
//     TOKEN_STRING,
//     TOKEN_OPERATOR,
//     TOKEN_BINARY_EXPRESSION,
//     TOKEN_SEMICOLON,
//     TOKEN_PARENTHESES,
//     TOKEN_PUNCTUATION,
//     TOKEN_BOOLEAN,
//     TOKEN_COMMENT,
//     TOKEN_ERROR,
//     TOKEN_EOF
// } TokenType;
// #define MAX_SYMBOLS 100

// typedef struct
// {
//     char name[50];
//     char type[50];
//     char scope[50];
//     char value[100];
//     char datatype[50];
// } Symbol;

// Symbol symbolTable[MAX_SYMBOLS];
int symbol_table_size = 0;
int symbolCount = 0;

int lookupSymbol(char *name)
{
    for (int i = 0; i < symbolCount; i++)
    {
        if (strcmp(symbolTable[i].name, name) == 0)
        {
            return i; // Return index if found
        }
    }
    return -1; // Not found
}

void insertSymbol(char *name, char *type, char *scope, char *value, char *datatype_check)
{
    if (lookupSymbol(name) != -1)
    {
        printf("Error: Variable '%s' is already declared!\n", name);
        return;
    }
    // print a,ll the parameters
    printf("Inserting Symbol: Name: %s, Type: %s, Scope: %s, Value: %s, Datatype: %s\n", name, type, scope, value, datatype_check);
    if (symbolCount < MAX_SYMBOLS)
    {
        strcpy(symbolTable[symbolCount].name, name);
        strcpy(symbolTable[symbolCount].type, type);
        strcpy(symbolTable[symbolCount].scope, scope);
        strcpy(symbolTable[symbolCount].value, value);
        strcpy(symbolTable[symbolCount].datatype, datatype_check);
        symbolCount++;
    }
    else
    {
        printf("Error: Symbol table overflow!\n");
    }
}

// Token Structure----------------------------------------------

// typedef struct
// {
//     TokenType type;
//     char lexeme[50];
//     int line;
// } Token;

// AST Nodes------------------------------------------------------

// typedef enum
// {
//     AST_VAR_DECL,
//     AST_ASSIGNMENT,
//     AST_LITERAL,
//     AST_BINARY_OP,
//     AST_IF_STMT,
//     AST_ELSE_STMT,
//     AST_BLOCK,
//     AST_WHILE_STMT,
//     AST_FOR_STMT,
//     AST_PRE_UPDATE,
//     AST_POST_UPDATE,
//     AST_FUNCTION,
//     AST_IDENTIFIER,
//     AST_LOG_STMT,
//     AST_FUNC_CALL
// } ASTNodeType;

// typedef struct ASTNode
// {
//     ASTNodeType type;
//     char *value;           // Variable name, operator, or literal
//     struct ASTNode *left;  // LHS (for assignment, binary op)
//     struct ASTNode *right; // RHS (for assignment, binary op)
//     struct ASTNode **body; // For block statements
//     int body_size;
// } ASTNode;

// Parsing Functions----------------------------------------------
ASTNode *parse_statement(Token tokens[], int *index);

// Function to create AST nodes
ASTNode *create_node(ASTNodeType type, char *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->left = node->right = NULL;
    node->body = NULL;
    node->body_size = 0;
    return node;
}

char *check_binary_expr(char *leftType, char *rightType, char op)
{
    if (strcmp(leftType, "number") == 0 && strcmp(rightType, "number") == 0)
    {
        return "number";
    }
    if (op == '+' && (strcmp(leftType, "string") == 0 || strcmp(rightType, "string") == 0))
    {
        return "string"; // String concatenation
    }
    printf("Type Error: Cannot apply '%c' to %s and %s\n", op, leftType, rightType);
    exit(1);
}

// Parsing an expression (numbers, identifiers, function calls)
ASTNode *parse_expression(Token tokens[], int *index)
{
    Token token = tokens[*index];

    if (token.type == TOKEN_NUMBER || token.type == TOKEN_STRING || token.type == TOKEN_IDENTIFIER || token.type == TOKEN_BOOLEAN)
    {
        (*index)++;
        return create_node(AST_LITERAL, token.lexeme);
    }

    return NULL;
}

ASTNode *parse_condition(Token tokens[], int *index)
{

    ASTNode *identifier = create_node(AST_IDENTIFIER, tokens[*index].lexeme);
    (*index)++;
    ASTNode *condition = create_node(AST_BINARY_OP, tokens[*index].lexeme);
    (*index)++;
    ASTNode *value = create_node(AST_LITERAL, tokens[*index].lexeme);
    condition->left = identifier;
    condition->right = value;

    (*index)++;
    return condition;
}

// Parsing a variable declaration (let/const x = expr)
ASTNode *parse_declaration(Token tokens[], int *index)
{
    Token keyword = tokens[*index]; // "let" or "const"
    (*index)++;
    Token identifier = tokens[*index]; // Variable name
    (*index)++;
    (*index)++;                   // Skip "="
    Token value = tokens[*index]; // Value

    ASTNode *varNode = create_node(AST_VAR_DECL, identifier.lexeme);
    ASTNode *assignNode = create_node(AST_ASSIGNMENT, "=");
    assignNode->left = varNode;
    assignNode->right = parse_expression(tokens, index);
    // print keyword lexmee
    printf("Keyword: %s\n", keyword.lexeme);
    printf("Keyword: %s\n", keyword.lexeme == "let" ? "VARIABLE" : (keyword.lexeme == "const" ? "CONSTANT" : "UNDEFINED"));
    insertSymbol(identifier.lexeme, keyword.lexeme == "let" ? "VARIABLE" : (keyword.lexeme == "const" ? "CONSTANT" : "UNDEFINED"), "LOCAL", value.lexeme, value.type == TOKEN_STRING ? "string" : (value.type == TOKEN_BOOLEAN ? "boolean" : "number")); // Insert into symbol table
    (*index)++;                                                                                                                                                                                                                                          // Skip ";"
    return assignNode;
}

// Parsing a print statement (console.log(expr))
ASTNode *parse_print_stmt(Token tokens[], int *index)
{
    (*index)++; // Skip "console"
    (*index)++; // Skip "."
    (*index)++; // Skip "log"
    (*index)++; // Skip "("
    ASTNode *expr = parse_expression(tokens, index);
    (*index)++; // Skip ")"
    (*index)++; // Skip ";"
    return create_node(AST_FUNC_CALL, "console.log");
}

ASTNode *parser_conditional_statement(Token tokens[], int *index)
{
    Token conditionKey = tokens[*index];
    (*index)++; // Skip "if" or "else"
    ASTNode *condition = NULL;

    if (strcmp(conditionKey.lexeme, "if") == 0)
    {
        if (tokens[*index].type != TOKEN_PUNCTUATION || strcmp(tokens[*index].lexeme, "(") != 0)
        {
            printf("Error: Expected '(' after 'if'\n");
            exit(1);
        }

        (*index)++; // Skip "("
        condition = parse_condition(tokens, index);

        if (tokens[*index].type != TOKEN_PUNCTUATION || strcmp(tokens[*index].lexeme, ")") != 0)
        {
            printf("Error: Expected ')' after condition\n");
            exit(1);
        }
        (*index)++; // Skip ")"
    }

    if (tokens[*index].type != TOKEN_PARENTHESES || strcmp(tokens[*index].lexeme, "{") != 0)
    {
        printf("Error: Expected '{' after if condition\n");
        exit(1);
    }
    (*index)++; // Skip "{"
    // Create block node
    ASTNode *block = create_node(AST_BLOCK, NULL);
    block->body = malloc(sizeof(ASTNode *) * 10); // Initial allocation
    block->body_size = 0;
    int capacity = 10;

    while (!(tokens[*index].type == TOKEN_PARENTHESES && strcmp(tokens[*index].lexeme, "}") == 0))
    {
        if (tokens[*index].type == TOKEN_EOF)
        {
            printf("Error: Unexpected end of file. Missing closing '}'.\n");
            exit(1);
        }

        // Expand memory if needed
        if (block->body_size >= capacity)
        {
            capacity *= 2;
            block->body = realloc(block->body, sizeof(ASTNode *) * capacity);
        }

        // Parse statement and add to block
        block->body[block->body_size++] = parse_statement(tokens, index);
    }
    // printf("Not printing This");
    (*index)++; // Skip "}"

    ASTNode *conditionNode = create_node(strcmp(conditionKey.lexeme, "if") == 0 ? AST_IF_STMT : AST_ELSE_STMT, strcmp(conditionKey.lexeme, "if") == 0 ? "if" : "else");
    if (strcmp(conditionKey.lexeme, "if") == 0)
    {
        conditionNode->left = condition;
    }
    conditionNode->right = block;
    // prin this node
    return conditionNode;
}

ASTNode *parse_update(Token tokens[], int *index)
{
    if (strcmp(tokens[*index].lexeme, "+") == 0 || strcmp(tokens[*index].lexeme, "-") == 0)
    {
        ASTNode *updateNode = create_node(AST_PRE_UPDATE, NULL);
        // how do i add all three lexemes like lexmee 1 is + 2nd is + and 3rd is i
        updateNode->value = strcat(strcat(tokens[*index].lexeme, tokens[(*index) + 1].lexeme), tokens[(*index) + 2].lexeme);
        (*index) += 3;
        return updateNode;
    }
    else if (tokens[*index].type == TOKEN_IDENTIFIER)
    {
        ASTNode *updateNode = create_node(AST_POST_UPDATE, tokens[*index].lexeme);
        updateNode->value = strcat(strcat(tokens[*index].lexeme, tokens[(*index) + 1].lexeme), tokens[(*index) + 2].lexeme);
        (*index) += 3;
        // print the string
        return updateNode;
    }
}

ASTNode *parser_looping_statement(Token tokens[], int *index)
{
    Token loopKey = tokens[*index];
    (*index)++; // Skip "for" or "while" or "do while"
    if (strcmp(tokens[*index].lexeme, "(") != 0)
    {
        printf("Error: Expected '('\n");
        return NULL;
    }
    (*index)++; // Skip "("
    if (strcmp(loopKey.lexeme, "while") == 0)
    {
        ASTNode *condition = parse_condition(tokens, index);
        if (strcmp(tokens[*index].lexeme, ")") != 0)
        {
            printf("Error: Expected ')'\n");
            return NULL;
        }
        (*index)++; // Move past ')'
        if (tokens[*index].type != TOKEN_PARENTHESES || strcmp(tokens[*index].lexeme, "{") != 0)
        {
            printf("Error: Expected '{' after if condition\n");
            exit(1);
        }
        (*index)++; // Skip "{"
        ASTNode *block = create_node(AST_BLOCK, NULL);
        block->body = malloc(sizeof(ASTNode *) * 10); // Initial allocation
        block->body_size = 0;
        int capacity = 10;

        while (!(tokens[*index].type == TOKEN_PARENTHESES && strcmp(tokens[*index].lexeme, "}") == 0))
        {
            if (tokens[*index].type == TOKEN_EOF)
            {
                printf("Error: Unexpected end of file. Missing closing '}'.\n");
                exit(1);
            }

            // Expand memory if needed
            if (block->body_size >= capacity)
            {
                capacity *= 2;
                block->body = realloc(block->body, sizeof(ASTNode *) * capacity);
            }

            // Parse statement and add to block
            block->body[block->body_size++] = parse_statement(tokens, index);
        }
        // printf("Not printing This");
        (*index)++; // Skip "}"

        ASTNode *whileNode = create_node(AST_WHILE_STMT, "while");
        whileNode->left = condition;
        whileNode->right = block;
        // prin this node
        return whileNode;
    }
    else if (strcmp(loopKey.lexeme, "for") == 0)
    {
        // Token identifier = tokens[(*index)++];           // Identifier
        ASTNode *init = parse_declaration(tokens, index);    // Initialization
        ASTNode *condition = parse_condition(tokens, index); // Condition
        (*index)++;
        ASTNode *update = parse_update(tokens, index); // Update
        (*index)++;
        if (tokens[*index].type != TOKEN_PARENTHESES || strcmp(tokens[*index].lexeme, "{") != 0)
        {
            printf("Error: Expected '{' after if condition\n");
            exit(1);
        }
        (*index)++; // Skip "{"
        ASTNode *block = create_node(AST_BLOCK, NULL);
        block->body = malloc(sizeof(ASTNode *) * 10); // Initial allocation
        block->body_size = 0;
        int capacity = 10;

        while (!(tokens[*index].type == TOKEN_PARENTHESES && strcmp(tokens[*index].lexeme, "}") == 0))
        {
            if (tokens[*index].type == TOKEN_EOF)
            {
                printf("Error: Unexpected end of file. Missing closing '}'.\n");
                exit(1);
            }

            // Expand memory if needed
            if (block->body_size >= capacity)
            {
                capacity *= 2;
                block->body = realloc(block->body, sizeof(ASTNode *) * capacity);
            }

            // Parse statement and add to block
            block->body[block->body_size++] = parse_statement(tokens, index);
        }
        // printf("Not printing This");
        (*index)++; // Skip "}"

        ASTNode *forNode = create_node(AST_FOR_STMT, "for");
        forNode->left = init;
        forNode->left->right = condition;
        forNode->right = update;
        forNode->right->right = block;
        // forNode->body = block->body;
        // print the forNode
        // printf("For Node: %s\n", forNode->value);
        // prin this node
        return forNode;
    }
}

// Parse a single statement
ASTNode *parse_statement(Token tokens[], int *index)
{
    if (strcmp(tokens[*index].lexeme, "console") == 0)
    {

        return parse_print_stmt(tokens, index);
    }
    if (tokens[*index].type == TOKEN_KEYWORD)
    {
        if (strcmp(tokens[*index].lexeme, "let") == 0 || strcmp(tokens[*index].lexeme, "const") == 0)
        {
            return parse_declaration(tokens, index);
        }
        else if (strcmp(tokens[*index].lexeme, "if") == 0 || strcmp(tokens[*index].lexeme, "else") == 0)
        {
            return parser_conditional_statement(tokens, index);
        }
        else if (strcmp(tokens[*index].lexeme, "for") == 0 || strcmp(tokens[*index].lexeme, "while") == 0)
        {
            return parser_looping_statement(tokens, index);
        }
    }
    if (tokens[*index].type == TOKEN_EOF)
    {
        return NULL;
    }
    // print the index
    printf("Unexpected token: %s, with line: %d, skipping...\n", tokens[*index].lexeme, tokens[*index].line);
    (*index)++;
    return NULL;
}

void printSymbolTable()
{
    printf("\nSymbol Table:\n");
    printf("----------------------------------------------------------------------------\n");
    printf("| %-10s | %-10s | %-10s | %-15s | %-10s |\n", "Name", "Type", "Scope", "Value", "Datatype");
    printf("----------------------------------------------------------------------------\n");

    for (int i = 0; i < symbolCount; i++)
    {
        printf("| %-10s | %-10s | %-10s | %-15s | %-10s |\n",
               symbolTable[i].name,
               symbolTable[i].type,
               symbolTable[i].scope,
               symbolTable[i].value,
               symbolTable[i].datatype);
    }
    printf("----------------------------------------------------------------------------\n");
}

// Constant Folding
ASTNode *fold_constants(ASTNode *node)
{
    if (!node)
        return NULL;
    node->left = fold_constants(node->left);
    node->right = fold_constants(node->right);

    if (node->type == AST_BINARY_OP && node->left && node->right)
    {
        if (node->left->type == AST_LITERAL && node->right->type == AST_LITERAL)
        {
            int left_val = atoi(node->left->value);
            int right_val = atoi(node->right->value);
            int result;
            if (strcmp(node->value, "+") == 0)
                result = left_val + right_val;
            else if (strcmp(node->value, "-") == 0)
                result = left_val - right_val;
            else if (strcmp(node->value, "*") == 0)
                result = left_val * right_val;
            else if (strcmp(node->value, "/") == 0)
                result = right_val != 0 ? left_val / right_val : 0;

            char buffer[20];
            sprintf(buffer, "%d", result);
            return create_node(AST_LITERAL, buffer);
        }
    }
    return node;
}

// Dead Code Elimination
ASTNode *eliminate_dead_code(ASTNode *node)
{
    if (!node)
        return NULL;
    if (node->type == AST_IF_STMT)
    {
        node->left = fold_constants(node->left);
        if (node->left->type == AST_LITERAL)
        {
            if (atoi(node->left->value) == 0)
                return NULL;
            return eliminate_dead_code(node->right);
        }
    }
    return node;
}

// Main----------------------------------------------------------

void print_ast(ASTNode *node, int depth)
{
    for (int i = 0; i < depth; i++)
        printf("  ");

    if (node->type == AST_VAR_DECL)
        printf("VarDecl(%s)\n", node->value);
    else if (node->type == AST_ASSIGNMENT)
    {
        printf("Assign\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
    }
    else if (node->type == AST_BINARY_OP)
    {
        printf("Condition\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
    }
    else if (node->type == AST_IDENTIFIER)
        printf("Identifier(%s)\n", node->value);
    else if (node->type == AST_FUNC_CALL)
        printf("FuncCall(%s)\n", node->value);
    else if (node->type == AST_LITERAL)
        printf("Literal(%s)\n", node->value);
    else if (node->type == AST_IF_STMT)
    {
        printf("IfStmt\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
    }
    else if (node->type == AST_WHILE_STMT)
    {
        printf("WhileStmt\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
    }
    else if (node->type == AST_POST_UPDATE)
    {
        printf("PostUpdate(%s)\n", node->value);
    }
    else if (node->type == AST_PRE_UPDATE)
    {
        printf("PreUpdate(%s)\n", node->value);
    }
    else if (node->type == AST_FOR_STMT)
    {
        printf("ForStmt\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
        print_ast(node->right->right, depth + 1);
    }
    else if (node->type == AST_ELSE_STMT)
    {
        printf("ElseStmt\n");
        print_ast(node->right, depth + 1);
    }
    else if (node->type == AST_BLOCK)
    {
        printf("Block\n");
        for (int i = 0; i < node->body_size; i++)
        {
            print_ast(node->body[i], depth + 1);
        }
    }
}

int tempCount = 0;
int labelCount = 0;

char *new_temp()
{
    static char temp[10];
    sprintf(temp, "t%d", tempCount++);
    return temp;
}

char *new_label()
{
    static char label[10];
    sprintf(label, "L%d", labelCount++);
    return label;
}

char *generate_expr(ASTNode *node)
{
    if (!node)
        return "";

    char *t1, *t2, *result;

    switch (node->type)
    {
    case AST_LITERAL:
    case AST_IDENTIFIER:
        return node->value;

    case AST_BINARY_OP:
        t1 = generate_expr(node->left);
        t2 = generate_expr(node->right);
        result = strdup(new_temp());
        printf("%s = %s %s %s\n", result, t1, node->value, t2);
        return result;

    default:
        return "";
    }
}

void generate_tac(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case AST_ASSIGNMENT:
    {
        char *rhs = generate_expr(node->right);
        printf("%s = %s\n", node->left->value, rhs);
        break;
    }

    case AST_IF_STMT:
    {
        char *cond = generate_expr(node->left);
        char *falseLabel = new_label();
        printf("ifFalse %s goto %s\n", cond, falseLabel);
        generate_tac(node->right); // if block
        printf("%s:\n", falseLabel);
        break;
    }

    case AST_WHILE_STMT:
    {
        char *startLabel = new_label();
        char *endLabel = new_label();
        printf("%s:\n", startLabel);
        char *cond = generate_expr(node->left);
        printf("ifFalse %s goto %s\n", cond, endLabel);
        generate_tac(node->right); // loop body
        printf("goto %s\n", startLabel);
        printf("%s:\n", endLabel);
        break;
    }

    case AST_FOR_STMT:
    {
        // assume left is init, right is condition + update + block
        ASTNode *init = node->left;
        ASTNode *cond = node->right->left;
        ASTNode *update = node->right->right;
        ASTNode *body = node->right->body[0];

        generate_tac(init); // initialization

        char *startLabel = new_label();
        char *endLabel = new_label();
        printf("%s:\n", startLabel);

        char *c = generate_expr(cond);
        printf("ifFalse %s goto %s\n", c, endLabel);

        generate_tac(body);   // loop body
        generate_tac(update); // post update

        printf("goto %s\n", startLabel);
        printf("%s:\n", endLabel);
        break;
    }

    case AST_BLOCK:
        for (int i = 0; i < node->body_size; i++)
            generate_tac(node->body[i]);
        break;

    case AST_FUNC_CALL:
        // assuming node->value is function name, and arguments in body[]
        for (int i = 0; i < node->body_size; i++)
        {
            char *arg = generate_expr(node->body[i]);
            printf("param %s\n", arg);
        }
        printf("call %s, %d\n", node->value, node->body_size);
        break;

    default:
        break;
    }
}

void free_ast(ASTNode *node)
{
    if (node == NULL)
        return;

    free_ast(node->left);
    free_ast(node->right);
    for (int i = 0; i < node->body_size; i++)
    {
        free_ast(node->body[i]);
    }
    free(node->body);
    free(node->value);
    free(node);
}

int regCount = 0;

char *get_register()
{
    static char reg[10];
    sprintf(reg, "R%d", regCount++);
    return reg;
}

void generate_instructions(ASTNode *node)
{
    if (!node)
        return;

    char *reg1, *reg2, *dest;
    char label[10];

    switch (node->type)
    {
    case AST_ASSIGNMENT:
        reg1 = get_register();
        generate_instructions(node->right); // load value
        printf("STORE %s, %s\n", node->left->value, reg1);
        break;

    case AST_LITERAL:
        reg1 = get_register();
        printf("MOV %s, %s\n", reg1, node->value);
        break;

    case AST_IDENTIFIER:
        reg1 = get_register();
        printf("LOAD %s, %s\n", reg1, node->value);
        break;

    case AST_BINARY_OP:
        reg1 = get_register();
        generate_instructions(node->left);
        reg2 = get_register();
        generate_instructions(node->right);
        dest = get_register();

        if (strcmp(node->value, "+") == 0)
            printf("ADD %s, %s, %s\n", dest, reg1, reg2);
        else if (strcmp(node->value, "-") == 0)
            printf("SUB %s, %s, %s\n", dest, reg1, reg2);
        else if (strcmp(node->value, "*") == 0)
            printf("MUL %s, %s, %s\n", dest, reg1, reg2);
        else if (strcmp(node->value, "/") == 0)
            printf("DIV %s, %s, %s\n", dest, reg1, reg2);
        break;

    case AST_IF_STMT:
        sprintf(label, "L%d", labelCount++);
        printf("CMP ");
        generate_instructions(node->left);
        printf("\n");
        printf("JE %s\n", label);
        generate_instructions(node->right);
        printf("%s:\n", label);
        break;

    case AST_BLOCK:
        for (int i = 0; i < node->body_size; i++)
            generate_instructions(node->body[i]);
        break;

    default:
        break;
    }
}

// int main()
// {
//     Token tokens[] = {
//         {TOKEN_KEYWORD, "let", 1},
//         {TOKEN_IDENTIFIER, "num", 1},
//         {TOKEN_OPERATOR, "=", 1},
//         {TOKEN_NUMBER, "42", 1},
//         {TOKEN_SEMICOLON, ";", 1},
//         {TOKEN_KEYWORD, "const", 2},
//         {TOKEN_IDENTIFIER, "hex", 2},
//         {TOKEN_OPERATOR, "=", 2},
//         {TOKEN_NUMBER, "0x1F", 2},
//         {TOKEN_SEMICOLON, ";", 2},
//         {TOKEN_KEYWORD, "const", 3},
//         {TOKEN_IDENTIFIER, "bin", 3},
//         {TOKEN_OPERATOR, "=", 3},
//         {TOKEN_NUMBER, "0b01010", 3},
//         {TOKEN_SEMICOLON, ";", 3},
//         {TOKEN_KEYWORD, "const", 4},
//         {TOKEN_IDENTIFIER, "str", 4},
//         {TOKEN_OPERATOR, "=", 4},
//         {TOKEN_STRING, "Hello World", 4},
//         {TOKEN_SEMICOLON, ";", 4},
//         {TOKEN_KEYWORD, "const", 5},
//         {TOKEN_IDENTIFIER, "bool", 5},
//         {TOKEN_OPERATOR, "=", 5},
//         {TOKEN_BOOLEAN, "true", 5},
//         {TOKEN_SEMICOLON, ";", 5},
//         {TOKEN_KEYWORD, "if", 6},
//         {TOKEN_PUNCTUATION, "(", 6},
//         {TOKEN_IDENTIFIER, "num", 6},
//         {TOKEN_OPERATOR, "===", 6},
//         {TOKEN_NUMBER, "42", 6},
//         {TOKEN_PUNCTUATION, ")", 6},
//         {TOKEN_PARENTHESES, "{", 6},
//         {TOKEN_IDENTIFIER, "console", 7},
//         {TOKEN_PUNCTUATION, ".", 7},
//         {TOKEN_IDENTIFIER, "log", 7},
//         {TOKEN_PUNCTUATION, "(", 7},
//         {TOKEN_IDENTIFIER, "str", 7},
//         {TOKEN_PUNCTUATION, ")", 7},
//         {TOKEN_SEMICOLON, ";", 7},
//         {TOKEN_PARENTHESES, "}", 8},
//         {TOKEN_KEYWORD, "else", 9},
//         {TOKEN_PARENTHESES, "{", 9},
//         {TOKEN_IDENTIFIER, "console", 10},
//         {TOKEN_PUNCTUATION, ".", 10},
//         {TOKEN_IDENTIFIER, "log", 10},
//         {TOKEN_PUNCTUATION, "(", 10},
//         {TOKEN_STRING, "Goodbye World", 10},
//         {TOKEN_PUNCTUATION, ")", 10},
//         {TOKEN_SEMICOLON, ";", 10},
//         {TOKEN_PARENTHESES, "}", 11},
//         {TOKEN_KEYWORD, "for", 12},
//         {TOKEN_PUNCTUATION, "(", 12},
//         {TOKEN_KEYWORD, "let", 12},
//         {TOKEN_IDENTIFIER, "i", 12},
//         {TOKEN_OPERATOR, "=", 12},
//         {TOKEN_NUMBER, "0", 12},
//         {TOKEN_SEMICOLON, ";", 12},
//         {TOKEN_IDENTIFIER, "i", 12},
//         {TOKEN_OPERATOR, "<", 12},
//         {TOKEN_NUMBER, "10", 12},
//         {TOKEN_SEMICOLON, ";", 12},
//         {TOKEN_IDENTIFIER, "i", 12},
//         {TOKEN_OPERATOR, "+", 12},
//         {TOKEN_OPERATOR, "+", 12},
//         {TOKEN_PUNCTUATION, ")", 12},
//         {TOKEN_PARENTHESES, "{", 12},
//         {TOKEN_IDENTIFIER, "console", 13},
//         {TOKEN_PUNCTUATION, ".", 13},
//         {TOKEN_IDENTIFIER, "log", 13},
//         {TOKEN_PUNCTUATION, "(", 13},
//         {TOKEN_IDENTIFIER, "i", 13},
//         {TOKEN_PUNCTUATION, ")", 13},
//         {TOKEN_SEMICOLON, ";", 13},
//         {TOKEN_PARENTHESES, "}", 14},
//         {TOKEN_EOF, "EOF", 15}};

//     int index = 0;
//     ASTNode *astList[100]; // Store multiple AST nodes
//     int astCount = 0;

//     while (tokens[index].type != TOKEN_ERROR && tokens[index].type != TOKEN_EOF)
//     {
//         ASTNode *ast = parse_statement(tokens, &index);
//         if (ast)
//         {
//             astList[astCount++] = ast; // Store the parsed AST
//             printf("AST Structure:\n");
//             print_ast(ast, 0);
//         }
//     }

//     // Generate TAC for all parsed statements
//     for (int i = 0; i < astCount; i++)
//     {
//         generate_tac(astList[i]);
//     }

//     // Free allocated memory for AST nodes
//     for (int i = 0; i < astCount; i++)
//     {
//         free_ast(astList[i]);
//     }

//     return 0;
// }
