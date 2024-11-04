/************************************
* VR472547
* Alex Merlin
* 10/06/24 - 09/07/24
*************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "../head/fun.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>


pid_t p1_pid = 0;
pid_t p2_pid = 0;
pid_t server_pid;
int time = 0;
int memoryid[15];
void *address[15];
int len_arrayid = 0;
int len_arrayadd = 0;
struct player *p1;
struct player *p2;


union semun{ 
	
    int val;
	struct semid_ds * buf;
	unsigned short * array;
};

void kill_all(int sig){
    
    printf("Client uccisi\n");
        
        if(p1_pid == 0 && p2_pid == 0){
            scollegamento(address,len_arrayadd);
            liberazione(memoryid,len_arrayid);
        }else if(p1_pid != 0 && p2_pid == 0){
            kill(p1_pid,SIGUSR1);
            scollegamento(address,len_arrayadd);
            liberazione(memoryid,len_arrayid);    
        }else{   
            kill(p1_pid,SIGUSR1);
            kill(p2_pid,SIGUSR1);
            scollegamento(address,len_arrayadd);
            liberazione(memoryid,len_arrayid);
        }    
    
    
    exit(1);
}

void mod_ctrl_c(int sig){

    printf("premere una seconda volta ctrl+c\n");
    time = time + 1;
    printf("time :%i",time);
    if(time == 1 ){
        signal(2,kill_all);
    }
    return;
};


void kill_lose(int sig){


    if(p1->win == -3)
        kill(p2->proc, SIGUSR2);
    else if(p2->win == -3){
        kill(p1->proc,SIGUSR2);
    }


    scollegamento(address, len_arrayadd);
    liberazione(memoryid,len_arrayid);
    printf("Il gioco è terminato per abbandono di un giocatore\n");
    exit(1);

}

void close_terminal(int sig){
         
         kill(p1_pid,SIGUSR1);
         kill(p2_pid,SIGUSR1);
         
         scollegamento(address,len_arrayadd);
         liberazione(memoryid,len_arrayid);
         
         exit(1);

}

int main(int argc, char *argv[]){
    
    if(argc < 4){
        printf("Hai avviato il gioco nel modo sbagliato\n");
        printf("Esempio di come avviare il programma: \n./TriServer 10 X O\n");
        printf("dove 10 è tempo che il giocatore ha per effetturare una mossa, inserire 0 per disattivare il timer e 'O' e 'X' sono i simboli assegnati ai rispettivi giocatori\n");
        exit(-1);
    }



    char (*matrix)[3]; //creo un puntatore ad un array di 3 elementi, sarà il puntatore alla matrice 
    union semun info;  
    unsigned short start_val[3] = {0,0,0}; //valori iniziali del semaforo
    int result;
    info.array = start_val;
    signal(2,mod_ctrl_c);
    signal(12,kill_lose);
    signal(1,close_terminal);
    
    
    int semid = semget(100, 3, IPC_CREAT| 0666);
    semctl(semid,0, SETALL, info);
    memoryid[len_arrayid++] = semid;
    //creao memoria condivisa 3*3 e salvo l'indirizzo in matrix;


    int shmid = shmget(99, sizeof(char[3][3]), IPC_CREAT | 0666);
    if(shmid == -1){
        erExit("shmget");
    }
    matrix = (char(*)[3])shmat(shmid, NULL, 0666);
    memoryid[len_arrayid++] = shmid;
    address[len_arrayadd++] = matrix;
    init_matrix(matrix);

    
    //Creazione memoria condivisa mossa
    int shm_mossa = shmget(110, sizeof(int), IPC_CREAT | 0666);
    if(shm_mossa == -1){
        erExit("shmget");
    }
    int *mossa = (int*)(shmat(shm_mossa,NULL,0666));

    memoryid[len_arrayid++]=shm_mossa;
    address[len_arrayadd++]=mossa;

    int shm_count_move = shmget(130, sizeof(int), IPC_CREAT | 0666);
    if(shm_count_move == -1){
        erExit("shmget");
    }
    int *count_move = shmat(shm_count_move, NULL, 0666);
    *count_move = 0;
    memoryid[len_arrayid++]=shm_count_move;
    address[len_arrayadd++]=count_move;

    print_board(matrix);
    //creo 3 semafori e li inizializzo a {0,0,0}

    //Creo Stringa condivisa con terminale del giocatore 1
    int shmid_term1 = shmget(103,sizeof(char[1024]), IPC_CREAT | 0666);
    char *term1 = (char*)shmat(shmid_term1,NULL,0666);
    memoryid[len_arrayid++]=shmid_term1;
    address[len_arrayadd++]=term1;
    //Creo Stringa condivisa con terminale giocatore 2
    int shmid_term2 = shmget(104,sizeof(char[1024]), IPC_CREAT | 0666);
    char *term2 = (char*)shmat(shmid_term2,NULL,0666);
    memoryid[len_arrayid++]=shmid_term2;
    address[len_arrayadd++]=term2;

    //memoria condivisa per nome giocatore
    int shmid_p1 = shmget(101,sizeof(struct player), IPC_CREAT | 0666); 
    if(shmid_p1 == -1)
        erExit("shmget error 1"); 
    memoryid[len_arrayid++]=shmid_p1;

    printf("La matrice è stata creata attendo il primo giocatore\n");

    //La prima volta che viene eseguito il server il player1 ha lunghezza del nome = 0.
    p1 = (struct player*)shmat(shmid_p1,NULL,0666);
    address[len_arrayadd++]=p1;
    strcpy(p1->name, "\0");
    p1->simbol = argv[2][0];
    p1->timer = atoi(argv[1]);
    p1->pid_server = getpid();
    //Provo a decrementare il semaforo, ma non posso devo prima incrementarlo dal client
    struct sembuf op = {.sem_num = 0, .sem_op = -1};
    
    do{    
        result = semop(semid, &op, 1);
    }while(result == -1 && errno == EINTR);

    
    sprintf(term1,"%s ti sei unito alla partita e giocherai con il simbolo (%c).\nAttendi il secondo giocatore\n ",p1->name,p1->simbol);
    printf("%s", term1);
    p1_pid = p1->proc;
    op.sem_num = 1; op.sem_op = 1;
    if(semop(semid, &op, 1))
        perror("semop error riga 46");
    
    int shmid_p2 = shmget(102,sizeof(struct player), IPC_CREAT | 0666);
    p2 = (struct player*)shmat(shmid_p2,NULL,0666);
    memoryid[len_arrayid++]=shmid_p2;
    address[len_arrayadd++]=p2;

    p2->bot[0] = 'n';
    p2->simbol = argv[3][0];
    p2->timer = atoi(argv[1]);
    p2->pid_server = getpid();
    //se non funziona si inserisce normalmente il secondo giocatore
    if(p1->bot[0]== '*'){
        if(fork() == 0){
            char computer[] = "Computer\n";
            printf("AVVIO IL BOT");
            p2->bot[0]='s';
            execl("TriClient", "TriClient", computer, (char*)NULL);
            printf("NON HA FUNZIONATO");
            exit(-1);
        }
    
    }
    
    
    op.sem_num = 2; op.sem_op = -1;
    do{
        result = semop(semid, &op, 1);
    }while(result == -1 && errno == EINTR);
    
    
    sprintf(term2,"%s ti sei unito alla partita. Giochi con il simbolo (%c).\nGiochi contro %s con il simbolo(%c)",p2->name, p2->simbol, p1->name,p1->simbol);
    sprintf(term1,"\n%s si unito alla partita e giocherà con il simbolo (%c)",p2->name,p2->simbol);
    printf("%s", term2);
    strcpy(p1->opponent,p2->name);
    strcpy(p2->opponent,p1->name);

   
    p2_pid = p2->proc;
    server_pid = getpid();

    op.sem_num = 1; op.sem_op = 1;
    if(semop(semid, &op, 1))
       perror("semop error");

    operazione(&op, 0, 2);
        if(semop(semid, &op, 1))
            perror("semop error");


    int p_turn1;
    int p_turn0;
    char simb = p1->simbol;
    int check;
    int win;
    //int count_move = 0; //conteggio mosse per pareggio
    while(1){
        
        p_turn1 = (p1->turn == 1) ? p1->num : p2->num;
        p_turn0 = (p1->turn == 0) ? p1->num : p2->num;
        
        time = 0;   //resetto il numero di volte che premo ctrl+c
        signal(2,mod_ctrl_c); //se premo ctrl+C 2 volte termino i giochi

        //supero semaforo giocatore con turn a 0(chi aspetta)
        operazione(&op,p_turn0,1);
        if(semop(semid, &op, 1))
            perror("semop error");

        //semaforo che si sblocca quando riceve la mossa effettuata 
        operazione(&op,p_turn1,-1);   
        do{
            result = semop(semid, &op, 1);
        }while(result == -1 && errno == EINTR);    

        p1->win = 0;   
        p2->win = 0;
        
        //controlliamo la mossa inserita
        //salvaimo il simbolo di chi detiene il turno
        simb = (p_turn1 == 1) ? p1->simbol : p2->simbol;
        //salviamp la mossa. Si occupa di contare anche il numero di mosse effettuate per vedere il pareggio
        //save_move(matrix,mossa,simb,&count_move);

        //check matrice per vedere se qualcuno ha vinto;
        //win = -1 pareggio
        //win = 1 vittoria
        //win = 0 continua il gioco
        if((win = check_win(matrix, (p_turn1 == 1)? p1 : p2, *count_move)) == 1){
            printf("ha vinto player %i", p_turn1);
            p1->win = (p_turn1 == 1)? 1 : -1;
            p2->win = (p_turn1 == 2)? 1 : -1;
        }else if(win == -1){
            p1->win = -2;
            p2->win = -2;    
        }
               
        //la mossa è stata inserita correttamente sblocca il semaforo per continuare
        operazione(&op,p_turn0,1);
        if(semop(semid, &op, 1))
            perror("semop error");  

        operazione(&op,p_turn1,1);
        if(semop(semid, &op, 1))
            perror("semop error");    

    
        
        if(p1->win == 1 || p2->win == 1){
             
            *count_move = 0;
            //mi blocco fino a che il giocatore vincente non avrà preso una decisione
            printf("ATTENDO DECISIONE GIOCATORE VINCENTE\n");
            operazione(&op,p_turn1,-1);
            if(semop(semid, &op, 1))
                perror("semop error");
            
            //carico la scelta del client vincente al client perdente
            if(p1->win == 1){
                term2[0] = term1[0];
            }else if(p2->win == 1){
                term1[0] = term2[0];
            }

            //decisione presa sblocco il perdente
            operazione(&op,p_turn0,1);
            if(semop(semid, &op, 1))
                perror("semop error");
        
            //se term turn1 = n dealloca tutto e termina altrimenti continua con una nuova partita e segna un punto;
            if(term1[0] == 'n'){
                scollegamento(address,len_arrayadd);
                liberazione(memoryid,len_arrayid);                //elimina memoria
                exit(1);
            }

           init_matrix(matrix); 
           operazione(&op,p_turn0,-1);
            if(semop(semid, &op, 1))
                perror("semop error");
        }else if(p1->win == -2 || p2->win == -2){
            init_matrix(matrix); 
            *count_move = 0;
        }
            //se scelto si si resetta il gioco e si rinizia a giocare assegnando un punto al vincitore, 
            //altrimenti stiamo giocando normalmente
                
            operazione(&op,0,2);
            if(semop(semid, &op, 1))
                perror("semop error");

    }
    return 0;
}

