#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Type Expression union structure

typedef union{
    //insert type fields here
    char* type;
}Type_exp;

// Parse tree stucture

typedef struct parseTree{
    char* nodename;
    int is_terminal;
    Type_exp type;
    struct parseTree ** children;
    int child_count;
}parseTree;

// Parse tree functions

parseTree * makenode(char * str,int terminal){
    parseTree * temp=(parseTree *)malloc(sizeof(parseTree));
    temp->nodename=str;
    temp->is_terminal=terminal;
    temp->children=(parseTree**)malloc(sizeof(parseTree*)*50); //no node will have more than 50 children
    temp->child_count=0;
    //add type expression here;
}

parseTree * insert(parseTree* node,char* str,int terminal){
    parseTree* temp=makenode(str,terminal);
    node->children[node->child_count]=temp;
    node->child_count++;
    return temp;
}

void printTree(parseTree* t){
    if(t==NULL){
        return;
    }
    printf("%s ",t->nodename);
    for(int i=0;i<t->child_count;i++){
        printTree(t->children[i]);
    }
    
    return;
}

parseTree * makeTreeCopy(parseTree* t){
    if(t==NULL){
        return NULL;
    }
    parseTree * res = makenode(t->nodename,t->is_terminal);
    res->child_count=t->child_count;
    for(int i=0;i<t->child_count;i++){
        res->children[i]=makeTreeCopy(t->children[i]);
        
    }
    return res;
}

void createParseTree(){
    //have a stack and a tree and do recursive calls passing tree and stack by value

    //now we need is a check function (this is where all work happens).
    return;
}

int main(){
    parseTree * t =makenode("startsymbol",0); // This node will be passed to all other functions
    insert(t,"2.1",0);
    parseTree * ch = insert(t,"2.2",0);
    insert(ch,"3.1",1);
    insert(t,"2.3",1);

    //make a stack and push 
    printTree(t);
    printf("\n");

    parseTree * t2 = makeTreeCopy(t);
    printTree(t2);

    printf("\n");
    return 0;
}