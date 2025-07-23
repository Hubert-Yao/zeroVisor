#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/utsname.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "../include/zv_log.h"
#include "../include/zv_config.h"
#include "../include/zv_types.h"
#include "../include/symbol.h"

/* Variables*/
int g_kernel_version_index = -1;

// kallsyms_lookup_name address exported by self
typedef unsigned long (*kallsyms_lookup_name_t)(const char *);
static kallsyms_lookup_name_t kallsyms_lookup_name = (kallsyms_lookup_name_t)KALLSYMS_LOOKUP_NAME_ADDR;


// static functions declarations
static void zv_print_logo(void);
static int zv_get_kernel_version_index(void);
static int zv_check_kaslr(void);

static int __init zeroVisor_init(void) {
    // variables init

    // sub-modules init
    zv_log_init();

    zv_log_write(LOG_NORMAL, "Core", "Hello, zeroVisor!");
    
    zv_print_logo();

    /* Check Kernel Version FOR use pre-definition symbol*/
    if (zv_get_kernel_version_index() == -1) {
        zv_log_write(LOG_NONE, "Core", "Kernel Version is not supported");
        zv_log_error(ERROR_KERNEL_VERSION_MISMATCH);
    }

    /* Check kASLR */
    if (zv_check_kaslr() == -1) {
        zv_log_write(LOG_DEBUG, "Core", "Kernel ASLR is enabled.");
    }
    
    return 0;
}

static void __exit zeroVisor_exit(void) {
    zv_log_write(LOG_NORMAL, "Core", "GoodBye, zeroVisor!");
    
    zv_log_exit();
}

/*
 * Print logo.
 */
static void zv_print_logo(void) {
    zv_log_write(LOG_NORMAL, "Core", "");
    zv_log_write(LOG_NORMAL, "Core", "███████╗███████╗██████╗  ██████╗     ██╗   ██╗██╗███████╗ ██████╗ ██████╗ ");
    zv_log_write(LOG_NORMAL, "Core", "╚══███╔╝██╔════╝██╔══██╗██╔═══██╗    ██║   ██║██║██╔════╝██╔═══██╗██╔══██╗");
    zv_log_write(LOG_NORMAL, "Core", "  ███╔╝ █████╗  ██████╔╝██║   ██║    ██║   ██║██║███████╗██║   ██║██████╔╝");
    zv_log_write(LOG_NORMAL, "Core", "  ███╔╝ █████╗  ██████╔╝██║   ██║    ██║   ██║██║███████╗██║   ██║██████╔╝");
    zv_log_write(LOG_NORMAL, "Core", " ███╔╝  ██╔══╝  ██╔══██╗██║   ██║    ╚██╗ ██╔╝██║╚════██║██║   ██║██╔══██╗");
    zv_log_write(LOG_NORMAL, "Core", "███████╗███████╗██║  ██║╚██████╔╝     ╚████╔╝ ██║███████║╚██████╔╝██║  ██║");
    zv_log_write(LOG_NORMAL, "Core", "╚══════╝╚══════╝╚═╝  ╚═╝ ╚═════╝       ╚═══╝  ╚═╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝");
    zv_log_write(LOG_NORMAL, "Core", "");
    zv_log_write(LOG_NORMAL, "Core", "                      A Type1.5 Hypervisor Skeleton");
    zv_log_write(LOG_NORMAL, "Core", "");
}

/*
 * Find matched index of current kernel version.
 */
static int zv_get_kernel_version_index(void) {
    int i;
    int match_index = -1;
    struct new_utsname* name;

    name = utsname();

    /* search kernel version table*/
    for (i = 0;  i < (sizeof(g_kernel_version) / sizeof(char*)); i ++) {
        if (strcmp(name->version, g_kernel_version[i]) == 0) {
            match_index = i;
            break;
        }
    }

    // transfer match_index to global varibale
    g_kernel_version_index = match_index;
    return match_index;
}

/*
 * Get address of kernel symbol.
 * kallsyms_lookup_name() does not have all symbol address which are in System.map.
 * if not found by kallsyms_lookup_name(), use pre-defined symbol table.
 */
u64 zv_get_symbol_address(char* symbol) {
    u64 symbol_address = 0;
    int i;

    symbol_address = kallsyms_lookup_name(symbol);

    // if cant found by kallsyms_lookup_name()
    if (symbol_address == 0) {
        if (g_kernel_version_index == -1) return 0;

        for (i = 0; i < SYMBOL_MAX_COUNT; i ++) {
            if (strcmp(g_symbol_table_array[g_kernel_version_index].symbol[i].name, symbol) == 0) {
                symbol_address = g_symbol_table_array[g_kernel_version_index].symbol[i].addr;
                break;
            }
        }
    }

    return symbol_address;
}

/*
 * Check kernel ASLR
 */
static int zv_check_kaslr(void) {
    int i;
    u64 static_text_addr;    
    u64 dynamic_text_addr = kallsyms_lookup_name("_etext");

    for (i = 0; i < SYMBOL_MAX_COUNT; i ++) {
        if (strcmp(g_symbol_table_array[g_kernel_version_index].symbol[i].name, "_etext") == 0) {
            static_text_addr = g_symbol_table_array[g_kernel_version_index].symbol[i].addr;
            break;
        }
    }

    if (static_text_addr != dynamic_text_addr) {
        zv_log_write(LOG_NORMAL, "Core", "_etext System.map=%lX Kallsyms=%lX", 
            static_text_addr, dynamic_text_addr);
        return -1;
    }

    return 0;
}

module_init(zeroVisor_init);
module_exit(zeroVisor_exit);

MODULE_AUTHOR("Hubert Yao");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("zeroVisor: A Type1.5 hypervisor skeleton —— Everything starts from zero.");
MODULE_VERSION("1.0");