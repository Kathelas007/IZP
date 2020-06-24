/**
 * Soubor:  proj2.c
 * Datum:   LISTOPAD 2018
 * Autor:   Kateřina Mušková, xmusko00@stud.fit.vutbr.cz
 * Projekt: Iterační výpočty
 * Popis:   Program implementuje výpočet přirozeného logaritmu a exponenciální
 *          funkce s obecným základem pouze pomocí matematických operací +,-,*,/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <float.h>

#define EPS 1e-8 //požadovaná přesnost u my_pow a my_log

enum {
    OK,      //program probehl standartne
    ERR_ARG, //spatne zadane argumenty
    ERR_ARGV //spatne hodnoty argumentu
};

enum {
    C_LOG, //vypocet logaritmu
    C_POW  //vypocet mocniny
};

void help_arguments();
void help();

double special_val_log(double x);
double taylor_log(double x, unsigned int n);
double cfrac_log(double x, unsigned int n);
double my_log(double x);

double special_val_pow(double x, double y);
double taylor_pow(double x, double y, unsigned int n);
double taylorcf_pow(double x, double y, unsigned int n);
double my_pow(double x, double y);

double str_to_double(char *str);
unsigned int str_to_int(char *str);

int init_arguments(int argc, char **argv, double *x, double *y, unsigned int *n, int *calc_type);
int check_params(int argc, char *argv[]);

void print_log_result(double x, unsigned int n);
void print_pow_result(double x, double y, unsigned int n);


//*********** Nápověda *******

void help_arguments() {
    printf("\n   --help nápověda\n");
    printf("   --log <x> <n> přirozený logaritmus z čísla x, n itarací\n");
    printf("   --pow <x> <y> <n> mocniana základu x s exponentem y, n iterací\n");
}

void help() {
    printf("\nSoubor:  proj2.c\n");
    printf("Datum:   LISTOPAD 2018\n");
    printf("Autor:   Kateřina Mušková, xmusko00@stud.fit.vutbr.cz\n");
    printf("Projekt: Iterační výpočty\n");
    printf("Popis:   Program implementuje výpočet přirozeného logaritmu a exponenciální\n"
           "         funkce s obecným základem pouze pomocí matematických operací +,-,*,/. \n");
    help_arguments();
}

//*********** Logaritmus *******

/**
 * Kontrola specialnich hodnot logaritmu
 *
 * @param argument logaritmu
 * @return hodnota logaritmu
 */
double special_val_log(double x) {
    if (x == NAN) return  NAN;

    if (x == 0) return -INFINITY;

    if (x == INFINITY) return INFINITY;

    if (x < 0) return NAN;

    return -1;
}

/**
 * Funkce pro přirozený logaritmus vypočtený pomocí Taylorova polynomu
 * implementace pro x <=1 a x>1
 *
 * @param x argument logaritmu
 * @param n počet iterací
 * @return vysledek logaritmu
 */
double taylor_log(double x, unsigned int n) {

    //specialni hodnoty
    double ret = special_val_log(x);
    if (ret != -1) return ret;

    //základní inicializace
    double numerator = 1;
    double sum = 0;

    //pro x <= 1
    if (x > 0 && x <= 1){

        double y = 1 - x;

        //vlastní cyklus
        for (unsigned int i = 1; i <= n; i++){
            numerator = numerator * (y);
            sum = sum - numerator / (double) i;
        }

        //pro x > 1
    }else if (x > 1){

        //vlastní cyklus
        for (unsigned int i = 1; i <= n; i++){
            numerator = numerator * ((x - 1) / x);
            sum = sum + (numerator / (double) i);
        }
    }
    return sum;
}

/**
 * Funkce pro přirozený logaritmus vypočtený pomocí zřetězených zlomků
 *
 * @param x argument logaritmu
 * @param n počet iterací
 * @return vysledek logaritmu
 */
double cfrac_log(double x, unsigned int n) {

    //specialni hodnoty
    double ret = special_val_log(x);
    if (ret != -1) return ret;

    //zlomek
    double frac = 0.0;

    //vyjadreni z ze zlomku
    double z = (x - 1.0) / (x + 1.0);

    double pow_z_2 = z * z;

    //vlastní cyklus
    for (int i = (n-1); i >= 1; i--){
        frac = (i * i * pow_z_2) / (1 + 2 * i - frac);
    }

    frac = (2 * z) / (1 - frac);
    return frac;
}

/**
 * Funkce logaritmu s přesností na 8 významných číslic
 *
 * @param x argument logaritmu
 * @return vysledek logaritmu
 */
double my_log(double x) {

    //specialni hodnoty
    double ret = special_val_log(x);
    if (ret != -1) return ret;

    long double taylor_res = 0, cfrac_res = 0;
    long double taylor_last = 0, cfrac_last = 0;
    unsigned int i = 1;

    double taylor_numerator = 1;

    if (x > 0 && x <= 1){
        double y = 1 - x;

        do{
            //taylor
            taylor_last = taylor_res;
            taylor_numerator = taylor_numerator * (y);
            taylor_res = taylor_res - taylor_numerator / (double) i;

            //cfrac
            cfrac_last = cfrac_res;
            cfrac_res = cfrac_log(x, i);

            i++;

        }while ((fabsl(taylor_last - taylor_res) / fabsl(taylor_res) >= EPS &&
                 fabsl(cfrac_last - cfrac_res) / fabsl(cfrac_res) >= EPS));

    }else if (x > 1){
        do{
            //taylor
            taylor_last = taylor_res;
            taylor_numerator = taylor_numerator * ((x - 1) / x);
            taylor_res = taylor_res + taylor_numerator / (double) i;

            //cfrac
            cfrac_last = cfrac_res;
            cfrac_res = cfrac_log(x, i);

            i++;
        }while (fabsl(taylor_last - taylor_res) / taylor_res >= EPS &&
                fabsl(cfrac_last - cfrac_res) / cfrac_res >= EPS);
    }

    if (fabsl((taylor_res - taylor_last) / taylor_res) < EPS){
        return (double) taylor_res;
    }

    return (double) cfrac_res;
}

//*********** Mocnina *******

/**
 * Kontrola speciálních hodnot mocniny
 *
 * @param x základ mocniny
 * @param y exponent mocniny
 * @return výsledek
 */
double special_val_pow(double x, double y) {
    //nan hodnoty
    if (isnan(x) || isnan(y)) return NAN;

    // inf, 0
    if (x == 0 && y == 0) return NAN;

    if (x == 0){
        if (y > 0) return 0;
        else return INFINITY;
    }

    if (y == INFINITY) return INFINITY;

    if (y == -INFINITY) return 0;

    if (y == 0) return 1;

    if (x == -INFINITY && floor(y) == y) return -INFINITY;

    if (isinf(x) && y > 0) return INFINITY;

    if (isinf(x)) return 0;

    //komplexni cisla
    if (x < 0 && floor(y) != y ) return NAN;

    return -1;
}
/**
 * Tato funkce umožňuje vypočítat mocninu ze záporného čísla
 *
 * @param x základ mocniny
 * @param y exponent
 * @return znaménko výsledku
 */
int edit_base(double *x, double y) {

    int result_sign = 1;

    if (*x < 0){
        *x = -*x;

        if ((int) floor(y) % 2 != 0){
            result_sign = -1;
        }
    }
    return result_sign;
}


/**
 * Exponenciální funkce využívající přirozeného logaritmu z taylor_log(double x, unsigned int n)
 *
 * @param x základ mocniny
 * @param y exponent mociny
 * @param n počet iterací
 * @return vysledek mocniny
 */
double taylor_pow(double x, double y, unsigned int n) {

    //kontrola spacialnich hodnot
    double ret = special_val_pow(x, y);
    if (ret != -1) return ret;

    int result_sign = edit_base(&x, y);

    double sum = 1;
    unsigned int i = 1;

    double exp_ln = y * taylor_log(x, n);

    double denominator = 1;
    double numerator = 1;

    //aplikace vzorce
    while (i <= n){
        denominator *= exp_ln;
        numerator *= i;

        sum += (denominator) / numerator;
        i++;
    }

    return sum * result_sign;
}

/**
 * Exponenciální funkce využívající přirozeného logaritmu z cfrac_log(double x, unsigned int n)
 *
 * @param x základ mocniny
 * @param y exponent mociny
 * @param n počet iterací
 * @return vysledek mocniny
 */
double taylorcf_pow(double x, double y, unsigned int n) {

    //kontrola spacialnich hodnot
    double ret = special_val_pow(x, y);
    if (ret != -1) return ret;

    int result_sign = edit_base(&x, y);

    double sum = 1;
    unsigned int i = 1;

    double exp_ln = y * cfrac_log(x, n);

    double denominator = 1;
    double numerator = 1;

    //aplikace vzorce
    while (i <= n){
        denominator *= exp_ln;
        numerator *= i;

        sum += (denominator) / numerator;
        i++;
    }
    return sum * result_sign;
}

/**
 * Funkce počítající mocninu na 8 platných číslic
 *
 * @param x základ mocniny
 * @param y exponent mociny
 * @return vysledek mocniny
 */
double my_pow(double x, double y) {

    //kontrola spacialnich hodnot
    double ret = special_val_pow(x, y);
    if (ret != -1) return ret;

    int result_sign = edit_base(&x, y);

    long double last_sum;
    long double sum = 1;
    int i = 1;

    double exp_ln = y * my_log(x);

    double denominator = 1;
    double numerator = 1;

    //aplikace vzorce
    do{
        last_sum = sum;

        denominator *= exp_ln;
        numerator *= i;

        sum += (denominator) / numerator;
        i++;

    }while (fabsl((sum - last_sum) / sum) >= EPS && sum != INFINITY);

    if (sum == INFINITY) return (double) last_sum;

    return (double) sum * result_sign;
}

//*********** Ostatní funkce *******

/**
 * Funkce vrací z řetezce double
 *
 * @param str vstupní řetězec
 * @return požadovaný double
 */
double str_to_double(char *str) {

    double dbl;

    char *p_end = NULL;
    dbl = strtod(str, &p_end);

    //chybová kontrola
    if (p_end == NULL || p_end == str || *p_end != 0){
        return DBL_MAX;
    }

    return dbl;
}

/**
 * Funkce vrací z řetezce unsigned int
 *
 * @param str vstupní řetězec
 * @return požadovaný unsigned int
 */
unsigned int str_to_int(char *str) {
    unsigned int n;

    if (str[0] == '-'){
        return UINT_MAX;
    }

    char *p_end = NULL;
    n = (unsigned int) strtol(str, &p_end, 10);

    //chybova kontrola
    if (p_end == NULL || p_end == str){
        return UINT_MAX;
    }
    return n;
}

/**
 * Inicializace argumentů pro funkce --log a --pow
 *
 * @param argc
 * @param argv
 * @param x základ mocniny, nebo argument logaritmu
 * @param y exponent mocniny
 * @param n počet iterací
 * @param calc_type zda je parametrem programu --log, nebo --pow
 * @return úspěšnost inicializace
 */
int init_arguments(int argc, char **argv, double *x, double *y, unsigned int *n, int *calc_type) {

    *x = str_to_double(argv[2]);
    *n = str_to_int(argv[argc - 1]);

    *y = 0.0;
    *calc_type = C_LOG;

    if (argc == 5){
        *y = str_to_double(argv[3]);
        *calc_type = C_POW;
    }

    // kontrola nekterých hodnot
    if (*x == DBL_MAX || *y == DBL_MAX || *n == UINT_MAX){
        printf("Špatně zadané argumenty.\n");
        return ERR_ARG;
    }

    if (*n <= 0){
        printf("Počet opakování musí být větší než 0.\n");
        return ERR_ARGV;
    }
    return OK;
}

/**
 * Kontrola parametrů programu
 *
 * @param argc
 * @param argv
 * @return úspěšnost kontroly
 */
int check_params(int argc, char **argv) {
    if (argc != 4 && argc != 5){
        printf("Program nedostal žádné argumenty.\n");
        return ERR_ARG;
    }

    if ((strcmp("--log", argv[1]) == 0 && argc == 4) || (strcmp("--pow", argv[1]) == 0 && argc == 5)){
        return OK;
    }

    printf("Špatně zadané argumenty.\n");
    return ERR_ARG;
}

//*********** Tisk výsledků *******

void print_log_result(double x, unsigned int n) {
    printf("             log(%g) = %.12g\n", x, log(x));
    printf("       cfrac_log(%g) = %.12g\n", x, cfrac_log(x, n));
    printf("      taylor_log(%g) = %.12g\n", x, taylor_log(x, n));
    printf("          my_log(%g) = %.7e\n", x, my_log(x));
}

void print_pow_result(double x, double y, unsigned int n) {
    printf("         pow(%g, %g) = %.12g\n", x, y, pow(x, y));
    printf("  taylor_pow(%g, %g) = %.12g\n", x, y, taylor_pow(x, y, n));
    printf("taylorcf_pow(%g, %g) = %.12g\n", x, y, taylorcf_pow(x, y, n));
    printf("      my_pow(%g, %g) = %.7g\n", x, y, my_pow(x, y));
}

int main(int argc, char *argv[]) {

    if (strcmp("--help", argv[1]) == 0) {
        help();
        return OK;
    }

    int ret;

    ret = check_params(argc, argv);
    if (ret != OK){
        help_arguments();
        return ret;
    }

    double x, y;
    unsigned int n;
    int calc_type;

    ret = init_arguments(argc, argv, &x, &y, &n, &calc_type);
    if (ret != OK){
        help_arguments();
        return ret;
    }

    if (calc_type == C_LOG){
        print_log_result(x, n);

    }else if (calc_type == C_POW){
        print_pow_result(x, y, n);
    }
    return OK;
}