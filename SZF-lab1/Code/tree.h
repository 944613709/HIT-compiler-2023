#ifndef TREE_H
#define TREE_H

#include <string.h>
typedef int NODE_TYPE;        // 结点类型
// 非终结符类型
#define NONTERMINAL 0         
// 终结符类型
#define INT_TYPE 1            // int 类型
#define FLOAT_TYPE 2          // float 类型
#define STRING_TYPE 3         // 可打印类型

struct Node {
    struct Node *child;       // 儿子结点
    struct Node *brother;     // 兄弟节点
    int lineNum;              // 行号
    char *name;               // 结点名字
    NODE_TYPE type;           // 结点类型
    union {//union中的变量共用一段内存空间，union中的变量只能有一个有值，union中最大的变量决定了union的大小
        char *id_name;        // ID 名字
        int intValue;         // int 值
        float floatValue;     // float 值
    };
};
struct Node *head;              // 语法分析树的根结点
// 函数的声明们
struct Node *createNode(char *name, int line, NODE_TYPE type);
struct Node *insertNode(struct Node *node, char *name, int line, NODE_TYPE type);
void printNode(struct Node *node, int depth,FILE *f);
void printTree(struct Node* head, int depth, FILE *f);
#endif
