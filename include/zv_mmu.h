#include <linux/types.h>

/*
 * Macros.
 */
/* Page size macro. */
#define VAL_256TB				((u64)256 * 1024 * 1024 * 1024 * 1024)
#define VAL_512GB				((u64)512 * 1024 * 1024 * 1024)
#define VAL_4GB					((u64)4* 1024 * 1024 * 1024)
#define VAL_1GB					((u64)1024 * 1024 * 1024)
#define VAL_2MB					((u64)2 * 1024 * 1024)
#define VAL_4KB					((u64)4 * 1024)


/* Variables */

/* The function protocol for walk_system_ram_range. */
typedef int (*my_walk_system_ram_range) (unsigned long start_pfn, unsigned long nr_pages, 
	void *arg, int (*func)(unsigned long, unsigned long, void*));



/* Function declarations */
u64 zv_get_max_ram_size(void);