class Token:
    def __init__(self, string, token_type):
        self.data = string
        self.type = token_type
    
    def __repr__(self) -> str:
        return f'{self.data}'

line = 1

keywords = { 
    "auto", "break", "case", "char", "const",
    "continue", "default", "do", "double", "else",
    "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short",
    "signed", "sizeof", "static", "struct", "switch",
    "typedef", "union", "unsigned", "void",
    "volatile", "while" 
}

operators = {
    '=', '<', '>', '!', '+', '-', '|', '&'
}

def isKeyword(string):
    return string in keywords

filename = 'file.txt'
with open(filename, 'r') as f:
    src = f.read()

i = 0

def next_token():
    global i, line
    while i < len(src):
        token_char = src[i]
        i += 1

        if token_char == '\n':
            line += 1
        
        elif token_char == '/' and i < len(src) and src[i] == '/':
            while i < len(src) and src[i] != '\n': 
                i += 1
            if i < len(src) and src[i] == '\n':
                line += 1
                i    += 1
        
        elif token_char == '/' and i < len(src) and src[i] == '*':
            i += 1
            while i < len(src) and (src[i] != '*' or (i+1 < len(src) and src[i+1] != '/')):
                if src[i] == '\n':
                    line += 1
                i += 1
            i += 2  

        elif token_char.isalpha() or token_char == '_':
            identifier = token_char
            while i < len(src) and (src[i].isalnum() or src[i] == '_'):
                identifier += src[i]
                i += 1
            
            if isKeyword(identifier):
                return Token(identifier, 'keyword')
            
            return Token(identifier, 'identifier')
        
        elif token_char.isdigit() or (token_char == '-' and i < len(src) and src[i].isdigit()):
            num = token_char
            while i < len(src) and src[i].isdigit():
                num += src[i]
                i += 1
            
            return Token(num, 'number')
            
        elif token_char == '\'' or token_char == '"':
            str_literal  = token_char
            closing_char = token_char
            
            while i < len(src) and src[i] != '\n' and src[i] != closing_char:
                if src[i] == '\\' and i + 1 < len(src):
                    str_literal += src[i]
                    i           += 1
                str_literal += src[i]
                i += 1
            
            if i < len(src) and src[i] == closing_char:
                str_literal += src[i]
                i += 1
            
            return Token(str_literal, 'string literal')
        
        elif token_char in operators:
            if i < len(src) and src[i] in operators:
                op  = token_char + src[i]
                i  += 1
                return Token(op, 'operator')
            return Token(token_char, 'operator')
        
        elif token_char in '^%#*?~,;:[](){}.':
            return Token(token_char, 'delimiter')
    
    return None

tokens = []
while i < len(src):
    tok = next_token()
    if tok is not None:
        tokens.append(tok)

for t in tokens:
    print(t)
