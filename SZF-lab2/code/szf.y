%{


#include<stdio.h>
#include<stdlib.h>
#include "szf_analyser.h"
#include "lex.yy.c"

Node* syntax_tree=NULL;         //最终的语法树

Node* CreateUnterm(int type,int line,Node* ch0,Node* ch1,Node* ch2){
    //为非终结符生成一个指定至多3个儿子的结点（和lab2没啥关系，建语法树用的）
    if(ch0 == NULL)
        ch0=ch1, ch1=ch2, ch2=NULL;
    if(ch1 == NULL)
        ch1=ch2, ch2=NULL;              //Note: 我们的语法树直接无脑去除空结点
    Node* node=NewNode();
    node->isTerminal=0;
    node->type=type;
    node->line=line;
    node->child=ch0;
    if(ch0 != NULL)
        ch0->next=ch1;
    if(ch1 != NULL)
        ch1->next=ch2;
    return node;
}

void AddChild(Node* node,Node* child){
    //为非终结符结点追加一个儿子
    if(child == NULL)
        return;
    if(node->child == NULL)
        node->child=child;
    else{
        for(Node* i=node->child; i; i=i->next)
            if(i->next == NULL){
                i->next=child;
                break;
            }
    }
}

%}
%union{
    Node* type_node;
}

%token <type_node> SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE FLOAT INT ID
%type <type_node> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier 
%type <type_node> OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt
%type <type_node> DefList Def DecList Dec
%type <type_node> Exp Exp1 Exp2 Exp3 Exp4 Exp5 Exp6 Exp7 Args

%%

//Program -> ExtDefList
Program : ExtDefList {
    syntax_tree=CreateUnterm(Unterm_Program,@$.first_line,$1,NULL,NULL);
    $$=syntax_tree;
};

//ExtDefList → ExtDef ExtDefList | e
ExtDefList : {  $$=NULL;    }
| ExtDef ExtDefList {
    $$=CreateUnterm(Unterm_ExtDefList,@$.first_line,$1,$2,NULL);
};

//ExtDef → Specifier ExtDecList SEMI | Specifier SEMI | Specifier FunDec CompSt
ExtDef : Specifier ExtDecList SEMI {
    $$=CreateUnterm(Unterm_ExtDef,@$.first_line,$1,$2,$3);
    $$->subtype=0;
}
| Specifier SEMI {
    $$=CreateUnterm(Unterm_ExtDef,@$.first_line,$1,$2,NULL);
    $$->subtype=1;
}
| Specifier FunDec CompSt {
    $$=CreateUnterm(Unterm_ExtDef,@$.first_line,$1,$2,$3);
    $$->subtype=2;
};

//ExtDecList → VarDec | VarDec COMMA ExtDecList
ExtDecList : VarDec {
    $$=CreateUnterm(Unterm_ExtDecList,@$.first_line,$1,NULL,NULL);
    $$->subtype=0;
}
| VarDec COMMA ExtDecList {
    $$=CreateUnterm(Unterm_ExtDecList,@$.first_line,$1,$2,$3);
    $$->subtype=1;
};

//Specifier → TYPE | StructSpecifier
Specifier : TYPE {
    $$=CreateUnterm(Unterm_Specifier,@$.first_line,$1,NULL,NULL);
    $$->subtype=0;
}
| StructSpecifier {
    $$=CreateUnterm(Unterm_Specifier,@$.first_line,$1,NULL,NULL);
    $$->subtype=1;
};

//StructSpecifier → STRUCT OptTag LC DefList RC | STRUCT Tag
StructSpecifier : STRUCT OptTag LC DefList RC {
    $$=CreateUnterm(Unterm_StructSpecifier,@$.first_line,$1,$2,$3);
    AddChild($$,$4);
    AddChild($$,$5);
    $$->subtype=0;
}
| STRUCT Tag {
    $$=CreateUnterm(Unterm_StructSpecifier,@$.first_line,$1,$2,NULL);
    $$->subtype=1;
};

//OptTag → ID | e
OptTag : {  $$=NULL;    }
| ID {
    $$=CreateUnterm(Unterm_OptTag,@$.first_line,$1,NULL,NULL);
};

//Tag → ID
Tag : ID {
    $$=CreateUnterm(Unterm_Tag,@$.first_line,$1,NULL,NULL);
};

//VarDec → ID | VarDec LB INT RB
//VarDec -> VarDec INT RB | VarDec LB Exp RB | VarDec LB INT error (Error)
VarDec : ID {
    $$=CreateUnterm(Unterm_VarDec,@$.first_line,$1,NULL,NULL);
    $$->subtype=0;
}
| VarDec LB INT RB {
    $$=CreateUnterm(Unterm_VarDec,@$.first_line,$1,$2,$3);
    AddChild($$,$4);
    $$->subtype=1;
};

//FunDec → ID LP VarList RP | ID LP RP
//FunDec -> ID VarList RP | ID LP VarList error | ID RP | ID LP error | LP VarList RP | ID LP error RP (Error)
FunDec : ID LP VarList RP {
    $$=CreateUnterm(Unterm_FunDec,@$.first_line,$1,$2,$3);
    AddChild($$,$4);
    $$->subtype=0;
}
| ID LP RP {
    $$=CreateUnterm(Unterm_FunDec,@$.first_line,$1,$2,$3);
    $$->subtype=1;
};

//VarList → ParamDec COMMA VarList | ParamDec
//VarList -> ParamDec VarList (Error)
VarList : ParamDec COMMA VarList {
    $$=CreateUnterm(Unterm_VarList,@$.first_line,$1,$2,$3);
    $$->subtype=0;
}
| ParamDec {
    $$=CreateUnterm(Unterm_VarList,@$.first_line,$1,NULL,NULL);
    $$->subtype=1;
};

//ParamDec → Specifier VarDec
ParamDec : Specifier VarDec {
    $$=CreateUnterm(Unterm_ParamDec,@$.first_line,$1,$2,NULL);
};

//CompSt → LC DefList StmtList RC
//CompSt ->LC DefList StmtList error (Error)
CompSt : LC DefList StmtList RC {
    $$=CreateUnterm(Unterm_CompSt,@$.first_line,$1,$2,$3);
    AddChild($$,$4);
};

//StmtList → Stmt StmtList | e
StmtList : {    $$=NULL;    }
| Stmt StmtList {
    $$=CreateUnterm(Unterm_StmtList,@$.first_line,$1,$2,NULL);
};

//Stmt → Exp SEMI | CompSt | RETURN Exp SEMI | IF LP Exp RP Stmt | IF LP Exp RP Stmt ELSE Stmt | WHILE LP Exp RP Stmt
//Stmt -> Exp error | RETURN Exp error | RETURN SEMI | RETURN error | IF LP Exp error | IF error
Stmt : Exp SEMI {
    $$=CreateUnterm(Unterm_Stmt,@$.first_line,$1,$2,NULL);
    $$->subtype=0;
}
| CompSt {
    $$=CreateUnterm(Unterm_Stmt,@$.first_line,$1,NULL,NULL);
    $$->subtype=1;
}
| RETURN Exp SEMI {
    $$=CreateUnterm(Unterm_Stmt,@$.first_line,$1,$2,$3);
    $$->subtype=2;
}
| IF LP Exp RP Stmt {
    $$=CreateUnterm(Unterm_Stmt,@$.first_line,$1,$2,$3);
    AddChild($$,$4);
    AddChild($$,$5);
    $$->subtype=3;
}
| IF LP Exp RP Stmt ELSE Stmt {
    $$=CreateUnterm(Unterm_Stmt,@$.first_line,$1,$2,$3);
    AddChild($$,$4);
    AddChild($$,$5);
    AddChild($$,$6);
    AddChild($$,$7);
    $$->subtype=4;
}
| WHILE LP Exp RP Stmt {
    $$=CreateUnterm(Unterm_Stmt,@$.first_line,$1,$2,$3);
    AddChild($$,$4);
    AddChild($$,$5);
    $$->subtype=5;
};

//DefList → Def DefList | e
DefList : { $$=NULL;    }
| Def DefList {
    $$=CreateUnterm(Unterm_DefList,@$.first_line,$1,$2,NULL);
};

//Def → Specifier DecList SEMI
Def : Specifier DecList SEMI {
    $$=CreateUnterm(Unterm_Def,@$.first_line,$1,$2,$3);
};

//DecList → Dec | Dec COMMA DecList
DecList : Dec {
    $$=CreateUnterm(Unterm_DecList,@$.first_line,$1,NULL,NULL);
    $$->subtype=0;
}
| Dec COMMA DecList {
    $$=CreateUnterm(Unterm_DecList,@$.first_line,$1,$2,$3);
    $$->subtype=1;
};

//Dec → VarDec | VarDec ASSIGNOP Exp
Dec : VarDec {
    $$=CreateUnterm(Unterm_Dec,@$.first_line,$1,NULL,NULL);
    $$->subtype=0;
}
| VarDec ASSIGNOP Exp {
    $$=CreateUnterm(Unterm_Dec,@$.first_line,$1,$2,$3);
    $$->subtype=1;
};

//基于优先级处理后的Exp产生式
//Exp1 -> ID | INT | FLOAT | LP Exp RP | ID LP RP | ID LP Args RP | Exp1 LB Exp RB | Exp1 DOT ID
Exp1 : ID {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,NULL,NULL);
    $$->subtype=0;
}
| INT {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,NULL,NULL);
    $$->subtype=1;
}
| FLOAT {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,NULL,NULL);
    $$->subtype=2;
}
| LP Exp RP {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=3;
}
| ID LP RP {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=4;
}
| ID LP Args RP {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    AddChild($$,$4);
    $$->subtype=5;
}
| Exp1 LB Exp RB {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    AddChild($$,$4);
    $$->subtype=6;
}
| Exp1 DOT ID {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=7;
};

//Exp2 -> MINUS Exp2 | NOT Exp2
Exp2 : Exp1 {
    $$=$1;      //Note：为了使语法树输出与原始C--文法完全相同，我们强制规定只在具有运算符的Exp产生式处创建节点
}
| MINUS Exp2 {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,NULL);
    $$->subtype=8;
}
| NOT Exp2 {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,NULL);
    $$->subtype=9;
};

//Exp3 -> Exp3 STAR Exp2 | Exp3 DIV Exp2
Exp3 : Exp2 {
    $$=$1;
}
| Exp3 STAR Exp2 {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=10;
}
| Exp3 DIV Exp2 {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=11;
};

//Exp4 -> Exp3 | Exp4 ADD Exp3 | Exp4 SUB Exp3
Exp4 : Exp3 {
    $$=$1;
}
| Exp4 PLUS Exp3 {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=12;
}
| Exp4 MINUS Exp3 {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=13;
};

//Exp5 -> Exp4 | Exp5 RELOP Exp4
Exp5 : Exp4 {
    $$=$1;
}
| Exp5 RELOP Exp4 {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=14;
};

//Exp6 -> Exp5 | Exp6 AND Exp5
Exp6 : Exp5 {
    $$=$1;
}
| Exp6 AND Exp5 {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=15;
};

//Exp7 -> Exp6 | Exp7 OR Exp6
Exp7 : Exp6 {
    $$=$1;
}
| Exp7 OR Exp6 {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=16;
};

//Exp -> Exp7 | Exp7 ASSIGNOP Exp
Exp : Exp7 {
    $$=$1;
}
| Exp7 ASSIGNOP Exp {
    $$=CreateUnterm(Unterm_Exp,@$.first_line,$1,$2,$3);
    $$->subtype=17;
};

//Args → Exp COMMA Args | Exp
Args : Exp {
    $$=CreateUnterm(Unterm_Args,@$.first_line,$1,NULL,NULL);
    $$->subtype=0;
}
| Exp COMMA Args {
    $$=CreateUnterm(Unterm_Args,@$.first_line,$1,$2,$3);
    $$->subtype=1;
};

%%

void yyerror(const char* msg){
    //do nothing?
}
