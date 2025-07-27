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

/* Page table flags. */
#define MASK_PAGEFLAG			((u64) 0xFF00000000000FFF)
#define MASK_PAGEFLAG_WO_DA		(((u64) 0xFF00000000000FFF) ^ (0x01 << 5) ^ (0x01 << 6))
#define MASK_INVALIDPAGEFLAG	((u64) 0x07FF000000000000)
#define MASK_PAGE_SIZE_FLAG		(0x01 << 7)
#define MASK_PAGEFLAG_WO_SIZE	(MASK_PAGEFLAG ^ MASK_PAGE_SIZE_FLAG)
#define MASK_PRESENT_FLAG		(0x01 << 0)
#define MASK_XD_FLAG			((u64)0x01 << 63)
#define MASK_PAGEADDR			((u64) 0xFFFFFFFFFFFFF000)

/* EPT page type. */
#define EPT_TYPE_PML4			0
#define EPT_TYPE_PDPTEPD		1
#define EPT_TYPE_PDEPT			2   
#define EPT_TYPE_PTE			3   

/* EPT flags */
#define EPT_READ				(0x01 << 0)
#define EPT_WRITE				(0x01 << 1)
#define EPT_EXECUTE				(0x01 << 2)
#define EPT_ALL_ACCESS			(EPT_READ | EPT_WRITE | EPT_EXECUTE)
#define EPT_BIT_PDE_2MB			(0x01 << 7)
#define EPT_BIT_MEM_TYPE_WB		(0x06 << 3)
#define EPT_PAGE_ENT_COUNT		512
#define EPT_PAGE_SIZE			4096

/* Structures */

/* EPT information structure */
struct zv_ept_info {
	/* Entry counts for each level */
	u64 pml4_ent_count;
	u64 pdpte_pd_ent_count;
    u64 pdept_ent_count;
    u64 pte_ent_count;

	/* Page counts for each level */
    u64 pml4_page_count;
    u64 pdpte_pd_page_count;
    u64 pdept_page_count;
    u64 pte_page_count;

	/* Page address arrays for each level */
    u64* pml4_page_addr_array;
    u64* pdpte_pd_page_addr_array;
    u64* pdept_page_addr_array;
    u64* pte_page_addr_array;
};

/* Page table structure. */
struct zv_pagetable
{
    u64 entry[512];
};

/* EPT table structure. */
struct zv_ept_pagetable
{
    u64 entry[512];
};


/* Variables */

/* The function protocol for walk_system_ram_range. */
typedef int (*my_walk_system_ram_range) (unsigned long start_pfn, unsigned long nr_pages, 
	void *arg, int (*func)(unsigned long, unsigned long, void*));



/* Function declarations */
u64 zv_get_max_ram_size(void);
int zv_alloc_ept_pages(void);
void zv_setup_ept_pagetables(void);
void* zv_get_pagetable_log_addr(int type, int index);
void* zv_get_pagetable_phy_addr(int type, int index);
void zv_set_ept_hide_page(u64 phy_addr);
void zv_protect_ept_pages(void);