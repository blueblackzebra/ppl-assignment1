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



int staticListCount(parseTree* static_cnst_list,int *line_num)
{
    int line = static_cnst_list->children[0]->line_num;
    if(line!=-1) *line_num=line;

    if(!strcmp(static_cnst_list->children[0]->nodename,"epsilon"))
    {
        return 0;
    }
    return 1+staticListCount(static_cnst_list->children[1],line_num);
}

eachVariable chkJagged2d(parseTree *dim_jarr2d,parseTree *ranges_desc)
{
    eachVariable retVal;
    retVal.field2 = 2;
    retVal.isDynamic = -1;
    jaggedArrTypeExpression j;
    j.dimensions = 2;
    int dim1 = atoi(dim_jarr2d->children[1]->lexeme);
    int dim2 = atoi(dim_jarr2d->children[3]->lexeme);
    int numRows = dim2-dim1+1;

    //populating R1 in typeExpr
    j.ranges_R1 = (char*) malloc(sizeof(dim_jarr2d->children[1]->lexeme)+sizeof(dim_jarr2d->children[3]->lexeme)+sizeof(char));
    strcat(j.ranges_R1,dim_jarr2d->children[1]->lexeme);
    strcat(j.ranges_R1," ");
    strcat(j.ranges_R1,dim_jarr2d->children[3]->lexeme);

    int rowSize,rowsYet=0;
    int rowSizeYet;
    int errFlag=0;
    parseTree *tempRange = ranges_desc,*tempOne,*tempDimVal;
    int line_num,temp_line_num;
    int sizeR2=0;
    j.ranges_R2=NULL;
    while(!strcmp(tempRange->nodename,"<ranges_desc>")) //this check isn't really required, might as well be while(1)
    {
        tempOne = tempRange->children[0];
        rowSize = atoi(tempOne->children[6]->lexeme);
        tempDimVal = tempOne->children[10];
        line_num = tempOne->children[0]->line_num;

        if(!errFlag)
        {   
            j.ranges_R2 = (char *) realloc(j.ranges_R2,sizeof(tempOne->children[6]->lexeme)+sizeof(char)+sizeR2);
            sizeR2+=sizeof(tempOne->children[6]->lexeme)+sizeof(char);
            strcat(j.ranges_R2,tempOne->children[6]->lexeme);
            strcat(j.ranges_R2," ");
        }

        rowSizeYet=0;
        rowsYet++;

        while(strcmp(tempDimVal->children[0]->nodename,"epsilon"))
        {
            int tempCount = staticListCount(tempDimVal->children[0],&temp_line_num); //tempCount should ALWAYS be 1, if NOT, error
            if(tempCount!=1) //error
            {
                //printf("E1\n");
                if(!errFlag)
                {
                    free(j.ranges_R1);
                    free(j.ranges_R2);
                }
                errFlag=1;
                if(line_num<temp_line_num)
                    line_num = temp_line_num;
                retVal.field2 = -1;

                printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",line_num,"Declaration",tempDimVal->depth,"2D JA dimension mismatch");

                // if(tempCount)
                //     printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",tempDimVal->line_num,"Declaration",tempDimVal->children[2]->children[0]->children[0]->depth,"2D JA dimension mismatch");
                // else
                //     printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",tempDimVal->line_num,"Declaration",tempDimVal->children[0]->depth,"2D JA dimension mismatch");
            }

            tempDimVal = tempDimVal->children[2];
            rowSizeYet++;

            // if(rowSizeYet>=rowSize) //error
            // {
            //     //printf("E2\n");
            //     if(!errFlag)
            //     {
            //         free(j.ranges_R1);
            //         free(j.ranges_R2);
            //     }
            //     errFlag=1;
            //     if(line_num<temp_line_num)
            //         line_num = temp_line_num;
            //     retVal.field2 = -1;

            //     printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",line_num,"Declaration",tempDimVal->depth,"2D JA dimension mismatch");

            //     // printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",tempDimVal->line_num,"Declaration",tempDimVal->children[0]->children[0]->depth,"2D JA dimension mismatch");
            // }
        } //<dim_values> for ONE row traversed

        if(staticListCount(tempOne->children[11],&temp_line_num)!=1) //error
        {
            //printf("E3\n");
            if(!errFlag)
            {
                free(j.ranges_R1);
                free(j.ranges_R2);
            }
            errFlag=1;
            if(line_num<temp_line_num)
                    line_num = temp_line_num;
            retVal.field2 = -1;

            printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",line_num,"Declaration",tempOne->depth,"2D JA dimension mismatch");

            // printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",tempOne->line_num,"Declaration",tempOne->children[11]->children[0]->depth,"2D JA dimension mismatch");
        }
        
        rowSizeYet++;

        if(rowSizeYet!=rowSize) //error
        {
            //printf("E4\n");
            if(!errFlag)
            {
                free(j.ranges_R1);
                free(j.ranges_R2);
            }
            errFlag=1;
            if(line_num<temp_line_num)
                line_num = temp_line_num;
            retVal.field2 = -1;
            printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",line_num,"Declaration",tempOne->depth,"2D JA dimension mismatch");
        }

        if(tempRange->child_count>1&&rowsYet<=numRows)
            tempRange = tempRange->children[1];
        else
            break;
    }

    if(rowsYet!=numRows) //error
    {
        //printf("E5\n");
        if(!errFlag)
        {
            free(j.ranges_R1);
            free(j.ranges_R2);
        }
        errFlag=1;
        if(line_num<temp_line_num)
            line_num = temp_line_num;
        retVal.field2 = -1;

        printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",line_num,"Declaration",tempRange->depth,"2D JA size mismatch");
    }

    if(errFlag) return retVal;
    //success
    retVal.typeExpression.j = j;
    return retVal;
}

eachVariable chkJagged3d(parseTree *dim_jarr3d,parseTree *ranges_desc)
{
    eachVariable retVal;
    retVal.field2 = 2;
    retVal.isDynamic = -1;
    jaggedArrTypeExpression j;
    j.dimensions = 3;
    int dim1 = atoi(dim_jarr3d->children[1]->lexeme);
    int dim2 = atoi(dim_jarr3d->children[3]->lexeme);
    int numRows = dim2-dim1+1;

    //populating R1 in typeExpr
    j.ranges_R1 = (char*) malloc(sizeof(dim_jarr3d->children[1]->lexeme)+sizeof(dim_jarr3d->children[3]->lexeme)+sizeof(char));
    strcat(j.ranges_R1,dim_jarr3d->children[1]->lexeme);
    strcat(j.ranges_R1," ");
    strcat(j.ranges_R1,dim_jarr3d->children[3]->lexeme);

    int rowSize,rowsYet=0,tempCount;
    int rowSizeYet;
    int errFlag=0;
    int line_num,temp_line_num;
    int sizeR2=0;
    j.ranges_R2=NULL;
    parseTree *tempRange = ranges_desc,*tempOne,*tempDimVal;
    while(!strcmp(tempRange->nodename,"<ranges_desc>")) //this check isn't really required, might as well be while(1)
    {
        tempOne = tempRange->children[0];
        rowSize = atoi(tempOne->children[6]->lexeme);
        tempDimVal = tempOne->children[10];
        line_num = tempOne->children[0]->line_num;

        if(!errFlag)
        {
            j.ranges_R2 = (char*) realloc(j.ranges_R2,sizeof(tempOne->children[6]->lexeme)+3*sizeof(char)+sizeR2);
            sizeR2+=sizeof(tempOne->children[6]->lexeme)+3*sizeof(char);
            strcat(j.ranges_R2,tempOne->children[6]->lexeme);
            strcat(j.ranges_R2," [ ");
        }

        rowSizeYet=0;
        rowsYet++;

        while(strcmp(tempDimVal->children[0]->nodename,"epsilon"))
        {
            tempCount = staticListCount(tempDimVal->children[0],&temp_line_num); //tempCount should NOT be zero
            if(!tempCount) //error
            {
                if(!errFlag)
                {
                    free(j.ranges_R1);
                    free(j.ranges_R2);
                }
                errFlag=1;
                if(line_num<temp_line_num)
                    line_num = temp_line_num;
                retVal.field2 = -1;
                
                printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",line_num,"Declaration",tempDimVal->depth,"3D JA dimension mismatch");
            }

            if(!errFlag)
            {
                int length = snprintf( NULL, 0, "%d", tempCount);
                char* string_TC = malloc( length + 1 );
                snprintf(string_TC, length + 1, "%d", tempCount);
                j.ranges_R2 = (char*) realloc(j.ranges_R2,sizeof(string_TC)+sizeof(char)+sizeR2);
                sizeR2+=sizeof(string_TC)+sizeof(char);
                strcat(j.ranges_R2,string_TC);
                strcat(j.ranges_R2," ");    
            }
            

            tempDimVal = tempDimVal->children[2];
            rowSizeYet++;

            // if(rowSizeYet>=rowSize) //error
            // {
            //     if(!errFlag)
            //     {
            //         free(j.ranges_R1);
            //         free(j.ranges_R2);
            //     }
            //     errFlag=1;
            //     if(line_num<temp_line_num)
            //         line_num = temp_line_num;
            //     retVal.field2 = -1;

            //     printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",line_num,"Declaration",tempDimVal->depth,"3D JA dimension mismatch");

            //     // printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",tempDimVal->line_num,"Declaration",tempDimVal->children[0]->children[0]->depth,"2D JA dimension mismatch");
            // }
        }//dim values traversed for one row

        if(!(tempCount=staticListCount(tempOne->children[11],&temp_line_num))) //error
        {
            if(!errFlag)
            {
                free(j.ranges_R1);
                free(j.ranges_R2);
            }
            errFlag=1;
            if(line_num<temp_line_num)
                line_num = temp_line_num;
            retVal.field2 = -1;

            printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",line_num,"Declaration",tempOne->depth,"3D JA dimension mismatch");

            // printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",tempOne->line_num,"Declaration",tempOne->children[11]->children[0]->depth,"2D JA dimension mismatch");
        }
        
        if(!errFlag)
        {
            int length = snprintf( NULL, 0, "%d", tempCount);
            char* string_TC = malloc( length + 1 );
            snprintf(string_TC, length + 1, "%d", tempCount);
            j.ranges_R2 = (char*) realloc(j.ranges_R2,sizeof(string_TC)+3*sizeof(char)+sizeR2);
            sizeR2+=sizeof(string_TC)+3*sizeof(char);
            strcat(j.ranges_R2,string_TC);
            strcat(j.ranges_R2," ] "); 
        }
    
        rowSizeYet++;

        if(rowSizeYet!=rowSize) //error
        {
            if(!errFlag)
            {
                free(j.ranges_R1);
                free(j.ranges_R2);
            }
            errFlag=1;
            if(line_num<temp_line_num)
                line_num = temp_line_num;
            retVal.field2 = -1;
    
            printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",line_num,"Declaration",tempOne->depth,"3D JA dimension mismatch");
        }

        if(tempRange->child_count>1&&rowsYet<=numRows)
            tempRange = tempRange->children[1];
        else
            break;
    }//one more row traversed

    if(rowsYet!=numRows) //error
    {
        if(!errFlag)
        {
            free(j.ranges_R1);
            free(j.ranges_R2);
        }
        errFlag=1;
        if(line_num<temp_line_num)
            line_num = temp_line_num;
        retVal.field2 = -1;
    
        printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",line_num,"Declaration",tempRange->depth,"3D JA size mismatch");
    }

    if(retVal.field2==-1)
        return retVal;
    //success
    retVal.typeExpression.j = j;
    return retVal;
}


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

char sourcename[]="testingJagged.txt";

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


void traverseDeclStmt(parseTree *root, struct eachVariable* typeExpressionTable, int *sizeTypeExpTable){
    //struct parseTree* single_decl = root->children[0]->children[0];
    struct parseTree* decl = root->children[0];
    // if(!strcmp(single_decl->nodename, "<single_decl>")){
    //     struct parseTree* single_line = single_decl->children[0];
    //     if(!strcmp(single_line->nodename, "<single_prim>")){
    //         typeExpressionTable = realloc(typeExpressionTable, sizeof(typeExpressionTable)*(*sizeTypeExpTable+1));
    //         strcpy(typeExpressionTable[*sizeTypeExpTable].var_name, single_line->children[0]->children[2]->lexeme);
    //         typeExpressionTable[*sizeTypeExpTable].field2 = 0;
    //         typeExpressionTable[*sizeTypeExpTable].isDynamic = -1;
    //         struct primitiveTypeExpression p;
    //         char *temp = single_line->children[0]->children[3]->lexeme;
    //         if(!strcmp(temp, "integer")){
    //             p.type = INTEGER;
    //         }   else if(!strcmp(temp, "real")){
    //             p.type = REAL;
    //         }   else{
    //             p.type = BOOLEAN;
    //         }
    //         typeExpressionTable[*sizeTypeExpTable].typeExpression.p = p;

    //         *sizeTypeExpTable++;
    //     }   else if(!strcmp(single_line->nodename, "<single_rarr>")){

    //     }   else if(!strcmp(single_line->nodename, "<single_jarr2d>")){

    //     }   else{ // <single_jarr3d>

    //     }
    // }
    eachVariable t;
    if(!strcmp(decl->children[0]->nodename,"<single_jarr2d>"))
    {
        t=chkJagged2d(decl->children[0]->children[5],decl->children[0]->children[9]);
        if(t.field2!=-1)
        printf("Checked single2d Jarr : \nfield2 : %d , isDyn : %d , dim = %d , R1 = %s , R2 = %s\n",t.field2,t.isDynamic,t.typeExpression.j.dimensions,t.typeExpression.j.ranges_R1,t.typeExpression.j.ranges_R2);
    }
    else if(!strcmp(decl->children[0]->nodename,"<list_jarr2d>"))
    {
        t=chkJagged2d(decl->children[0]->children[10],decl->children[0]->children[14]);
        if(t.field2!=-1)
        printf("Checked list2d Jarr : \nfield2 : %d , isDyn : %d , dim = %d , R1 = %s , R2 = %s\n",t.field2,t.isDynamic,t.typeExpression.j.dimensions,t.typeExpression.j.ranges_R1,t.typeExpression.j.ranges_R2);
    }
    else if(!strcmp(decl->children[0]->nodename,"<single_jarr3d>"))
    {
        t=chkJagged3d(decl->children[0]->children[5],decl->children[0]->children[9]);
        if(t.field2!=-1)
        printf("Checked single3d Jarr : \nfield2 : %d , isDyn : %d , dim = %d , R1 = %s , R2 = %s\n",t.field2,t.isDynamic,t.typeExpression.j.dimensions,t.typeExpression.j.ranges_R1,t.typeExpression.j.ranges_R2);
    }
    else if(!strcmp(decl->children[0]->nodename,"<list_jarr3d>"))
    {
        t=chkJagged3d(decl->children[0]->children[10],decl->children[0]->children[14]);
        if(t.field2!=-1)
        printf("Checked list3d Jarr : \nfield2 : %d , isDyn : %d , dim = %d , R1 = %s , R2 = %s\n",t.field2,t.isDynamic,t.typeExpression.j.dimensions,t.typeExpression.j.ranges_R1,t.typeExpression.j.ranges_R2);
    }
    // if(root->child_count==2){
    //     traverseDeclStmt(root->children[1], typeExpressionTable, sizeTypeExpTable);
    // }
}

void traverseAssgmtStmt(parseTree *root, struct eachVariable* typeExpressionTable, int *sizeTypeExpTable){

}

void traverseParseTree(parseTree *root, struct eachVariable* typeExpressionTable, int *sizeTypeExpTable){
    // if(strcmp(root->nodename, "<main_program>")){
    // for(int i=0; i<root->child_count; ++i){
        // if(strcmp(root->children[i]->nodename), "<decl_stmts>"){
    parseTree *temp = root->children[3];
    while(temp->child_count>1){
        traverseDeclStmt(temp->children[0], typeExpressionTable, sizeTypeExpTable);
        temp = temp->children[1];    
    }
    
        // }
    // }
    // for(int i=0; i<root->child_count; ++i){
        // if(strcmp(root->children[i]->nodename), "<assgmt_stmts>"){
    //traverseAssgmtStmt(root->children[4], typeExpressionTable, sizeTypeExpTable);
        // }
    // }
    // }
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

    struct eachVariable* typeExpressionTable;
    int *sizeTypeExpTable=(int*)malloc(sizeof(int));
    *sizeTypeExpTable=0;
    traverseParseTree(value, typeExpressionTable, sizeTypeExpTable);
    // getToken("{");


    return 0;
}
