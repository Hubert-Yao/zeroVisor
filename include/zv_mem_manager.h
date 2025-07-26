#include <linux/types.h>
#include <linux/gfp.h>

enum zv_mem_type {
    ZV_KMALLOC = 0,
    ZV_VMALLOC,
    ZV_ALLOC_PAGE,
};

/* Memory allocate log Structure*/
struct zv_alloc_node {
    void* ptr;
    enum zv_mem_type type;
    struct list_head list;
};

/* Functions */
/* Allocate */
void* zv_kmalloc(size_t size, gfp_t flags);
void* zv_vmalloc(size_t size);
void* zv_alloc_page(gfp_t flags);

/* Free */
void zv_kfree(void* ptr);
void zv_vfree(void* ptr);
void zv_free_page(void* ptr);

void zv_free_all(void);

/* Debug */
void zv_dump_allocs(void);