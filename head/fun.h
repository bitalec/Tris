/************************************
* VR472547
* Alex Merlin
* 10/06/24 - 09/07/24
*************************************/

#ifndef _FUN_H
#define _FUN_H

#define COL 3
#define ROW 3
#include <sys/sem.h>


struct player{

    char name[20];
    char opponent[20];
    short num;
    short turn;
    char simbol;
    int point;
    int timer;
    int win;
    pid_t proc;
    pid_t proc_figlio;
    char bot[2];
    pid_t pid_server;
    

};

//restituisce l'errore per il quale si è chiuso il processo
void erExit(const char *msg);
//stampa la matrice di gioco
void print_board(char matrice[ROW][COL]);
//salva la mossa dell' giocatore nella matrice
void save_move(char (*matrix)[3],int *mossa, char simbol,int *count);
//inizializza la matrice con tutti ' '
void init_matrix(char matrix[3][3]);
//controlla se hai vinto
int check_win(char matrix[3][3], struct player *p,int count);
//mossa causale in caso di gioco contro pc
void random_move(int *mossa, char (*matrix)[3]);
//scollega gli indirizzi logici dalla memoria fisica
void scollegamento(void *address[],int len);
//libera la memoria allocata ATTENZIONE per il solo primo elemento dell'array viene deallocato un semaforo
void liberazione(int memoryid[], int len);
//operazione da dare in pasto a semop
void operazione(struct sembuf *op, int numsem, int oper);

#endif
