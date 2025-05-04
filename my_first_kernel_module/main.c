#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DOH LANDRINE");
MODULE_DESCRIPTION("A simple module for learning purpose");

static int __init initialization_module(void){
    pr_info("My module is now loaded into my Kernel\n!");
    return 0;
}

static void __exit exit_module(void){
    pr_info("My module has been removed from my Kernel\n!");
}

module_init(initialization_module);
module_exit(exit_module);
