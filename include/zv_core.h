#include <linux/types.h>

/* Variables */
extern u64 g_max_ram_size;

/* Functions */
u64 zv_get_symbol_address(char* symbol);
void zv_hide_range(u64 start_addr, u64 end_addr, int alloc_type);