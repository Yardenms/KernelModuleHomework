#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

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
	{ 0xc6a83232, "module_layout" },
	{ 0xd83a484d, "unregister_binfmt" },
	{ 0x7813854b, "__register_binfmt" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x37a0cba, "kfree" },
	{ 0x6e31897, "fput" },
	{ 0xdc5b0bf4, "kernel_write" },
	{ 0x469dd7f9, "kernel_read" },
	{ 0x14208a30, "shmem_file_setup" },
	{ 0xe71025ce, "kmem_cache_alloc_trace" },
	{ 0xb0549baa, "kmalloc_caches" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "7ECCC4F87B753A8893E4B75");
