/*---include c standard lib---*/
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SP_DELIM "\r\n\b\t "  // space delimiters
#define MAX_CANON 256         // max characters per line

long line = 1;  // line counter

// token types
typedef enum {
    IDENTIF,  // identifier
    NUMBER,
    STR_LIT,  // string literal
    OPERATOR,
    DELIMITER,  // space delimiters
    KEYWORD,
    NONE
} TokenType;

typedef struct token
{
    TokenType type;
    char*     value;
} Token;

// token list node / tuple for token categrisation '<TokenType | Token>'
typedef struct Node
{
    Token*       tok;
    struct Node* next;
} Node;

// escapse sequences table
const char* _esc[] = {"\\b", "\\f", "\\n", "\\r", "\\t", "\\v", "\\\\", "\\", "\\\""};

// standard c keywords table
const char* keywords[] = {"auto",   "break",  "case",     "char",   "const",    "continue", "default",  "do",
                          "double", "else",   "enum",     "extern", "float",    "for",      "goto",     "if",
                          "int",    "long",   "register", "return", "short",    "signed",   "sizeof",   "static",
                          "struct", "switch", "typedef",  "union",  "unsigned", "void",     "volatile", "while"};

// check if a string is a keyword
bool is_keyword(const char* string) {
    const int n = 32;  // number of c keywords

    // find keyword
    for (int i = 0; i < n; i++)
    {
        if (strcmp(string, keywords[i]))
        {
            return true;
        }
    }

    // not found
    return false;
}

/*--file input--*/
char* read_file(const char* filename) {
    // open file in reading mode
    FILE* fp = fopen(filename, "r");
    if (!fp)
    {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }

    // get the file size
    fseek(fp, 0, SEEK_END);      // set pointer to the end
    long file_size = ftell(fp);  // set size
    fseek(fp, 0, SEEK_SET);      // reset file pointer

    // allocate memory for the buffer
    char* buf = (char*) malloc((file_size + 1) * sizeof(char));
    if (!buf)
    {
        fprintf(stderr, "Memory allocation failed!\n");
        fclose(fp);
        return NULL;
    }

    // load file content to memory
    if (fread(buf, sizeof(char), file_size, fp) != file_size)
    {
        fprintf(stderr, "Failed to read file!\n");
        free(buf);
        fclose(fp);
        return NULL;
    }

    fclose(fp);             // close file
    buf[file_size] = '\0';  // nul terminate buffer

    return buf;
}

Token* create_token(const char* _token, const TokenType type) {
    Token* tok = (Token*) malloc(sizeof(Token));
    if (!tok)
    {
        fprintf(stderr, "Malloc Failed!\n");
        return NULL;
    }

    tok->value = strdup(_token);
    if (!tok->value)
    {
        fprintf(stderr, "strdup() Failed!\n");
        free(tok);
        return NULL;
    }

    tok->type = type;
    return tok;
}

Node* create_node(Token* tok) {
    Node* node = (Node*) malloc(sizeof(Node));
    if (!node)
    {
        fprintf(stderr, "malloc() Failed!\n");
        return NULL;
    }

    node->tok  = tok;
    node->next = NULL;

    return node;
}

Node* append_node(Node* head, Node* new /* new is a cpp keyword */) {
    if (!new)
    {
        fprintf(stderr, "append_node : invalid argument\n");
        return NULL;
    }

    if (!head)
    {
        head       = new;
        head->next = NULL;
        return head;
    }

    Node* ptr = head;

    while (ptr->next)
    {
        ptr = ptr->next;
    }

    ptr->next = new;
    new->next = NULL;

    return head;
}

/*--space tokenize--*/
char** sp_tokens(char* str) {
    size_t size = BUFSIZ;  // initial buffer size

    // allocate memory for tokens array
    char** tokens = (char**) malloc(size * sizeof(char*));
    if (!tokens)
    {
        return NULL;
    }

    // first token
    char*  token = strtok(str, SP_DELIM);
    size_t i     = 0;  // iterator count

    // tokenize string
    while (token)
    {
        // append token to array
        tokens[i++] = token;

        // if buffer size if not enough
        // reallocate buffer memory
        if (i >= size)
        {
            size += size;  // double buffer size

            char* temp = (char*) realloc(tokens, size * sizeof(char*));
            if (!temp)
            {
                free(tokens);
                return NULL;
            }

            tokens = &temp;
        }
    }

    tokens[i] = NULL;  // null terminate array

    return tokens;
}

// next token in input stream
char* next(char** src, TokenType* type) {
    // while not at the end of the input buffer
    while (**src != '\0')
    {
        // copy the current position character
        char current = **src;
        (*src)++;  // go to the next character

        if (current == '\n')
        {
            // if the current character is newline
            // increment line counter
            line++;
        }

        // handle single line comments
        else if (current == '/' && **src == '/')
        {
            while (**src != '\0' && **src != '\n')
            {
                (*src)++;
            }

            if (**src == '\n')
            {
                line++;
                (*src)++;
            }
        }
        // handle multi-line comments
        else if (current == '/' && **src == '*')
        {
            (*src)++;  // move past the *

            while (**src != '\0')
            {
                if (**src == '*' && *(*src + 1) == '/')
                {
                    (*src) += 2;  // move past the '*/'
                    break;
                }

                if (**src == '\n')
                {
                    line++;
                }

                (*src)++;
            }
        }
        // if the current character is an identifier sequence starter
        else if (isalpha(current) || current == '_')
        {
            // allocate memory to copy identifier
            char* id = (char*) malloc(MAX_CANON * sizeof(char));
            if (!id)
            {
                return NULL;
            }

            int k   = 0;        // last position of buffer
            id[k++] = current;  // append character to identifier

            // while the next character is an identifier character
            // append it to the buffer
            while (isalpha(**src) || **src == '_' || isdigit(**src))
            {
                id[k++] = **src;
                (*src)++;
            }

            id[k] = '\0';  // null terminate

            // in case it's a keyword then change the token type
            if (is_keyword(id))
            {
                *type = KEYWORD;
                return id;
            }

            *type = IDENTIF;

            return id;
        }
        // if the current token is a digit
        else if (isdigit(current))
        {
            // store the digit as a string object
            char* num = (char*) malloc(MAX_CANON * sizeof(char));
            if (!num)
            {
                return NULL;
            }

            int k    = 0;
            num[k++] = current;

            // while the character is a digit
            // append it to the string
            while (isdigit(**src))
            {
                num[k++] = **src;
                (*src)++;
            }

            num[k] = '\0';
            *type  = NUMBER;

            return num;
        }
        // in case of a negative number
        // same as before but insert '-'
        else if (current == '-' && isdigit(**src))
        {
            char* num = (char*) malloc(MAX_CANON * sizeof(char));
            if (!num)
            {
                return NULL;
            }

            int k    = 0;
            num[k++] = current;

            while (isdigit(**src))
            {
                num[k++] = **src;
                (*src)++;
            }

            num[k] = '\0';
            *type  = NUMBER;

            return num;
        }
        // handle single and double quotes
        else if (current == '\'' || current == '"')
        {
            // allocate memory for the string literal
            char* str_lit = (char*) malloc(MAX_CANON * sizeof(char));
            if (!str_lit)
            {
                return NULL;
            }

            char* str_lit_start = str_lit;  // keep the start of the string for returning
            *str_lit            = current;  // append the current pos in src

            str_lit++;  // go to the next
            (*src)++;

            while (**src != '\0' && **src != '\n' && **src != current)
            {
                // handle common escape sequences
                if (**src == '\\')
                {
                    (*src)++;

                    switch (**src)
                    {
                    case 'n' :
                        *str_lit = '\n';
                        break;

                    case 't' :
                        *str_lit = '\t';
                        break;

                    case 'r' :
                        *str_lit = '\r';
                        break;

                    case 'b' :
                        *str_lit = '\b';
                        break;

                    case 'f' :
                        *str_lit = '\f';
                        break;

                    case 'v' :
                        *str_lit = '\v';
                        break;

                    case '\\' :
                        *str_lit = '\\';
                        break;

                    case '"' :
                        *str_lit = '"';
                        break;

                    case '\'' :
                        *str_lit = '\'';
                        break;
                    }
                }
                else
                {
                    *str_lit = **src;  // append the current pos to the string
                }

                // increment to the next position
                str_lit++;
                (*src)++;
            }

            // null terminate string
            *str_lit = '\0';

            if (**src == current)
            {
                (*src)++;
            }

            *type = STR_LIT;
            return str_lit_start;
        }
        // special operator case
        else if (current == '=' || current == '<' || current == '>' || current == '!' || current == '+'
                 || current == '-' || current == '|' || current == '&')
        {

            // store operator in a string object
            char* op = (char*) malloc(3 * sizeof(char));
            if (!op)
            {
                return NULL;
            }

            op[0] = current;  // append operator to string

            // in case '==' or '+=' or '++' etc...
            if (**src == '=')
            {
                op[1] = **src;
                op[2] = '\0';
                (*src)++;
            }
            else if (current == **src)
            {
                op[1] = **src;
                op[2] = '\0';
                (*src)++;
            }
            else if (current == '-' && **src == '>')
            {
                op[1] = **src;
                op[2] = '\0';
                (*src)++;
            }
            else
            {
                op[1] = '\0';
            }

            *type = OPERATOR;
            return op;
        }
        // special characters and parentheses
        else if (strchr("^%#*?~,;[](){}.", current))
        {
            char* token_str = (char*) malloc(2 * sizeof(char));
            if (!token_str)
            {
                return NULL;
            }

            token_str[0] = current;  // append current
            token_str[1] = '\0';     // null terminate

            *type = DELIMITER;
            return token_str;
        }
    }

    return NULL;  // if neither then ignore
}

// free the token list
void free_nodes(Node* head) {
    while (head)
    {
        Node* next = head->next;
        if (strcmp(head->tok->value, "") != 0)
        {
            if (head->tok->value)
            {
                free(head->tok->value);
            }

            if (head->tok)
            {
                free(head->tok);
            }

            free(head);
        }

        head = next;
    }
}

/*--tokenize the input buffer--*/
Node* get_tokens(char* buf) {
    if (!buf)
    {
        fprintf(stderr, "get_token : Invalid argument!\n");
        return NULL;
    }

    // initialize the tokens list
    Node*     list  = NULL;
    char*     token = NULL;
    TokenType type;

    // while the next token is not null
    while ((token = next(&buf, &type)))
    {
        Token* tok = create_token(token, type);
        if (!tok)
        {
            fprintf(stderr, "create_token() failed!\n");
            free_nodes(list);
            return NULL;
        }

        Node* node = create_node(tok);
        if (!node)
        {
            fprintf(stderr, "create_node() failed!\n");
            free_nodes(list);
            return NULL;
        }

        // Append token to the list
        list = append_node(list, node);
        if (!list)
        {
            return NULL;
        }
    }

    return list;
}

// convert the token type to string for output
const char* token_type_to_string(const TokenType* type) {
    assert(type);

    switch (*type)
    {
    case KEYWORD :
        return "KEYWORD";
    case IDENTIF :
        return "IDENTIFIER";
    case NUMBER :
        return "NUMBER";
    case STR_LIT :
        return "STRING_LITERAL";
    case OPERATOR :
        return "OPERATOR";
    case DELIMITER :
        return "DELIMITER";
    case NONE :
        return "UNKNOWN";
    default :
        return "UNKNOWN";
    }
}

/*
    usually this is used as a header file, i included this main 
    function inside this file for the testing, since this needs
    a lot of testing and edge case handling
*/
int main(void) {
    char* buf = read_file("token.c");
    if (!buf)
    {
        fprintf(stderr, "read_file failed!\n");
        return -1;
    }

    // get the token list given input buffer
    Node* list = get_tokens(buf);
    if (!list)
    {
        fprintf(stderr, "\nFailed to tokenize buffer!\n");
        free(buf);
        return -1;
    }

    free(buf);  // free buffer , we do not need it anymore

    // print the token list
    Node* p = list;

    while (p)
    {
        printf("%s\n", p->tok->value);
        p = p->next;
    }

    free_nodes(list);  // free token list

    return 0;
}
