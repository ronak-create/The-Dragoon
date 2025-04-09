#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <wctype.h>

#define MAX_TOKEN_LENGTH 256

typedef enum
{
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,
    TOKEN_PARENTHESES,
    TOKEN_SEMICOLON,
    TOKEN_BOOLEAN,
    TOKEN_EOF,
    TOKEN_COMMENT,
    TOKEN_ERROR
} TokenType;

typedef struct
{
    TokenType type;
    char lexeme[MAX_TOKEN_LENGTH];
    int line;
} Token;

const char *keywords[] = {"abstract", "arguments", "await", "boolean", "break", "byte", "case", "catch", "char", "class", "const", "continue", "debugger", "default", "delete", "do", "double", "else", "enum", "eval", "export", "extends", "false", "final", "finally", "float", "for", "function", "goto", "if", "implements", "import", "in", "instanceof", "int", "interface", "let", "long", "native", "new", "null", "package", "private", "protected", "public", "return", "short", "static", "super", "switch", "synchronized", "this", "throw", "throws", "transient", "try", "typeof", "var", "void", "volatile", "while", "with", "yield"};

const char *built_in_objects[] = {"Object", "Function", "Boolean", "Symbol", "Error", "EvalError", "RangeError", "ReferenceError", "SyntaxError", "TypeError", "URIError", "Number", "BigInt", "Math", "Date", "String", "RegExp", "Array", "Int8Array", "Uint8Array", "Uint8ClampedArray", "Int16Array", "Uint16Array", "Int32Array", "Uint32Array", "BigInt64Array", "BigUint64Array", "Float32Array", "Float64Array", "ArrayBuffer", "SharedArrayBuffer", "DataView", "Map", "Set", "WeakMap", "WeakSet", "JSON", "Atomics", "Promise", "Generator", "GeneratorFunction", "AsyncFunction", "Reflect", "Proxy"};


int is_builtInObject(const char *str)
{
    for (size_t i = 0; i < sizeof(built_in_objects) / sizeof(built_in_objects[0]); i++)
    {
        if (strcmp(str, built_in_objects[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int is_keyword(const char *str)
{
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
    {
        if (strcmp(str, keywords[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int advance(FILE *file)
{
    return fgetc(file); // Read the next character from the file
}

int peek(FILE *file)
{
    int ch = fgetc(file); // Read the next character
    ungetc(ch, file);     // Put it back for future reads
    return ch;            // Return the character
}

Token get_next_token(FILE *file)
{
    static int line = 1;
    int ch;

    while ((ch = fgetc(file)) != EOF)
    {
        if (isspace(ch))
        {
            if (ch == '\n')
                line++;
            continue;
        }

        Token token;
        token.line = line;

        if (isalpha(ch) || ch == '_')
        {
            // Identifier or keyword
            char buffer[MAX_TOKEN_LENGTH] = {ch};
            int index = 1;

            while (isalnum(ch = fgetc(file)) || ch == '_')
            {
                if (index < MAX_TOKEN_LENGTH - 1)
                {
                    buffer[index++] = ch;
                }
            }
            buffer[index] = '\0';
            ungetc(ch, file);

            strcpy(token.lexeme, buffer);
            //prepare above conditions in if else if statement:
            if (is_keyword(buffer))
            token.type = TOKEN_KEYWORD;
            else if ((strcmp(buffer, "true") == 0 || strcmp(buffer, "false") == 0))
            token.type = TOKEN_BOOLEAN;
            else
            token.type = TOKEN_IDENTIFIER;
            // token.type = (strcmp(buffer, "true") == 0 || strcmp(buffer, "false") == 0) ? TOKEN_BOOLEAN : TOKEN_IDENTIFIER;
            return token;
        }

        if (isdigit(ch))
        {
            char buffer[MAX_TOKEN_LENGTH];
            int index = 0;
            int has_dot = 0, has_exp = 0;

            buffer[index++] = ch;

            if (ch == '0')
            {
                // Check for hex, binary, octal
                char next = fgetc(file);
                if (next == 'x' || next == 'X')
                { // Hexadecimal
                    buffer[index++] = next;
                    while (isxdigit(ch = fgetc(file)))
                        buffer[index++] = ch;
                }
                else if (next == 'b' || next == 'B')
                { // Binary
                    buffer[index++] = next;
                    while (ch == '0' || ch == '1')
                        buffer[index++] = ch, ch = fgetc(file);
                }
                else if (next == 'o' || next == 'O')
                { // Octal
                    buffer[index++] = next;
                    while (ch >= '0' && ch <= '7')
                        buffer[index++] = ch, ch = fgetc(file);
                }
                else
                {
                    ungetc(next, file); // Put back if not part of a special format
                }
                if (!isalnum(ch) && ch != '_')
                {
                    ungetc(ch, file);
                }
            }
            else
            {
                while (1)
                {
                    ch = fgetc(file);

                    if (isdigit(ch))
                    {
                        buffer[index++] = ch;
                    }
                    else if (ch == '.' && !has_dot)
                    {
                        // Handle decimal point (only one allowed)
                        has_dot = 1;
                        buffer[index++] = ch;
                    }
                    else if ((ch == 'e' || ch == 'E') && !has_exp)
                    {
                        // Handle scientific notation
                        has_exp = 1;
                        buffer[index++] = ch;
                        ch = fgetc(file);

                        if (ch == '+' || ch == '-')
                        {
                            // Allow exponent sign
                            buffer[index++] = ch;
                        }
                        else
                        {
                            ungetc(ch, file); // Put back if not a valid sign
                        }
                    }
                    else
                    {
                        ungetc(ch, file); // Stop processing
                        break;
                    }
                }
            }

            buffer[index] = '\0';
            strcpy(token.lexeme, buffer);
            token.type = TOKEN_NUMBER;
            return token;
        }

        if (ch == '`')
        {
            advance(file);
            while (ch != '`' && ch != '\0')
            {
                if (ch == '$' && peek(file) == '{')
                {                  // Handle `${}`
                    advance(file); // Consume '$'
                    advance(file); // Consume '{'
                    while (ch != '}' && ch != '\0')
                    {
                        advance(file);
                    }
                    if (ch == '}')
                        advance(file); // Consume '}'
                }
                advance(file);
            }
            if (ch == '`')
                advance(file); // Consume closing backtick
            return (Token){TOKEN_STRING, "TEMPLATE_LITERAL"};
        }

        // if (ch == 'true' || ch == 'false')
        // {
        //     advance(file);
        //     return (Token){TOKEN_BOOLEAN, ch == 't' ? "true" : "false"};
        // }

        if (ch == '"' || ch == '\'')
        {
            char quoteType = ch;
            int index = 0;

            while ((ch = fgetc(file)) != quoteType && ch != EOF)
            {
                if (ch == '\\')
                { // Handle escape sequences
                    char next = fgetc(file);
                    if (next == 'n')
                        token.lexeme[index++] = '\n';
                    else if (next == 't')
                        token.lexeme[index++] = '\t';
                    else if (next == '\\')
                        token.lexeme[index++] = '\\';
                    else if (next == '"')
                        token.lexeme[index++] = '"';
                    else if (next == '\'')
                        token.lexeme[index++] = '\'';
                    else
                        token.lexeme[index++] = next;
                }
                else
                {
                    token.lexeme[index++] = ch;
                }
            }

            token.lexeme[index] = '\0';
            token.type = TOKEN_STRING;
            return token;
        }
        if (ch == '/')
        {
            char next = fgetc(file);

            if (next == '/')
            {
                // Single-line comment
                while ((ch = fgetc(file)) != '\n' && ch != EOF)
                    ;
                return get_next_token(file);
            }
            else if (next == '*')
            {
                // Multi-line comment
                while ((ch = fgetc(file)) != EOF)
                {
                    if (ch == '*' && fgetc(file) == '/')
                    {
                        break;
                    }
                }
                return get_next_token(file);
            }
            else
            {
                ungetc(next, file);
                token.lexeme[0] = '/';
                token.lexeme[1] = '\0';
                token.type = TOKEN_OPERATOR;
                return token;
            }
        }

        if (ch == '=' || ch == '!' || ch == '<' || ch == '>' || ch == '&' || ch == '|')
        {
            token.lexeme[0] = ch;
            token.lexeme[1] = '\0';

            char next = fgetc(file);
            if ((ch == '=' && next == '=') || (ch == '!' && next == '=') ||
                (ch == '<' && next == '=') || (ch == '>' && next == '=') ||
                (ch == '&' && next == '&') || (ch == '|' && next == '|'))
            {
                token.lexeme[1] = next;
                token.lexeme[2] = '\0';

                if ((ch == '=' && next == '=') || (ch == '!' && next == '='))
                {
                    // Check for triple equals (===) or !==
                    char next2 = fgetc(file);
                    if (next2 == '=')
                    {
                        token.lexeme[2] = next2;
                        token.lexeme[3] = '\0';
                    }
                    else
                    {
                        ungetc(next2, file);
                    }
                }
            }
            else
            {
                ungetc(next, file);
            }

            token.type = TOKEN_OPERATOR;
            return token;
        }

        if (ch == '*' && peek(file) == '*')
        {
            advance(file);
            advance(file);
            return (Token){TOKEN_OPERATOR, "EXPONENTIATION"};
        }
        if (ch == '?' && peek(file) == '?')
        {
            advance(file);
            advance(file);
            return (Token){TOKEN_OPERATOR, "NULLISH_COALESCING"};
        }
        if (ch == '?' && peek(file) == '.')
        {
            advance(file);
            advance(file);
            return (Token){TOKEN_OPERATOR, "OPTIONAL_CHAINING"};
        }

        if (ch == '/' && peek(file) == '*')
        {
            advance(file); // Consume '/'
            advance(file); // Consume '*'
            while (ch != '\0' && !(ch == '*' && peek(file) == '/'))
            {
                advance(file);
            }
            if (ch == '*')
            {
                advance(file); // Consume '*'
                advance(file); // Consume '/'
            }
            return (Token){TOKEN_COMMENT, "MULTI_LINE_COMMENT"};
        }

        if (strchr("+-*/=<>!&|", ch))
        {
            // Handle operators
            token.lexeme[0] = ch;
            token.lexeme[1] = '\0';
            token.type = TOKEN_OPERATOR;

            // Check for two-character operators (e.g., `==`, `!=`, `&&`, `||`)
            char next = fgetc(file);
            if ((ch == '=' && (next == '=')) || // ==
                (ch == '!' && next == '=') ||   // !=
                (ch == '&' && next == '&') ||   // &&
                (ch == '|' && next == '|'))     // ||
            {
                token.lexeme[1] = next;
                token.lexeme[2] = '\0';
            }
            else
            {
                ungetc(next, file); // Put back if not a double operator
            }
            return token;
        }
        else if (strchr(";,.(){}[]", ch))
        {
            // Handle punctuation
            token.lexeme[0] = ch;
            token.lexeme[1] = '\0';
            // if it is '{' or '}' then its type should be TOKEN_PARENTHESES and if it is ';' its type should be TOKEN_SEMICOLON
            if (ch == '{' || ch == '}')
            {
                token.type = TOKEN_PARENTHESES;
            }
            else if (ch == ';')
            {
                token.type = TOKEN_SEMICOLON;
            }
            else
            {
                token.type = TOKEN_PUNCTUATION;
            }
            return token;
        }

        token.type = TOKEN_ERROR;
        sprintf(token.lexeme, "Unknown: %c", ch);
        return token;
    }

    return (Token){TOKEN_EOF, "EOF", line};
}

// int main(int argc, char *argv[])
// {
//     if (argc < 2)
//     {
//         printf("Usage: %s <filename>\n", argv[0]);
//         return 1;
//     }

//     FILE *file = fopen(argv[1], "r");
//     if (!file)
//     {
//         printf("Error opening file: %s\n", argv[1]);
//         return 1;
//     }

//     Token token;
//     do
//     {
//         token = get_next_token(file);
//         printf("Token: %-10s | Lexeme: %-15s | Line: %d\n",
//                (token.type == TOKEN_IDENTIFIER ? "IDENTIFIER" : token.type == TOKEN_KEYWORD   ? "KEYWORD"
//                                                             : token.type == TOKEN_NUMBER      ? "NUMBER"
//                                                             : token.type == TOKEN_STRING      ? "STRING"
//                                                             : token.type == TOKEN_OPERATOR    ? "OPERATOR"
//                                                             : token.type == TOKEN_PUNCTUATION ? "PUNCTUATION"
//                                                             : token.type == TOKEN_COMMENT     ? "COMMENT"
//                                                             : token.type == TOKEN_ERROR       ? "ERROR"
//                                                                                               : "EOF"),
//                token.lexeme, token.line);
//     } while (token.type != TOKEN_EOF);

//     fclose(file);
//     return 0;
// }

// int main(int argc, char *argv[])
// {
//     if (argc < 2)
//     {
//         printf("Usage: %s <filename>\n", argv[0]);
//         return 1;
//     }

//     FILE *file = fopen(argv[1], "r");
//     if (!file)
//     {
//         printf("Error opening file: %s\n", argv[1]);
//         return 1;
//     }

//     FILE *outputFile = fopen("tokens.txt", "w");
//     if (!outputFile)
//     {
//         printf("Error opening output file.\n");
//         fclose(file);
//         return 1;
//     }

//     fprintf(outputFile, "Token tokens[] = {\n");

//     Token token;
//     do
//     {
//         token = get_next_token(file);
//         const char *tokenType = (token.type == TOKEN_IDENTIFIER    ? "TOKEN_IDENTIFIER"
//                                  : token.type == TOKEN_KEYWORD     ? "TOKEN_KEYWORD"
//                                  : token.type == TOKEN_NUMBER      ? "TOKEN_NUMBER"
//                                  : token.type == TOKEN_STRING      ? "TOKEN_STRING"
//                                  : token.type == TOKEN_OPERATOR    ? "TOKEN_OPERATOR"
//                                  : token.type == TOKEN_PARENTHESES ? "TOKEN_PARENTHESES"
//                                  : token.type == TOKEN_SEMICOLON   ? "TOKEN_SEMICOLON"
//                                  : token.type == TOKEN_PUNCTUATION ? "TOKEN_PUNCTUATION"
//                                  : token.type == TOKEN_COMMENT     ? "TOKEN_COMMENT"
//                                  : token.type == TOKEN_BOOLEAN     ? "TOKEN_BOOLEAN"
//                                  : token.type == TOKEN_ERROR       ? "TOKEN_ERROR"
//                                                                    : "TOKEN_EOF");

//         printf("Token: %-10s | Lexeme: %-15s | Line: %d\n",
//                tokenType, token.lexeme, token.line);

//         if (token.type != TOKEN_EOF)
//         {
//             fprintf(outputFile, "    {%s, \"%s\", %d},\n", tokenType, token.lexeme, token.line);
//         }

//     } while (token.type != TOKEN_EOF);

//     fprintf(outputFile, "};\n");

//     fclose(outputFile);
//     fclose(file);
//     return 0;
// }