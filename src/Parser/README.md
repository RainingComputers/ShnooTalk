# ShnooTalk grammar specification
This grammar spec is just for reference, the parser is handwritten, not generated. Each rule is a function.
(Except for the first few, which is done by the lexer)

### Legend
| Symbol  | Description 
|---------|-----------------------------
|[]       | Optional
|{}       | Zero or once or multiple times
|{}!      | Once or multiple times
|()       | Group
|\|       | Or
|? and :  | Ternary operator, changes how the non-terminal behaves based on the flag
|\<true>  | Set flag for the non terminal
|".."     | String should appear
|regex .. | Regex match

### Grammar specification
``` 
assignmentOperator  = "=" | "+=" | "-=" | "/=" | "*=" | "|=" | "&=" | "^="

unaryOperator = "!"

binaryOperator = "+" | "-" | "|" | "^" | ">>" | "<<" | "||"  |  ">" 
       | "<" | ">=" | "<=" | "==" | "!=" | "*" | "/" | "&" | "&&" 

castOperator = "'"

identifier = regex ([a-zA-Z_][a-zA-Z0-9_]*)

intLiteral = regex ([0-9]+)
hexLiteral = regex (0x[a-fA-f0-9]+)
binLiteral = regex (0b[0-1]+)
floatLiteral = regex ([0-9]*\.[0-9]+)

literal = intLiteral,
        | charLiteral,
        | floatLiteral,
        | hexLiteral,
        | binLiteral,
        | ("-" | "+") literal

use = "use" identifier {"," identifier}

from = "from" use

enumList = "enum" "[" identifier {"," identifier} "]"

def = "def" identifier literal

identifierWithSubscript = identifier {"[" (literalSubscriptOnly ? literal : expression)  "]"}

identifierWithQualidentAndSubscript = identifierWithSubscript<false> {identifierWithSubscript<false> "."}

moduleQualident = {identifier "::"}

typeDefinition = moduleQualident identifierWithSubscript<true>

identifierDeclaration = identifier ":" typeDefinition

identifierDeclarationAndInit = identifierDeclaration ["=" expression]

identifierDeclareList =  ("var" |  initAllowed ? "const") {initAllowed ?  identifierDeclarationAndInit : identifierDeclaration}!

structDefinition = "struct" identifier "{" {identifierDeclareList<false>} "}"

formalParameterList =   [["mut"] identifierDeclaration {"," ["mut"] identifierDeclaration}] 

functionDefinition = "fn" "(" formalParameterList ")" ["->" typeDefinition] block

print = ("print" | "println") "(" [expression {expression ,}] ")" 

input = "input" "(" expression ")"

actualParameterList = "(" [expression {expression ,}] ")"

functionCall = identifier actualParameterList

methodCall = {"." identifier actualParameterList}

sizeof = "sizeof" "(" moduleQualident identifier ")"

initializerList = "[" expression {"," expression} "]"

term = sizeof
     | functionCall
     | identifier castOperator term
     | moduleQualident term
     | identifierWithQualidentAndSubscript methodCall
     | "(" baseExpression "
     | unaryOperator term
     | literal

baseExpression = term {binaryOperator term}

expression = initializerList | stringLiteral | baseExpression

assignmentOrMethodCall = identifierWithQualidentAndSubscript (methodCall | (assignmentOperator expression))

moduleFunctionCall = moduleQualident functionCall

ifStatement = "if" expression block {"elseif" expression block} ["else" block]

whileLoop = "while" expression block

forLoop = "for" ["("] (identifierDeclareList<true> | assignmentOrMethodCall) ";" expression ";" assignmentOrMethodCall [")"] block

returnExpression = "return" (expression | "void")

statement = functionCall
          | moduleFunctionCall
          | assignmentOrMethodCall
          | identifierDeclarationList<true>
          | ifStatement
          | whileLoop
          | forLoop
          | "break"
          | "continue"
          | print
          | input
          | returnExpression

block = statement | ("{" {statement} "}")

program = {use} {from} {functionDefinition | structDefinition | enumList | identifierDeclareList<false> | def} endOfFile
```