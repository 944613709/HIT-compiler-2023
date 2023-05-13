#include <malloc.h>
#include <stdio.h>
#include "tree.h"
/**
 * 创建一个新的结点并返回
 * @param name 结点的名字
 * @param line 结点所在的行数
 * @param type 结点的类型
 * @return 新结点的指针
 */
struct Node *createNode(char *name, int line, NODE_TYPE type) {
    struct Node *pNode = (struct Node *) malloc(sizeof(struct Node));
    pNode->brother = NULL;         // 新结点的兄弟为空
    pNode->child = NULL;           // 新结点的子女为空
    pNode->lineNum = line;         // 记录行号，之后输出有用
    pNode->type = type;            // 记录结点类型，根据结点类型来输出
    pNode->name = strdup(name);    // 使用字符串拷贝赋予新结点的结点名
    pNode->intValue = 1;           // 将 int 值默认设为 1
    //printf("%s\n",name);
    return pNode;                  // 返回 pNode
}
/**
 * 向语法分析树中插入,并返回父结点的指针，插入的是父节点
 * @param node 底层结点（孩子结点）
 * @param name 父结点的名字
 * @param line 父结点所在的行数
 * @param type 父结点的类型
 * @return 父结点的指针
 */
struct Node *insertNode(struct Node *node, char *name, int line, NODE_TYPE type) {
    struct Node *father = (struct Node *) malloc(sizeof(struct Node));
    father->child = node;           // 给输入结点一个爹
    father->brother = NULL;         // 父亲结点的兄弟为空
    father->lineNum = line;         // 记录行号，之后输出有用
    father->type = type;            // 记录结点类型，根据结点类型来输出
    father->name = strdup(name);    // 使用字符串拷贝赋予新结点的结点名
    father->intValue = 1;           // 将 int 值默认设为 1
    head = father;                  // 将 head 置为 father
    //printf("%s %d\n",name,line);
    // if (node)
    //  fprintf(stdout, "%s -> %s   line : %d\n", father -> name, node -> name, line);
    return father;                  // 返回 father
}
/**
 * 根据结点的类型进行打印
 * @param node 结点指针
 * @param f 输出位置
 */
void printNode(struct Node *node,int depth, FILE *f) {
    // 如果是非终结符，且没有孩子，直接返回
    if(node->type == NONTERMINAL &&node->child==NULL)
        return;
    for (int i = 0; i < depth; i++)
        fprintf(f, "  ");                         // 打印语法树所需的空白（制表符）
    if (node->type == STRING_TYPE)
    {
        //String_type有3类
//         a) 如果当前结点是词法单元ID，则要求额外打印该标识符所对应的词素；
//         b) 如果当前结点是词法单元TYPE，则要求额外打印说明以该类型为int还是float；
//         c) 如果当前结点是其他的一个词法单元，则只要打印该词法单元的名称
        if(strcmp(node->name,"ID")==0)
        {
            fprintf(f, "%s : %s\n", node->name, node->id_name);     // string 类型的结点输出结点名和结点内容
        }
        else if(strcmp(node->name,"TYPE")==0)
        {
            fprintf(f, "%s : %s\n", node->name, node->id_name);     // string 类型的结点输出结点名和结点内容
        }
        else
            fprintf(f, "%s\n", node->name);     // string 类型的结点输出结点名和结点内容
    }
    else if (node->type == INT_TYPE)
        fprintf(f, "INT : %d\n", node->intValue);               // int 类型的结点输出 INT 和结点值
    else if (node->type == FLOAT_TYPE)
        fprintf(f, "FLOAT : %f\n", node->floatValue);           // float 类型的结点输出 FLOAT 和结点值
    else
        fprintf(f, "%s (%d)\n", node->name, node->lineNum);     // 非终结符输出结点名字和行号
}
/**
 * 递归法遍历打印语法树
 * @param head   语法树的根结点
 * @param depth  语法树的深度
 * @param f 输出位置
 */
void printTree(struct Node *head, int depth, FILE *f) {
    if (head == NULL) return;                       // 遇到空结点，函数结束
    printNode(head,depth,f);
    printTree(head->child, depth + 1, f);       // 考虑该结点的孩子，深度加一，进入下一层递归
    printTree((head->brother), depth, f);       // 考虑该结点的兄弟，深度不变，进入下一层递归
}
