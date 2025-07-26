#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/spinlock.h>

#include "../include/zv_mem_manager.h"
#include "../include/zv_log.h"

static LIST_HEAD(zv_alloc_list);
static DEFINE_SPINLOCK(zv_alloc_list_lock); // for multi-thread safe

static void zv_track_alloc(void* ptr, enum zv_mem_type type) {
    struct zv_alloc_node* node;

    if(!ptr) return ;

    node = kmalloc(sizeof(*node), GFP_KERNEL);
    if(!node) return;

    node -> ptr = ptr;
    node -> type = type;

    spin_lock(&zv_alloc_list_lock);
    list_add(&node->list, &zv_alloc_list);
    spin_unlock(&zv_alloc_list_lock);
}

void* zv_kmalloc(size_t size, gfp_t flags) {
    void* ptr = kmalloc(size, flags);
    zv_track_alloc(ptr, ZV_KMALLOC);
    return ptr;
}

void* zv_vmalloc(size_t size)
{
    void* ptr = vmalloc(size);
    zv_track_alloc(ptr, ZV_VMALLOC);
    return ptr;
}

void* zv_alloc_page(gfp_t flags)
{
    void* ptr = (void*)__get_free_page(flags);
    zv_track_alloc(ptr, ZV_ALLOC_PAGE);
    return ptr;
}

/* These function used to free a specified memory block */
static bool zv_remove_node(void *ptr) {
    struct zv_alloc_node *node, *tmp;

    spin_lock(&zv_alloc_list_lock);
    list_for_each_entry_safe(node, tmp, &zv_alloc_list, list) {
        if (node->ptr == ptr) {
            list_del(&node->list);
            kfree(node);
            spin_unlock(&zv_alloc_list_lock);
            return true;
        }
    }
    spin_unlock(&zv_alloc_list_lock);
    return false;
}

void zv_kfree(void* ptr) {
    if (! ptr) return;

    if (zv_remove_node(ptr)) {
        kfree(ptr);
    } else {
        zv_log_write(LOG_NONE, "Mem", "Warning: kfree called on untracked ptr %p", ptr);
    }
}

void zv_vfree(void* ptr) {
    if (! ptr) return;

    if (zv_remove_node(ptr)) {
        vfree(ptr);
    } else {
        zv_log_write(LOG_NONE, "Mem", "Warning: vfree called on untracked ptr %p", ptr);
    }
}

void zv_free_page(void* ptr) {
    if (! ptr) return;

    if(zv_remove_node(ptr)) {
        free_page((unsigned long) ptr);
    } else {
        zv_log_write(LOG_NONE, "Mem", "Warning: free_page called on untracked ptr %p", ptr);
    }
}

/* This function is used in rmmod(or similar cases) to free all the allocated memory */
void zv_free_all(void) {
    struct zv_alloc_node *node, *tmp;

    spin_lock(&zv_alloc_list_lock);
    list_for_each_entry_safe(node, tmp, &zv_alloc_list, list) {
        list_del(&node->list);
        spin_unlock(&zv_alloc_list_lock);  // avoid holding lock too long

        switch (node->type) {
            case ZV_KMALLOC:
                kfree(node->ptr);
                break;
            case ZV_VMALLOC:
                vfree(node->ptr);
                break;
            case ZV_ALLOC_PAGE:
                free_page((unsigned long)node->ptr);
                break;
        }
        kfree(node);

        spin_lock(&zv_alloc_list_lock);  // re-lock
    }
    spin_unlock(&zv_alloc_list_lock);
}

void zv_dump_allocs(void)
{
    struct zv_alloc_node *node;

    zv_log_write(LOG_DEBUG, "Mem", "Current tracked allocations: ");

    spin_lock(&zv_alloc_list_lock);
    list_for_each_entry(node, &zv_alloc_list, list) {
        zv_log_write(LOG_DEBUG, "Mem", "  - type = %d ptr = %p", node->type, node->ptr);
    }
    spin_unlock(&zv_alloc_list_lock);
}