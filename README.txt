Fase 2 del BIKAYA Project, Corso di Sistemi Operativi @Unibo 2019/2020
Federico De Marchi, Stefano Sgarzi, Manuel Vannini

In questa fase viene completato lo strato del kernel del SO Bikaya per il progetto di sistemi operativi.
Nel dettaglio sono implementate tutte le SYSCALL da 1 a 8 e sono gestiti gli interrupt di tutti i device esterni.
Lo scheduler resta praticamente invariato rispetto alla fase 1.5 precedente, con l'unica differenza che aiuta a tenere conto del
tempo di esecuzione di un processo e che gestisce lo svuotamento della ready queue entrando in idle state con interrupt abilitati.
La gestione delle rimanenti eccezioni TLB e PGMTRAP è rilegata all'uso della SYSCALL Spec_Passup e quindi non sono direttamente implementate.


#COME COMPILARE#
Il codice viene compilato utilizzando SCONS 3.1.2 (https://scons.org/).
Per la compilazione è necessario avere Python 2.7 o 3.x e la rispettiva libreria SCons, installabile tramite pip installer con 'pip install scons' da terminale o scaricabile su https://scons.org/pages/download.html . Necessari inoltre i pacchetti gcc-arm-none-eabi per la compilazione uARM e gcc-mipsel-linux-gnu per la compilazione uMPS.
La compilazione avviene con il comando 'scons TARGET' nella directory principale, dove TARGET è uarm o umps / umps2. Per pulire i file per un target si aggiunge la flag -c ('scons TARGET -c').
Compilare prima per un target e successivamente per un altro non richiede di pulire i file. Importante che venga mantenuta la gerarchia di directory perché il file SConstruct nella directory superiore si appoggia a dei file SConscript nelle directory sottostanti.
