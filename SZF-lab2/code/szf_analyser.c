
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<dirent.h>
#include "szf_analyser.h"
#include "szf.tab.h"
#include "datastruct.h"

extern Node* syntax_tree;       //语法树根节点

extern int yyparse();
extern void yyrestart(FILE*);
extern int yylineno;

#define MAX_VAR_CNT     1000    //所有数组的容量上限直接设为1000

String msgs[MAX_VAR_CNT];       //每行的错误信息
int flags[MAX_VAR_CNT];
void SemError(int type,int line,const char* msg){   
    static char buf[1000];
    sprintf(buf,"Error type %d at Line %d: %s\n",type,line,msg);
    if(!flags[line])
        flags[line]=1, msgs[line]=GenStr(buf);
}

void run(const char* filename){     //lab1里继承来的，解析输入文件，得到语法树
    syntax_tree=NULL;
    yylineno=1;
    FILE* f=fopen(filename,"r");
    if(f == NULL){
        printf("Error: Cannot open file '%s'\n",filename);
        exit(-1);
    }
    yyrestart(f);
    yyparse();
    fclose(f);
}

HashMap type_table;         //类型表，key为类型名，val为类型的Type*
HashMap var_table;          //变量表，key为变量名，val为变量的Type*
HashMap func_table;         //函数表，key为函数名，val为Func*

Type* emptystruct;          //预先定义的空结构体类型，当一个变量或类型声明出错无法确定类型时，
                            //语义分析返回这个类型，为了不对后续分析造成太大冲击
Type* inttype;              //预先定义的INT类型
Type* floattype;            //预先定义的FLOAT类型

struct pair{                
    String name;
    Type* type;
};

struct pair solve_vardec(Node* tree,Type* basetype){

    //VarDec → ID | VarDec LB INT RB
    if(tree->subtype == 0){ //得到最深处的ID
        //VarDec->ID
        //举个栗子：int a的a
        struct pair pr;
        pr.type=basetype;
        pr.name=GenStr(tree->child->str_val);
        return pr;
    }else{
        //VarDec->VarDec LB INT RB
        //举个栗子：int a[1] 的a[1]
        struct pair pr=solve_vardec(tree->child,basetype);
        Type* type=(Type*)malloc(sizeof(Type)); //在内层类型之外套一层数组
        type->type=Type_Array;
        type->list=NULL;
        type->names=NULL;
        type->arrbase=pr.type;
        pr.type=type;
        return pr;
    }
}

Type* solve_exp(Node* tree){
    //解析表达式的类型以及检查表达式是否有错
    /*
    Exp -> ID | INT | FLOAT | LP Exp RP | ID LP RP | ID LP Args RP | Exp LB Exp RB | Exp DOT ID
    Exp -> MINUS Exp | NOT Exp
    Exp -> Exp STAR Exp | Exp DIV Exp
    Exp -> Exp ADD Exp | Exp SUB Exp
    Exp -> Exp RELOP Exp
    Exp -> Exp AND Exp
    Exp -> Exp OR Exp
    Exp -> Exp ASSIGNOP Exp
    */
    switch(tree->subtype){
        case 0:{    //Exp->ID，这应为已定义的变量
            String id=GenStr(tree->child->str_val); //变量名
            Type* type=FindMap(var_table,id);   //查找变量表中是否存在
            if(type == NULL){
                //error 1: 未定义的变量
                SemError(1,tree->line,"Undefined variable");
                tree->mustright=0;
                return emptystruct; //返回空类型
            }
            tree->mustright=(type->type == Type_Array); //如果这个变量是数组类型，只能作为右值，不能被赋值
            return type;
        }
        case 1:{    //Exp->INT
            tree->mustright=1;  //常数不能被赋值，只能作为右值
            return inttype;
        }
        case 2:{    //Exp->FLOAT
            tree->mustright=1;
            return floattype;
        }
        case 3:{    
            //LP Exp RP，剥开括号递归解析即可
            //tree->child->next即为第二个儿子Exp
            //要对里面的Exp进行类型检查
            Type* ret=solve_exp(tree->child->next);
            tree->mustright=tree->child->next->mustright;
            return ret;
        }
        case 5:     //ID LP Args RP
        case 4:{    //ID LP RP
            //ID ( )

            //先检查是否有这个函数
            tree->mustright=1;  //只能作为右值
            String name=GenStr(tree->child->str_val);   //ID，函数名
            Func* func=FindMap(func_table,name);
            if(func == NULL){
                //检查是否为普通变量，选择性报错
                if(FindMap(var_table,name) != NULL)
                    SemError(11,tree->line,"Cannot use '()' with normal variable");//error 11: 不能对普通变量使用()
                else
                    SemError(2,tree->line,"Undefined function");//error 2: 未定义的函数
                return emptystruct;
            }
            //检查是否匹配函数参数列表
            Vector argtypes=NewVector();    //用一个动态数组存调用时传入的参数类型，成员为Type*
            if(!tree->child->next->next->isTerminal){   //具有非空的Args
                Node* args=tree->child->next->next; //Args非终结符
                //Args → Exp COMMA Args | Exp
                while(1){   //用非递归的方式把Args直接剥开解析，因为其结构简单
                    //args->child即为当前的参数Exp，递归解析其类型
                    argtypes=PushBack(argtypes,solve_exp(args->child));
                    if(args->subtype == 0)  //Args->Exp
                        break;
                    args=args->child->next->next;   //剥离这个参数，继续
                }
            }
            int flag=0;
            if(argtypes.size != func->argcnt)   //参数数量就不匹配
                flag=1;
            else{   //检查每个参数的类型是否匹配
                for(int i=0; i < func->argcnt && !flag; i++)
                    if(!EquType(argtypes.ptr[i],func->args[i]))
                        flag=1;
                //Note：通过argtypes.ptr[i]顺序遍历Vector中的元素
            }
            if(flag)//参数不匹配
                SemError(9,tree->line,"Function is not applicable for arguments");//error 9: 调用参数不匹配
            return func->ret;
        }
        case 6:{    
            //exp->subtype = 6 代表 Exp LB Exp RB
            //这里以非递归方式展开多维数组
            tree->mustright=0;
            Node* exp=tree;
            int dim=0;//记录数组维数
            while(exp->subtype == 6){
                //把外面的LB Exp RB都剥离开，记录剥了多少层（维度数）
                //不断循环直到剥到Exp->ID
                dim++;
                Type* ind=solve_exp(exp->child->next->next);
                if(ind->type != Type_INT)
                    SemError(12,exp->line,"Array index must be integer expression");//error 12: 数组索引必须是整型表达式
                exp=exp->child;
            }
            //此时exp是一个Exp->ID的Exp结点
            //即拿到的是a[1][9][1][9][8][1]中的a
            if(!(exp->type == Unterm_Exp && !exp->isTerminal && exp->subtype == 0)){
                //如果剥离到最后不是一个Exp->ID的结点，报错
                SemError(10,exp->line,"Cannot use '[]' for non-array variable");//error 10: 不能对非数组变量使用[]
                return emptystruct;
            }else{
                //此时exp是一个Exp->ID的Exp结点
                String id=GenStr(exp->child->str_val);  //变量名,即a
                Type* type=FindMap(var_table,id);
                if(type == NULL){   //变量表里没有这个变量
                    SemError(1,exp->line,"Undefined variable");
                    return emptystruct;
                }
                //判断维数是否匹配
                int rdim=0; //变量表里声称的这个变量的真正类型的数组维数
                while(type->type == Type_Array)
                    type=type->arrbase, rdim++;
                if(rdim != dim){
                    if(rdim)
                        SemError(10,exp->line,"Cannot just use part of array dimensions");//
                    else    //rdim=0，根本就不是数组变量
                        SemError(10,exp->line,"Cannot use '[]' for non-array variable");//
                }
                return type;
            }
        }
        case 7:{    //Exp DOT ID
            //正确通过编译的例子: struct A{int val;}; struct A a; a.val;
            Type* ltype=solve_exp(tree->child);
            if(ltype->type != Type_Struct){ //左边不是结构体类型
                SemError(13,tree->line,"Illegal use of '.' --- Cannot use '.' for non-struct variable");//error 13: 不能对非结构体变量使用.
                //这里报错的例子有：int a; a.b;
                return emptystruct;
            }   
            //这里的tree->child->next->next就是产生式里的ID儿子
            String member=GenStr(tree->child->next->next->str_val); //右边的ID成员名
            int flag=1;
            Type* type=emptystruct;
            for(int i=0; i < ltype->size && flag; i++)  //检查左边的结构体类型里是否有这个成员
                if(EquStr(ltype->names[i],member))
                    flag=0, type=ltype->list[i];
            if(flag)
                SemError(14,tree->line,"Non-existent field ---- Cannot find the member in struct");//error 14: 不能在结构体中找到成员
            tree->mustright=(type->type == Type_Array);
            return type;
        }
        case 8:     //MINUS Exp
        case 9:{    //NOT Exp
            //这俩的产生式结构都是一样的，在语义分析看来他俩没区别
            //只要保证Exp必须是INT/FLOAT即可
            Type* type=solve_exp(tree->child->next);
            tree->mustright=1;  //已经有了运算因此只能作为右值
            if(type->type != Type_INT && type->type != Type_FLOAT)
                SemError(7,tree->line,"Invalid operation on such type");//error 7: 对此类型无效的操作
                //报错例子：int a; -a;
            return type;
        }
        case 10:    //Exp STAR Exp
        case 11:    //Exp DIV Exp
        case 12:    //Exp PLUS Exp
        case 13:    //Exp MINUS Exp
        case 14:    //Exp RELOP Exp
        case 15:    //Exp AND Exp
        case 16:{   //Exp OR Exp
            //这里就非常爽，这几个产生式都是形如Exp OP Exp的形式
            //在语义分析看来它们没区别，只需要检查两边操作数类型是否是一致的INT/FLOAT即可
            tree->mustright=1;
            Type* ta=solve_exp(tree->child);
            Type* tb=solve_exp(tree->child->next->next);
            if(!((ta->type == Type_INT || ta->type == Type_FLOAT) && ta->type == tb->type))
               SemError(7,tree->line,"Type mismatched for operands");
            return ta;  
        }
        case 17:{   //Exp = Exp，赋值
            tree->mustright=0;
            Type* ta=solve_exp(tree->child);
            Type* tb=solve_exp(tree->child->next->next);
            if(tree->child->mustright)  //左边不能只有右值，否则不能给他赋值
                SemError(6,tree->line,"Cannot assign expression to right value");
                //报错例子: 
            else if(!EquType(ta,tb))    //类型不匹配
                SemError(5,tree->line," Type mismatched for assignment");
            return ta;
        }
    }
}

Type* solve_speifier(Node* tree){             
    //解析Specifier的语法树描述的类型，若为新类型则在类型表中注册
    
    //Specifier → TYPE | StructSpecifier
    if(tree->subtype == 0){
        //Specifier → TYPE
        return (tree->child->str_val[0] == 'i') ? inttype : floattype;
    }else{
        //Specifier -> StructSpecifier
        //struct在这里被解析！
        tree=tree->child;
        Node* structspec=tree;
        /*
        StructSpecifier → STRUCT OptTag LC DefList RC | STRUCT Tag
        OptTag → ID | e
        Tag → ID
        */
        if(structspec->subtype == 0){
            //找到DefList并直接非递归解析，得到结构体定义的所有成员
            Node* ptr=structspec->child;
            //那么structspec的儿子只有STRUCT LC RC，这样
            while(!(ptr->isTerminal && ptr->type == LC))
                ptr=ptr->next;
            Node* deflist=NULL;
            if(!(ptr->next->isTerminal && ptr->next->type == RC))
                deflist=ptr->next;
            /*
            DefList → Def DefList | e
            Def → Specifier DecList SEMI
            DecList → Dec | Dec COMMA DecList
            Dec → VarDec | VarDec ASSIGNOP Exp
            VarDec → ID | VarDec LB INT RB
            */
            Vector types=NewVector();   //存储结构体里所有成员的Type*
            Vector names=NewVector();   //存储结构体里所有成员的名称，用char*存
            while(deflist != NULL){ 
                //这个循环会一行一行地遍历deflist的所有def
                //每个def形如int a,b[3][2],c;这样的形式
                //因此里面还需要再来一个循环遍历这个def中的成员名
                Node* def=deflist->child;
                //Def → Specifier DecList SEMI
                deflist=def->next;
                Type* stype=solve_speifier(def->child); //这行定义的成员基本类型
                Node* declist=def->child->next;
                while(1){   //遍历这行def的新成员名
                    Node* dec=declist->child;
                    //DecList → Dec | Dec COMMA DecList
                    //Dec → VarDec | VarDec ASSIGNOP Exp
                    if(dec->subtype == 1)   //Dec->VarDec ASSIGNOP Exp，这在结构体中是非法的
                        SemError(15,dec->line,"Cannot init member in struct");
                    Node* vardec=dec->child;
                    struct pair pr=solve_vardec(vardec,stype);  //解析VarDec（如果为数组定义的话）
                    for(int i=0; i < names.size; i++)   //检查struct里之前是否有同名成员
                        if(strcmp((char*)names.ptr[i],pr.name.str) == 0)
                            SemError(15,vardec->line,"Redefined field --- Conflict members by name in struct");
                    types=PushBack(types,pr.type);
                    names=PushBack(names,pr.name.str);
                    if(declist->subtype == 0)   //DecList->Dec
                        break;
                    declist=declist->child->next->next; //DecList->Dec COMMA DecList
                }
            }
            Type* type=(Type*)malloc(sizeof(Type));
            type->type=Type_Struct;
            type->size=types.size;
            if(types.size){ //把成员信息分配空间并复制进去
                type->names=(String*)malloc(types.size * sizeof(String));
                type->list=(Type**)malloc(types.size * sizeof(Type*));
                for(int i=0; i < types.size; i++){
                    type->names[i].length=strlen((char*)names.ptr[i]);
                    type->names[i].str=names.ptr[i];
                    type->list[i]=types.ptr[i];
                }
            }
            //若具有名称则装入类型表
            if(!tree->child->next->isTerminal){ //OptTag非空
                Node* id=tree->child->next->child;  //OptTag->ID
                String name=GenStr(id->str_val);
                type->structname=name;
                if(FindMap(type_table,name) != NULL)    //重复定义struct
                    SemError(16,id->line,"Duplicated name --- Redefinition struct with same name");
                else{
                    InsertMap(type_table,name,type);    //插入类型表
                }
            }else
                type->structname=GenStrRand();  //若为匿名的结构体，则生成一个随机名称
            return type;
        }else{
            //STRUCT Tag，已定义的struct
            String name=GenStr(tree->child->next->child->str_val);
            Type* type=FindMap(type_table,name);    //查找类型表里是否有这个struct
            if(type == NULL){
                SemError(17,tree->line,"Undefined structure");
                return emptystruct;
            }
            return type;
        }
    }
}

void solve_var_defs(Node* tree,Type* type){               
    //解析变量定义列表，完成变量定义，插入变量表
    //tree可以是ExtDecList/DecList结点
    /*
    ExtDecList → VarDec | VarDec COMMA ExtDecList
    DecList → Dec | Dec COMMA DecList
    Dec → VarDec | VarDec ASSIGNOP Exp
    VarDec → ID | VarDec LB INT RB
    */
    Node* vardec;
    if(tree->type == Unterm_ExtDecList) 
        vardec=tree->child; //ExtDecList → VarDec | VarDec COMMA ExtDecList
    else
        vardec=tree->child->child;  //DecList → Dec | Dec COMMA DecList
                                    //Dec → VarDec | VarDec ASSIGNOP Exp
    struct pair pr=solve_vardec(vardec,type);   //解析这个vardec的真正类型与变量名
    //检查重复定义
    if(FindMap(var_table,pr.name) != NULL)
        SemError(3,vardec->line,"Redefinition variable with same name");
    else{
        if(FindMap(type_table,pr.name) != NULL) //出于报错的奇怪要求还得查是不是之前定义的struct名
            SemError(3,vardec->line,"Conflict definition of var and struct");
        else{
            InsertMap(var_table,pr.name,pr.type);   //插入变量表
        }
    }
    //ExtDecList → VarDec COMMA ExtDecList，递归解析ExtDecList
    if(tree->type == Unterm_ExtDecList && tree->subtype == 1)
        solve_var_defs(tree->child->next->next,type);
    
    if(tree->type == Unterm_DecList){
        //对于Dec而言，还要检查赋初值
        //DecList → Dec | Dec COMMA DecList
        Node* dec=tree->child;
        if(dec->subtype == 1){
            //Dec → VarDec ASSIGNOP Exp
            Node* exp=dec->child->next->next;
            if(pr.type->type == Type_Array) //数组不能直接赋初值
                SemError(6,dec->line,"Array cannot be assigned directly");
            else{
                Type* rtp=solve_exp(exp);   //检查赋初值时类型是否匹配
                if(!EquType(rtp,pr.type))
                    SemError(7,dec->line,"Cannot assign with different types");
            }
        }
        if(tree->subtype == 1)  //DecList → Dec COMMA DecList，递归解析DecList
            solve_var_defs(tree->child->next->next,type);
    }
}

Func* nowfunc=NULL;     //当前语句所在的函数体的函数定义，主要用于检查Return类型匹配

void solve(Node* tree){             //对语法树进行语义检查
    if(tree == NULL)
        return;
    if(tree->isTerminal){
        //终结符，啥都不用管就行了
    }else{
        switch(tree->type){
        case Unterm_ExtDef:{
            //ExtDef → Specifier ExtDecList SEMI | Specifier SEMI | Specifier FunDec CompSt
            Type* type=solve_speifier(tree->child); //先解析开头的Specifier类型
            if(tree->subtype == 0)  //ExtDef → Specifier ExtDecList SEMI
                solve_var_defs(tree->child->next,type);
            else if(tree->subtype == 2){
                /*
                FunDec → ID LP VarList RP | ID LP RP
                VarList → ParamDec COMMA VarList | ParamDec
                ParamDec → Specifier VarDec
                */
                Node* fundec=tree->child->next;
                Vector types=NewVector();   //参数们的类型Type*
                Vector names=NewVector();   //参数们的名称，用char*存
                if(fundec->subtype == 0){
                    //FunDec → ID LP VarList RP
                    Node* varlist=fundec->child->next->next;
                    //非递归解析VarList，遍历其中的ParamDec
                    while(1){
                        //VarList → ParamDec COMMA VarList | ParamDec
                        Node* paramdec=varlist->child;
                        //ParamDec → Specifier VarDec
                        Type* argtype=solve_speifier(paramdec->child);  //基本类型
                        struct pair pr=solve_vardec(paramdec->child->next,argtype);
                        //解析vardec得到名称与真正类型
                        types=PushBack(types,pr.type);
                        names=PushBack(names,pr.name.str);
                        if(varlist->subtype == 1)   //VarList->ParamDec
                            break;
                        varlist=varlist->child->next->next; //VarList → ParamDec COMMA VarList
                    }
                }
                //检查是否在函数声明表中冲突
                String name=GenStr(fundec->child->str_val);
                Func* func=(Func*)malloc(sizeof(Func));
                func->argcnt=types.size;
                func->ret=type;
                func->args=types.ptr;
                func->name=name;
                Func* other=FindMap(func_table,name);   //检查是否重复定义
                if(other != NULL)
                    SemError(4,tree->line,"Redefined function");
                else
                    InsertMap(func_table,name,func);
                if(tree->subtype == 2){
                    //将参数注册成函数体内的变量
                    for(int i=0; i < names.size; i++){
                        String arg=GenStr(names.ptr[i]);
                        if(FindMap(var_table,arg) != NULL)
                            SemError(3,tree->line,"Redefinition argument with same name");
                        else if(FindMap(type_table,arg) != NULL)
                            SemError(3,tree->line,"Conflict argument definition with struct");
                        else
                            InsertMap(var_table,arg,types.ptr[i]);
                    }
                    nowfunc=func;
                    //ExtDef → Specifier FunDec CompSt，递归解析CompSt函数体
                    solve(tree->child->next->next);
                }
            }
            break;
        }
        case Unterm_CompSt:{
            //CompSt → LC DefList StmtList RC
            //直接非递归解析deflist
            //stmtlist直接无脑递归处理
            Node* deflist=tree->child->next;
            Node* stmtlist=tree->child->next;
            //这里要注意deflist和stmtlist可能是空产生式，前面说了语法树上没有空儿子
            if(!deflist->isTerminal && deflist->type == Unterm_DefList){
                stmtlist=stmtlist->next;
                //DefList → Def DefList | e
                //非递归解析DefList，遍历所有Def
                while(deflist != NULL){
                    Node* def=deflist->child;
                    //Def → Specifier DecList SEMI
                    Type* type=solve_speifier(def->child);
                    solve_var_defs(def->child->next,type);  //解析DecList，注册定义的变量
                    deflist=deflist->child->next;
                }
            }
            if(!stmtlist->isTerminal && stmtlist->type == Unterm_StmtList)
                solve(stmtlist);    //递归处理StmtList
            break;
        }
        case Unterm_Stmt:{
            /*
            Stmt → Exp SEMI | CompSt | RETURN Exp SEMI | IF LP Exp RP Stmt 
                | IF LP Exp RP Stmt ELSE Stmt | WHILE LP Exp RP Stmt
            */
            switch(tree->subtype){
            case 0:     //Exp SEMI
                solve_exp(tree->child); break;
            case 1:     //CompSt
                solve(tree->child); break;
            case 2:{    //RETURN Exp SEMI
                Type* type=solve_exp(tree->child->next);
                if(!EquType(nowfunc->ret,type)) //判断return的是否和当前函数的类型匹配
                    SemError(8,tree->line,"Type mismatched for return");
                break;
            }
            case 3:     //IF LP Exp RP Stmt 
            case 4:     //IF LP Exp RP Stmt ELSE Stmt
            case 5:{    //WHILE LP Exp RP Stmt （说真的这个和IF其实没有任何区别）
                Type* condtype=solve_exp(tree->child->next->next);
                if(condtype->type != Type_INT)  //条件表达式必须是INT类型
                    SemError(7,tree->line,"If/While condition must be integer expression");
                solve(tree->child->next->next->next->next); //解析第一个Stmt，位置都一样
                if(tree->subtype == 4)  //IF LP Exp RP Stmt ELSE Stmt，解析第二个Stmt
                    solve(tree->child->next->next->next->next->next->next);
                break;
            }
            }
            break;
        }
        default:{   //Program、StmtList等无关紧要的非终结符，无脑解析所有儿子即可
            for(Node* ptr=tree->child; ptr != NULL; ptr=ptr->next)
                solve(ptr);
        }
        }
    }
}

int main(int argc,char** argv){
    if(argc < 2){
        printf("Error: Too few args.\n");
        return -1;
    }
    run(argv[1]);

    emptystruct=(Type*)malloc(sizeof(Type));
    emptystruct->list=NULL;
    emptystruct->names=NULL;
    emptystruct->size=0;
    emptystruct->type=Type_Struct;
    inttype=(Type*)malloc(sizeof(Type));
    inttype->type=Type_INT;
    floattype=(Type*)malloc(sizeof(Type));
    floattype->type=Type_FLOAT;

    var_table=CreateMap();
    type_table=CreateMap();
    func_table=CreateMap();

    solve(syntax_tree);

    for(int i=0; i < MAX_VAR_CNT; i++)
        if(flags[i])
            printf("%s",msgs[i].str);

    return 0;
}
