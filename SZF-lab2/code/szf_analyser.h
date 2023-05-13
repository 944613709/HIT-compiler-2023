
#ifndef SZF_ANALYSER
#define SZF_ANALYSER


typedef struct _node{       //表示一个语法树上的结点（终结符/非终结符）
    int isTerminal;         //是否为终结符
    int type;               //类型，对于终结符和非终结符有不同的意义
                            //若结点为终结符则取值为szf.y里定义的token值（例如：INT、FLOAT……）
                            //若结点为非终结符则取值为Unterm_....
    int subtype;            //对于非终结符确定其是哪个产生式（用于lab2）
    int line;               //语法树结点的行号（用于报错）
    int mustright;          //表达式是否必须作为右值（用于lab2中解析Exp）
    union{                  //终结符的值
        int int_val;        //INT终结符的值（在lab2中无用）
        float float_val;    //FLOAT终结符的值（在lab2中无用）
        char str_val[33];   //ID终结符或者TYPE终结符的字符串值
    };
    struct _node* next;     //右兄弟指针
    struct _node* child;    //最靠左的儿子指针
}Node;

typedef Node* pNode;

//非终结符类型定义（Node.type的取值）
#define Unterm_Program          0
#define Unterm_ExtDefList       1
#define Unterm_ExtDef           2
#define Unterm_ExtDecList       3
#define Unterm_Specifier        4
#define Unterm_StructSpecifier  5
#define Unterm_OptTag           6
#define Unterm_Tag              7
#define Unterm_VarDec           8
#define Unterm_FunDec           9
#define Unterm_VarList          10
#define Unterm_ParamDec         11
#define Unterm_CompSt           12
#define Unterm_StmtList         13
#define Unterm_Stmt             14
#define Unterm_DefList          15
#define Unterm_Def              16
#define Unterm_DecList          17
#define Unterm_Dec              18
#define Unterm_Exp              19
#define Unterm_Args             20

#endif