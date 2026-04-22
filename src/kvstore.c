#include "kvstore.h"

#include <stddef.h>
#include <stdlib.h>

#define N_BUCKETS_INIT 32

typedef struct kvstore_entry_t
{
    kvstore_key_t key;
    kvstore_value_t value;
    struct kvstore_entry_t *nextEntry;
} kvstore_entry_t;

typedef struct kvstore_table_t
{
    kvstore_entry_t **buckets;
    size_t nBuckets;
} kvstore_table_t;

unsigned long djb2_hash(const char *str)
{
    unsigned long h = 5381;
    int c;
    while ((c = *str++))
        h = h * 33 + c;
    return h;
}

kvstore_res_t kvstore_createTable(const char *table_name)
{
    (void)table_name; // unused for now

    kvstore_table_t *table = malloc(sizeof *table);
    if (!table)
    {
        return (kvstore_res_t){
            .tag = KVSTORE_RES_TAG_ERR,
            .errmsg = "Failed to allocate table"};
    }

    table->nBuckets = N_BUCKETS_INIT;

    table->buckets = calloc(table->nBuckets, sizeof *table->buckets);
    if (!table->buckets)
    {
        free(table);
        return (kvstore_res_t){
            .tag = KVSTORE_RES_TAG_ERR,
            .errmsg = "Failed to allocate buckets"};
    }

    return (kvstore_res_t){
        .tag = KVSTORE_RES_TAG_OK,
        .value = table};
}