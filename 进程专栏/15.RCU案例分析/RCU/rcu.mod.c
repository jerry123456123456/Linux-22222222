#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xf9a482f9, "msleep" },
	{ 0x4c03a563, "random_kmalloc_seed" },
	{ 0xa63b4eed, "kmalloc_caches" },
	{ 0x59ffeca6, "kmalloc_trace" },
	{ 0x28aa6a67, "call_rcu" },
	{ 0x8d522714, "__rcu_read_lock" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x2469810f, "__rcu_read_unlock" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0x8142e62c, "kthread_create_on_node" },
	{ 0x4dd5a22c, "wake_up_process" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x122c3a7e, "_printk" },
	{ 0x37a0cba, "kfree" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xc0d3d03d, "kthread_stop" },
	{ 0xf079b8f9, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "EF7D3F0BC4888FED807765B");
