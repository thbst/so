Tudorica Constantin-Alexandru, 333CA
	
	Am implementat tema folosind mailslots deoarece mi se pareau ca se potriveau
foarte bine pe ceea ce trebuie facut. M-am folosit mult de codul scris pe wiki
in laboratorul 5 si de codul de la versiunea de linux. Am avut mai putine
probleme, doar testul 9 apela executabilul de test fara a adauga extensia .exe
si imi dadea eroare la CreateProcess, am regandit un pic metoda de trimitere a
parametrilor catre procese si a mers.
	Am creat ca si pe linux mailbox-urile in mpirun si apoi am trimis handle-ul
catre procesle copil prin parametru, aveam probleme daca il deschideam in
MPI_Init fiind ca procesele incercau sa scrie in mailslot inainte ca acesta sa
fie creat (se crea un race condition).