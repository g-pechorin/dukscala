// define a grammar called Hello
grammar Define;

RESERVED : 'function';
VOID : 'void';
ATOMIC : ('sint'('8'|'16'|'32')) | 'single' | 'double' | 'string';

WS  : [ \t\r\n]+ -> skip ;
LINE_COMMENT : '//' ~('\n')* ('\n'|EOF) -> skip ;

fragment
Letter : [a-z]|[A-Z];

fragment
Digit : [0-9];


NAME : Letter (Letter|Digit)*;

typeId
    : ATOMIC #builtin
    | NAME #defined
    ;
returnType : typeId | VOID;

packname : (NAME '.')+ NAME;

module :
    'module' packname
    '{'
        declaration*
    '}'
;

declaration
    : 'global' NAME definition? #declGlo
    | 'native' NAME definition? #declNat
    | 'script' NAME definition? #declScr
    | 'select' NAME '{' NAME (',' NAME)+ '}' #declSel
    ;

value: NAME ':' typeId;

member
    : 'def' NAME '(' (value (',' value)*)? ')' (':' returnType)? #method
    | ('val'|'def') value #read
    | 'var' value #accessor
    ;

definition :
    '{'
        member*
    '}'
;