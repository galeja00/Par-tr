#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>


#define SIZE_BUFFER 1000

// struktura pro předání dat do vlákna
struct tr_data {
    int thread_id;
    char* str;
    char* from;
    char* to;
};

// spočítá délku char array po koncoví znak '\0'
int len_chararray(char* str, int size) {
    int i = 0;
    while (str[i] != '\0' && i < size) {
        i++;
    }
    return i - 1;
}

// převede char array na int
int char_arr_to_int(char* str) {
    int i = 0;
    int res = 0;
    while (str[i] != '\0') {
        res = res * 10 + (str[i] - '0');
        i++;
    }
    return res;
}

// provadí nahrazovaní znaků ve vláknu
void* tr(void* data) {
    struct tr_data* d = (struct tr_data*) data;
    char* str = d->str;
    char* from = d->from;
    char* to = d->to;
    int j;
    int i = 0;
    while (str[i] != '\0') {
        j = 0;
        while (from[j] != '\0') {
            if (str[i] == from[j]) {
                str[i] = to[j];
            }
            j++;
        }
        i++;
    }
    d->str = str;
    pthread_exit(NULL);
}

// provadí odstranění znaků ve vláknu
void* dtr(void* data) {
    struct tr_data* d = (struct tr_data*) data;
    char* str = d->str;
    char* from = d->from;
    int j;
    int i = 0;
    int count = 0;
    // spočítá počet znaků, které budem alokovat pro nový string
    while (str[i] != '\0') {
        j = 0;
        while (from[j] != '\0') {
            if (str[i] == from[j]) {
                count--;
            }
            j++;
        }
        i++;
        count++;
    }
    char* new_str = malloc((count + 1) * sizeof(char));
    i = 0;
    int pos = 0;
    int insert;
    // přmemisťuje znaky do nového stringu
    while (str[i] != '\0') {
        insert = 1;
        j = 0;
        while (from[j] != '\0') {
            if (str[i] == from[j]) {
                insert = 0;
                break;
            }
            j++;
        }
        if (insert) {
            new_str[pos] = str[i];
            pos++;
        }
        i++;
    }
    new_str[pos] = '\0';
    d->str = new_str;
    pthread_exit(NULL);
}

// rozdělí string na několik stringů pro vlákna
char** split(char* str, int size, int threads) {
    char** strs = malloc(sizeof(char*) * threads);
    int len = size / threads;
    int rest = size % threads;
    int pos = 0;
    for(int i = 0; i < threads; i++) {
        if (i == threads - 1) {
            strs[i] = malloc(sizeof(char) * (len + rest));
            for(int j = 0; j < len + rest; j++) {
                strs[i][j] = str[pos];
                pos++;
            }
        } else {
            strs[i] = malloc(sizeof(char) * len);
            for(int j = 0; j < len; j++) {
                strs[i][j] = str[pos];
                pos++;
            }
        }
    }
    return strs;
}

struct tr_data set_data(int thread, char* str, char* from, char* to) {
    struct tr_data data;
    data.thread_id = thread;
    data.str = str;
    data.from = from;
    data.to = to;
    return data;
}


int main(int argc, char* argv[]) {
    char line[SIZE_BUFFER];
    
    while (fgets(line, SIZE_BUFFER, stdin) != NULL) {
        int len = len_chararray(line, SIZE_BUFFER);
        if (argv[1] == NULL || argv[2] == NULL) {
            printf("Error: missing operand for threading");
            return -1;
        }
        int threads = char_arr_to_int(argv[2]);
        int count_chars;
        // pokud je vlaken více jak znaků, tak se nastaví počet znaků na 1 a vlákna na délku stringu
        if (threads > len) {
            threads = len;
            count_chars = 1;
        } 
        // vytvoření pole stringů pro vlákna
        char** strs = split(line, len, threads);

        // vytvoření vláken a data pro vlákna
        pthread_t thread[threads];
        struct tr_data* all_data = malloc(sizeof(struct tr_data) * threads);
        if (argv[3][1] == 'd' && argv[3][0] == '-') {
            if (argv[4] == NULL) {
                printf("Error: missing operand");
            }
            // vyvolání vláken pro odstranění znaků
            for(int i = 0; i < threads; i++) {   
                all_data[i] = set_data(i, strs[i], argv[4], "");
                pthread_create(&thread[i], NULL, dtr, &all_data[i]);
            }
        } else {
            // ukončí program pokud from a to nemaji stejnou delku
            if (argv[4] == NULL || argv[3] == NULL) {
                printf("Error: missing operand");
                return -1;
            }
            if (len_chararray(argv[3], SIZE_BUFFER) != len_chararray(argv[4], SIZE_BUFFER)) {
                printf("Error: from and to must have same length\n");
                return -1;
            }
            // vyvolání vláken pro nahrazení znaků
            for(int i = 0; i < threads; i++) {
                all_data[i] = set_data(i, strs[i], argv[3], argv[4]);
                pthread_create(&thread[i], NULL, tr, &all_data[i]);
            }
        }
        // pockani na vlákna 
        for (int i = 0; i < threads; i++) {
            pthread_join(thread[i], NULL);
        }
        // vypsání výsledku
        for (int i = 0; i < threads; i++) {
            printf("%s", all_data[i].str);
        }
        printf("\n");
        
        // uvolnění paměti
        for (int i = 0; i < threads; i++) {
            free(strs[i]);
        }
        free(strs);
    }
    return 0;
}
