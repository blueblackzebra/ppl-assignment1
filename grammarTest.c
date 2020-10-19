#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node{
    char * piece;
    struct Node * next;
};

// typedef struct Node Node;

typedef struct {
    struct Node ** rules;
} grammar;

char filename[]="Eggrammar.txt";

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
    
    
    // printf("%s\n",p[1]->piece);
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

        // printf("String is %s\n",tempStr);

        trav->piece=(char *)malloc((strlen(tempStr)+1)*sizeof(char));
        strcpy(trav->piece,tempStr);
        // printf("%s\n",trav->piece);
        // printf("%p\n",(void *) trav->piece);
        trav->next=(struct Node *)malloc(sizeof(struct Node));
        // trav=trav->next;
        tempStr=strtok(NULL," ");

        
        while (tempStr){
            trav=trav->next;

            // printf("String is %s\n",tempStr);
            // trav=(Node *)malloc(sizeof(Node));
            trav->piece=(char *)malloc((strlen(tempStr)+1)*sizeof(char));
            strcpy(trav->piece,tempStr);
            // printf("%s\n",trav->piece);
            // printf("%p\n",(void *) trav->piece);

            // Node * n=temp[count];
            // printf("%s\n",n->piece);
            trav->next=(struct Node *)malloc(sizeof(struct Node));
            tempStr=strtok(NULL," ");
        }

        // free(trav->next);
        trav->next=NULL;
        

        // printf("%s\n",G->rules[count]->piece);
        // traverseG(temp[count]);

        

        // Node * q=n->next;
        // printf("%s\n",q->piece);
        // printf("%s\n",line);
        count++;
        
    }
    fclose(filep);
    return G;
}



int main(){
    grammar * temp;
    temp=readGrammar(filename);
    traverseG(temp);
}