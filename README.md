# lexir
this is a mechanical lexical analyzer of the c programming language
the program spilts the source code into a stream of tokens to be used
in other future steps of the compilation process

# technique 

  the input stream of characters (in this case the source code) is a series of characters where 
every character influences the meaning of the next character, so the tokenisation would be based 
on grouping characters together in a way that makes sens with regards to the language rules.

  this lexer uses a function 'next()' that was inspired by the way some language interpreters work, 
whenever it is called , it returns the next token in the sequence and then we take it an add it to a 
linked list of tokens.
the 'next()' function takes as input a pointer to the current position in the input source code, it 
checks for the value of the current character and applies conditions - made clear by the C language 
syntax rules - to predict where the token should end and returns the token ,while updating the current
position in the source code.
 
  tokens are grouped in a linked list where the token has value of a string and a pointer to the
next token and a token type if necessary.
