#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int __init zeroVisor_init(void) {
    printk(KERN_INFO "Hello, zeroVisor loaded!\n");
    return 0;
}

static void __exit zeroVisor_exit(void) {
    printk(KERN_INFO "GoodBye, zeroVisor!\n");
}

module_init(zeroVisor_init);
module_exit(zeroVisor_exit);


MODULE_AUTHOR("Hubert Yao");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("zeroVisor: A Type1.5 hypervisor skeleton —— Everything starts from zero.");