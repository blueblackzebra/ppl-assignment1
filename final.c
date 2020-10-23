#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* ParseTree module */

typedef union{
    //insert type fields here
    char* type;
}Type_exp;

// Parse tree stucture

typedef struct parseTree{
    char* nodename;
    int is_terminal;
    
    //Type_exp type;

    struct parseTree ** children;
    int child_count;

    struct parseTree * copy;  // for making tree copy and link it to new stack
}parseTree;

// Parse tree functions

parseTree * makenode(char * str,int terminal){
    parseTree * temp=(parseTree *)malloc(sizeof(parseTree));
    temp->nodename=str;
    temp->is_terminal=terminal;
    temp->children=(parseTree**)malloc(sizeof(parseTree*)*50); //no node will have more than 50 children
    temp->child_count=0;
    temp->copy=NULL;
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
    t->copy=res; //updated copy pointer
    for(int i=0;i<t->child_count;i++){
        res->children[i]=makeTreeCopy(t->children[i]);
        
    }
    return res;
}

/* Stack module */

typedef struct stackNode{
    char * data;
    parseTree * treeptr;
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
    if(*s==NULL){
        return 1;
    }
    return 0;
}

stackNode* makestackNode(char * str,int terminal,parseTree * ptr){
    stackNode* temp=(stackNode*)malloc(sizeof(stackNode));
    temp->data=str;
    temp->isterminal=terminal;
    temp->next=NULL;
    temp->treeptr=ptr;
    return temp;
}

stackNode * makecopy2(stackNode ** s){
    stackNode * temp=NULL;
    stackNode * res=NULL;
    // printf("2\n");
    temp=*s;
    // printf("2\n");
    while(temp!=NULL){
        
        stackNode* n = makestackNode(temp->data,temp->isterminal,temp->treeptr);
        
        push(&res,n);
        
        temp=temp->next;
    }
    
    //stackNode* res2=makecopy2(&res);
    return res;
}

stackNode * makecopy(stackNode ** s){
    stackNode * temp=NULL;
    stackNode * res=NULL;
    // printf("2\n");
    temp=*s;
    // printf("2\n");
    while(temp!=NULL){
        
        stackNode* n = makestackNode(temp->data,temp->isterminal,temp->treeptr->copy);

        temp->treeptr->copy=NULL;
        
        push(&res,n);
        
        temp=temp->next;
    }

    stackNode* res2=makecopy2(&res);
    return res2;
}

/* Grammar module */

struct Node{
    char * piece;
    struct Node * next;
};

typedef struct {
    struct Node ** rules;
} grammar;

char filename[]="grammar.txt";

void traverseG(grammar * G){
    struct Node ** p=G->rules;

    int i=0;
    // printf("Start\n");

    while (p[i]){
        struct Node * n=p[i];
        // printf("%d\n",i);
        printf("%s\n",n->piece);
        while (n->next){
            n=n->next;
            printf("%s\n",n->piece);
            // printf("%p\n",(void *) n->piece);
            

        }
        printf("\n");
        i++;
    }
  
}

grammar * readGrammar(char * filename){
    FILE * filep=fopen(filename,"r");

    grammar * G=(grammar *)malloc(sizeof(grammar));

    struct Node * temp;
    G->rules=(struct Node **)malloc(1000*sizeof(struct Node *));
    int count=0;

    char * line=(char *)malloc(200*sizeof(char));
    char * check;
    while (1){
        check=fgets(line,200,filep);
        if (!check){
            break;
        }
        if (line[strlen(line)-1]=='\n'){
            line[strlen(line)-1]='\0';

        }
        // temp[count]=(Node *)malloc(sizeof(Node));
        struct Node * trav=(struct Node *)malloc(sizeof(struct Node));
        // printf("Address allocated to trav at %p\n",(void *) trav);
        G->rules[count]=trav;

        char * tempStr;
        tempStr=strtok(line," ");

        trav->piece=(char *)malloc((strlen(tempStr)+1)*sizeof(char));
        strcpy(trav->piece,tempStr);
       
        trav->next=(struct Node *)malloc(sizeof(struct Node));
        // trav=trav->next;
        tempStr=strtok(NULL," ");

        
        while (tempStr){
            trav=trav->next;

            trav->piece=(char *)malloc((strlen(tempStr)+1)*sizeof(char));
            strcpy(trav->piece,tempStr);
            
            trav->next=(struct Node *)malloc(sizeof(struct Node));
            tempStr=strtok(NULL," ");
        }

        // free(trav->next);
        trav->next=NULL;
        
        count++;
        
    }
    fclose(filep);
    return G;
}

/* Token Stream module */

struct Symbol{
    int line_num;
    char * lexeme;
    char * token;
    struct Symbol * next;
};

typedef struct Symbol tokenStream;

char sourcename[]="Egsourcecode.txt";

void traverseS(tokenStream * s){
    printf("\nTraversal begins\n");
    while (s!=NULL){
        printf("%s %d\n",s->lexeme,s->line_num);
        s=s->next;
    }
}

char * getToken(char * lex){
    char * tok=(char *)malloc(30*sizeof(char));

    if (!strcmp(lex,"{")){
        tok="COB";   
    }
    else if(!strcmp(lex,"}")){
        tok="CCB";
    }
    else if(!strcmp(lex,"[")){
        tok="SOB";
    }
    else if(!strcmp(lex,"]")){
        tok="SCB";
    }
    else if(!strcmp(lex,"()")){
        tok="RB";
    }
    else if(!strcmp(lex,"..")){
        tok="TO";
    }
    else if(!strcmp(lex,";")){
        tok="SC";
    }
    else if(!strcmp(lex,":")){
        tok="COL";
    }
    else if(!strcmp(lex,"=")){
        tok="ASSGN_OP";
    }
    else if(!strcmp(lex,"&&&")){
        tok="BOOL_AND";
    }
    else if(!strcmp(lex,"|||")){
        tok="BOOL_OR";
    }
    else if(!strcmp(lex,"*") || !strcmp(lex,"/")){
        tok="AR_OP1";
    }
    else if(!strcmp(lex,"+") || !strcmp(lex,"-")){
        tok="AR_OP2";
    }
    else if(!strcmp(lex,"program")){
        tok="program";
    }
    else if(!strcmp(lex,"declare")){
        tok="declare";
    }
    else if(!strcmp(lex,"list")){
        tok="list";
    }
    else if(!strcmp(lex,"of")){
        tok="of";
    }
    else if(!strcmp(lex,"variables")){
        tok="variables";
    }
    else if(!strcmp(lex,"array")){
        tok="array";
    }
    else if(!strcmp(lex,"size")){
        tok="size";
    }
    else if(!strcmp(lex,"values")){
        tok="values";
    }
    else if(!strcmp(lex,"jagged")){
        tok="jagged";
    }
    else if(!strcmp(lex,"of")){
        tok="of";
    }
    else if(!strcmp(lex,"integer")){
        tok="integer";
    }
    else if(!strcmp(lex,"boolean")){
        tok="boolean";
    }
    else if(!strcmp(lex,"real")){
        tok="real";
    }
    else if(!strcmp(lex,"R1")){
        tok="R1";
    }
    else if(lex[0]>=48 && lex[0]<57){
        tok="STATIC_CNST";
    }
    else {
        tok="VAR_NAME";
    }

    return tok;
}


tokenStream * tokeniseSourceCode(char * filename,tokenStream *s){
    FILE * filep=fopen(filename,"r");
    s=(tokenStream *)malloc(sizeof(tokenStream));
    tokenStream * ret=s;

    char * line=(char *)malloc(200*sizeof(char));
    char * check;
    int count=1;

    while (1){
        check=fgets(line,200,filep);
        if (!check){
            break;
        }
        // printf("%s\n",line);
        if (line[strlen(line)-1]=='\n'){
            line[strlen(line)-1]='\0';

        }

        char * tempStr;
        tempStr=strtok(line," ");

        // printf("%s\n",tempStr);

        while (tempStr){
            s->line_num=count;
            s->lexeme=(char *)malloc((strlen(tempStr)+1)*sizeof(char));
            strcpy(s->lexeme,tempStr);
            s->token=(char *)malloc((strlen(tempStr)+1)*sizeof(char));
            strcpy(s->token,getToken(tempStr));
            // printf("%s %d\n",s->lexeme,s->line_num);
            tempStr=strtok(NULL," ");

            s->next=(tokenStream *)malloc(sizeof(tokenStream));
            s=s->next;
        }

        count++;
        // printf("%d\n",count);
    }
    s=ret;

    // Check
    while (s->next->line_num<count && s->next->line_num>0){
        // printf("%d\n",s->next->line_num);
        s=s->next;
    }
    s->next=NULL;

    return ret;
}


void pushRule(struct Node * n, stackNode ** s){
    stackNode * non_term = top(s);
    pop(s);
    non_term->next=NULL;

    stackNode * rev=NULL;
    while (n->next){
        n=n->next;
        stackNode * temps;
        parseTree * tempt;
        
        if (!strcmp(n->piece,"epsilon")){
            insert(non_term->treeptr,"epsilon",1);
            break;
        }
        if ((n->piece)[0]=='<'){
            tempt=insert(non_term->treeptr,n->piece,0);
            temps=makestackNode(n->piece,0,tempt);
        }
        else {
            tempt=insert(non_term->treeptr,n->piece,1);
            temps=makestackNode(n->piece,1,tempt);
        }
        push(&rev,temps);
        
        temps=top(&rev);
        // printf("%s\n",temp->data);
        

    }
    // printf("%d\n",isempty(&rev));
    int count=0;
    while (!isempty(&rev)){
        
        stackNode * temp=top(&rev);
        // printf("%s\n",temp->data);
        stackNode * temp2=top(s);
        // printf("S is %s\n",temp2->data);
        pop(&rev);
        temp->next=NULL;
        push(s,temp);
    }
}

int terminalMatch(stackNode ** s,tokenStream ** ts){
    stackNode * temp=top(s);
    
    while (temp!=NULL && temp->isterminal){
        if (strcmp(temp->data,(*ts)->token)){
            return 0;
        }
        printf("Matched is %s\n",temp->data);
        pop(s);
        temp=top(s);
        (*ts)=(*ts)->next;
    }

    return 1;
}

void printLevelTree(stackNode ** first, stackNode ** second){

    while (!isempty(first)){
        stackNode * temp3=top(first);
        pop(first);
        temp3->next=NULL;
        push(second,temp3);
    }

    if (isempty(second)){
        return;
    }

    while (!isempty(second)){
        // Read node
        // Push to first
        stackNode * temp=top(second);
        pop(second);
        temp->next=NULL;
        printf("%s ",temp->treeptr->nodename);
        
        for(int i=0;i<temp->treeptr->child_count;i++){
            stackNode * temp2=makestackNode(NULL,-1,temp->treeptr->children[i]);
            push(first,temp2);
        }
    }
    printf("\n");

    printLevelTree(first,second);
}

int genTree(parseTree* root,stackNode ** s, tokenStream ** ts, grammar * G){
    int check=terminalMatch(s,ts);

    printf("current tree is: \n");
    printTree(root);
    printf("\nLevel tree is \n");
    stackNode * first=NULL;
    stackNode * second=NULL;
    stackNode * x=makestackNode(NULL,-1,root);
    push(&first,x);
    printLevelTree(&first,&second);
    printf("Tree end\n\n");

    if (!check){
        return 0;
    }

    if (isempty(s)){
        return 1;
        
    }

    for (int i=0;i<60;i++){
        struct Node * iterRule=(G->rules)[i];
        stackNode * iterStack=top(s);
        if (!strcmp(iterRule->piece,iterStack->data)){
            printf("Rule number selected is %d\n",i+1);
            printf("Token pos is %s\n",(*ts)->token);
            


            parseTree* root2 = makeTreeCopy(root);
            stackNode * s2=makecopy(s);
            tokenStream * ts2=(*ts); 

            // Tree code here

            pushRule(iterRule,&s2);
            stackNode * aaaa =top(&s2);
            printf("%s\n",aaaa->data);
            int check2=genTree(root2,&s2,&ts2,G);
            if (check2){
                return 1;
            }
        }
    }

    return 0;
}

void createParseTree(stackNode ** s,tokenStream ** ts){
    // generate tree
}



int main(){
    
    stackNode * s=NULL;
    stackNode * t=NULL;
    parseTree * root=NULL;
    
    grammar * temp;
    temp=readGrammar(filename);

    tokenStream * stream;
    stream=tokeniseSourceCode(sourcename,stream);

    root=makenode("<main_program>",0);

    t=makestackNode("<main_program>",0,root);
    push(&s,t);

    int value=genTree(root,&s,&stream,temp);
    printf("%d\n",value);

    
    
    // getToken("{");


    // t=top(&s);
    // printf("%s\n",t->data);

    // pushRule((temp->rules)[0],&s);

    // t=top(&s);
    // printf("%s\n",t->data);
    

    
    // // printf("gsdgsd\n");

    // printf("Lexeme is %s\n",stream->lexeme);

    // t=top(&s);
    // printf("top is %s\n",t->data);


    // terminalMatch(&s,&stream);

    // printf("Lexeme is %s\n",stream->lexeme);

    // t=top(&s);
    // printf("top is %s\n",t->data);



   
    // stackNode* s2 =makecopy(&s);
    
    // stackNode * a =top(&s);
    
    // printf("%s\n",a->data);
    // pop(&s);
    // a =top(&s);
    
    // printf("%s\n",a->data);   

    // a =top(&s2);
    
    // printf("%s\n",a->data);
    // pop(&s2);
    // a =top(&s2);
    
    // printf("%s\n",a->data);


    return 0;
}