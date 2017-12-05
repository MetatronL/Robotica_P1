# Robotica_P1

Documentatie:

Proiect: Snake

	Principiile jocului
		-snake merge mereu in ultima directie primita din joystick
		-snake va "manca" orice mar intalnit , crescandui lungimea
		-pozitia mancarii va fi mereu randomizata , fara a se intersecta cu snake
		-daca snake isi atinge un block din corpul sau , ai pierdut jocului
		-snake poate sa treaca pe langa o alta portiune a corpului sau atat timp cat noul patratel in care isi muta capul nu ii apartine corpului
		-Pe easy:
			-se pooate trece prin bordura( din dreaptam ecranului apare in stanga, si viceversa)
		-Pe medium/Hard
			-daca atinge bordura -> game over
		-Se poate pune pauza apasand pe butonul joystick-ului


	Descriere hardware
		- matrice pentru snake + mancare
		- lcd pentru scor / descriere dificultati
		- potentiometru pentru luminozitate lcd
		- micro-controler -> matrice
		- joystick
	
	Descriere algorithm
		- folosesc o clasa pentru coada "queue"
			-tine minte pozitiile snake-ului
		- folosesc o clasa pentru game cu functiile generale si scor
		- folosesc o clasa pentru snake , derivata din game si queue
		- in loop
			- verific daca e apasat butonul de pauza
				- pornesc/opresc pauza conform 
			- folosesc o variabila in care tin starea jocului
			- un switch pentru stare
				- la inceput scriu pe lcd detaliile despre dificultate, si astept input corespunzator
					- pornesc snake-ul
				- Daca snake e pornit
					- continui jocului
		
		- cand incep snake
			- setez snake-ul initial de 3 blocuri orizontale 
			- setez un mar randomizat
			
		- cand continui snake
			- updatez inputurile
			- calculez directia
			- verific daca e timpul de updatez
			-daca da
				- noua pozitie goala 
					-> adaug noua pozitie in coada
					-> sterg cea mai veche pozitie din coada
				- noua pozitie este un mar
					-> generez un amr noua
				- noua pozitie deja snake
					-> game over 
			
				