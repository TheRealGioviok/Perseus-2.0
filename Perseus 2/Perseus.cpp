// Perseus.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#define debug false
#include "chessBoard.h"
#include "uci.h"
#include "evaluation.h"
int main()
{
    //init tables
    initAll();
    //uci Loop
    if (debug) {
        //debug code
        std::cout << "DEBUG MODE\n";
        moves moveList;
        Game game("K6k/4n3/2NbRp2/p3r3/1qRbQp2/PpBnBb1r/PnPrPpP1/8 w - - 0 1");
        game.generateMoves(&moveList);
        
        printMoveList(&moveList);
    }
    else {
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
