%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "tree.h"

    extern int yylineno;
    void yyerror(char*);
    void myerror(char *msg);
    int yylex();
    int errors = 0;                 // 记录找出的语法错误次数
    #define YYSTYPE struct Node*    // 将所有的 token 类型设置为 Node*
%}


%token INT                         /* int 类型 */
%token FLOAT                       /* float 类型 */
%token TYPE                        /* TYPE 终结符 */
%token LF                          /* 换行符 \n */
%token ID                          /* 标识符 */ 
%token SEMI COMMA DOT              /* 结束符号 ; , */
%token ASSIGNOP RELOP              /* 比较赋值符号 = > < >= <= == != */
%token PLUS MINUS STAR DIV         /* 运算符 + - * / */
%token AND OR NOT                  /* 判断符号 && || ! */
%token LP RP LB RB LC RC           /* 括号 ( ) [ ] { } */
%token STRUCT                      /* struct */
%token RETURN                      /* return */
%token IF                          /* if */
%token ELSE                        /* else */
%token WHILE                       /* while */

// 定义结合性和优先级次序
%right ASSIGNOP
%left OR
%left AND
%nonassoc RELOP
%left PLUS MINUS
%left STAR DIV
%right NAGATE NOT
%right DOT LP LB RP RB
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

 /*-----------------------------------------|
 |          High-level Definitions          |
 |-----------------------------------------*/
 //产生式左边的非终结符对应的属性值用$$表示，右边的
//几个符号的属性值按从左到右的顺序依次对应为$1、$2、$3等。
Program : ExtDefList { // ExtDefList is the root of the AST
        $$ = insertNode($1, "Program", $1->lineNum, NONTERMINAL); // Add a new node called "Program" as the root of the AST
    }
    ;
//其中的first_line和first_column分别是该语法单元对应的第一个词素出现的行号和列号，
//而last_line和last_column分别是该语法单元对应的最后一个词素出现的行号和列号。
ExtDefList : ExtDef ExtDefList {
        $$ = insertNode($1, "ExtDefList", @1.first_line, NONTERMINAL);
        $1->brother = $2;
    }
    | {
        $$ = insertNode(NULL, "ExtDefList", yylineno, NONTERMINAL);
    }

    ;

ExtDef : Specifier ExtDecList SEMI {
        $$ = insertNode($1, "ExtDef", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | Specifier SEMI {
        $$ = insertNode($1, "ExtDef", @1.first_line, NONTERMINAL);
        $1->brother = $2;
    }
    | Specifier FunDec CompSt {
        $$ = insertNode($1, "ExtDef", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }


    ;

ExtDecList : VarDec {
        $$ = insertNode($1, "ExtDecList", @1.first_line, NONTERMINAL);
    }
    | VarDec COMMA ExtDecList {
        $$ = insertNode($1, "ExtDecList", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }


    ;

 /*-----------------------------------------|
 |                Specifiers                |
 |-----------------------------------------*/
Specifier : TYPE {
        $$ = insertNode($1, "Specifier", @1.first_line, NONTERMINAL);
    }
    | StructSpecifier {
        $$ = insertNode($1, "Specifier", @1.first_line, NONTERMINAL);
    }
    ;

StructSpecifier : STRUCT OptTag LC DefList RC {
        $$ = insertNode($1, "StructSpecifier", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
        $3->brother = $4;
        $4->brother = $5;
    }
    | STRUCT Tag {
        $$ = insertNode($1, "StructSpecifier", @1.first_line, NONTERMINAL);
        $1->brother = $2;
    }

    ;

OptTag : ID {
        $$ = insertNode($1, "OptTag", @1.first_line, NONTERMINAL);
    }
    | {
        $$ = insertNode(NULL, "OptTag", yylineno, NONTERMINAL);
    }
    ;

Tag : ID {
        $$ = insertNode($1, "Tag", @1.first_line, NONTERMINAL);
    }
    ;

 /*-----------------------------------------|
 |               Declarators                |
 |-----------------------------------------*/
VarDec : ID {
        $$ = insertNode($1, "VarDec", @1.first_line, NONTERMINAL);
    }
    | VarDec LB INT RB {
        $$ = insertNode($1, "VarDec", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
        $3->brother = $4;
    }
    ;

FunDec : ID LP VarList RP {
        $$ = insertNode($1, "FunDec", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
        $3->brother = $4;
    }
    | ID LP RP {
        $$ = insertNode($1, "FunDec", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    ;

VarList : ParamDec COMMA VarList {
        $$ = insertNode($1, "VarList", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | ParamDec {
        $$ = insertNode($1, "VarList", @1.first_line, NONTERMINAL);
    }
    ;

ParamDec : Specifier VarDec {
        $$ = insertNode($1, "ParamDec", @1.first_line, NONTERMINAL);
        $1->brother = $2;
    }
    ;

 /*-----------------------------------------|
 |                Statements                |
 |-----------------------------------------*/
CompSt : LC DefList StmtList RC {
        $$ = insertNode($1, "CompSt", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
        $3->brother = $4;
    }
    ;

StmtList : Stmt StmtList {
        $$ = insertNode($1, "StmtList", @1.first_line, NONTERMINAL);
        $1->brother = $2;
    }
    | {
        $$ = insertNode(NULL, "FunDec", yylineno, NONTERMINAL);
    }

    ;
    
Stmt : Exp SEMI {
        $$ = insertNode($1, "Stmt", @1.first_line, NONTERMINAL);
        $1->brother = $2;
    }
    | CompSt {
        $$ = insertNode($1, "Stmt", @1.first_line, NONTERMINAL);
    }
    | RETURN Exp SEMI {
        $$ = insertNode($1, "Stmt", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | IF LP Exp RP Stmt{
        $$ = insertNode($1, "Stmt", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
        $3->brother = $4;
        $4->brother = $5;
    }
    | IF LP Exp RP Stmt ELSE Stmt {
        $$ = insertNode($1, "Stmt", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
        $3->brother = $4;
        $4->brother = $5;
        $5->brother = $6;
        $6->brother = $7;
    }
    | WHILE LP Exp RP Stmt {
        $$ = insertNode($1, "Stmt", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
        $3->brother = $4;
        $4->brother = $5;
    }//error ->如果当前状态并没有针对这个词法单元的动作，那就会认为输入文件里出现了语法错误
    | error RC{ 
        char msg[100];
        sprintf( msg, "error RC:Missing \";\"");//认为此错误原因是->缺少分号;
        // printf("8\n");
        myerror( msg );  
    }

    ;

 /*-----------------------------------------|
 |             Local Definitions            |
 |-----------------------------------------*/
DefList : Def DefList {
        $$ = insertNode($1, "DefList", @1.first_line, NONTERMINAL);
        $1->brother = $2;
    }
    | {
        $$ = insertNode(NULL, "DefList", yylineno, NONTERMINAL);
    }
    ;

Def : Specifier DecList SEMI {
        $$ = insertNode($1, "Def", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }

    ;

DecList : Dec {
        $$ = insertNode($1, "DecList", @1.first_line, NONTERMINAL);
    }
    | Dec COMMA DecList {
        $$ = insertNode($1, "DecList", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }

    ;

Dec : VarDec {
        $$ = insertNode($1, "Dec", @1.first_line, NONTERMINAL);
    }
    | VarDec ASSIGNOP Exp {
        $$ = insertNode($1, "Dec", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    ;

 /*-----------------------------------------|
 |               Expressions                |
 |-----------------------------------------*/
Exp : Exp ASSIGNOP Exp {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | Exp AND Exp {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | Exp OR Exp {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | Exp RELOP Exp {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | Exp PLUS Exp {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | Exp MINUS Exp {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | Exp STAR Exp {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | Exp DIV Exp {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | LP Exp RP {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | MINUS Exp {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
    }
    | NOT Exp {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
    }
    | ID LP Args RP {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
        $3->brother = $4;
    }
    | ID LP RP {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | Exp LB Exp RB {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
        $3->brother = $4;
    }
    | Exp DOT ID {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | ID {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
    }
    | INT {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
    }
    | FLOAT {
        $$ = insertNode($1, "Exp", @1.first_line, NONTERMINAL);
    }//每当语法分析程序从yylex()得到了一个词法单元，如果当前状态并没有针对这个词法单元的动作，那就会认为输入文件里出现了语法错误.本质上相当于让 RB---"]" 作为错误恢复的同步符号
    | error RB { 
        char msg[100];
        sprintf( msg, "Missing \"]\"");
        // printf("3\n");
        myerror( msg );                // error
    }
    | error INT { 
        char msg[100];
        sprintf( msg, "error INT:Missing \"]\"");
        // printf("3\n");
        myerror( msg );                // error
    }
    | FLOAT error ID{ 
        char msg[100];
        sprintf( msg, "Syntax error.");
        // printf("6\n");
        myerror( msg );  
    }
    | INT error ID{ 
        char msg[100];
        sprintf( msg, "INT error ID:Missing \";\"");
        // printf("7\n");
        myerror( msg );  
    }
    | INT error INT{ 
        char msg[100];
        sprintf( msg, "INT error INT:Missing \";\"");
        // printf("8\n");
        myerror( msg );  
    }

    
    ;

Args : Exp COMMA Args {
        $$ = insertNode($1, "CompSt", @1.first_line, NONTERMINAL);
        $1->brother = $2;
        $2->brother = $3;
    }
    | Exp {
        $$ = insertNode($1, "CompSt", @1.first_line, NONTERMINAL);
    }
    ;

%%

#include "lex.yy.c"

int main(int argc, char **argv) {
    if (argc <= 1) return 1;
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yylineno = 1;
    yyrestart(f);
    yyparse();

    // 输出语法树
    f = fopen("result.txt", "w");
    if (!f) {   
        perror(argv[1]);
        return 1;
    }//如果errors==0则把结果放到result.txt中
    if (errors == 0) {
        f = fopen("result.txt", "w");
        printTree(head, 0, f);
    }
    //如果errors>=1就在之前就已经利用myerror()把错误信息打印出来了
    return 0;
}
//yyerror 函数是一个错误处理函数，它在编译器遇到语法错误时被调用。它的作用是打印出错误信息，以便程序员可以找到并修复错误。但是我们要用自己的myerror函数来代替yyerror函数，所以要把yyerror函数注释掉。
// 重载，令 yyerror 函数失效
void yyerror(char *msg)
{
    // fprintf(stderr, "Error type B at Line %d: %s\n", yylineno,  msg);
    //  printf( "%d: %s\n", yylineno,  msg);
    // errors++;
}

//这个是针对Type B的错误
// 设置自定义的 myerror
void myerror(char *msg)
{
    fprintf(stderr, "Error type B at Line %d: %s \n", yylineno,  msg);
    errors++;
}
