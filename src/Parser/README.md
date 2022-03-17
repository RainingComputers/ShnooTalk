# ShnooTalk grammar specification
This grammar spec is just for reference, the parser is handwritten, not generated. Each rule is a function.
(Except for the first few, which is done by the lexer)

### Legend
| Symbol  | Description 
|---------|-----------------------------
|[]       | Optional
|{}       | Zero or once or multiple times
|()       | Group
|\|       | Or
|? and :  | Ternary operator, changes how the non-terminal behaves based on the flag
|\<true>  | Set flag for the non terminal
|".."     | String should appear
|regex .. | Regex match

### Grammar specification
``` 
assignmentOperator  = "=" |  "<-" | "+=" | "-=" | "/=" | "*=" | "|=" | "&=" | "^=" 

unaryOperator = "!"

binaryOperator = "+" | "-" | "|" | "^" | ">>" | "<<" | "||"  |  ">" 
       | "<" | ">=" | "<=" | "==" | "!=" | "*" | "/" | "&" | "&&" 

walrusOperator = ":="

castOperator = "`"

pointerCastOperator = "*`"

arrayPointerCastOperator = "[]`"

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

generic = "generic"  "[" identifier {"," identifier} "]"

use = "use" stringLiteral "as" identifier

from = "from" (identifier | stringLiteral) "use" identifier {"," identifier}

enumList = "enum" "[" identifier {"," identifier} "]"

def = "def" identifier (literal | stringLiteral)

identifierWithOptionalSubscript = identifier {"[" (literalSubscriptOnly? literal : expression)  "]"}

identifierWithPointerStar = identifier "*"

identifierWithEmptySubscript = identifier "[]"

identifierWithQualidentAndSubscript = identifierWithOptionalSubscript<false> {"." identifierWithOptionalSubscript<false>}

genericParams = "[" typeDefinition {"," typeDefinition} "]"

identifierWithGeneric = identifier genericParams

moduleQualident = {identifier "::"}

typeDefinition = moduleQualident (identifierWithOptionalSubscript<true> | identifierWithPointerStar | identifierWithEmptySubscripts | identifierWithGeneric)

identifierDeclaration = identifier ":" typeDefinition

identifierDeclarationOptionalInit = identifierDeclaration [ ("=" | "<-") expression]

identifierDeclareListOptionalInit =  "var" initAllowed? identifierDeclarationOptionalInit : identifierDeclaration {"," initAllowed? identifierDeclarationOptionalInit : identifierDeclaration}

identifierDeclarationRequiredInit = identifierDeclaration ("=" | "<-") expression

identifierDeclareListRequiredInit "const" identifierDeclarationRequiredInit {"," identifierDeclarationRequiredInit}

walrusDeclaration = ("const" | "var") identifier walrusOperator expression

destructureList = "[" identifier {"," identifier} "]"

destructureDeclaration = ("const" | "var") destructureList walrusOperator expression

structDefinition = "struct" identifier "{" {identifierDeclareList<false>} "}"

formalParameterList =   ["mut"] identifierDeclaration {"," ["mut"] identifierDeclaration}

functionDeceleration = identifier "(" [formalParameterList] ")" ["->" typeDefinition]

functionDefinition = "fn" functionDeceleration block

externFunctionDefinition = "extfn" functionDeceleration [stringLiteral]

print = ("print" | "println") "(" [expression {("," | ";") expression}] ")" 

input = "input" "(" term ")"

actualParameterList = "(" [expression {"," expression}] ")"

functionCall = identifier actualParameterList

genericFunctionCall = identifier genericParams actualParameterList

methodCall = {"." identifier actualParameterList}

sizeof = "sizeof" "(" moduleQualident identifier ")"

make = "make" "(" typeDefinition {"," expression}  ")"

addr = "addr" "(" expression ")"

initializerList = "[" expression {"," expression} "]"

term = sizeof
     | make
     | functionCall
     | genericFunctionCall
     | identifier castOperator term
     | identifier pointerCastOperator term
     | identifier arrayPointerCastOperator term
     | moduleQualident term
     | identifierWithQualidentAndSubscript methodCall
     | "(" baseExpression ")"
     | unaryOperator term
     | literal

baseExpression = term {binaryOperator term}

multiLineStringLiteral = stringLiteral {stringLiteral}

expression = initializerList | multiLineStringLiteral | baseExpression

assignmentOrMethodCall = identifierWithQualidentAndSubscript (methodCall | (assignmentOperator expression))

moduleFunctionCall = moduleQualident (functionCall | genericFunctionCall)

ifStatement = "if" expression block {"elseif" expression block} ["else" block]

whileLoop = "while" expression block

doWhileLoop = "do" block "while" expression

forLoop = "for" ["("] (identifierDeclareList<true> | assignmentOrMethodCall) ";" expression ";" assignmentOrMethodCall [")"] block

infiniteLoop = "loop" block

returnExpression = "return" (expression | "void")

statement = functionCall
          | genericFunctionCall
          | moduleFunctionCall
          | assignmentOrMethodCall
          | walrusDeclaration
          | destructureDeclaration
          | identifierDeclarationList<true>
          | ifStatement
          | whileLoop
          | forLoop
          | infiniteLoop
          | "break"
          | "continue"
          | print
          | input
          | returnExpression

block =  ("{" {statement} "}") | statement

programModule = {use} {from} {def | enumList | identifierDeclareList<false>  | structDefinition | functionDefinition | externFunctionDefinition} endOfFile
```
