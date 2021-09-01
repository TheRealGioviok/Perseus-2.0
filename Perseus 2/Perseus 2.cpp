// Perseus 2.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//
#include <iostream>
#define debug false
#include "chessBoard.h"
#include "uci.h"
#include "evaluation.h"

#include <atomic>

tt * hashTable;

int main()
{
    
    if (false) {
        initAll();
        Game g(startPosition);
        
        for (int i = 1; i < 8; i++) {
            perftDriver(i, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");
        }


        moves* moveList = new moves();
        g.generateMoves(moveList);
        printMoveList(moveList);

        return 0;
    }

    else {
        //init tables
        initAll();
        //uci Loop
        hashTable = new tt[hashSize];
        for (int i = 0; i < hashSize; i++) {
            hashTable[i] = tt();
        }
        wipeTT();
        uciLoop();
    }
    // terminating succesfully
    return 0;
}

// Per eseguire il programma: CTRL+F5 oppure Debug > Avvia senza eseguire debug
// Per eseguire il debug del programma: F5 oppure Debug > Avvia debug

// Suggerimenti per iniziare: 
//   1. Usare la finestra Esplora soluzioni per aggiungere/gestire i file
//   2. Usare la finestra Team Explorer per connettersi al controllo del codice sorgente
//   3. Usare la finestra di output per visualizzare l'output di compilazione e altri messaggi
//   4. Usare la finestra Elenco errori per visualizzare gli errori
//   5. Passare a Progetto > Aggiungi nuovo elemento per creare nuovi file di codice oppure a Progetto > Aggiungi elemento esistente per aggiungere file di codice esistenti al progetto
//   6. Per aprire di nuovo questo progetto in futuro, passare a File > Apri > Progetto e selezionare il file con estensione sln
