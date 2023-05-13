%{

/*
    直接延续了lab1，并且把lab1的语法/词法错误处理都删了
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "szf_analyser.h"
#include "szf.tab.h"
int yycolumn = 1;
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;

char error_msg[1024];
char tmpbuf[1024];

Node* NewNode(){
    Node* node=(Node*)malloc(sizeof(Node));
    memset(node,0,sizeof(Node));
    return node;
}

%}
%option yylineno

%%

"//" {
    //单行注释
    char ch=input();
    while(ch != '\n' && ~ch)    
        ch=input();
    yycolumn=1;
}

"/*" {
    //多行注释
    char pre_ch=0,ch=input();
    while(!(pre_ch == '*' && ch == '/') && (~ch) && ch)
        pre_ch=ch, ch=input();
}

";" {
    //终结符SEMI
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=SEMI;
    node->line=yylineno;
    yylval.type_node=node;
    return SEMI;
}

"," {
    //终结符COMMA
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=COMMA;
    node->line=yylineno;
    yylval.type_node=node;
    return COMMA;
}

"=" {
    //终结符ASSIGNOP
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=ASSIGNOP;
    node->line=yylineno;
    yylval.type_node=node;
    return ASSIGNOP;
}

((>)|(<)|(>=)|(<=)|(==)|(!=)) {
    //终结符RELOP
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=RELOP;
    node->line=yylineno;
    yylval.type_node=node;
    return RELOP;
}

"+" {
    //终结符PLUS
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=PLUS;
    node->line=yylineno;
    yylval.type_node=node;
    return PLUS;
}

"-" {
    //终结符MINUS
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=MINUS;
    node->line=yylineno;
    yylval.type_node=node;
    return MINUS;
}

"*" {
    //终结符STAR
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=STAR;
    node->line=yylineno;
    yylval.type_node=node;
    return STAR;
}

"/" {
    //终结符DIV
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=DIV;
    node->line=yylineno;
    yylval.type_node=node;
    return DIV;
}

"&&" {
    //终结符AND
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=AND;
    node->line=yylineno;
    yylval.type_node=node;
    return AND;
}

"||" {
    //终结符OR
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=OR;
    node->line=yylineno;
    yylval.type_node=node;
    return OR;
}

"." {
    //终结符DOT
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=DOT;
    node->line=yylineno;
    yylval.type_node=node;
    return DOT;
}

"!" {
    //终结符NOT
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=NOT;
    node->line=yylineno;
    yylval.type_node=node;
    return NOT;
}

((int)|(float)) {
    //终结符TYPE
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=TYPE;
    node->line=yylineno;
    strcpy(node->str_val,yytext);
    yylval.type_node=node;
    return TYPE;
}

"(" {
    //终结符LP
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=LP;
    node->line=yylineno;
    yylval.type_node=node;
    return LP;
}

")" {
    //终结符RP
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=RP;
    node->line=yylineno;
    yylval.type_node=node;
    return RP;
}

"[" {
    //终结符LB
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=LB;
    node->line=yylineno;
    yylval.type_node=node;
    return LB;
}

"]" {
    //终结符RB
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=RB;
    node->line=yylineno;
    yylval.type_node=node;
    return RB;
}

"{" {
    //终结符LC
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=LC;
    node->line=yylineno;
    yylval.type_node=node;
    return LC;
}

"}" {
    //终结符RC
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=RC;
    node->line=yylineno;
    yylval.type_node=node;
    return RC;
}

"struct" {
    //终结符STRUCT
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=STRUCT;
    node->line=yylineno;
    yylval.type_node=node;
    return STRUCT;
}

"return" {
    //终结符RETURN
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=RETURN;
    node->line=yylineno;
    yylval.type_node=node;
    return RETURN;
}

"if" {
    //终结符IF
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=IF;
    node->line=yylineno;
    yylval.type_node=node;
    return IF;
}

"else" {
    //终结符ELSE
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=ELSE;
    node->line=yylineno;
    yylval.type_node=node;
    return ELSE;
}

"while" {
    //终结符WHILE
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=WHILE;
    node->line=yylineno;
    yylval.type_node=node;
    return WHILE;
}

((([0-9])+\.([0-9])+)|(((([0-9])+\.([0-9])*)|(([0-9])*\.([0-9])+))[Ee]([+-]?)([0-9])+)) {
    //终结符FLOAT
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=FLOAT;
    node->line=yylineno;
    sscanf(yytext,"%f",&node->float_val);
    yylval.type_node=node;
    return FLOAT;
}

(([1-9]([0-9])*)|(0([0-7])*)|(0[xX]([0-9a-fA-F])*)) {
    //终结符INT
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=INT;
    node->line=yylineno;
    if(yytext[0] == '0' && (yytext[1] == 'x' || yytext[1] == 'X'))
        sscanf(yytext + 2,"%x",&node->int_val);
    else if(yytext[0] == '0')
        sscanf(yytext,"%o",&node->int_val);
    else
        sscanf(yytext,"%d",&node->int_val);
    yylval.type_node=node;
    return INT;
}

[_A-Za-z]([_A-Za-z0-9])* {
    //终结符ID
    Node* node=NewNode();
    node->isTerminal=1;
    node->type=ID;
    node->line=yylineno;
    strcpy(node->str_val,yytext);
    yylval.type_node=node;
    return ID;
}

\n { yycolumn=1; }

((" ")|(\t)|\r) {}

. { 
    
}
%%
