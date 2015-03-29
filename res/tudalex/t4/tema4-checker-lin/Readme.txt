Tudorica Constantin-Alexandru, 333CA

	Am facut conform indicatiilor din enunt. Mi s-a parut ca in enunt s-au dat
destul de multe indicatii astfel incat ce trebuia facut era destul de mura-n 
gura. Ca mecanism de IPC am folosit cate un semafor pentru fiecare thread,
fiecare thread transferand mai departe "stafeta" catre alt thread si blocandu-se
el.
Probleme de care m-am lovit:
	1) Incurcam semafoarele cand incercam sa schimb thread-urile si faceam wait
pe semaforul pe care nu trebuia
	2) Fiind ca consider primul so_fork ca instructiune cand nu aveam nici un
thread creat si cuanta de timp setata prost se ducea preemptarea.
	3) La terminarea unui thread nu preemptam fara sa mai blochez si ramaneau
thread-uri blocate.