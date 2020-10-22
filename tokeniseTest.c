#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Symbol{
    int line_num;
    char * lexeme;
    char * token;
    struct Symbol * next;
};

typedef struct Symbol tokenStream;

char filename[]="Egsourcecode.txt";

void traverseS(tokenStream * s){
    printf("\nTraversal begins\n");
    while (s!=NULL){
        printf("%s %d\n",s->lexeme,s->line_num);
        s=s->next;
    }
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
            printf("%s %d\n",s->lexeme,s->line_num);
            tempStr=strtok(NULL," ");

            s->next=(tokenStream *)malloc(sizeof(tokenStream));
            s=s->next;
        }

        count++;
    }
    s=ret;

    while (s->next->line_num){
        s=s->next;
    }
    s->next=NULL;

    return ret;
}

int main(){
    tokenStream * stream;
    stream=tokeniseSourceCode(filename,stream);
    traverseS(stream);
    traverseS(stream);
}