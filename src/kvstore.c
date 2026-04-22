#define _POSIX_C_SOURCE 200809L

#include "kvstore.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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

kvstore_res_t kvstore_set(kvstore_table_t *table, kvstore_key_t key, kvstore_value_t value)
{
    if (!table || !key || !value)
    {
        return (kvstore_res_t){
            .tag = KVSTORE_RES_TAG_ERR,
            .errmsg = "Invalid argument"};
    }

    unsigned long hash = djb2_hash(key);
    size_t index = hash & (table->nBuckets - 1);

    kvstore_entry_t *curr = table->buckets[index];

    // Check if key exists (update path)
    while (curr)
    {
        if (strcmp(curr->key, key) == 0)
        {
            char *newval = strdup(value);
            if (!newval)
            {
                return (kvstore_res_t){
                    .tag = KVSTORE_RES_TAG_ERR,
                    .errmsg = "Failed to allocate value"};
            }

            free(curr->value);
            curr->value = newval;

            return (kvstore_res_t){
                .tag = KVSTORE_RES_TAG_OK,
                .value = NULL};
        }
        curr = curr->nextEntry;
    }

    // Insert new entry at head
    kvstore_entry_t *newEntry = malloc(sizeof *newEntry);
    if (!newEntry)
    {
        return (kvstore_res_t){
            .tag = KVSTORE_RES_TAG_ERR,
            .errmsg = "Failed to allocate entry"};
    }

    newEntry->key = strdup(key);
    if (!newEntry->key)
    {
        free(newEntry);
        return (kvstore_res_t){
            .tag = KVSTORE_RES_TAG_ERR,
            .errmsg = "Failed to allocate key"};
    }

    newEntry->value = strdup(value);
    if (!newEntry->value)
    {
        free(newEntry->key);
        free(newEntry);
        return (kvstore_res_t){
            .tag = KVSTORE_RES_TAG_ERR,
            .errmsg = "Failed to allocate value"};
    }

    newEntry->nextEntry = table->buckets[index];
    table->buckets[index] = newEntry;

    return (kvstore_res_t){
        .tag = KVSTORE_RES_TAG_OK,
        .value = NULL};
}

kvstore_res_t kvstore_get(kvstore_table_t *table, kvstore_key_t key)
{
    if (!table || !key)
    {
        return (kvstore_res_t){
            .tag = KVSTORE_RES_TAG_ERR,
            .errmsg = "Invalid argument"};
    }

    unsigned long hash = djb2_hash(key);
    size_t index = hash & (table->nBuckets - 1);

    kvstore_entry_t *curr = table->buckets[index];

    while (curr)
    {
        if (strcmp(curr->key, key) == 0)
        {
            return (kvstore_res_t){
                .tag = KVSTORE_RES_TAG_OK,
                .value = curr->value};
        }
        curr = curr->nextEntry;
    }

    return (kvstore_res_t){
        .tag = KVSTORE_RES_TAG_NOT_FOUND,
        .value = NULL};
}

kvstore_res_t kvstore_delete(kvstore_table_t *table, kvstore_key_t key)
{
    if (!table || !key)
    {
        return (kvstore_res_t){
            .tag = KVSTORE_RES_TAG_ERR,
            .errmsg = "Invalid argument"};
    }

    unsigned long hash = djb2_hash(key);
    size_t index = hash & (table->nBuckets - 1);

    kvstore_entry_t **link = &table->buckets[index];

    while (*link)
    {
        kvstore_entry_t *curr = *link;

        if (strcmp(curr->key, key) == 0)
        {
            *link = curr->nextEntry; // unlink

            free(curr->key);
            free(curr->value);
            free(curr);

            return (kvstore_res_t){
                .tag = KVSTORE_RES_TAG_OK,
                .value = NULL};
        }

        link = &curr->nextEntry;
    }

    return (kvstore_res_t){
        .tag = KVSTORE_RES_TAG_NOT_FOUND,
        .value = NULL};
}

void kvstore_destroyTable(kvstore_table_t *table)
{
    if (!table)
        return;

    for (size_t i = 0; i < table->nBuckets; i++)
    {
        kvstore_entry_t *curr = table->buckets[i];

        while (curr)
        {
            kvstore_entry_t *next = curr->nextEntry;

            free(curr->key);
            free(curr->value);
            free(curr);

            curr = next;
        }
    }

    free(table->buckets);
    free(table);
}