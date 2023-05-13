
#ifndef DATASTRUCT

#define DATASTRUCT

#include<string.h>
#include<stdlib.h>
#include<stdio.h>


typedef struct{     //方便操作的字符串类String
    char* str;      
    int length;     
}String;

String GenStr(const char* str){     //构造一个String
    int len=strlen(str);
    String s;
    s.length=len;
    s.str=(char*)malloc(len + 2);
    memcpy(s.str,str,len + 1);
    return s;
}

String GenStrN(int n){      //根据数字构造一个String
    static char buf[30];
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%d",n);
    return GenStr(buf);
}

String GenStrRand(){        //生成一个随机字符串（用于给匿名的struct命名）
    static char buf[30];
    for(int i=0; i < 5; i++)
        buf[i]=rand() % 26 + 'a';
    buf[5]=0;
    return GenStr(buf);
}

String CopyStr(String str){ //字符串拷贝
    return GenStr(str.str);
}

String AddStr(String a,String b){   //字符串拼接
    String c;
    c.length=a.length + b.length;
    c.str=(char*)malloc(a.length + b.length + 2);
    memcpy(c.str,a.str,a.length + 1);
    memcpy(c.str + a.length,b.str,b.length + 1);
    return c;
}

String AddStrR(String a,const char* b){ //字符串拼接
    int lb=strlen(b);
    String c;
    c.length=a.length + lb;
    c.str=(char*)malloc(a.length + lb + 2);
    memcpy(c.str,a.str,a.length + 1);
    memcpy(c.str + a.length,b,lb + 1);
    return c;
}

int EquStr(String a,String b){  //判断两个String是否相等
    if(a.length != b.length)
        return 0;
    return strcmp(a.str,b.str) == 0;
}

typedef struct _hn{         
    String str;
    void* value;            //值，可以是任何类型对象的指针（这是抽象数据结构）
    struct _hn* next;
}HashNode;

typedef HashNode** HashMap; 

HashMap CreateMap(){        //初始化映射表
    HashMap hm=(HashMap)malloc(sizeof(HashNode*));
    hm[0]=NULL;
    return hm;
}

void* FindMap(HashMap map,String str){  //在这个链表里暴力查找字符串对应的值
    for(HashNode* ptr=map[0]; ptr != NULL; ptr=ptr->next)
        if(EquStr(ptr->str,str))
            return ptr->value;
    return NULL;                        //查找失败
}

void InsertMap(HashMap map,String key,void* value){      //简单链表插入，保证不存在重复key
    HashNode* nd=(HashNode*)malloc(sizeof(HashNode));
    nd->next=map[0];
    nd->str=CopyStr(key);
    nd->value=value;
    map[0]=nd;
}

void RemoveMap(HashMap map,String key){ //删除key为指定字符串的结点
    if(map[0] == NULL)
        return;
    if(EquStr(map[0]->str,key)){
        HashNode* tmp=map[0]->next;
        map[0]=tmp;
        return;
    }
    for(HashNode* ptr=map[0]; ptr->next != NULL; ptr=ptr->next){
        if(EquStr(ptr->next->str,key)){
            HashNode* tmp=ptr->next;
            ptr->next=tmp->next;
            return;
        }
    }
}

#define Type_INT    0
#define Type_FLOAT  1
#define Type_Struct 2
#define Type_Array  3

typedef struct _tp{     //表示一个类型(数值/数组/struct)具体的信息
    int type;
    int size;           //对于array为数组容量，对于struct为成员个数
    struct _tp* arrbase;    //对于array是数组基类型
    struct _tp** list;  //这是一个动态分配的变长数组，仅对struct有效，表示各个成员的类型
    String* names;      //是和list保持一致的变长数组，仅对struct有效，便是各个成员的名称
    String structname;  //结构体名称
}Type;

// //非选做3的代码
// int EquType(Type* a,Type* b){
//     //两个类型是否等价？
//     if(a->type != b->type)
//         return 0;
//     if(a->type == Type_INT || a->type == Type_FLOAT)
//         return 1;
//     if(a->type == Type_Struct)
//         return EquStr(a->structname,b->structname); //结构体直接判名相等
//     if(a->size != b->size)  //维度容量不一致
//         return 0;
//     return EquType(a->arrbase,b->arrbase);  //递归判断
// }

//选做3的代码
int isStructEqual(Type* a, Type* b)
{
    Type** list_class_a = a->list;
    Type** list_class_b = b->list;
    int i=0;
    while(list_class_a[i] != NULL && list_class_b[i] != NULL)//遍历当前struct的成员
    {
        if(!EquType(list_class_a[i],list_class_b[i]))
            return 0;
        //i++必须放在最后
        i++;
    }
    if(list_class_a[i] != NULL || list_class_b[i] != NULL)
        return 0;
    return 1;
}
//这里是选做3的代码
int EquType(Type* a,Type* b){
    if(a->type != b->type)
        return 0;
    if(a->type == Type_INT || a->type == Type_FLOAT)
        return 1;
    if(a->type == Type_Struct)//TODO 2.3对于结构体用结构等价
    {
        if(isStructEqual(a,b) == 0)
            return 0;
        else
            return 1;
    }
    //如果是数组的话还需要判断数组的维度是否相等
    if(a->size != b->size)  //维度容量不一致
        return 0;
    return EquType(a->arrbase,b->arrbase);  //递归判断
}

typedef struct{     //函数声明，作为函数表结点的值
    int argcnt;     //参数个数
    Type* ret;      //返回类型
    Type** args;    //动态分配的变长数组，各个参数的类型
    String name;    //函数名
}Func;

typedef struct{     //抽象的动态数组
    void** ptr;     //动态数组的内存区，成员可以是任何类型对象的指针
    int size;       //元素个数
    int limit;      //分配空间用的
}Vector;

Vector NewVector(){ //初始化Vector
    Vector vec;
    vec.ptr=(void**)malloc(sizeof(void*));
    vec.limit=1;
    vec.size=0;
    return vec;
}

Vector PushBack(Vector vec,void* obj){
    //向Vector结尾追加一个对象
    if(vec.size == vec.limit){
        void** np=(void**)malloc(vec.limit * 2 * sizeof(void*));
        memcpy(np,vec.ptr,vec.limit * sizeof(void*));
        vec.limit <<= 1;
        free(vec.ptr);
        vec.ptr=np;
    }
    vec.ptr[vec.size++]=obj;
    return vec;
}

#endif