#include "cachelab.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <getopt.h>

#define SUCCESS     (0)
#define FAIL        (1)
#define SAR(X, Y)   ((X) >> (Y))
#define SHL(X, Y)   ((X) << (Y))

typedef unsigned long lru_t;

// line_t structure
typedef struct line_t {
    bool    valid;
    int     tag;
    lru_t   lru;
} line_t;
typedef line_t* set_t;
typedef set_t*  cache_t;

// global variable
int         s, b;
int         S, B, E;
size_t      s_mask;
size_t      t_mask;
int64_t     miss = 0;
int64_t     hit = 0;
int64_t     evict = 0;
cache_t     cache;
char*       trace_loc;
FILE*       trace_file;

int 
find_lru(size_t index) 
{
    lru_t   max_lru;
    int     max_j;

    max_lru = 0;
    max_j = 0;
    for(int j=0; j<E; j++) {
        if(max_lru <= cache[index][j].lru) {
            max_lru = cache[index][j].lru;
            max_j = j;
        }
    }
    return max_j;
}

void 
cache_init() 
{
    size_t          st_part;
    unsigned long   full;
    
    if(!S || !E || !B) exit(EXIT_FAILURE);

    cache = (set_t*)malloc(sizeof(set_t) * S);
    if(!cache) exit(FAIL);
    for(int i=0; i<S; i++) {
        cache[i] = (line_t*)malloc(sizeof(line_t) * E);
        if(!cache[i]) exit(FAIL);
        for(int j=0; j<E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru = 0;
        }
    }
    full = 0xffffffff;
    t_mask = SHL(SAR(full, s+b), s+b);
    st_part = SHL(SAR(full, b), b);
    s_mask = t_mask ^ st_part;
}

void 
cache_destroy() 
{
    for(int i=0; i<S; i++) free(cache[i]);
    free(cache);
}

void 
cache_simulate(size_t address) 
{
    size_t      index;
    size_t      tag;
    size_t      lru;
    int         j;
    int         empty_j;
    bool        flag;

    index = SAR(address&s_mask, b);
    tag = SAR(address&t_mask, s+b);
    empty_j = -1;
    flag = 0;

    // hit?    
    for(j=0; j<E; j++) {
        if(!cache[index][j].valid) {
            if(empty_j < 0) empty_j = j;
            continue;
        }
        if(tag == cache[index][j].tag) {
            flag = 1;
            cache[index][j].lru = 0;
        }
        else cache[index][j].lru++;
    }

    // hit!
    if(flag) { 
        hit++;
        return;
    }

    miss++;
    // do not need eviction.
    if(empty_j >= 0) { 
        cache[index][empty_j].valid = 1;
        cache[index][empty_j].lru = 0;
        cache[index][empty_j].tag = tag;

        return;
    }

    // need eviction.
    lru = find_lru(index);
    cache[index][lru].lru = 0;
    cache[index][lru].tag = tag;
    evict++;
}

int main(int argc, char *argv[]) 
{
    char            op;
    char            query;
    int             size;
    unsigned long   adr;

    while((op = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch(op) {
            case 's':
                s = atoi(optarg);
                if(!s) return FAIL;
                S = SHL(1, s);
                break;
            case 'E':
                E = atoi(optarg);
                if(!E) return FAIL;
                break;
            case 'b':
                b = atoi(optarg);
                if(!b) return FAIL;
                B = SHL(1, b);
                break;
            case 't':
                trace_loc = optarg;
                if(!trace_loc) return FAIL;
                break;
            default:
                return FAIL;
        }
    }

    // test body
    cache_init();

    trace_file = fopen(trace_loc, "r");
    if(!trace_file) exit(EXIT_FAILURE);

    while (fscanf(trace_file, " %c %lx,%d", &query, &adr, &size) == 3) {
        if(query=='I') continue;
        cache_simulate(adr);
        if(query=='M') cache_simulate(adr);
    }

    printSummary(hit, miss, evict);

    cache_destroy();
    fclose(trace_file);

    // test end
    return SUCCESS;
}