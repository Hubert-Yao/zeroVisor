#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "../include/zv_log.h"

static int __init zeroVisor_init(void) {
    printk(KERN_INFO "Hello, zeroVisor loaded!\n");
    zv_log_init();

    zv_log_write(LOG_NORMAL, "Core", "Test for log.");
    return 0;
}

static void __exit zeroVisor_exit(void) {
    printk(KERN_INFO "GoodBye, zeroVisor!\n");
    
    zv_log_exit();
}

module_init(zeroVisor_init);
module_exit(zeroVisor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hubert Yao");
MODULE_DESCRIPTION("zeroVisor: A Type1.5 hypervisor skeleton —— Everything starts from zero.");
MODULE_VERSION("1.0");