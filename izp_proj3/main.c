/**
 * Soubor:  proj3.c
 * Datum:   PROSINEC 2018
 * Autor:   Kateřina Mušková, xmusko00@stud.fit.vutbr.cz
 * Projekt: Shluková analýza
 * Popis:   Implementace shlukové analýzy metodu nejbližšího souseda
 */

#define DNDEBUG

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <float.h>

//********************** MAKRA *******************************************

#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)

#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

//********************** DATOVÉ TYPY A STRUKTURY *********************************

enum {
    OK, ERR_ARGC, ERR_ARGV, ERR_FILE = -1
};

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

//*********************  HLAVIČKY FUNKCÍ  ***************************************

void init_cluster(struct cluster_t *c, int cap);

void clear_cluster(struct cluster_t *c);

extern const int CLUSTER_CHUNK;

struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap);

void append_cluster(struct cluster_t *c, struct obj_t obj);

void merge_clusters(struct cluster_t *c1, struct cluster_t *c2);

int remove_cluster(struct cluster_t *carr, int narr, int idx);


float obj_distance(struct obj_t *o1, struct obj_t *o2);

float cluster_distance(struct cluster_t *c1, struct cluster_t *c2);

void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2);

void cluster_analysis_iteraction(struct cluster_t *carr, int narr);


static int obj_sort_compar(const void *a, const void *b);

void sort_cluster(struct cluster_t *c);


int process_first_line(FILE *f);

int line_to_obj(char *str, struct obj_t *obj);

int load_clusters(char *filename, struct cluster_t **arr);


void print_cluster(struct cluster_t *c);

void print_clusters(struct cluster_t *carr, int narr);


int argv_to_int(char *str, int *i);

int process_arg(int argc, char *argv[], int *num);


//************************** FUNKCE ***************************************

void help(){
    printf("Soubor:   proj3.c\n"
           "Datum:    PROSINEC 2018\n"
           "Autor:    Kateřina Mušková, xmusko00@stud.fit.vutbr.cz\n"
           "Projekt:  Shluková analýza"
           "Popis:    Implementace shlukové analýzy metodu nejbližšího souseda\n"
           "\n"
           "Spuštění: <nazev_souboru_s_objekty> <max_pocet_shluku>\n"
           "Nápověda: --help\n");

}

/**************************************************************************
 *Základní editace shluků.
 */


/**
 * Inicializace shluků
 * @param c ukazatel na shluk
 * @param cap počet objektů
 */
void init_cluster(struct cluster_t *c, int cap) {

    assert(c != NULL);
    assert(cap >= 0);

    if (cap == 0){
        c->obj = NULL;
    }else{
        c->obj = (struct obj_t *) malloc(cap * sizeof(struct obj_t));
    }

    c->size = 0;
    c->capacity = cap;
}


/**
 * Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 * @param c ukazatel na shluk
 */
void clear_cluster(struct cluster_t *c) {
    assert(c != NULL);

    free(c->obj);
    init_cluster(c, 0);
}

/*

 */
/**
 * Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 * @param c ukazetel na shluk
 * @param new_cap
 * @return ukazatel na výsledný shluk
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap) {
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);


    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t *) arr;
    c->capacity = new_cap;
    return c;
}

// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/**
 * Přidá objekt 'obj' na konec shluku 'c'. Rozšíří shluk, pokud se do něj objekt
 * nevejde.
 * @param c ukazatel na shluk
 * @param obj přídávaný objekt
 */
void append_cluster(struct cluster_t *c, struct obj_t obj) {
    assert(c->capacity >= c->size);

    if (c->capacity >= c->size){
        c = resize_cluster(c, c->capacity + CLUSTER_CHUNK);
    }
    c->obj[c->size].id = obj.id;
    c->obj[c->size].x = obj.x;
    c->obj[c->size].y = obj.y;
    c->size++;
}

/**
 * Do shluku 'c1' prida objekty shluku 'c2' a seřadí je.
 * @param c1 ukazatel na cílový shluk
 * @param c2 ukazatel na zdrojový shluk
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2) {
    assert(c1 != NULL);
    assert(c2 != NULL);

    for (int i = 0; i < c2->size; i++){
        append_cluster(c1, c2->obj[i]);
    }

    sort_cluster(c1);
}

/**
 * Odstraní shluk z pole shluků 'carr'.
 * @param carr pole shluků
 * @param narr počet shluků v poli
 * @param idx index odstraňovaného shluku
 * @return počet shluků v poli po odstranění
 */
int remove_cluster(struct cluster_t *carr, int narr, int idx) {
    assert(idx < narr);
    assert(narr > 0);

    clear_cluster(&carr[idx]);

    for (int i = idx; i < (narr - 1); i++){
        carr[i] = carr[i + 1];
    }

    return narr - 1;
}

/**********************************************************************/
/* Samotná shluková analýza */


/**
 * Výpočet Euklidovské vzdalenosti mezi dvema objekty.
 * @param o1 ukazatel na první objekt
 * @param o2 ukazatel na druhý objekt
 * @return požadovaná vzdálenost
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2) {
    assert(o1 != NULL);
    assert(o2 != NULL);

    float dist;

    float a = o1->x - o2->x;
    float b = o1->y - o2->y;

    dist = sqrtf(a * a + b * b);
    return dist;

}

/**
 * Výpočet vzdálenosti 2 shluků.
 * @param c1 ukazatel na první shluk
 * @param c2 ukazatel na druhý shluk
 * @return požadovaná vzdálenost
 */
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2) {
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    float dist, ret_dist = FLT_MAX;

    for (int i = 0; i < c1->size; i++){
        for (int j = 0; j < c2->size; j++){
            dist = obj_distance(&c1->obj[i], &c2->obj[j]);
            if (dist < ret_dist){
                ret_dist = dist;
            }
        }
    }
    return ret_dist;
}

/**
 * Nalezení dvou nejbližších shluků.
 * @param carr pole shluků
 * @param narr počet shluků v poli
 * @param c1 ukazatel na první nalezený shluk
 * @param c2 ukazatel na druhý nalezený shluk
 */
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2) {
    assert(narr > 0);

    float dist, min_dist = FLT_MAX;

    for (int i = 0; i < narr; i++){
        for (int j = 1; j < narr; j++){

            if (i == j) continue;

            dist = cluster_distance(&carr[i], &carr[j]);

            if (dist < min_dist){
                *c1 = i;
                *c2 = j;
                min_dist = dist;
            }
        }
    }
    dfloat(min_dist);
}

/**
 * Jedna iterace shlukové analýzy.
 * @param carr ukazatel na pole shluků
 * @param narr počet shluků
 */
void cluster_analysis_iteraction(struct cluster_t *carr, int narr) {
    int neighbour1, neighbour2;
    find_neighbours(carr, narr, &neighbour1, &neighbour2);
    merge_clusters(&carr[neighbour1], &carr[neighbour2]);
    remove_cluster(carr, narr, neighbour2);
}


/**********************************************************************/
/* Řazení shluků */

/**
 * Pomocna funkce pro řazení shluků.
 */
static int obj_sort_compar(const void *a, const void *b) {

    const struct obj_t *o1 = (const struct obj_t *) a;
    const struct obj_t *o2 = (const struct obj_t *) b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/**
 * Seřazení objektů ve shluku 'c' vzestupně podle jejich identifikačního čísla.
 * @param c ukazatel na shluk
 */
void sort_cluster(struct cluster_t *c) {
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/**********************************************************************/
/* Načtení shluků do pole */

/**
 * Zpracuje první řádek souboru zadaného argumentem programu
 * @param f ukazatel na soubor
 * @return číslo na prvním řádku souboru
 */
int process_first_line(FILE *f) {

    int max_chars = 20;
    char first_line[max_chars];

    if (fgets(first_line, max_chars - 1, f) == NULL){
        fprintf(stderr, "Nepodařilo se načíst první řádek souboru.\n");
        return ERR_FILE;
    }

    dfmt("first line: %s", first_line);
    char *count_ekv = "count=";

    if (strncmp(count_ekv, first_line, strlen(count_ekv) - 1) != 0){
        fprintf(stderr, "První řádek neodpovídá \"count=\"\n.");
        return ERR_FILE;
    }

    char str_count[max_chars];
    int int_count;

    strcpy(str_count, first_line + 6);
    dfmt("str count: %s", str_count);

    char *p_end = NULL;
    int_count = (int) strtol(str_count, &p_end, 10);

    //chybova kontrola
    if (p_end == NULL || p_end == str_count){
        fprintf(stderr, "Nepodařilo se spracovat první řádek souboru.\n");
        return ERR_FILE;
    }

    if (int_count <= 0){
        fprintf(stderr, "Počet shluků v souboru (count) musí být kladný.\n");
    }

    dfmt("int count: %d\n", int_count);

    return int_count;
}
/**
 * Zpracuje jeden řádek souboru obsahující informace o jednom objektu.
 * @param str řádek
 * @param obj ukazatel na objekt
 * @return úspěšnost zpracování
 */
int line_to_obj(char *str, struct obj_t *obj) {
    if (sscanf(str, " %d %f %f", &obj->id, &obj->x, &obj->y) != 3){
        fprintf(stderr, "Nebylo možné přečíst některé řádky vstupního souboru.\n");
        return ERR_FILE;
    }

    if (0 <= obj->x && obj->x <= 1000 && 0 <= obj->y && obj->y <= 1000){
        return OK;
    }

    fprintf(stderr, "Pro souřadnice objektu musí platit, že 0 <= X <= 1000, 0 <= Y <= 1000.\n");
    return OK;
}


/**
 * Načtení objektů ze souboru filename. Každý objekt je následně uložen do samostatného shluku,
 * který patří do pole shluků.
 * @param filename název souboru
 * @param arr ukazatel na pole shluků
 * @return počet načtených shluků
 */
int load_clusters(char *filename, struct cluster_t **arr) {
    assert(arr != NULL);

    FILE *f;
    f = fopen(filename, "r");

    if (f == NULL){
        fprintf(stderr, "Soubor se nepodařilo otevřít.\n");
        *arr = NULL;
        return ERR_FILE;
    }

    int size_cl = process_first_line(f);
    if (size_cl <= 0){
        *arr = NULL;
        fclose(f);
        return ERR_FILE;
    }

    struct cluster_t *cls_arr = malloc(size_cl * sizeof(struct cluster_t));
    struct obj_t obj;

    int max_char = 17;
    char line[max_char];
    int i = 0;

    int ret;

    while (fgets(line, max_char, f) != NULL && i < size_cl){
        struct cluster_t c;

        ret = line_to_obj(line, &obj);
        if (ret != OK){
            *arr = NULL;
            fclose(f);
            return ret;
        }

        init_cluster(&c, 1);
        append_cluster(&c, obj);
        strcpy(line, "");
        cls_arr[i] = c;
        i++;
    }

    dfmt("loaded clusters: %d\n", i);

    *arr = &cls_arr[0];
    fclose(f);
    return i;

}

/**********************************************************************/
/* Tisk shluků */

/**
 * Tisk shluku na stdout.
 * @param c ukazatel na shluk
 */
void print_cluster(struct cluster_t *c) {

    for (int i = 0; i < c->size; i++){
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/**
 * Tisk pole shluku.
 * @param carr ukazatel na první položku
 * @param narr tisk prvnich narr shluku
 */
void print_clusters(struct cluster_t *carr, int narr) {
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++){
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

/**********************************************************************/
/* Zpracování argumentů programu */

/**
 * Převedení argumentu programu počet shluků na číslo
 * @param str argument
 * @param i požadovaný počet
 * @return návratová hodnota
 */
int argv_to_int(char *str, int *i) {

    if (str[0] == '-'){
        fprintf(stderr, "Pocet shluků musí být nezáporný.\n");
        return ERR_ARGV;
    }

    char *p_end = NULL;
    *i = (int) strtol(str, &p_end, 10);

    //chybova kontrola
    if (p_end == NULL || p_end == str){
        fprintf(stderr, "Špatně zadané argumenty.\n");
        return ERR_ARGV;
    }

    return OK;
}

/**
 * Ověření správnosti argumentů programu
 * @param argc počet argumentů
 * @param argv pole orgumentů
 * @param num
 * @return
 */
int process_arg(int argc, char *argv[], int *num) {

    if (argc == 2){
        *num = 1;
        return OK;

    }else if (argc == 3){
        return argv_to_int(argv[2], num);

    }else{
        fprintf(stderr, "Spatně zadané argumenty.\n");
        return ERR_ARGC;
    }

}


int main(int argc, char *argv[]) {
    int ret;
    int cl_num_argv;
    int cl_num_current;

    if(strcmp(argv[1], "--help") == 0){
        help();
        return OK;
    }

    ret = process_arg(argc, argv, &cl_num_argv);
    if (ret != OK){
        return ret;
    }

    struct cluster_t *clusters;

    cl_num_current = load_clusters(argv[1], &clusters);
    if (cl_num_current <= 0){
        return cl_num_current;
    }

    while (cl_num_current > cl_num_argv){
        cluster_analysis_iteraction(clusters, cl_num_current);
        cl_num_current--;
    }

    print_clusters(clusters, cl_num_current);

    for (int i = 0; i < cl_num_current; i++){
        clear_cluster(&(clusters[i]));
    }

    free(clusters);

    debug("free all\n");

    return OK;

}