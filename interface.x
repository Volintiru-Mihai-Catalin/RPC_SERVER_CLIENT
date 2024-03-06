/* O structura care contine numele unei resurse si permisiunile ei */
struct resource {
    string res_type<>;
    string perms<>;
};

/* O structura folosita pentru a pune bazele "bazei de date" pe care serverul
 o mentine pentru a stoca informatii despre anumiti clienti */
struct user {
    string username<>;
    string req_auth_token<>;
    string resources_token<>;
    string reg_token<>;
    int val_time;
    int automatic_refresh;
    struct resource *res;
};

/* Structura care contine tokenul de acces la resurse si tokenul de refresh,
 insotit de valabilitatea primului token, folosit drept raspuns la o functia
 de request REQUEST_ACCESS_TOKEN */
struct token_pair {
    string resources_token<>;
    string reg_token<>;
    int val_time;
};

/* Structura  care contine tokenii de access la resurse si de refresh, insotit
 de statusul operatiei (daca a fost executata cu succes sau si-a luat DENIED)
 folosita drept raspunsul apelului functiei VALIDATE_DELEGATED_ACTION */
struct status {
    struct token_pair pair;
    string stat<>;
};

program INTERFACE_PROG {
    version INTERFACE_VERS {

        /* Functia prin care se trimite userul si se primeste tokenul de 
         autorizare */
        string REQUEST_AUTHORIZATION(string) = 1;
        
        /* Functia prin care tokenul este "semnat" de utilizator */
        string APPROVE_REQUEST(string) = 2;

        /* Functia prin care se primeste numele userului si tokenul de autorizare
         si se elibereaza tokenii de acces si de refresh; de asementa, adauga
         si utilizatorul in baza de date */
        struct token_pair REQUEST_ACCESS_TOKEN(string, string, string, int) = 3;
        
        /* Functie care este apelata cand se doreste executarea unei actiuni
         pe o anumita resursa; intoarce statusul actiunii si, daca
         regenerarea tokenului de acces este setata la "automat", intoarce
         tokenii noi generati inapoi la client pentru a-i putea actualiza */
        struct status VALIDATE_DELEGATED_ACTION(string, string, string) = 4;
    } = 1;
} = 0x33554466;