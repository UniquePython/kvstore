#include "kvstore.h"

#include <stddef.h>

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