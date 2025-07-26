#include <linux/mm.h>

#include "../include/zv_mmu.h"
#include "../include/zv_core.h"
#include "../include/zv_log.h"
#include "../include/zv_mem_manager.h"

/* Variables */
static u64 g_ram_end;
struct zv_ept_info g_ept_info = {0, };


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

int zv_alloc_ept_pages(void) {
    int i;

    g_ept_info.pml4_ent_count = CEIL(g_max_ram_size, VAL_512GB);
    g_ept_info.pdpte_pd_ent_count = CEIL(g_max_ram_size, VAL_1GB);
    g_ept_info.pdept_ent_count = CEIL(g_max_ram_size, VAL_2MB);
	g_ept_info.pte_ent_count = CEIL(g_max_ram_size, VAL_4KB);

    g_ept_info.pml4_page_count = CEIL(g_ept_info.pml4_ent_count, EPT_PAGE_ENT_COUNT);
	g_ept_info.pdpte_pd_page_count = CEIL(g_ept_info.pdpte_pd_ent_count, EPT_PAGE_ENT_COUNT);
	g_ept_info.pdept_page_count = CEIL(g_ept_info.pdept_ent_count, EPT_PAGE_ENT_COUNT);
	g_ept_info.pte_page_count = CEIL(g_ept_info.pte_ent_count, EPT_PAGE_ENT_COUNT);

    zv_log_write(LOG_DEBUG, "MMU", "Setup EPT, Max RAM Size %ld", g_max_ram_size);
    zv_log_write(LOG_DEBUG, "MMU", "    [*] EPT Size: %d", 
        (int)sizeof(struct zv_ept_pagetable));
    zv_log_write(LOG_DEBUG, "MMU", "    [*] PML4 Entry Count: %d", 
        (int)g_ept_info.pml4_ent_count);
    zv_log_write(LOG_DEBUG, "MMU", "    [*] PDPTE PD Entry Count: %d", 
        (int)g_ept_info.pdpte_pd_ent_count);
    zv_log_write(LOG_DEBUG, "MMU", "    [*] PDE PT Entry Count: %d", 
        (int)g_ept_info.pdept_ent_count);
    zv_log_write(LOG_DEBUG, "MMU", "    [*] PTE Entry Count: %d", 
        (int)g_ept_info.pte_ent_count);

    zv_log_write(LOG_DEBUG, "MMU", "    [*] PML4 Page Count: %d", 
        (int)g_ept_info.pml4_page_count);
    zv_log_write(LOG_DEBUG, "MMU", "    [*] PDPTE PD Page Count: %d", 
        (int)g_ept_info.pdpte_pd_page_count);
    zv_log_write(LOG_DEBUG, "MMU", "    [*] PDE PT Page Count: %d", 
        (int)g_ept_info.pdept_page_count);
    zv_log_write(LOG_DEBUG, "MMU", "    [*] PTE Page Count: %d", 
        (int)g_ept_info.pte_page_count);

    /* Allocate memory for page table */
    g_ept_info.pml4_page_addr_array = (u64*)zv_vmalloc(g_ept_info.pml4_page_count * sizeof(u64*));
    g_ept_info.pdpte_pd_page_addr_array = (u64*)zv_vmalloc(g_ept_info.pdpte_pd_page_count * sizeof(u64*));
    g_ept_info.pdept_page_addr_array = (u64*)zv_vmalloc(g_ept_info.pdept_page_count * sizeof(u64*));
    g_ept_info.pte_page_addr_array = (u64*)zv_vmalloc(g_ept_info.pte_page_count * sizeof(u64*));

    if (   (! g_ept_info.pml4_page_addr_array)
        || (! g_ept_info.pdpte_pd_page_addr_array)
        || (! g_ept_info.pdept_page_addr_array)
        || (! g_ept_info.pte_page_addr_array)
    ) {
        zv_log_write(LOG_NONE, "MMU", "zv_alloc_ept_pages alloc fail");
		return -1;
	}

    for (i = 0; i < g_ept_info.pml4_page_count; i ++) {
        g_ept_info.pml4_page_addr_array[i] = (u64)zv_kmalloc(0x1000, GFP_KERNEL);
        
        if (! g_ept_info.pml4_page_addr_array[i]) {
            zv_log_write(LOG_NONE, "MMU", "zv_alloc_ept_pages alloc fail");
            return -1;
        }
    }

    for (i = 0; i < g_ept_info.pdpte_pd_page_count; i ++) {
        g_ept_info.pdpte_pd_page_addr_array[i] = (u64)zv_kmalloc(0x1000, GFP_KERNEL);

        if (! g_ept_info.pdpte_pd_page_addr_array[i]) {
            zv_log_write(LOG_NONE, "MMU", "zv_alloc_ept_pages alloc fail");
            return -1;
        }
    }

    for (i = 0; i < g_ept_info.pdept_page_count; i ++) {
        g_ept_info.pdept_page_addr_array[i] = (u64)zv_kmalloc(0x1000, GFP_KERNEL);
        
        if (! g_ept_info.pdept_page_addr_array[i]) {
            zv_log_write(LOG_NONE, "MMU", "zv_alloc_ept_pages alloc fail");
            return -1;
        }
    }
    
    for (i = 0; i < g_ept_info.pte_page_count; i ++) {
        g_ept_info.pte_page_addr_array[i] = (u64)zv_kmalloc(0x1000, GFP_KERNEL);
        
        if (! g_ept_info.pte_page_addr_array[i]) {
            zv_log_write(LOG_NONE, "MMU", "zv_alloc_ept_pages alloc fail");
            return -1;
        }
    }

    zv_log_write(LOG_DETAIL, "MMU", "   [*] Page Table Memory Allocate Success");

    return 0;
}

/* Setup EPT */
void zv_setup_ept_pagetables(void) {
    struct zv_ept_pagetable* ept_info;
    u64 next_page_table_addr;
    u64 i, j;
    u64 loop_cnt;
    u64 base_addr;

    /* Setup PML4 */
    zv_log_write(LOG_DETAIL, "MMU", "Setup PML4");
    ept_info = (struct zv_ept_pagetable*)zv_get_pagetable_log_addr(EPT_TYPE_PML4, 0);
    zv_log_write(LOG_DETAIL, "MMU", "   [*] Setup PML4 %016lX", (u64)ept_info);
    memset(ept_info, 0, sizeof(struct zv_ept_pagetable));

    base_addr = 0;
    for (i = 0; i < EPT_PAGE_ENT_COUNT; i ++) {
        if (i < g_ept_info.pml4_ent_count) {
            next_page_table_addr = (u64)zv_get_pagetable_phy_addr(EPT_TYPE_PDPTEPD, i);
            ept_info->entry[i] = next_page_table_addr | EPT_ALL_ACCESS;

            if (i == 0) {
                zv_log_write(LOG_DETAIL, "MMU", "   [*] %016lX", (u64)next_page_table_addr);
            }
        } else {
            ept_info->entry[i] = base_addr | EPT_ALL_ACCESS;
        }

        base_addr += VAL_512GB;
    }

    /* Setup PDPTE PD */
    zv_log_write(LOG_DETAIL, "MMU", "Setup PDPTE PD");
    base_addr = 0;
    for (j = 0; j < g_ept_info.pdpte_pd_ent_count; j ++) {
        ept_info = (struct zv_ept_pagetable*)zv_get_pagetable_log_addr(EPT_TYPE_PDPTEPD, j);
        zv_log_write(LOG_DETAIL, "MMU", "   [*] Setup PDPTEPD [%d] %016lX", j, (u64)ept_info);
        memset(ept_info, 0, sizeof(struct zv_ept_pagetable));

        loop_cnt = g_ept_info.pdpte_pd_ent_count - (j * EPT_PAGE_ENT_COUNT);
        // loop_cnt = loop_cnt > EPT_PAGE_ENT_COUNT ? EPT_PAGE_ENT_COUNT : loop_cnt;
        loop_cnt = min(loop_cnt, EPT_PAGE_ENT_COUNT);


        for (i = 0; i < EPT_PAGE_ENT_COUNT; i ++) {
            if (i < loop_cnt) {
                next_page_table_addr = (u64)zv_get_pagetable_phy_addr(EPT_TYPE_PDEPT,
                    (j * EPT_PAGE_ENT_COUNT) + i);
                ept_info->entry[i] = next_page_table_addr | EPT_ALL_ACCESS;

                if (i == 0) {
                    zv_log_write(LOG_DETAIL, "MMU", "   [*] %016lX", (u64)next_page_table_addr);
                }
            } else {
                ept_info->entry[i] = base_addr | EPT_ALL_ACCESS;
            }

            base_addr += VAL_1GB;
        }
    }

    /* Setup PDEPT */
    zv_log_write(LOG_DETAIL, "MMU", "Setup PDEPT");
    base_addr = 0;
    for (j = 0; j < g_ept_info.pdept_page_count; j ++) {
        ept_info = (struct zv_ept_pagetable*)zv_get_pagetable_phy_addr(EPT_TYPE_PDEPT, j);
        zv_log_write(LOG_DETAIL, "MMU", "   [*] Setup PDEPT [%d] %016lX", j, (u64)ept_info);
        memset(ept_info, 0, sizeof(struct zv_ept_pagetable));

        loop_cnt = g_ept_info.pdept_ent_count - (j * EPT_PAGE_ENT_COUNT);
        loop_cnt = min(loop_cnt, EPT_PAGE_ENT_COUNT);

        for (i = 0; i < EPT_PAGE_ENT_COUNT; i ++) {
            if (i < loop_cnt) {
                next_page_table_addr = (u64)zv_get_pagetable_phy_addr(EPT_TYPE_PTE, 
                    (j * EPT_PAGE_ENT_COUNT) + i);
                ept_info->entry[i] = next_page_table_addr | EPT_ALL_ACCESS;

                if (i == 0) {
                    zv_log_write(LOG_DETAIL, "MMU", "   [*] %016lX", (u64)next_page_table_addr);
                }
            } else {
                ept_info->entry[i] = base_addr | EPT_ALL_ACCESS;
            }

            base_addr += VAL_2MB;
        }
    }

    /* Setup PTE */
    zv_log_write(LOG_DETAIL, "MMU", "Setup PTE");
    for (j = 0; j < g_ept_info.pte_page_count; j ++) {
        ept_info = (struct zv_ept_pagetable*)zv_get_pagetable_phy_addr(EPT_TYPE_PTE, j);
        memset(ept_info, 0, sizeof(struct zv_ept_pagetable));

        loop_cnt = g_ept_info.pte_ent_count - (j * EPT_PAGE_ENT_COUNT);
        loop_cnt = min(loop_cnt, EPT_PAGE_ENT_COUNT);

        for (i = 0; i < EPT_PAGE_ENT_COUNT; i ++) {
            if (i < loop_cnt) {
                next_page_table_addr = ((u64)j * EPT_PAGE_ENT_COUNT + i) * EPT_PAGE_SIZE;

                /*
				 * Set uncacheable type by default.
				 * Set write-back type to "System RAM" areas at the end of this
				 * function.
				 */
				ept_info->entry[i] = next_page_table_addr | EPT_ALL_ACCESS;
            } else {
                ept_info->entry[i] = base_addr | EPT_ALL_ACCESS;
            }

            base_addr += VAL_4KB;
        }
    }

    /* Set write-back type to "System RAM" areas */
    zv_setup_ept_system_ram_range();
}

/* Get logical address of page table pointer of index and type */
void* zv_get_pagetable_log_addr(int type, int index) {
    u64* table_array_addr;
    
    switch (type) {
        case EPT_TYPE_PML4: 
            table_array_addr = g_ept_info.pml4_page_addr_array;
            break;
        
        case EPT_TYPE_PDPTEPD:
            table_array_addr = g_ept_info.pdpte_pd_page_addr_array;
            break;
        
        case EPT_TYPE_PDEPT:
            table_array_addr = g_ept_info.pdept_page_addr_array;
            break;

        case EPT_TYPE_PTE:
            table_array_addr = g_ept_info.pte_page_addr_array;
            break;
        
        default:
            table_array_addr = g_ept_info.pte_page_addr_array;
            break;
    }

    return (void*)table_array_addr[index];
}

/* Get physical address of page table pointer of index and type */
void* zv_get_pagetable_phy_addr(int type, int index) {
    void* table_log_addr;
    
    table_log_addr = zv_get_pagetable_log_addr(type, index);
    return (void*)virt_to_phys(table_log_addr);
}

/* Set write-back permission to System RAM area */
static void zv_setup_ept_system_ram_range(void) {
    my_walk_system_ram_range func = NULL;

    func = (my_walk_system_ram_range)zv_get_symbol_address("walk_system_ram_range");
    if (! func) {
        zv_log_write(LOG_NONE, "MMU", "walk_system_ram_range address get fail");
        return;
    }

    func(0, g_max_ram_size / PAGE_SIZE, NULL, zv_callback_set_write_back_to_ram);
}

/*
 * Process callback of walk_system_ram_range().
 *
 * This function sets write-back cache type to EPT page.
 */
static int zv_callback_set_write_back_to_ram(
    unsigned long start,
    unsigned long size,
    void* arg
) {
    struct zv_ept_pagetable* ept_info;
    unsigned long i;

    zv_log_write(LOG_DEBUG, "MMU", "System RAM start %016lX, end %016lX, size %016lX",
        start * PAGE_SIZE, (start + size) * PAGE_SIZE, size * PAGE_SIZE);

    for (i = start; i < start + size; i ++) {
        ept_info = (struct zv_ept_pagetable*)zv_get_pagetable_log_addr(EPT_TYPE_PTE, i / EPT_PAGE_ENT_COUNT);
        ept_info->entry[i % EPT_PAGE_ENT_COUNT] |= EPT_BIT_MEM_TYPE_WB;
    }

    return 0;
}

/* Protect page table memory for EPT */
void zv_protect_ept_pages(void) {
    int i;
    u64 end;

    zv_log_write(LOG_DEBUG, "MMU", "Protect EPT");

    /* Hide the EPT page table */
    // TODO
}

