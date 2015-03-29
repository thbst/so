Tudorica Constantin-Alexandru, 333CA
	
	Am implementat tema folosind posix queues, mi s-au parut cele mai potrivite
pentru acesta tema deoarece implemntau deja o mare parte din functionalitatea
ceruta (erau blocante si erau cozi :) ). Pentru a trimite informatii despre
enviroment (rank si size) am trimis parametrii suprascriind variabilele pasate
catre executabil in modul urmatoru:
	Primul parametru era de forma m%d si reprezenta contextul, al doilea
parametru era un numar si reprezenta rank-ul procesului, iar al treilea era tot
numar si reprezenta dimensiunea topolgiei.
	Pe cozi trimit o structura definita de mine numita mpi_message ce contine un
header cu datele necesare si un buffer de aproximativ 8k de date ce pot fi 
trimise pe coada (mai mult decat se trimite in teste).
	Cam asta e tot ce e mai special din ce ma facut in tema, am stat mult sa
debugui faptul ca imi suprascriam cu bufferul de date alti parametrii ai
structurii (aparent pe valgrind nu il deranja asta). 