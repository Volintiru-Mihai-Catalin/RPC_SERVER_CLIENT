		Tema 1 SPRC

	Volintiru Mihai Catalin 343 C3



	Pentru inceput, voi pleca de la implementarea interfetei, astfel ca
structurile folosite de mine sunt urmatoarele: 
	-> struct token_pair (folosit pentru a putea intoarce de la server la
client tokenii pe care cel din urma ii genereaza)
	-> struct resource (folosit pentru a putea stoca informatiile despre
resurse si permisiunile acestora)
	-> struct user (folosit pentru a putea contura baza de date cu informatiile
necesare despre fiecare user)
	-> struct status (folosit pentru a putea intoarce statusul unei comenzi de
tip validate, dar si tokenii refreshuiti, in cazul in care clientul are setata
reinnoirea automata a tokenilor de acces si refresh)
	De mentionat ca pentru a genera fisierele pe baza interfetei .x am ales 
urmatoarele flaguri: -NC. "N" pentru a putea introduce mai multi parametrii
intr-o functie si "C" pentru a genera output ANSI C.
	
	Cat despre requesturi, acestea intorc fie stringuri, fie structuri astfel:
	-> REQUEST_AUTHORIZATION primeste userul si intoarce tokenu de acces la
resurse
	-> APPROVE_REQUEST primeste tokenul de access la resurse si il "semneaza"
trimitand inapoi catre client permisiunile asociate acelui token, indiferent
daca ele sunt valide sau nu
	-> REQUEST_ACCESS_TOKEN primeste username-ul, tokenul de autorizare 
si informatie cu privire la regenerarea automata a tokenului si intoarce, pe
baza permisiunilor, tokenul de acces la resurse si tokenul de regenerare. In
cazul in care permisiunile nu sunt valide ( *,- ), acesta intoarce eroarea
REQUEST_DENIED. Practic, userul "semneaza" tokenul intorcand efectiv
permisiunile. Daca userul intoarce *,- inseamna ca tokenul nu a fost "semnat",
iar in caz contrar, daca intoarce efectiv permisiunile, inseamna ca tokenul a
fost "semnat".
	-> VALIDATE_DELEGATED_ACTION primeste o actiune, resursa asupra careia se
va aplica actiunea, username-ul si verifica pe rand: daca are tokenul de acces
la resurse valid (caz contrar intoarce PERMISSION_DENIED), daca tokenul nu a 
expirat (daca a expirat se uita daca optiunea de refresh automata este setata
daca nu, intoarce o eroare, in caz contrar regenereaza tokenii de access la 
resurse si tokenul de regenerare), daca resursa este valida (daca nu e,
intoarce eroarea RESOURCE_NOT_FOUND), daca are permisiuni pentru resursa
respectiva (daca nu, intoarce OPERATION_NOT_PERMITTED), iar in final, daca
totul a fost ok, intoarce PERMISSION GRANTED.

	Pentru aboradrea temei, am ales sa ma foloses de cozi, pentru a putea oferi
serverului permisiunile date de utilizator, respectiv pentru a putea oferi 
clientului comenzile pe care trebuie sa le execute.
	Foarte important, in main-ul stub-ului server fac citirea din fisiere a 
userilor, a resurselor si a permisiunilor si alcatuiesc "baza de date" a 
serverului, adica vecotrul "users_list". Toate acestea, alaturi de cateva
functii pentru operatii pe cozi, sunt expuse cu ajutorul keyword-ului "extern"
serverului pentru a le putea accesa.
	Clientul are si el o mini "baza de date" unde isi tine userii si tokenii
de acces corespunzator fiecaruia pentru a putea face requesturi de tip
VALIDATE_DELEGATED_ACTION catre server fara a trimite numele clientului.
Daca clientul executa o actiune de tip REQUEST, face pe rand aceste apeluri
catre server: REQUEST_AUTHORIZATION, REQUEST_AUTHORIZATION, 
VALIDATE_DELEGATED_ACTION, urmand sa actualizeze baza de date cu informatiile
primite.

	Pentru generarea de output, a trebuit sa folosesc o solutie pe care am
gasit-o pe forum, pentru ca serverul nu imi genera output-ul necesar in
fisierul server.out. Comanda este "setbuf(stdout, NULL);", dupa ce am 
folosit-o, totul a fost ok.

	FOARTE IMPORTANT, stub-ul server nu trebuie modificat, deci nu trebuie
regenerat. In caz contrar, se pierd functionalitatile implementate. Comenzile
din Makefile care nu ar impacta structura temei sunt "make server" care
genereaza executabilul aferent serverului si "make client" care face acelasi
lucru, dar pentru client. De asemenea, si "make clean" este o comanda fara
risc, deoarece sterge cele 2 executabile mentionate mai sus.