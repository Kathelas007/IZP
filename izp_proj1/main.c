/*
 * Soubor:  proj1.c
 * Datum:   LISTOPAD 2018
 * Autor:   Kateřina Mušková, xmusko00@stud.fit.vutbr.cz
 * Projekt: Prace s textem
 * Popis:   Proudový neinteraktivní editor
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#define N 1001

enum {OK, LOADING, COMMAND};
//chybova hlaseni, ktera prerusi beh programu
// OK - vse probehlo v poradku
// LOADING - chyba v nacitani
// COMMAND - chyba v syntaxi prikazu

int get_out = 0;

void help() {
    printf("Soubor:  proj1.c\n");
    printf("Datum:   LISTOPAD 2018\n");
    printf("Autor:   Kateřina Mušková, xmusko00@stud.fit.vutbr.cz\n");
    printf("Projekt: Prace s textem\n");
    printf("Popis:   Program slozi k radkove editaci textu pomoci prikazu v textovem souboru, ktery je zadan jako argument programu. \n\n");
}

int num_lines(char *name, int *index) {
//spocita pocet radku v souboru
    FILE *f;
    f = fopen(name, "r");

    int index_temp;

    char line[N];

    if(f != NULL) {
        while(fgets(line, N-1, f) != NULL) {
            index_temp++;
        }

        *index = index_temp;

        fclose(f);
        return OK;

    } else {
        printf("Nebylo mozne nasist soubor s prikazy.");
        return LOADING;
    }
}


int n_to_int(const char *comm) {
//prevede argument prikazu na cislo
    int num;
    char *ptr;

    char comm_temp[strlen(comm)-1];
    comm_temp[0] = 0;

    for(unsigned int i=0; i<strlen(comm)-1; i++) {
        comm_temp[i] = comm[i+1];
    }

    comm_temp[strlen(comm)-1] = 0;
    num = strtol(comm_temp, &ptr, 10);

    return num;
}

int load_input_line(char *input_line) {
//nacte radek ze vstupu
    if(fgets(input_line, N, stdin) != NULL) {
        return 0;
    } else {
        return -1;
    }
}

int load_commands(char (*comm_lines)[N], char *name) {
//nacte prikazy ze souboru uvedenem jako argument programu do 2D pole
    FILE *commands;
    commands = fopen(name, "r");

    if(commands == NULL) {
        printf("Soubor se vstupnimi prikazy se nepodarilo nacist.");
        return -1;
    }

    char str[N];
    int  i=0;

    while(fgets(str, N-1, commands) != NULL) {
        strcpy(comm_lines[i], str);
        i++;
    }

    if(i == 0) {
        printf("Soubor s prikazy je prazdny.");
        fclose(commands);
        return -1;
    }

    fclose(commands);
    return 0;
}


int str_pattern_replacement(const char *comm, char *patt, char *rep) {
//z prikazu substituce separuje stringy pattern a replacement
    if(strlen(comm) >= 3) {
        char sep = comm[1];
        unsigned int i = 2;

        while(comm[i]!=sep && i<(strlen(comm)-1)) {
            patt[i-2] = comm[i];
            i++;
        }
        patt[i-2] = '\0';

        if(i < strlen(comm)-1) {
            for(unsigned int j=i+1; j<strlen(comm)-1; j++) {
                rep[j-(i+1)] = comm[j];
                rep[j-(i)] = '\0';
            }
        } else {
            //pokud replacement neni zadan
            rep[0] = '\0';
        }

    } else {
        printf("Prikaz: %sje prilis kratky.", comm);
        get_out = COMMAND;
        return -1;
    }
    return 0;
}

int str_pattern(const char *comm, char *patt) {
//z prikazu separuje pouze comm
    if(strlen(comm) >= 3) {
        for(unsigned int i=1; i<strlen(comm) -1; i++) {
            patt[i-1] = comm[i];
        }

    } else {
        printf("Prikaz %sje prilis kratky.", comm);
        get_out = COMMAND;
        return -1;
    }
    return 0;
}

void print_rest(char *line) {
//tisk zbytku textu
    do {
        printf("DO: %s", line);
    } while(load_input_line(line) == 0);
}

//********************************
//prikazy ze zadani

void iCONTENT(const char *comm) {
//vloží před aktuální řádek řádek s obsahem CONTENT.

    //kontrole delky prikazu
    if(strlen(comm) >= 2) {
        for(unsigned int i=1; i<strlen(comm); i++) {
            //tisk vlozeneho radku
            printf("%c", comm[i]);
        }
    } else {
        printf("Prikaz: %sje prilis kratky.", comm);
        get_out = COMMAND;
    }
}

void bCONTENT(const char *comm, char *line) {
//vloží na začátek aktuálního řádku obsah CONTENT.

    //konstrola delky prikazu
    if(strlen(comm) >= 3) {
        char str[strlen(comm) + strlen(line) -1 ];

        //vlozi constend
        for(unsigned int i=1; i<(strlen(comm)-1); i++) {
            str[i-1] = comm[i];
        }

        //vlozi zbytek radku
        for(unsigned int i=(strlen(comm)-2); i<(strlen(comm) + strlen(line) -1); i++) {
            str[i] = line[i-(strlen(comm)-2)];
        }
        //zkopiruje do line
        strcpy(line, str);

    } else {
        printf("Prikaz: %sje prilis kratky.", comm);
        get_out = COMMAND;
    }
}

void aCONTENT(const char *comm, char *line) {
//vloží na konec aktuálního řádku obsah CONTENT

    //konstrole delky prikazu
    if(strlen(comm)>=3) {
        char str[strlen(comm) + strlen(line) -2];

        //zkopirovani puvodniho radku
        for(unsigned int i=0; i<(strlen(line)-1); i++) {
            str[i] = line[i];
        }

        //pridani CONTEND
        for(unsigned int i=(strlen(line)-1); i<=(strlen(comm) + strlen(line) -2); i++) {
            str[i] = comm[i-(strlen(line) -2)];
        }

        //vysledek
        strcpy(line, str);

    } else {
        printf("Prikaz: %sje prilis kratky.", comm);
        get_out = COMMAND;
    }
}

int dN(const char *comm, char *line) {
//smaže N řádků

    int n = n_to_int(comm);

    //zadny argumnet == 1 opakovani
    if(n == 0) {
        n = 1;
    }

    //nacitani dalsich radku
    for(int i=0; i<n; i++) {
        if(load_input_line(line) != 0) {
            return 1;
        }
    }
    return 0;
}

void r(char *line) {
//na aktuálním řádku odstraní znak konce řádku

    //kontrola
    if(strlen(line) >= 2) {
        //smazani konce radku
        if(line[strlen(line)-1] == '\n') {
            line[strlen(line)-1] = '\0';

        } else {
            printf("Na radku: %s uz byl odstranen konec radku.", line);
            get_out = COMMAND;
        }
    } else {
        printf("Radek: %sje prilis kratky.", line);
        get_out = COMMAND;
    }
}

void s_PATTERN_REPLACEMENT(const char *comm, char *line) {
//na aktuálním řádku provede substituci (nahrazení) výskytu řetězce PATTERN za řetězec REPLACEMENT
    char patt[strlen(comm)];
    char rep[strlen(comm)];
    int index;

    char original[strlen(line)];
    strcpy(original, line);

    if (str_pattern_replacement(comm, patt, rep) == 0) {

        if(strlen(patt) !=0) {
            // kontrola, zda line obashuje substring pattern
            if(strstr(line, patt) != NULL) {

                //index charu, na kterem zacina pattern
                index = strstr(line, patt) - line;

                //prepsani pomoci replacement
                for(unsigned int i=index; i<(index + strlen(rep)); i++) {
                    line[i] = rep[i-index];
                }

                //doplneni zbytku line
                for(unsigned int j=index + strlen(rep); j<=(strlen(original) - strlen(patt) + strlen(rep)); j++) {
                    line[j] = original[j - (strlen(rep) - strlen(patt))];
                }
            }
        }

    }
}

void S_PATTERN_REPLACEMENT(const char *comm, char *line) {
//na aktuálním řádku provede substituci (nahrazení) výskytu všech řetězců PATTERN za řetězec REPLACEMENT
    char patt[strlen(comm)];
    char rep[strlen(comm)];
    int index;

    char original[strlen(line)];
    char test_line[strlen(line)];

    strcpy(original, line);
    strcpy(test_line, line);

    int pos_array[strlen(original)];
    int pos_index = 0;
    int del_chars = 0;

    if(str_pattern_replacement(comm, patt, rep) == 0) {

        if(strlen(patt) !=0) {

            //hleda indexi, na kterych zacinaji pattern
            while(strstr(test_line, patt) != NULL) {
                index = strstr(test_line, patt) - test_line;

                strncpy(test_line, test_line + index + strlen(patt), strlen(test_line) - index);

                pos_array[pos_index] = index + del_chars;
                del_chars = del_chars + index + strlen(patt);
                pos_index++;

            }

            int diff_lenght = strlen(rep) - strlen(patt);

            pos_array[pos_index] = pos_array[pos_index-1] + diff_lenght * (pos_index)  -  (pos_index+1) * diff_lenght ;

            for(int f=0; f<(pos_index); f++) {
                //doplni do line 1 replacement
                for(unsigned int i = pos_array[f] + (f) * diff_lenght; i < (pos_array[f] + (f) * diff_lenght + strlen(rep)); i++) {
                    line[i] = rep[i - (pos_array[f] + (f) * diff_lenght)];
                }
                //doplni do line cast po replacement, nebo po konec posledniho znaku posledniho replacement
                for(int j = pos_array[f] + (f) * diff_lenght + strlen(rep); j < pos_array[f+1] + (f+1) * diff_lenght; j++) {
                    line[j] = original[j - ( (f+1) * diff_lenght )];
                }
            }
            //doplni do line zbytek za poslednim replacement
            for(unsigned int k = pos_array[pos_index-1] + strlen(patt) ; k <= strlen(original); k++) {
                line[k + pos_index * diff_lenght] = original[k];
            }
        }
    }
}

int nN(char *comm, char *line) {
//posun aktuálního řádku o jeden níž
    int num = n_to_int(comm);

    //pokud za prikazem neni zadny parametr, znamena to prikaz n - pouze 1 opakovani
    if(num == 0) {
        num = 1;
    }

    //tisk dalsich radku, pokud existuji
    for(int i=0; i<(num); i++) {

        printf("%s", line);

        if(load_input_line(line) != 0) {
            return 1;
        }
    }
    return 0;
}

void gX(const char *comm, int *curr_comm, int c) {
//přeskočí s aktuálním příkazem pro editaci na X-tý příkaz
    int X = n_to_int(comm);

    if(X<c) {
        *curr_comm = X-2;

    } else if(X==c) {
        printf("Prikaz %sse snazi preskocit sam na sebe.", comm);
        get_out = COMMAND;
    } else {
        printf("Prikaz %sse snazi preskocit na neexistujici prikaz.", comm);
        get_out = COMMAND;
    }
}

//***************
//nepovinne prikazy

void EOL(char line[N]) {
//přidá na konec aktuálního řádku znak konce řádku
    int len = strlen(line);
    line[len] = '\n';
    line[len+1] = '\0';
}

int main(int argc, char *argv[]) {

    //kontrala argumentu
    if(argc != 2) {
        help();
        return 1;
    }

    //***********
    //nacitani prvniho radku

    char line[N];

    if(load_input_line(line) != 0) {
        printf("Neni, co nacist. Soubor presmerovany do stdin je zdrejme prazdny.");
        return LOADING;
    }

    //***********
    //nacitani prikazu

    int c;
    if(num_lines(argv[1], &c) != 0) {
        return LOADING;
    }

    char cmd_lines[c][N];

    if(load_commands(cmd_lines, argv[1]) != 0) {
        return LOADING;
    }

    //************
    // switch s jednotlivymi prikazy k editaci

    // opousti cyklus a nasledne se ukonci program,
    //narozdil od chybovych hlaseni definovanych nahore programu nenastala v programu zadna chybe
    int leave = 0;

    int curr_cmd = 0;

    while(leave == 0) {

        char cmd_type = cmd_lines[curr_cmd][0];

        switch(cmd_type) {

        case 'i':
            iCONTENT(cmd_lines[curr_cmd]);
            break;

        case 'b':
            bCONTENT(cmd_lines[curr_cmd], line);
            break;

        case 'a':
            aCONTENT(cmd_lines[curr_cmd], line);
            break;

        case 'd':
            leave = dN(cmd_lines[curr_cmd], line);
            break;

        case 'r':
            r(line);
            break;

        case 's':
            s_PATTERN_REPLACEMENT(cmd_lines[curr_cmd], line);
            break;

        case 'S':
            S_PATTERN_REPLACEMENT(cmd_lines[curr_cmd], line);
            break;

        case 'n':
            leave = nN(cmd_lines[curr_cmd], line);
            break;

        case 'g':
            gX(cmd_lines[curr_cmd], &curr_cmd, c);
            break;

        case 'q':
            leave = 1;
            break;

        case 'e':
            EOL(line);
            break;

        default:
            printf("Prikaz: %sje neznamy.", cmd_lines[curr_cmd]);
            get_out = COMMAND;
        }

        //ukonci program
        if(get_out != 0) {
            return get_out;
        }

        //konec prikazu
        if(curr_cmd+1 >= c && cmd_type !='q' && leave ==0) {
            leave = 1;
            print_rest(line);
        }

        curr_cmd++;
    }
    return 0;
}
