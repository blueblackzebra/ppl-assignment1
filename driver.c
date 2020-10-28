/*
    GROUP No. 52

    HARSHIT GARG            2018A7PS0218P
    SPARSH KASANA           2018A7PS0247P
    ARPIT JAIN              2018A7PS0267P
    SAMINA SHIRAJ MULANI    2018A7PS0314P

*/

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char filename[] = "grammar.txt";

char sourcename[] = "testcases/t6.txt";

enum primitiveType { INTEGER = 1,
                     REAL = 2,
                     BOOLEAN = 3 };

typedef struct primitiveTypeExpression {
    enum primitiveType type;
} primitiveTypeExpression;

typedef struct recArrTypeExpression {
    int dimensions;
    char *ranges;
} recArrTypeExpression;

typedef struct jaggedArrTypeExpression {
    int dimensions;
    char *ranges_R1;
    char *ranges_R2;
    // int **ranges_R3;
} jaggedArrTypeExpression;

typedef union typeExpressionUnion {
    primitiveTypeExpression p;
    recArrTypeExpression r;
    jaggedArrTypeExpression j;
} typeExpressionUnion;

typedef struct eachVariable {
    char var_name[21];
    int field2;     //prim, recArr, or jaggedArr
    int isDynamic;  // -1 means "not_applicable", 0 means static, 1 means dynamic
    typeExpressionUnion typeExpression;
} eachVariable;

typedef struct error {
    int lineNumber;
    char stmt_type;  // 0 for decl, 1 for assign
    char operator;
    // lexemes obtained through eachVariable
    eachVariable type_first_operand;
    eachVariable type_second_operand;
    int depth;
    char message[31];
} error;


// Parse tree stucture

/* ParseTree module */

typedef struct parseTree {
    char *nodename;
    int is_terminal;

    //Type_exp type;

    struct parseTree *children[18];
    int child_count;
    char *lexeme;
    int gramRule;
    int line_num;
    int depth;
    struct eachVariable typeExpression;
    struct parseTree *copy;  // for making tree copy and link it to new stack
} parseTree;

// Parse tree functions

parseTree *makenode(char *str, int terminal, char *lex, int grule, int num, int d) {
    parseTree *temp = (parseTree *)malloc(sizeof(parseTree));
    temp->nodename = str;
    temp->is_terminal = terminal;
    // temp->children=(parseTree**)malloc(sizeof(parseTree*)*18); //no node will have more than 50 children
    temp->child_count = 0;
    temp->copy = NULL;
    temp->lexeme = (char *)malloc(21 * sizeof(char));
    strcpy(temp->lexeme, lex);
    temp->gramRule = grule;
    temp->line_num = num;
    temp->depth = d;
    (temp->typeExpression).field2=-2;
    //add type expression here;
}

parseTree *insert(parseTree *node, char *str, int terminal, char *lex, int grule, int num, int d) {
    parseTree *temp = makenode(str, terminal, lex, grule, num, d);
    node->children[node->child_count] = temp;
    node->child_count++;
    return temp;
}

parseTree *makeTreeCopy(parseTree *t) {
    if (t == NULL) {
        return NULL;
    }
    parseTree *res = makenode(t->nodename, t->is_terminal, t->lexeme, t->gramRule, t->line_num, t->depth);
    res->child_count = t->child_count;
    t->copy = res;  //updated copy pointer
    for (int i = 0; i < t->child_count; i++) {
        res->children[i] = makeTreeCopy(t->children[i]);
    }
    return res;
}

void destroytreecopy(parseTree *root) {
    for (int i = 0; i < root->child_count; i++) {
        destroytreecopy(root->children[i]);
    }
    // free(root->nodename);
    free(root->lexeme);
    free(root);
    return;
}

/* Stack module */

typedef struct stackNode {
    char *data;
    parseTree *treeptr;
    int isterminal;
    struct stackNode *next;
} stackNode;

void push(stackNode **s, stackNode *n) {
    if (*s == NULL) {
        *s = n;
    } else {
        n->next = *s;
        *s = n;
    }
    return;
}

void pop(stackNode **s) {
    if (*s == NULL) {
        return;
    } else {
        *s = (*s)->next;
    }
    return;
}

stackNode *top(stackNode **s) {
    if (*s == NULL) {
        return NULL;
    } else {
        return (*s);
    }
}

int isempty(stackNode **s) {
    if (*s == NULL) {
        return 1;
    }
    return 0;
}

stackNode *makestackNode(char *str, int terminal, parseTree *ptr) {
    stackNode *temp = (stackNode *)malloc(sizeof(stackNode));
    temp->data = str;
    temp->isterminal = terminal;
    temp->next = NULL;
    temp->treeptr = ptr;
    return temp;
}

stackNode *makecopy2(stackNode **s) {
    stackNode *temp = NULL;
    stackNode *res = NULL;
    // printf("2\n");
    temp = *s;
    // printf("2\n");
    while (temp != NULL) {
        stackNode *n = makestackNode(temp->data, temp->isterminal, temp->treeptr);

        push(&res, n);

        temp = temp->next;
    }

    //stackNode* res2=makecopy2(&res);

    return res;
}

void destroystackcopy(stackNode **s) {
    while (!isempty(s)) {
        stackNode *temp = top(s);
        pop(s);
        temp->next = NULL;

        // free(temp->data);
        free(temp);
    }
    return;
}

stackNode *makecopy(stackNode **s) {
    stackNode *temp = NULL;
    stackNode *res = NULL;
    // printf("2\n");
    temp = *s;
    // printf("2\n");
    while (temp != NULL) {
        stackNode *n = makestackNode(temp->data, temp->isterminal, temp->treeptr->copy);

        temp->treeptr->copy = NULL;

        push(&res, n);

        temp = temp->next;
    }

    stackNode *res2 = makecopy2(&res);
    destroystackcopy(&res);
    return res2;
}

/* Grammar module */

struct Node {
    char *piece;
    struct Node *next;
};

typedef struct {
    struct Node **rules;
} grammar;

/* Token Stream module */

struct Symbol {
    int line_num;
    char *lexeme;
    char *token;
    struct Symbol *next;
};

typedef struct Symbol tokenStream;

char *getToken(char *lex) {
    char *tok = (char *)malloc(12 * sizeof(char));

    if (!strcmp(lex, "{")) {
        tok = "COB";
    } else if (!strcmp(lex, "}")) {
        tok = "CCB";
    } else if (!strcmp(lex, "[")) {
        tok = "SOB";
    } else if (!strcmp(lex, "]")) {
        tok = "SCB";
    } else if (!strcmp(lex, "(")) {
        tok = "LRB";
    } else if (!strcmp(lex, ")")) {
        tok = "RRB";
    } else if (!strcmp(lex, "..")) {
        tok = "TO";
    } else if (!strcmp(lex, ";")) {
        tok = "SC";
    } else if (!strcmp(lex, ":")) {
        tok = "COL";
    } else if (!strcmp(lex, "=")) {
        tok = "ASSGN_OP";
    } else if (!strcmp(lex, "&&&")) {
        tok = "BOOL_AND";
    } else if (!strcmp(lex, "|||")) {
        tok = "BOOL_OR";
    } else if (!strcmp(lex, "*") || !strcmp(lex, "/")) {
        tok = "AR_OP1";
    } else if (!strcmp(lex, "+") || !strcmp(lex, "-")) {
        tok = "AR_OP2";
    } else if (!strcmp(lex, "program")) {
        tok = "program";
    } else if (!strcmp(lex, "declare")) {
        tok = "declare";
    } else if (!strcmp(lex, "list")) {
        tok = "list";
    } else if (!strcmp(lex, "of")) {
        tok = "of";
    } else if (!strcmp(lex, "variables")) {
        tok = "variables";
    } else if (!strcmp(lex, "array")) {
        tok = "array";
    } else if (!strcmp(lex, "size")) {
        tok = "size";
    } else if (!strcmp(lex, "values")) {
        tok = "values";
    } else if (!strcmp(lex, "jagged")) {
        tok = "jagged";
    } else if (!strcmp(lex, "of")) {
        tok = "of";
    } else if (!strcmp(lex, "integer")) {
        tok = "integer";
    } else if (!strcmp(lex, "boolean")) {
        tok = "boolean";
    } else if (!strcmp(lex, "real")) {
        tok = "real";
    } else if (!strcmp(lex, "R1")) {
        tok = "R1";
    } else if (lex[0] >= 48 && lex[0] <= 57) {
        tok = "STATIC_CNST";
    } else {
        tok = "VAR_NAME";
    }

    return tok;
}

void pushRule(struct Node *n, stackNode **s, int grule) {
    stackNode *non_term = top(s);
    pop(s);
    non_term->next = NULL;

    stackNode *rev = NULL;
    while (n->next) {
        n = n->next;
        stackNode *temps;
        parseTree *tempt;

        if (!strcmp(n->piece, "epsilon")) {
            tempt = insert(non_term->treeptr, "epsilon", 1, "epsilon", grule, -1, -1);
            tempt->depth = non_term->treeptr->depth + 1;

            break;
        }
        if ((n->piece)[0] == '<') {
            tempt = insert(non_term->treeptr, n->piece, 0, "", grule, -1, -1);
            temps = makestackNode(n->piece, 0, tempt);
        } else {
            tempt = insert(non_term->treeptr, n->piece, 1, "", grule, -1, -1);
            temps = makestackNode(n->piece, 1, tempt);
        }
        tempt->depth = non_term->treeptr->depth + 1;
        push(&rev, temps);

        temps = top(&rev);
    }
    int count = 0;
    while (!isempty(&rev)) {
        stackNode *temp = top(&rev);
        stackNode *temp2 = top(s);
        pop(&rev);
        temp->next = NULL;
        push(s, temp);
    }
    free(non_term);
}

int terminalMatch(stackNode **s, tokenStream **ts) {
    stackNode *temp = top(s);

    while (temp != NULL && temp->isterminal) {
        if (strcmp(temp->data, (*ts)->token)) {
            return 0;
        }
        strcpy(temp->treeptr->lexeme, (*ts)->lexeme);
        temp->treeptr->line_num = (*ts)->line_num;
        // printf("Matched is %s\n",temp->data);
        pop(s);
        free(temp);
        temp = top(s);
        (*ts) = (*ts)->next;
    }

    return 1;
}

parseTree *genTree(parseTree *root, stackNode **s, tokenStream **ts, grammar *G) {
    int check = terminalMatch(s, ts);

    if (!check) {
        destroytreecopy(root);
        destroystackcopy(s);
        return NULL;
    }

    if (isempty(s)) {
        return root;
    }

    for (int i = 0; i < 59; i++) {
        struct Node *iterRule = (G->rules)[i];
        stackNode *iterStack = top(s);
        if (!strcmp(iterRule->piece, iterStack->data)) {
            // printf("Rule number selected is %d\n",i+1);
            // printf("Token pos is %s\n",(*ts)->token);

            parseTree *root2 = makeTreeCopy(root);
            stackNode *s2 = makecopy(s);
            tokenStream *ts2 = (*ts);

            // Tree code here

            pushRule(iterRule, &s2, i + 1);
            // stackNode * aaaa =top(&s2);
            // printf("%s\n",aaaa->data);
            parseTree *check2 = genTree(root2, &s2, &ts2, G);
            if (check2) {
                destroytreecopy(root);
                destroystackcopy(s);
                return check2;
            }
        }
    }
    destroystackcopy(s);
    destroytreecopy(root);
    return NULL;
}

void printLevelTree(stackNode **first, stackNode **second) {
    while (!isempty(first)) {
        stackNode *temp3 = top(first);
        pop(first);
        temp3->next = NULL;
        push(second, temp3);
    }

    if (isempty(second)) {
        return;
    }

    while (!isempty(second)) {
        // Read node
        // Push to first
        stackNode *temp = top(second);
        pop(second);
        temp->next = NULL;
        printf("%s {%d %d} ", temp->treeptr->nodename, temp->treeptr->line_num, temp->treeptr->depth);

        for (int i = 0; i < temp->treeptr->child_count; i++) {
            stackNode *temp2 = makestackNode(NULL, -1, temp->treeptr->children[i]);
            push(first, temp2);
        }
    }
    printf("\n");

    printLevelTree(first, second);
}

// Building type expression table

void pushTypeTable(eachVariable **typeExpressionTable, int *sizeTypeExpTable, eachVariable t) {
    *typeExpressionTable = (eachVariable *)realloc(*typeExpressionTable, sizeof(eachVariable) * (*sizeTypeExpTable + 1));
    (*typeExpressionTable)[*sizeTypeExpTable] = t;
    (*sizeTypeExpTable)++;
    return;
}

int staticListCount(parseTree *static_cnst_list, int *line_num) {
    int line = static_cnst_list->children[0]->line_num;
    if (line != -1) *line_num = line;

    if (!strcmp(static_cnst_list->children[0]->nodename, "epsilon")) {
        return 0;
    }
    return 1 + staticListCount(static_cnst_list->children[1], line_num);
}

eachVariable chkJagged2d(parseTree *dim_jarr2d, parseTree *ranges_desc) {
    eachVariable retVal;
    retVal.field2 = 2;
    retVal.isDynamic = -1;
    jaggedArrTypeExpression j;
    j.dimensions = 2;
    int dim1 = atoi(dim_jarr2d->children[1]->lexeme);
    int dim2 = atoi(dim_jarr2d->children[3]->lexeme);
    int numRows = dim2 - dim1 + 1;

    //populating R1 in typeExpr
    j.ranges_R1 = (char *)malloc(strlen(dim_jarr2d->children[1]->lexeme) + strlen(dim_jarr2d->children[3]->lexeme) + sizeof(char));
    j.ranges_R1[0] = '\0';
    strcat(j.ranges_R1, dim_jarr2d->children[1]->lexeme);
    strcat(j.ranges_R1, " ");
    strcat(j.ranges_R1, dim_jarr2d->children[3]->lexeme);

    int rowSize, rowsYet = 0;
    int rowSizeYet;
    int errFlag = 0;
    parseTree *tempRange = ranges_desc, *tempOne, *tempDimVal;
    int line_num, temp_line_num;
    int sizeR2 = 0;
    j.ranges_R2 = NULL;
    int firstIter = 0;
    while (!strcmp(tempRange->nodename, "<ranges_desc>"))  //this check isn't really required, might as well be while(1)
    {
        tempOne = tempRange->children[0];
        rowSize = atoi(tempOne->children[6]->lexeme);
        tempDimVal = tempOne->children[10];
        line_num = tempOne->children[0]->line_num;

        if (!errFlag) {
            j.ranges_R2 = (char *)realloc(j.ranges_R2, strlen(tempOne->children[6]->lexeme) + sizeof(char) + sizeR2);
            if (!firstIter) j.ranges_R2[0] = '\0';
            sizeR2 += strlen(tempOne->children[6]->lexeme) + sizeof(char);
            strcat(j.ranges_R2, tempOne->children[6]->lexeme);
            strcat(j.ranges_R2, " ");
            firstIter = 1;
        }

        rowSizeYet = 0;
        rowsYet++;

        while (strcmp(tempDimVal->children[0]->nodename, "epsilon")) {
            int tempCount = staticListCount(tempDimVal->children[0], &temp_line_num);  //tempCount should ALWAYS be 1, if NOT, error
            if (tempCount != 1)                                                        //error
            {
                //printf("E1\n");
                if (!errFlag) {
                    free(j.ranges_R1);
                    free(j.ranges_R2);
                }
                errFlag = 1;
                if (line_num < temp_line_num)
                    line_num = temp_line_num;
                retVal.field2 = -1;
                printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                       line_num,
                       "Declaration",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       tempDimVal->depth,
                       "2D JA dimension mismatch");

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
        }  //<dim_values> for ONE row traversed

        if (staticListCount(tempOne->children[11], &temp_line_num) != 1)  //error
        {
            //printf("E3\n");
            if (!errFlag) {
                free(j.ranges_R1);
                free(j.ranges_R2);
            }
            errFlag = 1;
            if (line_num < temp_line_num)
                line_num = temp_line_num;
            retVal.field2 = -1;

            printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                   line_num,
                   "Declaration",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   tempOne->depth,
                   "2D JA dimension mismatch");

            // printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",tempOne->line_num,"Declaration",tempOne->children[11]->children[0]->depth,"2D JA dimension mismatch");
        }

        rowSizeYet++;

        if (rowSizeYet != rowSize)  //error
        {
            //printf("E4\n");
            if (!errFlag) {
                free(j.ranges_R1);
                free(j.ranges_R2);
            }
            errFlag = 1;
            if (line_num < temp_line_num)
                line_num = temp_line_num;
            retVal.field2 = -1;
            printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                   line_num,
                   "Declaration",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   tempOne->depth,
                   "2D JA dimension mismatch");
        }

        if (tempRange->child_count > 1 && rowsYet <= numRows)
            tempRange = tempRange->children[1];
        else
            break;
    }

    if (rowsYet != numRows)  //error
    {
        //printf("E5\n");
        if (!errFlag) {
            free(j.ranges_R1);
            free(j.ranges_R2);
        }
        errFlag = 1;
        if (line_num < temp_line_num)
            line_num = temp_line_num;
        retVal.field2 = -1;
        printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
               line_num,
               "Declaration",
               "***",
               "***",
               "***",
               "***",
               "***",
               "***",
               "***",
               tempRange->depth,
               "2D JA size mismatch");
    }

    if (errFlag) return retVal;
    //success
    retVal.typeExpression.j = j;
    return retVal;
}

eachVariable chkJagged3d(parseTree *dim_jarr3d, parseTree *ranges_desc) {
    eachVariable retVal;
    retVal.field2 = 2;
    retVal.isDynamic = -1;
    jaggedArrTypeExpression j;
    j.dimensions = 3;
    int dim1 = atoi(dim_jarr3d->children[1]->lexeme);
    int dim2 = atoi(dim_jarr3d->children[3]->lexeme);
    int numRows = dim2 - dim1 + 1;

    //populating R1 in typeExpr
    j.ranges_R1 = (char *)malloc(strlen(dim_jarr3d->children[1]->lexeme) + strlen(dim_jarr3d->children[3]->lexeme) + sizeof(char));
    j.ranges_R1[0] = '\0';
    strcat(j.ranges_R1, dim_jarr3d->children[1]->lexeme);
    strcat(j.ranges_R1, " ");
    strcat(j.ranges_R1, dim_jarr3d->children[3]->lexeme);

    int rowSize, rowsYet = 0, tempCount;
    int rowSizeYet;
    int errFlag = 0;
    int line_num, temp_line_num;
    int sizeR2 = 0;
    j.ranges_R2 = NULL;
    int firstIter = 0;
    parseTree *tempRange = ranges_desc, *tempOne, *tempDimVal;
    while (!strcmp(tempRange->nodename, "<ranges_desc>"))  //this check isn't really required, might as well be while(1)
    {
        tempOne = tempRange->children[0];
        rowSize = atoi(tempOne->children[6]->lexeme);
        tempDimVal = tempOne->children[10];
        line_num = tempOne->children[0]->line_num;

        if (!errFlag) {
            j.ranges_R2 = (char *)realloc(j.ranges_R2, strlen(tempOne->children[6]->lexeme) + 3 * sizeof(char) + sizeR2);
            if (!firstIter) j.ranges_R2[0] = '\0';
            sizeR2 += strlen(tempOne->children[6]->lexeme) + 3 * sizeof(char);
            strcat(j.ranges_R2, tempOne->children[6]->lexeme);
            strcat(j.ranges_R2, " [ ");
            firstIter = 1;
        }

        rowSizeYet = 0;
        rowsYet++;

        while (strcmp(tempDimVal->children[0]->nodename, "epsilon")) {
            tempCount = staticListCount(tempDimVal->children[0], &temp_line_num);  //tempCount should NOT be zero
            if (!tempCount)                                                        //error
            {
                if (!errFlag) {
                    free(j.ranges_R1);
                    free(j.ranges_R2);
                }
                errFlag = 1;
                if (line_num < temp_line_num)
                    line_num = temp_line_num;
                retVal.field2 = -1;
                printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                       line_num,
                       "Declaration",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       tempDimVal->depth,
                       "3D JA dimension mismatch");
            }

            if (!errFlag) {
                int length = snprintf(NULL, 0, "%d", tempCount);
                char *string_TC = malloc(length + 1);
                snprintf(string_TC, length + 1, "%d", tempCount);
                j.ranges_R2 = (char *)realloc(j.ranges_R2, sizeof(string_TC) + sizeof(char) + sizeR2);
                sizeR2 += sizeof(string_TC) + sizeof(char);
                strcat(j.ranges_R2, string_TC);
                strcat(j.ranges_R2, " ");
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
        }  //dim values traversed for one row

        if (!(tempCount = staticListCount(tempOne->children[11], &temp_line_num)))  //error
        {
            if (!errFlag) {
                free(j.ranges_R1);
                free(j.ranges_R2);
            }
            errFlag = 1;
            if (line_num < temp_line_num)
                line_num = temp_line_num;
            retVal.field2 = -1;
            printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                   line_num,
                   "Declaration",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   tempOne->depth,
                   "3D JA dimension mismatch");

            // printf("ERROR : %4d %12s *** *** *** *** *** %4d %30s\n",tempOne->line_num,"Declaration",tempOne->children[11]->children[0]->depth,"2D JA dimension mismatch");
        }

        if (!errFlag) {
            int length = snprintf(NULL, 0, "%d", tempCount);
            char *string_TC = malloc(length + 1);
            snprintf(string_TC, length + 1, "%d", tempCount);
            j.ranges_R2 = (char *)realloc(j.ranges_R2, sizeof(string_TC) + 3 * sizeof(char) + sizeR2);
            sizeR2 += sizeof(string_TC) + 3 * sizeof(char);
            strcat(j.ranges_R2, string_TC);
            strcat(j.ranges_R2, " ] ");
        }

        rowSizeYet++;

        if (rowSizeYet != rowSize)  //error
        {
            if (!errFlag) {
                free(j.ranges_R1);
                free(j.ranges_R2);
            }
            errFlag = 1;
            if (line_num < temp_line_num)
                line_num = temp_line_num;
            retVal.field2 = -1;
            printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                   line_num,
                   "Declaration",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   tempOne->depth,
                   "3D JA dimension mismatch");
        }

        if (tempRange->child_count > 1 && rowsYet <= numRows)
            tempRange = tempRange->children[1];
        else
            break;
    }  //one more row traversed

    if (rowsYet != numRows)  //error
    {
        if (!errFlag) {
            free(j.ranges_R1);
            free(j.ranges_R2);
        }
        errFlag = 1;
        if (line_num < temp_line_num)
            line_num = temp_line_num;
        retVal.field2 = -1;
        printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
               line_num,
               "Declaration",
               "***",
               "***",
               "***",
               "***",
               "***",
               "***",
               "***",
               tempRange->depth,
               "3D JA size mismatch");
    }

    if (retVal.field2 == -1)
        return retVal;
    //success
    retVal.typeExpression.j = j;
    return retVal;
}

eachVariable singlePrim(parseTree *root, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
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
    root->typeExpression = t;
    return t;
}

eachVariable singleJarr2d(parseTree *single_jarr2d, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    eachVariable retVal;
    retVal = chkJagged2d(single_jarr2d->children[5], single_jarr2d->children[9]);
    strcpy(retVal.var_name, single_jarr2d->children[1]->lexeme);
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, retVal);
    single_jarr2d->typeExpression = retVal;
    return retVal;
}

eachVariable singleJarr3d(parseTree *single_jarr3d, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    eachVariable retVal;
    retVal = chkJagged3d(single_jarr3d->children[5], single_jarr3d->children[9]);
    strcpy(retVal.var_name, single_jarr3d->children[1]->lexeme);
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, retVal);
    single_jarr3d->typeExpression = retVal;
    return retVal;
}

char *dimrarr(parseTree *root, char *ranges, int *dim, int *dyn) {
    char *lval = root->children[0]->children[1]->children[0]->lexeme;
    char *rval = root->children[0]->children[3]->children[0]->lexeme;
    *dyn = 0;
    if (!(lval[0] >= 48 && lval[0] <= 57 && rval[0] >= 48 && rval[0] <= 57)) {
        *dyn = 1;
    }

    // strcat(ranges, "[");
    // printf("\nHEH %x\n", &ranges);
    if (ranges != NULL) {
        ranges = (char *)realloc(ranges, sizeof(char) * (strlen(ranges) + strlen(lval) + strlen(rval) + 1));
    } else {
        ranges = (char *)malloc(sizeof(char) * (0 + strlen(lval) + strlen(rval) + 1));
        ranges[0] = '\0';
    }
    strcat(ranges, lval);
    // strcat(ranges, ' ');
    strcat(ranges, ",");
    strcat(ranges, rval);
    strcat(ranges, ";");
    // strcat(ranges, "]");
    (*dim)++;
    if (root->child_count == 2) {
        dimrarr(root->children[1], ranges, dim, dyn);
    }
    return ranges;
}

eachVariable singleRarr(parseTree *root, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    eachVariable t;
    strcpy(t.var_name, root->children[1]->lexeme);
    t.field2 = 1;
    t.typeExpression.r.ranges = "";
    char *ranges = NULL;
    int *dim = (int *)malloc(sizeof(int));
    *dim = 0;
    int *dyn = (int *)malloc(sizeof(int));
    *dyn = 0;
    ranges = dimrarr(root->children[4], ranges, dim, dyn);
    t.isDynamic = *dyn;
    t.typeExpression.r.ranges = (char *)malloc(sizeof(ranges));
    t.typeExpression.r.ranges[0] = '\0';
    strcat(t.typeExpression.r.ranges, ranges);
    t.typeExpression.r.dimensions = *dim;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    root->typeExpression = t;
    return t;
}

eachVariable singleDecl(parseTree *root, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    if (root->children[0]->nodename[8] == 'p') {
        root->typeExpression = singlePrim(root->children[0], typeExpressionTable, sizeTypeExpTable);
    } else if (root->children[0]->nodename[8] == 'r') {
        root->typeExpression = singleRarr(root->children[0], typeExpressionTable, sizeTypeExpTable);
    } else if (!strcmp(root->children[0]->nodename, "<single_jarr2d>")) {
        root->typeExpression = singleJarr2d(root->children[0], typeExpressionTable, sizeTypeExpTable);
    } else if (!strcmp(root->children[0]->nodename, "<single_jarr3d>")) {
        root->typeExpression = singleJarr3d(root->children[0], typeExpressionTable, sizeTypeExpTable);
    }
    return root->typeExpression;
}

void varList(parseTree *root, eachVariable **typeExpressionTable, int *sizeTypeExpTable, eachVariable t) {
    if (!strcmp(root->children[0]->nodename, "epsilon")) {
        return;
    }
    strcpy(t.var_name, root->children[0]->lexeme);
    root->children[0]->typeExpression = t;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    strcpy(t.var_name, "N/A");
    root->typeExpression = t;
    varList(root->children[1], typeExpressionTable, sizeTypeExpTable, t);
}

eachVariable listPrim(parseTree *root, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
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
    strcpy(t.var_name, "N/A");
    root->typeExpression = t;
    return t;
}

eachVariable listRarr(parseTree *root, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    eachVariable t;
    t.field2 = 1;
    char *ranges = NULL;
    int *dim = (int *)malloc(sizeof(int));
    *dim = 0;
    int *dyn = (int *)malloc(sizeof(int));
    *dyn = 0;
    ranges = dimrarr(root->children[9], ranges, dim, dyn);
    t.isDynamic = *dyn;
    // printf("\nHEH %x\n", ranges);
    // printf("\nI WAS CALLED %d\n", sizeof(char)*(strlen(ranges)));
    t.typeExpression.r.dimensions = *dim;
    t.typeExpression.r.ranges = (char *)malloc(sizeof(ranges));
    t.typeExpression.r.ranges[0] = '\0';
    strcat(t.typeExpression.r.ranges, ranges);
    strcpy(t.var_name, root->children[4]->lexeme);
    root->children[4]->typeExpression = t;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    strcpy(t.var_name, root->children[5]->lexeme);
    root->children[5]->typeExpression = t;
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, t);
    varList(root->children[6], typeExpressionTable, sizeTypeExpTable, t);
    strcpy(t.var_name, "N/A");
    root->typeExpression = t;
    return t;
}

eachVariable listJarr2d(parseTree *list_jarr2d, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    eachVariable retVal;
    retVal = chkJagged2d(list_jarr2d->children[10], list_jarr2d->children[14]);
    strcpy(retVal.var_name, list_jarr2d->children[4]->lexeme);
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, retVal);
    strcpy(retVal.var_name, list_jarr2d->children[5]->lexeme);
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, retVal);
    varList(list_jarr2d->children[6], typeExpressionTable, sizeTypeExpTable, retVal);
    list_jarr2d->typeExpression = retVal;
    strcpy(retVal.var_name, "N/A");
    return retVal;
}

eachVariable listJarr3d(parseTree *list_jarr3d, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    eachVariable retVal;
    retVal = chkJagged3d(list_jarr3d->children[10], list_jarr3d->children[14]);
    strcpy(retVal.var_name, list_jarr3d->children[4]->lexeme);
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, retVal);
    strcpy(retVal.var_name, list_jarr3d->children[5]->lexeme);
    pushTypeTable(typeExpressionTable, sizeTypeExpTable, retVal);
    varList(list_jarr3d->children[6], typeExpressionTable, sizeTypeExpTable, retVal);
    list_jarr3d->typeExpression = retVal;
    strcpy(retVal.var_name, "N/A");
    return retVal;
}

eachVariable declList(parseTree *root, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    if (root->children[0]->nodename[6] == 'p') {
        root->typeExpression = listPrim(root->children[0], typeExpressionTable, sizeTypeExpTable);
    } else if (root->children[0]->nodename[6] == 'r') {
        root->typeExpression = listRarr(root->children[0], typeExpressionTable, sizeTypeExpTable);
    } else if (!strcmp(root->children[0]->nodename, "<list_jarr2d>")) {
        root->typeExpression = listJarr2d(root->children[0], typeExpressionTable, sizeTypeExpTable);
    } else if (!strcmp(root->children[0]->nodename, "<list_jarr3d>")) {
        root->typeExpression = listJarr3d(root->children[0], typeExpressionTable, sizeTypeExpTable);
    }
    return root->typeExpression;
}

eachVariable eachDecl(parseTree *root, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    if (root->children[0]->nodename[1] == 's') {
        root->typeExpression = singleDecl(root->children[0], typeExpressionTable, sizeTypeExpTable);
    } else {
        root->typeExpression = declList(root->children[0], typeExpressionTable, sizeTypeExpTable);
    }
    return root->typeExpression;
}

eachVariable searchTypeTable(eachVariable *typeExpressionTable, int size, char var_name[21]) {
    for (int i = 0; i < size; ++i) {
        if (!strcmp(typeExpressionTable[i].var_name, var_name)) {
            return typeExpressionTable[i];
        }
    }
    return typeExpressionTable[0];
}

eachVariable index_(parseTree *root, eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    if (root->children[0]->nodename[0] == 'V') {
        return searchTypeTable(typeExpressionTable, sizeTypeExpTable, root->children[0]->lexeme);
    } else {
        eachVariable t;
        strcpy(t.var_name, root->children[0]->lexeme);
        t.field2 = 0;
        t.isDynamic = -1;
        t.typeExpression.p.type = INTEGER;
        return t;
    }
}

int chkBound(eachVariable t, parseTree *index_, parseTree *index_list, eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    if (t.field2 == 1)  //rectangular array
    {
        //populating an array to store the dimension ranges from the type expression
        int dimRanges[t.typeExpression.r.dimensions * 2];
        int ind = 0;
        char *ranges_dup = strdup(t.typeExpression.r.ranges);
        char *token = strtok(ranges_dup, ",;");
        while (token != NULL) {
            dimRanges[ind] = atoi(token);
            ind++;
            token = strtok(NULL, ",;");
        }
        free(ranges_dup);

        //checking if indices in bound or notby traversing index_ and index_list
        ind = 0;

        do {
            if (!strcmp(index_->children[0]->nodename, "VAR_NAME")) {
                eachVariable varType = searchTypeTable(typeExpressionTable, sizeTypeExpTable, index_->children[0]->lexeme);
                if (varType.field2 != 0 || (varType.field2 == 0 && varType.typeExpression.p.type != INTEGER)) {
                    printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                           index_->children[0]->line_num,
                           "Assignment",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           index_->children[0]->depth,
                           "Invalid index type");
                    return 1;
                }
                printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                       index_->children[0]->line_num,
                       "Warning",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       index_->children[0]->depth,
                       "Dynamic indexing-Runtime check");
                return 0;
            } else if (!strcmp(index_->children[0]->nodename, "STATIC_CNST")) {
                int left = dimRanges[ind * 2];
                int right = dimRanges[ind * 2 + 1];
                int currIndVal = atoi(index_->children[0]->lexeme);
                if (!(left <= currIndVal && right >= currIndVal)) {
                    printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                           index_->children[0]->line_num,
                           "Assignment",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           index_->children[0]->depth,
                           "Invalid index value");
                    return 1;
                }
            }

            index_ = index_list->children[0];
            if (index_list->child_count > 1)
                index_list = index_list->children[1];
            ind++;

        } while (strcmp(index_->nodename, "epsilon"));
        return 0;
    }

    else if (t.field2 == 2)  //jagged array
    {
        int range1[2], firstIndex;
        char *R1_dup = strdup(t.typeExpression.j.ranges_R1);
        int ind = 0;
        char *token = strtok(R1_dup, " ");
        while (token != NULL) {
            range1[ind] = atoi(token);
            ind++;
            token = strtok(NULL, " ");
        }
        free(R1_dup);

        if (!strcmp(index_->children[0]->nodename, "VAR_NAME")) {
            eachVariable varType = searchTypeTable(typeExpressionTable, sizeTypeExpTable, index_->children[0]->lexeme);
            if (varType.field2 != 0 || (varType.field2 == 0 && varType.typeExpression.p.type != INTEGER)) {
                printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                       index_->children[0]->line_num,
                       "Assignment",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       index_->children[0]->depth,
                       "Invalid index type");
                return 1;
            }
            printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                   index_->children[0]->line_num,
                   "Warning",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   "***",
                   index_->children[0]->depth,
                   "Dynamic indexing-Runtime check");
            return 0;
        } else if (!strcmp(index_->children[0]->nodename, "STATIC_CNST")) {
            firstIndex = atoi(index_->children[0]->lexeme);
            if (!(range1[0] <= firstIndex && range1[1] >= firstIndex)) {
                printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                       index_->children[0]->line_num,
                       "Assignment",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       index_->children[0]->depth,
                       "Invalid index value");
                return 1;
            }
        }

        if (t.typeExpression.j.dimensions == 2)  //2djagged
        {
            //only one more index to check
            char *R2_dup = strdup(t.typeExpression.j.ranges_R2);
            char *token = strtok(R2_dup, " ");
            int indexPos = range1[0];
            int secondIndMax = atoi(token);
            while (indexPos != firstIndex) {
                token = strtok(NULL, " ");
                indexPos++;
                secondIndMax = atoi(token);
            }
            index_ = index_list->children[0];
            free(R2_dup);
            if (!strcmp(index_->children[0]->nodename, "VAR_NAME")) {
                eachVariable varType = searchTypeTable(typeExpressionTable, sizeTypeExpTable, index_->children[0]->lexeme);
                if (varType.field2 != 0 || (varType.field2 == 0 && varType.typeExpression.p.type != INTEGER)) {
                    printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                           index_->children[0]->line_num,
                           "Assignment",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           index_->children[0]->depth,
                           "Invalid index type");
                    return 1;
                }

                printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                       index_->children[0]->line_num,
                       "Warning",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       index_->children[0]->depth,
                       "Dynamic indexing-Runtime check");
                return 0;
            } else if (!strcmp(index_->children[0]->nodename, "STATIC_CNST")) {
                int currIndVal = atoi(index_->children[0]->lexeme);
                if (!(1 <= currIndVal && secondIndMax >= currIndVal)) {
                    printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                           index_->children[0]->line_num,
                           "Assignment",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           index_->children[0]->depth,
                           "Invalid index value");
                    return 1;
                }
            }
            return 0;
        }

        else if (t.typeExpression.j.dimensions == 3)  //3d jagged
        {
            //check 2nd index
            index_ = index_list->children[0];
            int secondIndex;
            char *R2_dup = strdup(t.typeExpression.j.ranges_R2);
            char *token = strtok(R2_dup, "]"), *token2;
            int indexPos = range1[0];
            while (indexPos != firstIndex) {
                token = strtok(NULL, "]");
                indexPos++;
            }
            token2 = strtok(token, " [");
            int secondIndMax = atoi(token2);
            if (!strcmp(index_->children[0]->nodename, "VAR_NAME")) {
                eachVariable varType = searchTypeTable(typeExpressionTable, sizeTypeExpTable, index_->children[0]->lexeme);
                if (varType.field2 != 0 || (varType.field2 == 0 && varType.typeExpression.p.type != INTEGER)) {
                    printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                           index_->children[0]->line_num,
                           "Assignment",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           index_->children[0]->depth,
                           "Invalid index type");
                    return 1;
                }

                printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                       index_->children[0]->line_num,
                       "Warning",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       index_->children[0]->depth,
                       "Dynamic indexing-Runtime check");
                return 0;
            } else if (!strcmp(index_->children[0]->nodename, "STATIC_CNST")) {
                secondIndex = atoi(index_->children[0]->lexeme);
                if (!(1 <= secondIndex && secondIndMax >= secondIndex)) {
                    printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                           index_->children[0]->line_num,
                           "Assignment",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           index_->children[0]->depth,
                           "Invalid index value");
                    return 1;
                }
            }

            //check 3rd index
            int thirdIndex, thirdIndMax;
            for (int k = 1; k <= secondIndex; k++) {
                token2 = strtok(NULL, " [");
            }
            thirdIndMax = atoi(token2);
            index_ = index_list->children[1]->children[0];

            if (!strcmp(index_->children[0]->nodename, "VAR_NAME")) {
                eachVariable varType = searchTypeTable(typeExpressionTable, sizeTypeExpTable, index_->children[0]->lexeme);
                if (varType.field2 != 0 || (varType.field2 == 0 && varType.typeExpression.p.type != INTEGER)) {
                    printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                           index_->children[0]->line_num,
                           "Assignment",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           index_->children[0]->depth,
                           "Invalid index type");
                    return 1;
                }

                printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                       index_->children[0]->line_num,
                       "Warning",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       "***",
                       index_->children[0]->depth,
                       "Dynamic indexing-Runtime check");
                return 0;
            } else if (!strcmp(index_->children[0]->nodename, "STATIC_CNST")) {
                thirdIndex = atoi(index_->children[0]->lexeme);
                if (!(1 <= thirdIndex && thirdIndMax >= thirdIndex)) {
                    printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
                           index_->children[0]->line_num,
                           "Assignment",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           "***",
                           index_->children[0]->depth,
                           "Invalid index value");
                    return 1;
                }
            }
            free(R2_dup);
            return 0;
        }
    }
}

eachVariable array_elem(parseTree *root, eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    eachVariable t = searchTypeTable(typeExpressionTable, sizeTypeExpTable, root->children[0]->lexeme);
    eachVariable ret;
    ret.field2 = 0;
    ret.isDynamic = -1;
    ret.typeExpression.p.type = INTEGER;
    strcpy(ret.var_name, t.var_name);
    if (t.field2 == -1)  //variable has error in declaration
    {
        ret.field2 = -1;
        root->typeExpression = ret;
        return ret;
    }
    if (t.isDynamic == 1) {
        printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
               root->children[0]->line_num,
               "Warning",
               "***",
               "***",
               "***",
               "***",
               "***",
               "***",
               "***",
               root->children[0]->depth,
               "Dynamic indexing-Runtime check");
        root->typeExpression = ret;
        return ret;
    } else if (t.field2 == 1 || t.field2 == 2) {  //ARRAY
        int r = chkBound(t, root->children[2], root->children[3], typeExpressionTable, sizeTypeExpTable);
        root->typeExpression = ret;
        if (!r)  //no error
            return ret;
    }

    ret.field2 = -1;
    root->typeExpression = ret;
    return ret;
}

char **returnVar(eachVariable t) {
    char **ret = (char **)(malloc(sizeof(char *) * 4));
    ret[0] = strdup(t.var_name);
    if (t.field2 == -1) {
        ret[2] = strdup("ERROR");
    } else {
        switch (t.isDynamic) {
            case -1:
                ret[2] = strdup("not applicable");
                break;

            case 0:
                ret[2] = strdup("static");
                break;

            case 1:
                ret[2] = strdup("dynamic");
                break;

            default:
                break;
        }
    }
    if (t.field2 == 0) {
        switch (t.typeExpression.p.type) {
            case INTEGER:
                ret[3] = strdup("integer");
                break;
            case REAL:
                ret[3] = strdup("real");
                break;
            case BOOLEAN:
                ret[3] = strdup("boolean");
                break;
            default:
                break;
        }
        ret[1] = strdup("primitive");
    } else if (t.field2 == 1) {
        ret[1] = strdup("rectangularArray ");
        ret[3] = (char *)malloc(sizeof(char) * 200);
        ret[3][0] = '\0';
        strcat(ret[3], "integer ");
        strcat(ret[3], "Dimensions: ");

        int length = snprintf(NULL, 0, "%d", t.typeExpression.r.dimensions);
        char *string_TC = malloc(length + 1);
        snprintf(string_TC, length + 1, "%d", t.typeExpression.r.dimensions);

        strcat(ret[3], string_TC);
        strcat(ret[3], " Ranges: ");
        strcat(ret[3], t.typeExpression.r.ranges);
    } else if (t.field2 == 2) {
        ret[1] = strdup("jaggedArray ");
        ret[3] = (char *)malloc(sizeof(char) * 200);
        ret[3][0] = '\0';
        strcat(ret[3], "integer ");
        strcat(ret[3], "Dimensions: ");

        int length = snprintf(NULL, 0, "%d", t.typeExpression.j.dimensions);
        char *string_TC = malloc(length + 1);
        snprintf(string_TC, length + 1, "%d", t.typeExpression.j.dimensions);

        strcat(ret[3], string_TC);
        strcat(ret[3], " Range1: ");
        strcat(ret[3], t.typeExpression.j.ranges_R1);
        strcat(ret[3], " Range2: ");
        strcat(ret[3], t.typeExpression.j.ranges_R2);
    }
    return ret;
}

void printAssError(parseTree *root, eachVariable t1, eachVariable t2, char *shortMessage) {
    char **t1_str = returnVar(t1);
    char **t2_str = returnVar(t2);
    if(t1.field2 == -1 && t2.field2 == -1){
        printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
               root->children[1]->line_num,
               "Assignment",
               root->children[1]->lexeme,
               "N/A",
               "error",
               "",
               "N/A",
               "error",
               "",
               root->depth,
               shortMessage);
    }
    else if (t1.field2 == -1) {
        printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
               root->children[1]->line_num,
               "Assignment",
               root->children[1]->lexeme,
               "N/A",
               "error",
               "",
               t2_str[0],
               t2_str[1],
               t2_str[3],
               root->depth,
               shortMessage);
    } else if (t2.field2 == -1) {
        printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
               root->children[1]->line_num,
               "Assignment",
               root->children[1]->lexeme,
               t1_str[0],
               t1_str[1],
               t1_str[3],
               "N/A",
               "error",
               "",
               root->depth,
               shortMessage);
    } else {
        printf("%-15d %-15s %-10s %-20s %-20s %-70s %-20s %-20s %-70s %-5d %-30s\n",
               root->children[1]->line_num,
               "Assignment",
               root->children[1]->lexeme,
               t1_str[0],
               t1_str[1],
               t1_str[3],
               t2_str[0],
               t2_str[1],
               t2_str[3],
               root->depth,
               shortMessage);
    }
    return;
}

eachVariable computeExpr4(parseTree *root, eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    eachVariable t;
    if (root->children[0]->children[0]->nodename[1] == 'i') {
        t = index_(root->children[0]->children[0], typeExpressionTable, sizeTypeExpTable);
    } else {
        t = array_elem(root->children[0]->children[0], typeExpressionTable, sizeTypeExpTable);
    }
    root->children[0]->typeExpression = t;
    if (root->child_count == 3) {
        strcpy(root->typeExpression.var_name, "N/A");
        char shortMessage[31];
        shortMessage[0] = '\0';
        strcat(shortMessage, "Incompatible type for ");
        (!strcmp(root->children[1]->lexeme, "/")) ? strcat(shortMessage, "division") : strcat(shortMessage, "multiply");
        eachVariable t2 = computeExpr4(root->children[2], typeExpressionTable, sizeTypeExpTable);
        if (t.field2 == -1 || t2.field2 == -1) {
            root->typeExpression.field2 = -1;
            strcpy(shortMessage, "Error propagated from RHS of ");
            strcat(shortMessage, root->children[1]->lexeme);
            printAssError(root, t, t2, shortMessage);
            return root->typeExpression;
        }
        if (t.field2 == 0 && t2.field2 == 0) {
            if (!strcmp(root->children[1]->lexeme, "/")) {
                if ((t.typeExpression.p.type == INTEGER && t2.typeExpression.p.type == INTEGER) || (t.typeExpression.p.type == REAL && t2.typeExpression.p.type == REAL)) {
                    root->typeExpression.field2 = 0;
                    root->typeExpression.typeExpression.p.type = REAL;
                    return root->typeExpression;
                }
            } else {
                if (t.typeExpression.p.type == t2.typeExpression.p.type) {
                    root->typeExpression = t;
                    strcpy(root->typeExpression.var_name, "N/A");
                    return root->typeExpression;
                }
            }
        } else if (t.field2 == 1 && t2.field2 == 1) {
            if (!strcmp(root->children[1]->lexeme, "/")) {
                strcpy(shortMessage, "Arrays don't support division");
            } else {
                if (!strcmp(t.typeExpression.r.ranges, t2.typeExpression.r.ranges)) {
                    root->typeExpression = t;
                    strcpy(root->typeExpression.var_name, "N/A");
                    return root->typeExpression;
                }
            }
        } else if (t.field2 == 2 && t2.field2 == 2) {
            if (!strcmp(root->children[1]->lexeme, "/")) {
                strcpy(shortMessage, "Arrays don't support division");
            } else {
                if (!strcmp(t.typeExpression.j.ranges_R1, t2.typeExpression.j.ranges_R1) && !strcmp(t.typeExpression.j.ranges_R2, t2.typeExpression.j.ranges_R2)) {
                    root->typeExpression = t;
                    strcpy(root->typeExpression.var_name, "N/A");
                    return root->typeExpression;
                }
            }
        }
        printAssError(root, t, t2, shortMessage);
        root->typeExpression.field2 = -1;
        return root->typeExpression;
    }
    root->typeExpression = t;
    return t;
}

eachVariable computeExpr3(parseTree *root, eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    eachVariable t;
    t = computeExpr4(root->children[0], typeExpressionTable, sizeTypeExpTable);
    if (root->child_count == 3) {
        strcpy(root->typeExpression.var_name, "N/A");
        char shortMessage[31];
        shortMessage[0] = '\0';
        strcat(shortMessage, "Incompatible type for ");
        (!strcmp(root->children[1]->lexeme, "+")) ? strcat(shortMessage, "addition") : strcat(shortMessage, "subtract");
        eachVariable t2 = computeExpr3(root->children[2], typeExpressionTable, sizeTypeExpTable);
        if (t.field2 == -1 || t2.field2 == -1) {
            root->typeExpression.field2 = -1;
            strcpy(shortMessage, "Error propagated from RHS of ");
            strcat(shortMessage, root->children[1]->lexeme);
            printAssError(root, t, t2, shortMessage);
            return root->typeExpression;
        }
        if (t.field2 == 0 && t2.field2 == 0) {
            if (t.typeExpression.p.type == t2.typeExpression.p.type) {
                root->typeExpression = t;
                strcpy(root->typeExpression.var_name, "N/A");
                return root->typeExpression;
            }
        } else if (t.field2 == 1 && t2.field2 == 1) {
            if (!strcmp(t.typeExpression.r.ranges, t2.typeExpression.r.ranges)) {
                root->typeExpression = t;
                strcpy(root->typeExpression.var_name, "N/A");
                return root->typeExpression;
            }
        } else if (t.field2 == 2 && t2.field2 == 2) {
            if (!strcmp(t.typeExpression.j.ranges_R1, t2.typeExpression.j.ranges_R1) && !strcmp(t.typeExpression.j.ranges_R2, t2.typeExpression.j.ranges_R2)) {
                root->typeExpression = t;
                strcpy(root->typeExpression.var_name, "N/A");
                return root->typeExpression;
            }
        }
        printAssError(root, t, t2, shortMessage);
        root->typeExpression.field2 = -1;
        return root->typeExpression;
    }
    root->typeExpression = t;
    return t;
}

eachVariable computeExpr2(parseTree *root, eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    eachVariable t;
    t = computeExpr3(root->children[0], typeExpressionTable, sizeTypeExpTable);
    if (root->child_count == 3) {
        char shortMessage[31];
        shortMessage[0] = '\0';
        strcat(shortMessage, "Incompatible type for AND.");
        eachVariable t2 = computeExpr2(root->children[2], typeExpressionTable, sizeTypeExpTable);
        if (t.field2 == -1 || t2.field2 == -1) {
            root->typeExpression.field2 = -1;
            printAssError(root, t, t2, "Error propagated from RHS of AND");
            return root->typeExpression;
        }
        if (t.field2 == 0 && t2.field2 == 0) {
            if (t.typeExpression.p.type == BOOLEAN && t2.typeExpression.p.type == BOOLEAN) {
                root->typeExpression = t;
                return t;
            }
        }
        printAssError(root, t, t2, shortMessage);
        root->typeExpression.field2 = -1;
        return root->typeExpression;
    }
    root->typeExpression = t;
    return t;
}

eachVariable computeExpr1(parseTree *root, eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    eachVariable t;
    t = computeExpr2(root->children[0], typeExpressionTable, sizeTypeExpTable);
    if (root->child_count == 3) {
        char shortMessage[31];
        shortMessage[0] = '\0';
        strcat(shortMessage, "Incompatible type for OR.");
        strcpy(root->typeExpression.var_name, "N/A");
        eachVariable t2 = computeExpr1(root->children[2], typeExpressionTable, sizeTypeExpTable);
        if (t.field2 == -1 || t2.field2 == -1) {
            root->typeExpression.field2 = -1;
            printAssError(root, t, t2, "Error propagated from RHS of OR");
            return root->typeExpression;
        }
        if (t.field2 == 0 && t2.field2 == 0) {
            if (t.typeExpression.p.type == BOOLEAN && t2.typeExpression.p.type == BOOLEAN) {
                root->typeExpression = t;
                strcpy(root->typeExpression.var_name, "N/A");
                return t;
            }
        }
        printAssError(root, t, t2, shortMessage);
        root->typeExpression.field2 = -1;
        return root->typeExpression;
    }
    root->typeExpression = t;
    return t;
}

eachVariable computeLhsOp(parseTree *root, eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    eachVariable t;
    if (root->children[0]->nodename[0] == 'V') {
        t = searchTypeTable(typeExpressionTable, sizeTypeExpTable, root->children[0]->lexeme);
    } else {
        t = array_elem(root->children[0], typeExpressionTable, sizeTypeExpTable);
    }
    root->typeExpression = t;
    return t;
}

int sameType(eachVariable t1, eachVariable t2) {
    if (t1.field2 == -1 || t2.field2 == -1) {
        return -1;
    } else {
        if (t1.field2 == t2.field2) {
            if (t1.field2 == 0) {
                if (t1.typeExpression.p.type == t2.typeExpression.p.type) {
                    return 1;
                }
            } else if (t1.field2 == 1) {
                if (!strcmp(t1.typeExpression.r.ranges, t2.typeExpression.r.ranges)) {
                    return 1;
                }
            } else {
                if (!strcmp(t1.typeExpression.j.ranges_R1, t2.typeExpression.j.ranges_R1) && !strcmp(t1.typeExpression.j.ranges_R2, t2.typeExpression.j.ranges_R2)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void oneAssgmt(parseTree *root, eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    eachVariable lhs_op = computeLhsOp(root->children[0], typeExpressionTable, sizeTypeExpTable);
    eachVariable expr1 = computeExpr1(root->children[2], typeExpressionTable, sizeTypeExpTable);

    int ret = sameType(lhs_op, expr1);
    if (ret == 1) {
        root->typeExpression = lhs_op;
        return;
    } else if (ret == -1) {
        root->typeExpression.field2 = -1;
        printAssError(root, lhs_op, expr1, "Error at assignment propagated from RHS");
        return;
    }
    printAssError(root, lhs_op, expr1, "Types don't match for assignment");
}

void printVar(eachVariable t) {
    printf("%-20s", t.var_name);
    if (t.field2 == -1) {
        printf("ERROR");
    } else {
        switch (t.isDynamic) {
            case -1:
                printf("%-17s", "not applicable");
                break;

            case 0:
                printf("%-17s", "static");
                break;

            case 1:
                printf("%-17s", "dynamic");
                break;

            default:
                break;
        }
    }
    if (t.field2 == 0) {
        switch (t.typeExpression.p.type) {
            case INTEGER:
                printf("%-22s", "integer");
                break;
            case REAL:
                printf("%-22s", "real");
                break;

            case BOOLEAN:
                printf("%-22s", "boolean");
                break;
            default:
                break;
        }
        printf("primitive");
    } else if (t.field2 == 1) {
        printf("%-22s", "integer");
        printf("rectangularArray ");
        printf("Dimensions: %d, Ranges: %s", t.typeExpression.r.dimensions, t.typeExpression.r.ranges);
    } else if (t.field2 == 2) {
        printf("%-22s", "integer");
        printf("jaggedArray ");
        printf("Dimensions: %d, Range1: %s, Range2: %s", t.typeExpression.j.dimensions, t.typeExpression.j.ranges_R1, t.typeExpression.j.ranges_R2);
    }
    printf("\n");
}

void traverseAssgmtStmt(parseTree *root, eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    oneAssgmt(root->children[0], typeExpressionTable, sizeTypeExpTable);
    if (root->child_count == 2) {
        traverseAssgmtStmt(root->children[1], typeExpressionTable, sizeTypeExpTable);
    }
    return;
}

void traverseDeclStmt(parseTree *root, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    eachDecl(root->children[0], typeExpressionTable, sizeTypeExpTable);
    if (root->child_count == 2) {
        traverseDeclStmt(root->children[1], typeExpressionTable, sizeTypeExpTable);
    }
    return;
}

/* GIVEN BELOW ARE THE FUNCTIONS SPECIFIED IN THE ASSIGNMENT SHEET AND THE DRIVER CODE */
/* ABOVE THIS WE HAVE DEFINED ALL THE DATA STRUCTURES AND HELPER FUNCTIONS */
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

grammar *readGrammar(char *filename) {
    FILE *filep = fopen(filename, "r");

    grammar *G = (grammar *)malloc(sizeof(grammar));

    struct Node *temp;
    G->rules = (struct Node **)malloc(65 * sizeof(struct Node *));
    int count = 0;

    char *line = (char *)malloc(150 * sizeof(char));
    char *check;
    while (1) {
        check = fgets(line, 150, filep);
        if (!check) {
            break;
        }
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        struct Node *trav = (struct Node *)malloc(sizeof(struct Node));
        G->rules[count] = trav;

        char *tempStr;
        tempStr = strtok(line, " ");

        trav->piece = (char *)malloc((strlen(tempStr) + 1) * sizeof(char));
        strcpy(trav->piece, tempStr);

        trav->next = (struct Node *)malloc(sizeof(struct Node));
        // trav=trav->next;
        tempStr = strtok(NULL, " ");

        while (tempStr) {
            trav = trav->next;

            trav->piece = (char *)malloc((strlen(tempStr) + 1) * sizeof(char));
            strcpy(trav->piece, tempStr);

            trav->next = (struct Node *)malloc(sizeof(struct Node));
            tempStr = strtok(NULL, " ");
        }

        trav->next = NULL;

        count++;
    }
    fclose(filep);
    return G;
}

tokenStream *tokeniseSourceCode(char *filename, tokenStream *s) {
    FILE *filep = fopen(filename, "r");
    if (filep==NULL){
        return NULL;
    }
    s = (tokenStream *)malloc(sizeof(tokenStream));
    tokenStream *ret = s;

    char *line = (char *)malloc(200 * sizeof(char));
    char *check;
    int count = 1;

    while (1) {
        check = fgets(line, 200, filep);
        if (!check) {
            break;
        }
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }

        char *tempStr;
        tempStr = strtok(line, " \t\r\n");

        while (tempStr) {
            s->line_num = count;
            s->lexeme = (char *)malloc((strlen(tempStr) + 1) * sizeof(char));
            strcpy(s->lexeme, tempStr);
            s->token = (char *)malloc((strlen(tempStr) + 1) * sizeof(char));
            strcpy(s->token, getToken(tempStr));
            
            tempStr = strtok(NULL, " \t\r\n");

            s->next = (tokenStream *)malloc(sizeof(tokenStream));
            s = s->next;
        }

        count++;
    }
    s = ret;

    while (s->next->line_num < count && s->next->line_num > 0) {
        s = s->next;
    }
    s->next = NULL;

    return ret;
}

parseTree *createParseTree(parseTree *root, tokenStream *ts, grammar *G) {
    stackNode *s = NULL;
    stackNode *t = NULL;
    t = makestackNode("<main_program>", 0, root);
    push(&s, t);

    parseTree *value = genTree(root, &s, &ts, G);

    return value;
}

void traverseParseTree(parseTree *root, eachVariable **typeExpressionTable, int *sizeTypeExpTable) {
    printf("%-15s %-15s %-10s %-20s %-91s %-20s %-91s %-5s %-30s\n", "Line number", "Statement type", "Operator", "Lexeme of 1st", "Type of 1st", "Lexeme of 2nd", "Type of 2nd", "Depth", "Short Message");
    for (int i = 0; i < 300; i++) printf("-");
    printf("\n");
    traverseDeclStmt(root->children[4], typeExpressionTable, sizeTypeExpTable);
    
    traverseAssgmtStmt(root->children[5], *typeExpressionTable, *sizeTypeExpTable);
}

void printParseTree(parseTree *t) {
    char ** typexp= returnVar(t->typeExpression);

    if (t == NULL) {
        return;
    }
    if (t->is_terminal) {
        printf("%-20s TERMINAL       %-20s %-12d %-15d %-10d %-20s %-80s\n", t->nodename, t->lexeme, t->line_num, t->gramRule, t->depth,"not defined for leaf nodes","");
    } else if ((t->typeExpression).field2==-1) {
        char ran[] = "NOT DEFINED";
        printf("%-20s NON-TERMINAL   %-20s %-12s %-15d %-10d %-20s %-80s\n", t->nodename, ran, ran, t->gramRule, t->depth,"error","");
    }
    else if ((t->typeExpression).field2==-2){
        char ran[] = "NOT DEFINED";

        printf("%-20s NON-TERMINAL   %-20s %-12s %-15d %-10d %-20s %-80s\n", t->nodename, ran, ran, t->gramRule, t->depth,"not applicable for this node","");

    }
    else {
        char ran[] = "NOT DEFINED";

        printf("%-20s NON-TERMINAL   %-20s %-12s %-15d %-10d %-20s %-80s\n", t->nodename, ran, ran, t->gramRule, t->depth,typexp[1],typexp[3]);

    }

    for (int i = 0; i < t->child_count; i++) {
        printParseTree(t->children[i]);
    }

    return;
}

void printTypeExpressionTable(eachVariable *typeExpressionTable, int sizeTypeExpTable) {
    printf("\n");
    printf("%-19s %-16s %-21s %-60s\n", "Variable name", "Is Dynamic?", "basicElementType", "Data type");
    for (int i = 0; i < 120; i++) printf("-");
    printf("\n");
    for (int i = 0; i < sizeTypeExpTable; ++i) {
        eachVariable t = typeExpressionTable[i];
        printVar(t);
    }
}

int main(int argc, char * argv[]) {
    if (argc<2){
        printf("Please enter a filename as command line argument while executing\n");
        exit(0);
    }

    parseTree *root = NULL;

    grammar *temp;
    temp = readGrammar(filename);

    tokenStream *stream;
    stream = tokeniseSourceCode(argv[1], stream);

    if (stream==NULL){
        printf("Invalid file name. Please enter a filename present in the current working directory.\n");
        exit(0);
    }

    printf("Available command numbers:- \n");
    printf("1 -> Creates a parse tree for the file specified. Does not print any of the data structures.\n");
    printf("2 -> Creates a parse tree and traverses it, printing all the errors found in the source code.\n");
    printf("3 -> Creates a parse tree and prints it as per the format specified\n");
    printf("4 -> Creates a parse tree, traverses it and prints all the errors found as well as the Type Expression table.\n\n");
    
    while (1){
        int command;
        
        printf("ENTER COMMAND NUMBER\n");
        fflush(stdout);
        scanf("%d",&command);
        printf("Option selected was %d\n\n",command);

        if (command==0){
            printf("Program has been exited\n");
            break;
        }
        else if (command==1){
            root = makenode("<main_program>", 0, "", 1, -1, 0);
            parseTree *value = createParseTree(root, stream, temp);
            printf("Parse tree was created.\n\n");
            destroytreecopy(value);
        }
        else if (command==2){
            root = makenode("<main_program>", 0, "", 1, -1, 0);
            parseTree *value = createParseTree(root, stream, temp);
            printf("Parse tree was created.\n");
            printf("Traversing parse tree\n\n");
            printf("Printing all the errors!\n\n");
            
            eachVariable *typeExpressionTable = NULL;
            int *sizeTypeExpTable = (int *)malloc(sizeof(int));
            *sizeTypeExpTable = 0;
            traverseParseTree(value, &typeExpressionTable, sizeTypeExpTable);
            printf("\n");
            destroytreecopy(value);
        }
        else if (command==3){
            root = makenode("<main_program>", 0, "", 1, -1, 0);
            parseTree *value = createParseTree(root, stream, temp);
            printf("Parse tree was created.\n");
            printf("Traversing parse tree\n\n");
            printf("Printing all the errors!\n\n");
            
            eachVariable *typeExpressionTable = NULL;
            int *sizeTypeExpTable = (int *)malloc(sizeof(int));
            *sizeTypeExpTable = 0;
            traverseParseTree(value, &typeExpressionTable, sizeTypeExpTable);
            printf("\n");
            printf("Printing parse tree\n\n");
            printf("%-20s IsTerminal     %-20s %-12s %-15s %-10s %-100s\n", "Symbol Name","Lexeme", "Line Number", "Grammar Rule", "Depth","Type Expression(for non-leaf)");
            for (int i = 0; i < 160; i++) printf("-");
            printf("\n");

            printParseTree(value);
            printf("\n");
            destroytreecopy(value);
        }
        else if (command==4){
            root = makenode("<main_program>", 0, "", 1, -1, 0);
            parseTree *value = createParseTree(root, stream, temp);
            printf("Parse tree was created.\n");
            printf("Traversing parse tree\n\n");
            printf("Printing all the errors!\n\n");
            
            eachVariable *typeExpressionTable = NULL;
            int *sizeTypeExpTable = (int *)malloc(sizeof(int));
            *sizeTypeExpTable = 0;
            traverseParseTree(value, &typeExpressionTable, sizeTypeExpTable);
            printf("\nPrinting the Type Expression table\n");
            printTypeExpressionTable(typeExpressionTable, *sizeTypeExpTable);
            printf("\n");
            destroytreecopy(value);
        }
        else {
            printf("Invalid command number. Please enter a command number among the following given:- \n\n");
            printf("Available command numbers:- \n");
            printf("1 -> Creates a parse tree for the file specified. Does not print any of the data structures.\n");
            printf("2 -> Creates a parse tree and traverses it, printing all the errors found in the source code.\n");
            printf("3 -> Creates a parse tree and prints it as per the format specified\n");
            printf("4 -> Creates a parse tree, traverses it and prints all the errors found as well as the Type Expression table.\n");
            printf("\n");
        }

        printf("COMMAND COMPLETED\n\n");
    }


    
    return 0;
}
