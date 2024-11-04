/************************************
* VR472547
* Alex Merlin
* 10/06/24 - 09/07/24
*************************************/


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "../head/fun.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#define COL 3
#define ROW 3


void erExit(const char *msg){
    perror(msg);
    exit(1);

};

void operazione(struct sembuf *op, int numsem, int oper){

    op->sem_num = numsem;
    op->sem_op = oper;   
};


//funzione per stampare una matrice
void print_board(char matrice[ROW][COL]){

    printf("\n --------------------\n");

    for(int i = 0; i < ROW; i++){
        printf(" | ");
        for(int j = 0; j < COL; j++){
            printf(" %c ", matrice[i][j]);
            printf(" | ");

        }
        
        printf("\n --------------------\n");
        
    }

}

void save_move(char (*matrix)[3],int *mossa, char simbol,int * count){

    if(*mossa == -1){
        return;
    }else if(*mossa < -2 || *mossa > 9 || *mossa == 0){
        *mossa = -3;
    }
    else{
        char *seq;
        seq = &matrix[0][0];    //rendo l'accesso sequenziale;
        *mossa = (*mossa) - 1; 
        printf("STAI INSERENDO LA MOSSA NELLA posizione %i\n", *mossa);
        if(seq[*mossa] == ' '){
            seq[*mossa] = simbol;
            *count = *count +1;
        }
        else
            *mossa = -2;    
    }
    //*mossa = -1;
    return;
}

void init_matrix(char matrix[3][3]){

for(int i = 0; i < ROW; i++){
        for(int j = 0; j < COL; j++){
            matrix[i][j] = ' ';

        }
    }


}

int check_win(char matrix[3][3], struct player *p, int count){

    int i; int j;
    int flag = 1;

    //controllo righe
    for(i = 0; i < 3; i++){
        flag = 1;
        for(j = 0; j < 3 && flag == 1; j++){
            if(matrix[i][j] == p->simbol)
                flag = 1;
            else    
                flag = 0;
        }
        if(flag == 1){
            
            printf("vinto per riga\n");
            return 1;
        }
    }
    


    //Controllo colonne
    flag = 1;
    for(j = 0; j < 3; j++){
        flag = 1;
        for(i = 0; i < 3 && flag == 1; i++){
            if(matrix[i][j] == p->simbol)
                flag = 1;
            else    
                flag = 0;
        }
        if(flag == 1){
            
            printf("vinto per colonna\n");
            return 1;
        }
    }

    //Controllo diagonale
    j= 0;
    flag = 1;
    for(i = 0; i < 3 && flag == 1; i++){
        if(matrix[i][j] == p->simbol)
                flag = 1;
            else    
                flag = 0;
        j++;   
    }
    if(flag == 1){
            
            printf("vinto per diagonale\n");
            return 1;
        }

    
    //Controllo anti-diagonale
    j= 2;
    flag = 1;
    for(i = 0; i < 3 && flag == 1; i++){
        if(matrix[i][j] == p->simbol)
                flag = 1;
            else    
                flag = 0;
        j--;   
    }
    if(flag == 1){
            printf("vinto per anti diagonale\n");
            return 1;
        }        

    if(count == 9)
        return (-1);

    return 0;

}


void random_move(int *mossa, char (*matrix)[3]){
    srand(time(NULL));
    int pc_move;
    int flag = 1;
    char *seq = matrix[0];

    while(flag == 1){

        pc_move = (rand() % 9);
        printf("piccmove: %i\n",pc_move);

        if(*(seq + pc_move) != 32){
            flag = 1;
        }
        else{
            flag = 0;
            *mossa = pc_move+1;  //perchè il successivo controllo della mossa ragiona come essere umano quindi si aspetta un valore da 1 a 9
                                //e non da 0 a 9 come ragionano gli array
        }

    }
    sleep(2);

};


void scollegamento(void *address[],int len){

    for(int i = 0; i < len; i++){
        if(shmdt(address[i]) == -1){
            perror("errore scollegamento");
            printf("scollegato %p\n",address[i]);
        }
    }



}

void liberazione(int memoryid[], int len){

    semctl(memoryid[0], 0, IPC_RMID, NULL);
    for(int i = 1; i < len; i++){
        if(shmctl(memoryid[i], IPC_RMID,NULL) == -1){
            perror("ERRORE CANCELLAZIONE");
            printf("liberata memoria: %i\n",memoryid[i]);
        }
    }


}

