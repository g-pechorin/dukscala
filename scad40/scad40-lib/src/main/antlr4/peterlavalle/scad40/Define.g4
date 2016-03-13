// define a grammar called Hello
grammar Define;

RESERVED : 'function' | 'String';
VOID : 'void';
ATOMIC : 'bool' | 'double' | 'single' | ('sint'('8'|'16'|'32')) | 'string';

TIKTEXT : '`' ~('`')* '`';

WS  : [ \t\r\n]+ -> skip ;
LINE_COMMENT : '//' ~('\n')* ('\n'|EOF) -> skip ;


fragment
ULetter : [A-Z];

fragment
LLetter : [a-z];

fragment
Digit : [0-9];

UNAME : ULetter (LLetter|ULetter|Digit)*;
LNAME : UNAME|(LLetter (LLetter|ULetter|Digit)*);

typeId
    : ATOMIC #builtin
    | UNAME #defined
    ;
returnType : typeId | VOID;

packname : (LNAME '.')+ LNAME;

module :
    'module' packname
    '{'
        declaration*
    '}'
;

declaration
    : clazz=('native'|'global') UNAME dufinition? #declNat
    | 'script' UNAME definition? #declScr
    | 'select' UNAME '{' UNAME (',' UNAME)+ '}' #declSel
    ;


definition :
    '{'
        member*
    '}'
;

dufinition :
    '{'
        mumber*
    '}'
;

mumber
    : 'raw' LNAME ':' TIKTEXT #rm
    | member #mm
    ;

value: LNAME ':' typeId;

member
    : 'def' LNAME '(' (value (',' value)*)? ')' (':' returnType)? #method
    | ('val'|'def') value #read
    | 'var' value #accessor
    ;


