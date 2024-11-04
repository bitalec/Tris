/************************************
* VR472547
* Alex Merlin
* 10/06/24 - 09/07/24
*************************************/

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "../head/fun.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

struct player *p;
int shmid_p;
void *address[10];
int len_arrayadd = 0;


void mod_allarm(int sig){
        scollegamento(address,len_arrayadd);
        exit(2);

};

void kill_client(int sig){

    printf("\33c\e[3J");
    printf("Client ucciso dal server\n");
    if(p->turn == 1)
        kill(p->proc_figlio,SIGALRM);
        //kill(p->proc_figlio,SIGKILL);
    
    scollegamento(address,len_arrayadd);
    

    exit(1);

};

void kill_lose(int sig){
        printf("\33c\e[3J");
        printf("HAI PERSO PER ABBANDONO\n");
        p->win = -3;
        if(p->turn == 1){
            kill(p->proc_figlio,SIGALRM);
            //kill(p->proc_figlio, SIGKILL);
        }
        kill(p->pid_server, SIGUSR2);
        scollegamento(address,len_arrayadd);

        exit(1);
};

void win_abandonment(int sig){
        printf("\33c\e[3J");
        printf("HAI VINTO PER ABBANDONO DELL' AVVERSARIO, CLIENT CHIUSO");
        scollegamento(address,len_arrayadd);
        exit(1);

};

void close_terminal(int sig){
         scollegamento(address,len_arrayadd);
         exit(1);

}


union semun{ 
	
    int val;
	struct semid_ds * buf;
	unsigned short * array;
};


int main(int argc, char *argv[]){

    signal(14, mod_allarm);
    signal(10,kill_client);
    signal(2, kill_lose);
    signal(1, kill_lose); //per chiusura terminale
    signal(12, win_abandonment);
    union semun info;
    unsigned short start_val[3] = {0,0,0};
    info.array = start_val;

    int shmid_term;
    char *term;

    char (*matrix)[3];
    int shm_matrix = shmget(99, sizeof(char[3][3]), 0666);
    if(shm_matrix == -1){
        erExit("shmget matrice. Avviare prima il server");
    }
    matrix = (char(*)[3])shmat(shm_matrix, NULL, 0666);
    address[len_arrayadd++] = matrix;


    int shm_mossa = shmget(110, sizeof(int), 0666);
    if(shm_mossa == -1){
        erExit("shmget mossa");
    }
    int *mossa = (int*)shmat(shm_mossa,NULL,0666);
    address[len_arrayadd++] = mossa;


     int shm_count_move = shmget(130, sizeof(int),0666);
    if(shm_count_move == -1){
        erExit("shmget");
    }
    int *count_move = shmat(shm_count_move, NULL, 0666);
    address[len_arrayadd++] = count_move;
    //apertura semaforo creato dal server
    int semid = semget(100, 0 , 0666);
    if(semid == -1)
        perror("semget");


    //Mi attacco alla memoria condivisa creata dal server per salvare i dati del player 1
    shmid_p = shmget(101,sizeof(struct player),0666);
    p = (struct player *)shmat(shmid_p,NULL,0666);
    if(p == (struct player *)-1)
        perror("MEMORIA NON COLLEGATA");
        
    
    //Se il nome ha lunghezza maggiore di zero allora il primo giocatore è già stato inizializzato, Serve quindi attaccarsi alla memoria del player 2
    //Ci colleghiamo anche alla stringa condivisa per il server del player 2
    if(strlen(p->name) !=  0){
        
        shmdt(p); //scollego il primo player
        shmid_p = shmget(102,sizeof(struct player),0666);
        p = (struct player *)shmat(shmid_p,NULL,0666);

        shmid_term = shmget(104,sizeof(char[1024]),0666);
        term = (char*)shmat(shmid_term,NULL,0666);
        p->num = 2; //giocatore 2
        p->turn = 0;
        
    }else{

        p->turn = 1;
        p->num = 1; //giocatore 1;
        shmid_term = shmget(103,sizeof(char[1024]),0666);
        term = (char*)shmat(shmid_term,NULL,0666);
        if(argc == 3){
            p->bot[0] = argv[2][0];
        }else{
            p->bot[0] = 'n';
        }
       
    }
    address[len_arrayadd++] = p;
    p->win = 0;
    p->proc = getpid();
    
    /*Da questo momento in poi struct player p farà rifermento sempre solo al primo giocatore (se è stato avviato il client per la prima volta) o al secondo */
    
    //copio il nome del giocatore
    strcpy(p->name, argv[1]);

    struct sembuf op;
    //sblocca il semaforo del server per chiamare un nuovo giocatore
    if(p->num == 2){
        op.sem_num = 2, op.sem_op = 1;
        if(semop(semid, &op, 1))
            perror("semop error");    
    }else{

    op.sem_num = 0, op.sem_op = 1;
    if(semop(semid, &op, 1))
        perror("semop error");
    }
    //Mi fermo fino a che il nome del giocatore non verrà salvato nella memoria condivisa dopo di che stampo
    operazione(&op, 1, -1);
    //op.sem_num = 1; op.sem_op = -1;
    if(semop(semid, &op, 1))
        perror("semop error");
    
    if(p->num == 1)
        printf("%s\n ", term);
    //Cerco di decrementare il semaforo del server che si sbloccherà quando il server avrà caricato le stringhe di inizio partita in memoria
    operazione(&op,0,-1);
    if(semop(semid, &op, 1))
        perror("semop error");
    
    printf("%s, il gioco inizierà tra pochi secondi \n", term); 

    int num_p;
    int status;
    pid_t pid;
    char continua_gioco;
    *mossa = 0;
    sleep(4);
    
    while(1){
        num_p = p->num;
        
        
        //Se è il turno del giocatore avremo turn = 1, altrimenti 0;
        switch (p->turn)
        {
        case 1:
            pid = fork();
            if(pid == 0){
                printf("\33c\e[3J"); //\033[3J\033[H\033[2J più corretto
                p->proc_figlio = getpid();
                if(*mossa == -1)
                    printf("TEMPO DELL'AVVERSARIO SCADUTO");
                if(*mossa == -2||*mossa == -3)
                    printf("L'avversario ha selezionato una casella non valida\n");
                print_board(matrix);
                alarm(p->timer);
                printf("E il tuo turno.Scrivi la tua mossa(%c):", p->simbol);
                if(p->bot[0] == 's'){
                    random_move(mossa,matrix);
                }else{
                    scanf("%i", mossa);
                }
                alarm(0);
                scollegamento(address,len_arrayadd);
                printf("hai concluso il turno");
                exit(1);

            }else{
                wait(&status);
                //esaminiamo status e ricaviamo il codice di uscita di exit figlio
                //Se uguale a 2 allora avvisiamo il sever che non abbiamo fatto nessuna mossa
                if(WEXITSTATUS(status) == 2){
                   *mossa = -1;
                   
                   //printf("\33c\e[3J");
                }
                save_move(matrix,mossa,p->simbol,count_move);
            }
            
            operazione(&op,num_p,1);
                if(semop(semid, &op, 1))
                    perror("semop error");
            
            break;
        
        case 0:
            printf("\33c\e[3J");
            if(WEXITSTATUS(status) == 2)
                printf("TEMPO SCADUTO HAI CEDUTO IL TURNO");
            if(*mossa == -2){
                printf("POSIZIONE GIà OCCUPATA\n");
            }else if(*mossa == -3){
                printf("mossa non valida, il prossimo turno seleziona una casella libera da uno a nove\n");
            }
            print_board(matrix);
            printf("Attendi la mossa di %s\n", p->opponent);
            

            operazione(&op,num_p,-1);
             if(semop(semid, &op, 1))
              //  perror("semop error");
            //printf("\33c\e[3J");
            break;
        }   
        
        
            
        p->turn = (p->turn == 1)? 0 : 1;
        operazione(&op,p->num,-1);
        if(semop(semid, &op, 1))
            perror("semop error");
            
        
        //Se c'è, il vincitore entra nel primo if altrimenti nell'else if
        if(p->win == 1){
            
            printf("033[3J\033[H\033[2J");
            print_board(matrix);
            printf("Hai vinto la partita vuoi continuare?\n");
            if(p->bot[0] == 's'){
                sleep(3);
                term[0] = 's';
            }
            else{     
                //fflush(stdin);
                scanf(" %c", &term[0]);
            }
            operazione(&op,p->num,1);

            if(semop(semid, &op, 1))
                perror("semop error win");
            if(term[0] == 'n'){
                printf("Hai scelto di non continuare la partita chiusura del client");
                scollegamento(address,len_arrayadd);
                exit(1);
            }
            printf("Hai scelto di continuare la partita, la partita sta per riniziare\n");
            sleep(2);
        }else if(p->win == -1){
            printf("033[3J\033[H\033[2J");
            print_board(matrix);
            printf("Hai perso la partita attendi la decisione dell'avversario\n");
            operazione(&op,p->num,-1);
                if(semop(semid, &op, 1))
                perror("semop error lose");
            
            if(term[0] == 'n'){
                operazione(&op,p->num,1);
                if(semop(semid, &op, 1))
                    perror("semop error lose");
                printf("L'avversario ha scelto di non continuare chiusura del client");
                scollegamento(address,len_arrayadd);
                exit(1);
            }else{
                //si protrebbe mettere il nome dell'avversario
                printf("L'avversario ha scelto di continuare la partita, rinizierà tra pochi secondi\n");
                sleep(2);
                operazione(&op,p->num,1);
                if(semop(semid, &op, 1))
                    perror("semop error lose");
            }
                
        }else if(p->win == -2){

            printf("033[3J\033[H\033[2J");
            print_board(matrix);
            printf("Avete pareggiato La partita sta per riniziare\n;");
            sleep(2);


        }
        
        
        operazione(&op,0,-1);
        if(semop(semid, &op, 1))
            perror("semop error");

        
    }

    

    //shmctl(shmid_term, IPC_RMID,NULL);
    //shmctl(shmid_p, IPC_RMID,NULL);
    //semctl(semid, 0, IPC_RMID, NULL);
    return 0;
}
