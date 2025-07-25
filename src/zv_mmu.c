#include <linux/mm.h>

#include "../include/zv_mmu.h"
#include "../include/zv_core.h"
#include "../include/zv_log.h"

/* Variables */
static u64 g_ram_end;


static int zv_callback_walk_ram(unsigned long start, unsigned long size, void* arg) {
    zv_log_write(LOG_DEBUG, "MMU", "System RAM start %016lX, end %016lX, "
        "size %016lX", start * PAGE_SIZE, start * PAGE_SIZE + size * PAGE_SIZE,
		size * PAGE_SIZE);

    if (g_ram_end < ((start + size) * PAGE_SIZE)) {
        g_ram_end = (start + size) * PAGE_SIZE;
    }

    return 0;
}

/* Calculate System RAM size */
u64 zv_get_max_ram_size(void) {
    my_walk_system_ram_range func = NULL;
    unsigned long *p_max_pfn = NULL;
    unsigned long range_end = 0;

    g_ram_end = 0;

    func = (my_walk_system_ram_range)zv_get_symbol_address("walk_system_ram_range");
    if (func == NULL) {
        zv_log_write(LOG_NONE, "MMU", "walk_system_ram_range address get fail");
        return totalram_pages() * 2 * PAGE_SIZE;
    }

    // get max_pfn
    p_max_pfn = (unsigned long *)zv_get_symbol_address("max_pfn");
    if (p_max_pfn && *p_max_pfn > 0) {
        range_end = *p_max_pfn;
        zv_log_write(LOG_DEBUG, "MMU", "Using max_pfn = %lu", range_end);
    } else {
        // fallback
        range_end = totalram_pages() * 2;
        zv_log_write(LOG_DEBUG, "MMU", "Fallback: max_pfn not found, using totalram_pages * 2 = %lu", range_end);
    }

    func(0, range_end, NULL, zv_callback_walk_ram);
    
    return g_ram_end;
}