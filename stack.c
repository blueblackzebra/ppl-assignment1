#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct stackNode{
    char * data;
    //parseTree * treeptr;
    int isterminal;
    struct stackNode * next;
}stackNode;


void push(stackNode **s,stackNode * n){
    if(*s==NULL){
        *s=n;
    }
    else{
        n->next=*s;
        *s=n;
    }
    return;
}

void pop(stackNode **s){
    if(*s==NULL){
        return;
    }
    else{
        *s=(*s)->next;
    }
    return;
}

stackNode * top(stackNode **s){
    if(*s==NULL){
        return NULL;
    }
    else{
        return (*s);
    }
}

int isempty(stackNode **s){
    if(s==NULL){
        return 0;
    }
    return 1;
}

stackNode* makestackNode(char * str,int terminal){
    stackNode* temp=(stackNode*)malloc(sizeof(stackNode));
    temp->data=str;
    temp->isterminal=terminal;
    temp->next=NULL;
    return temp;
}

stackNode * makecopy2(stackNode ** s){
    stackNode * temp=NULL;
    stackNode * res=NULL;
    printf("2\n");
    temp=*s;
    printf("2\n");
    while(temp!=NULL){
        
        stackNode* n = makestackNode(temp->data,temp->isterminal);
        
        push(&res,n);
        
        temp=temp->next;
    }
    
    //stackNode* res2=makecopy2(&res);
    return res;
}

stackNode * makecopy(stackNode ** s){
    stackNode * temp=NULL;
    stackNode * res=NULL;
    printf("2\n");
    temp=*s;
    printf("2\n");
    while(temp!=NULL){
        
        stackNode* n = makestackNode(temp->data,temp->isterminal);
        
        push(&res,n);
        
        temp=temp->next;
    }

    stackNode* res2=makecopy2(&res);
    return res2;
}



int main(){
    
    stackNode * s=NULL;
    
    
    stackNode * n = makestackNode("arpit",0);
    stackNode * n1 = makestackNode("jain",0);
    
    push(&s,n);
    push(&s,n1);
   
    stackNode* s2 =makecopy(&s);
    
    stackNode * a =top(&s);
    
    printf("%s\n",a->data);
    pop(&s);
    a =top(&s);
    
    printf("%s\n",a->data);   

    a =top(&s2);
    
    printf("%s\n",a->data);
    pop(&s2);
    a =top(&s2);
    
    printf("%s\n",a->data);


    return 0;
}