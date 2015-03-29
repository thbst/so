Tudorica Constantin-Alexandru, 333CA

	Am folosit scheletul furnizat.
	Singurele cazuri mai speciale fata de linux au fost cele cu rularea
in paralel si cu pipe. La rularea in paralel creez un nou thread ce va
procesa acea jumatate din arborele de rulare.
	La pipe-uri se complica putin. Fiecare process_command primeste 2
pointeri catre Handle-uri de input si respectiv de output. Daca acestia
sunt diferiti de NULL inseamna ca vor trebui folositi in acel subarbore.
	Pentru a porni in paralel procesel o sa pornesc procesele fara a mai
astepta imediat dupa ele si o sa le pun Process Handle-ul intr-un vector
de handle-uri. La terminarea programului o sa fac WaitMultipleObjects pe
tot vectorul respectiv pana nu mai ramane nici un proces.
	In momentul cand o sa am thread-uri o sa creez cate un nou vector
pentru a nu avea probleme cu accesul concomitent la memorie.
	Fiecare thread va avea grija sa wait-uie dupa procesele create in
cadrul sau.
	Cam asta e tot ce mai special in cod.