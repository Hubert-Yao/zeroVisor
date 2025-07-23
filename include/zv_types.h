#include <linux/types.h>

#include "zv_config.h"

/*symbol table entry structure*/
struct zv_symbol_table_entry {
    char* name;
    u64 addr;
};

/*symbol table structure*/
struct zv_symbol_table {
    struct zv_symbol_table_entry symbol[SYMBOL_MAX_COUNT];
};
