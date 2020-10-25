#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <malloc.h>


enum primitiveType {INTEGER=1,REAL=2,BOOLEAN=3};

typedef struct primitiveTypeExpression{
    enum primitiveType type;
}primitiveTypeExpression;

typedef struct recArrTypeExpression{
    int dimensions;
    char *ranges;
}recArrTypeExpression;

typedef struct jaggedArrTypeExpression{
    int dimensions;
    char *ranges_R1;
    char *ranges_R2;
    // int **ranges_R3;
}jaggedArrTypeExpression;

typedef union typeExpressionUnion{
        primitiveTypeExpression p;
        recArrTypeExpression r;
        jaggedArrTypeExpression j;
}typeExpressionUnion;

typedef struct eachVariable{
    char var_name[21];
    int field2; //prim, recArr, or jaggedArr
    int isDynamic; // -1 means "not_applicable", 0 means static, 1 means dynamic
    typeExpressionUnion typeExpression;
}eachVariable;

typedef struct error{
    int lineNumber;
    char stmt_type; // 0 for decl, 1 for assign
    char operator;
    // lexemes obtained through eachVariable
    eachVariable type_first_operand;
    eachVariable type_second_operand;
    int depth;
    char message[31];
}error;


// typedef union{
//     //insert type fields here
//     char* type;
// }Type_exp;

// Parse tree stucture

/* ParseTree module */

typedef struct parseTree{
    char* nodename;
    int is_terminal;
    
    //Type_exp type;

    struct parseTree ** children;
    int child_count;
    char * lexeme;
    int gramRule;
    int line_num;
    int depth;
    struct eachVariable typeExpression;
    struct parseTree * copy;  // for making tree copy and link it to new stack
}parseTree;

// Parse tree functions

parseTree * makenode(char * str,int terminal, char * lex, int grule, int num, int d){
    parseTree * temp=(parseTree *)malloc(sizeof(parseTree));
    temp->nodename=str;
    temp->is_terminal=terminal;
    temp->children=(parseTree**)malloc(sizeof(parseTree*)*50); //no node will have more than 50 children
    temp->child_count=0;
    temp->copy=NULL;
    temp->lexeme=(char *)malloc(21*sizeof(char));
    strcpy(temp->lexeme,lex);
    temp->gramRule=grule;
    temp->line_num=num;
    temp->depth=d;
    //add type expression here;
}

parseTree * insert(parseTree* node,char* str,int terminal, char * lex, int grule, int num, int d){
    parseTree* temp=makenode(str,terminal,lex,grule,num,d);
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
    parseTree * res = makenode(t->nodename,t->is_terminal,t->lexeme,t->gramRule,t->line_num,t->depth);
    res->child_count=t->child_count;
    t->copy=res; //updated copy pointer
    for(int i=0;i<t->child_count;i++){
        res->children[i]=makeTreeCopy(t->children[i]);
        
    }
    return res;
}

void destroytreecopy(parseTree *root){
    for(int i=0;i<root->child_count;i++){
        destroytreecopy(root->children[i]);
    }
    // free(root->nodename);
    free(root->lexeme);
    free(root);
    return;
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

void destroystackcopy(stackNode ** s){
    while(!isempty(s)){
        stackNode * temp=top(s);
        pop(s);
        temp->next=NULL;

        // free(temp->data);
        free(temp);
    }
    return;
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

    while (p[i]){
        struct Node * n=p[i];
        printf("%s\n",n->piece);
        while (n->next){
            n=n->next;
            printf("%s\n",n->piece);
            

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
        struct Node * trav=(struct Node *)malloc(sizeof(struct Node));
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
    else if(lex[0]>=48 && lex[0]<=57){
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
        if (line[strlen(line)-1]=='\n'){
            line[strlen(line)-1]='\0';

        }

        char * tempStr;
        tempStr=strtok(line," ");


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
        
    }
    s=ret;

    
    while (s->next->line_num<count && s->next->line_num>0){

        s=s->next;
    }
    s->next=NULL;

    return ret;
}


void pushRule(struct Node * n, stackNode ** s, int grule){
    stackNode * non_term = top(s);
    pop(s);
    non_term->next=NULL;

    stackNode * rev=NULL;
    while (n->next){
        n=n->next;
        stackNode * temps;
        parseTree * tempt;
        
        if (!strcmp(n->piece,"epsilon")){
            tempt=insert(non_term->treeptr,"epsilon",1,"epsilon",grule,-1,-1);
            tempt->depth=non_term->treeptr->depth+1;

            break;
        }
        if ((n->piece)[0]=='<'){
            tempt=insert(non_term->treeptr,n->piece,0,"",grule,-1,-1);
            temps=makestackNode(n->piece,0,tempt);
        }
        else {
            tempt=insert(non_term->treeptr,n->piece,1,"",grule,-1,-1);
            temps=makestackNode(n->piece,1,tempt);
        }
        tempt->depth=non_term->treeptr->depth+1;
        push(&rev,temps);
        
        temps=top(&rev);
        

    }
    int count=0;
    while (!isempty(&rev)){
        
        stackNode * temp=top(&rev);
        stackNode * temp2=top(s);
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
        strcpy(temp->treeptr->lexeme,(*ts)->lexeme);
        temp->treeptr->line_num=(*ts)->line_num;
        // printf("Matched is %s\n",temp->data);
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
        printf("%s {%d %d} ",temp->treeptr->nodename,temp->treeptr->line_num,temp->treeptr->depth);
        
        for(int i=0;i<temp->treeptr->child_count;i++){
            stackNode * temp2=makestackNode(NULL,-1,temp->treeptr->children[i]);
            push(first,temp2);
        }
    }
    printf("\n");

    printLevelTree(first,second);
}

parseTree * genTree(parseTree* root,stackNode ** s, tokenStream ** ts, grammar * G){
    int check=terminalMatch(s,ts);

    
    if (!check){
        destroytreecopy(root);
        destroystackcopy(s);
        return NULL;
    }

    if (isempty(s)){
        return root;
    }

    for (int i=0;i<59;i++){
        struct Node * iterRule=(G->rules)[i];
        stackNode * iterStack=top(s);
        if (!strcmp(iterRule->piece,iterStack->data)){
            // printf("Rule number selected is %d\n",i+1);
            // printf("Token pos is %s\n",(*ts)->token);
            


            parseTree* root2 = makeTreeCopy(root);
            stackNode * s2=makecopy(s);
            tokenStream * ts2=(*ts); 

            // Tree code here

            pushRule(iterRule,&s2,i+1);
            // stackNode * aaaa =top(&s2);
            // printf("%s\n",aaaa->data);
            parseTree * check2=genTree(root2,&s2,&ts2,G);
            if (check2){
                return check2;
            }
        }
    }
    destroystackcopy(s);
    destroytreecopy(root);
    return NULL;
}

parseTree * createParseTree(parseTree * root,tokenStream * ts, grammar * G){
    stackNode * s=NULL;
    stackNode * t=NULL;
    t=makestackNode("<main_program>",0,root);
    push(&s,t);

    parseTree * value=genTree(root,&s,&ts,G);

    return value;

}
void printTypeExp(eachVariable *typeExpressionTable, int sizeTypeExpTable);
void pushTypeTable(eachVariable *typeExpressionTable, int *sizeTypeExpTable, eachVariable t){
    typeExpressionTable = (eachVariable*)realloc(typeExpressionTable, sizeof(eachVariable)*(*sizeTypeExpTable+1));
    typeExpressionTable[*sizeTypeExpTable] = t;
    (*sizeTypeExpTable)++;
    // printTypeExp(typeExpressionTable, *sizeTypeExpTable);
}

eachVariable singlePrim(parseTree *root, eachVariable *typeExpressionTable, int *sizeTypeExpTable) {
    eachVariable t;
    strcpy(t.var_name, root->children[1]->lexeme);
    t.field2 = 0;
    t.isDynamic = -1;
    switch (root->children[3]->nodename[0]) {
        case 'i':
            t.typeExpression.p.type = INTEGER;
            break;

        case 'b':
            t.typeExpression.p.type = BOOLEAN;
            break;

        case 'r':
            t.typeExpression.p.type = REAL;
            break;

        default:
            break;
    }
    root->children[1]->typeExpression = t;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    return t;
}

int dimrarr(parseTree *root, char *ranges, int *dim){
    char *lval = root->children[0]->children[1]->children[0]->lexeme;
    char *rval = root->children[0]->children[3]->children[0]->lexeme;
    int dyn=0;
    if(!(lval[0]>=48 && lval[0]<=57 && rval[0]>=48 && rval[0]<=57)){
        dyn=1;
    }
    // strcat(ranges, "[");
    strcat(ranges, lval);
    // strcat(ranges, ",");
    strcat(ranges, rval);
    // strcat(ranges, "]");
    (*dim)++;
    if(root->child_count==2){
        dyn+=dimrarr(root->children[1], ranges, dim);
    }
    if(dyn>0) return 1;
    else return 0;
}

eachVariable singleRarr(parseTree *root, eachVariable *typeExpressionTable, int *sizeTypeExpTable) {
    eachVariable t;
    strcpy(t.var_name, root->children[1]->lexeme);
    t.field2 = 1;
    t.typeExpression.r.ranges = "";
    char *ranges;
    ranges="";
    int *dim = (int*)malloc(sizeof(int));
    *dim=0;
    t.isDynamic = dimrarr(root->children[4], ranges, dim);
    strcat(t.typeExpression.r.ranges, ranges);
    t.typeExpression.r.dimensions = *dim;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    return t;
}

eachVariable singleDecl(parseTree *root, eachVariable *typeExpressionTable, int *sizeTypeExpTable) {
    if (root->children[0]->nodename[8] == 'p') {
        root->typeExpression = singlePrim(root->children[0], typeExpressionTable, sizeTypeExpTable);
    } else if (root->children[0]->nodename[8] == 'r') {
        root->typeExpression = singleRarr(root->children[0], typeExpressionTable, sizeTypeExpTable);
    }
    return root->typeExpression;
}

void varList(parseTree *root, eachVariable *typeExpressionTable, int *sizeTypeExpTable, eachVariable t){
    if(!strcpy(root->children[0]->nodename, "epsilon")){
        return;
    }
    strcpy(t.var_name, root->children[0]->lexeme);
    root->children[0]->typeExpression = t;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    varList(root->children[1], typeExpressionTable, sizeTypeExpTable, t);
}

eachVariable listPrim(parseTree *root, eachVariable *typeExpressionTable, int *sizeTypeExpTable) {
    eachVariable t;
    t.isDynamic = -1;
    t.field2 = 0;
    switch (root->children[8]->nodename[0]) {
        case 'i':
            t.typeExpression.p.type = INTEGER;
            break;

        case 'b':
            t.typeExpression.p.type = BOOLEAN;
            break;

        case 'r':
            t.typeExpression.p.type = REAL;
            break;

        default:
            break;
    }
    strcpy(t.var_name, root->children[4]->lexeme);
    root->children[4]->typeExpression = t;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    strcpy(t.var_name, root->children[5]->lexeme);
    root->children[5]->typeExpression = t;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    // printf("\nI WAS CALLEDHEHE%s\n", root->children[5]->lexeme);
    varList(root->children[6], typeExpressionTable, sizeTypeExpTable, t);
    return t;
}

eachVariable listRarr(parseTree *root, eachVariable *typeExpressionTable, int *sizeTypeExpTable) {
    eachVariable t;
    t.field2 = 1;
    char *ranges;
    ranges = "";
    int *dim = (int*)malloc(sizeof(int));
    *dim=0;
    t.isDynamic = dimrarr(root->children[9], ranges, dim);
    t.typeExpression.r.dimensions = *dim;
    strcat(t.typeExpression.r.ranges, ranges);
    strcpy(t.var_name, root->children[4]->lexeme);
    root->children[4]->typeExpression = t;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    strcpy(t.var_name, root->children[5]->lexeme);
    root->children[5]->typeExpression = t;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    varList(root->children[6], typeExpressionTable, sizeTypeExpTable, t);
    return t;
}

eachVariable declList(parseTree *root, eachVariable *typeExpressionTable, int *sizeTypeExpTable) {
    if (root->children[0]->nodename[6] == 'p') {
        root->typeExpression = listPrim(root->children[0], typeExpressionTable, sizeTypeExpTable);
    } else if (root->children[0]->nodename[6] == 'r') {
        root->typeExpression = listRarr(root->children[0], typeExpressionTable, sizeTypeExpTable);
    }
    return root->typeExpression;
}

eachVariable eachDecl(parseTree *root, eachVariable *typeExpressionTable, int *sizeTypeExpTable) {
    if (root->children[0]->nodename[1] == 's') {
        root->typeExpression = singleDecl(root->children[0], typeExpressionTable, sizeTypeExpTable);
    } else {
        root->typeExpression = declList(root->children[0], typeExpressionTable, sizeTypeExpTable);
    }
    return root->typeExpression;
}

void traverseDeclStmt(parseTree *root, eachVariable *typeExpressionTable, int *sizeTypeExpTable) {
    eachDecl(root->children[0], typeExpressionTable, sizeTypeExpTable);
    if (root->child_count == 2) {
        traverseDeclStmt(root->children[1], typeExpressionTable, sizeTypeExpTable);
    }
    return;
}

void traverseAssgmtStmt(parseTree *root, eachVariable* typeExpressionTable, int *sizeTypeExpTable){

}

void traverseParseTree(parseTree *root, eachVariable* typeExpressionTable, int *sizeTypeExpTable){
    traverseDeclStmt(root->children[3], typeExpressionTable, sizeTypeExpTable);
    traverseAssgmtStmt(root->children[4], typeExpressionTable, sizeTypeExpTable);
}

void printTypeExp(eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    for (int i = 0; i < sizeTypeExpTable; ++i) {
        eachVariable t = typeExpressionTable[i];
        printf("Variable name: %s,", t.var_name);
        printf("\t");
        switch (t.isDynamic) {
            case -1:
                printf("not applicable");
                break;

            case 0:
                printf("static");
                break;

            case 1:
                printf("dynamic");
                break;

            default:
                break;
        }
        printf("\ttype: ");
        // printf("jaggedArray");
        if (t.field2 == 0) {
            printf("primitive\t");
            printf("basicElementType: %d", t.typeExpression.p.type);
        } else if (t.field2 == 1) {
            printf("rectangularArray\t");
            printf("Dimensions: %d, Ranges: %s", t.typeExpression.r.dimensions, t.typeExpression.r.ranges);
        }
        printf("\n");
    }
}

int main(){
    
    
    parseTree * root=NULL;
    
    grammar * temp;
    temp=readGrammar(filename);

    tokenStream * stream;
    stream=tokeniseSourceCode(sourcename,stream);

    root=makenode("<main_program>",0,"",1,-1,0);

    parseTree * value=createParseTree(root,stream,temp);

    // printf("current tree is: \n");
    // printTree(value);
    printf("\nLevel tree is \n");
    stackNode * first=NULL;
    stackNode * second=NULL;
    stackNode * x=makestackNode(NULL,-1,value);
    push(&first,x);
    printLevelTree(&first,&second);
    printf("Tree end\n\n");

    if (value){
        printf("1\n");
    }
    else {
        printf("0\n");
    }
    
    // malloc_stats();

    eachVariable* typeExpressionTable = NULL;
    // eachVariable* typeExpressionTable = (eachVariable *)malloc(sizeof(eachVariable));
    int *sizeTypeExpTable=(int*)malloc(sizeof(int));
    *sizeTypeExpTable=0;
    traverseParseTree(value, typeExpressionTable, sizeTypeExpTable);
    // getToken("{");
    printTypeExp(typeExpressionTable, *sizeTypeExpTable);

    return 0;
}
