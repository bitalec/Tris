# Progetto S.O. - Gioco Multiplayer su Terminale
**Studente**: Alex Merlin

## Descrizione del Progetto

Questo progetto implementa un gioco multiplayer a turni da terminale, sviluppato come parte del corso di Sistemi Operativi. Il gioco permette a due giocatori di competere seguendo regole specifiche, con gestione dei turni e delle mosse attraverso variabili in memoria condivisa e semafori.

## Funzionalità Principali

- **Comandi di Esecuzione**: I file eseguibili sono generati nella cartella `bin` tramite il comando `make`.
- **Modalità di Gioco**:
  - Le mosse sono rappresentate da coordinate numeriche (1-9).
  - Un timer limita il tempo di ciascun giocatore per effettuare la mossa; in caso di timeout, il turno passa all'avversario.
  - In caso di pareggio, la partita viene riavviata automaticamente.
  - Il vincitore può decidere se continuare o terminare.
  - La chiusura del terminale da parte di un giocatore comporta la perdita per abbandono.
  - Per il client automatico, il timer deve essere maggiore di 5 secondi per una migliore esperienza di gioco.

## Struttura del Codice e Scelte Implementative

- **Memoria Condivisa e Semafori**:  
  - Le variabili di gioco, come il contatore delle mosse e la matrice di gioco, sono salvate in memoria condivisa.
  - Sono presenti variabili per la comunicazione con i giocatori e la gestione delle informazioni di ciascun partecipante.

- **Gestione della Memoria Condivisa**:  
  - Ogni variabile condivisa è memorizzata in due array globali, permettendo di liberare facilmente la memoria al termine del gioco.

- **Inserimento della Mossa**:
  - Ogni turno attiva un timer tramite un processo figlio che gestisce la scadenza del tempo. In caso di scadenza, il turno passa all’avversario.
  - Le mosse non valide sono indicate con valori sentinella:
    - `-2`: Posizione già occupata.
    - `-3`: Valore fuori dal range consentito (1-9).

## Requisiti di Sistema

- **Librerie e Dipendenze**: Compilazione e gestione delle variabili di memoria condivisa richiedono un sistema POSIX-compliant per l'uso di semafori e IPC.
