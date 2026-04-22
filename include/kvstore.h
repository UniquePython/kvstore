#ifndef KVSTORE_H_
#define KVSTORE_H_

typedef struct
{
    enum
    {
        KVSTORE_RES_TAG_OK,
        KVSTORE_RES_TAG_ERR,
        KVSTORE_RES_TAG_NOT_FOUND
    } tag;
    union
    {
        void *value;
        char *errmsg;
    };
} kvstore_res_t;

typedef struct kvstore_table_t kvstore_table_t;

typedef char *kvstore_key_t;
typedef char *kvstore_value_t;

kvstore_res_t kvstore_createTable(const char *table_name);

kvstore_res_t kvstore_set(kvstore_table_t *table, kvstore_key_t key, kvstore_value_t value);
kvstore_res_t kvstore_get(kvstore_table_t *table, kvstore_key_t key);
kvstore_res_t kvstore_delete(kvstore_table_t *table, kvstore_key_t key);

void kvstore_destroyTable(kvstore_table_t *table);

#define KVSTORE_CREATE_TABLE(table_name, table, err)         \
    do                                                       \
    {                                                        \
        kvstore_res_t res = kvstore_createTable(table_name); \
        if (res.tag == KVSTORE_RES_TAG_OK)                   \
        {                                                    \
            (table) = (kvstore_table_t *)res.value;          \
            (err) = NULL;                                    \
        }                                                    \
        else                                                 \
        {                                                    \
            (table) = NULL;                                  \
            (err) = res.errmsg;                              \
        }                                                    \
    } while (0)

#define KVSTORE_GET(table, key, val, err)            \
    do                                               \
    {                                                \
        kvstore_res_t res = kvstore_get(table, key); \
        if (res.tag == KVSTORE_RES_TAG_OK)           \
        {                                            \
            (val) = (kvstore_value_t)res.value;      \
            (err) = NULL;                            \
        }                                            \
        else                                         \
        {                                            \
            (val) = NULL;                            \
            (err) = res.errmsg;                      \
        }                                            \
    } while (0)

#endif