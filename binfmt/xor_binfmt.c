#include <linux/module.h>
#include <linux/binfmts.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/shmem_fs.h>

#define MY_MAGIC "\x12\x34\x56\x78"

static int load_xor_binary(struct linux_binprm *bprm)
{
    struct file *mem_file;
    char *buf;
    loff_t pos = 5;
    loff_t out_pos = 0;
    ssize_t nread;
    int i, retval;
    unsigned char key = bprm->buf[4];

    if (memcmp(bprm->buf, MY_MAGIC, 4) != 0)
        return -ENOEXEC;

    buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!buf) return -ENOMEM;

    mem_file = shmem_file_setup("decrypted_mem", i_size_read(file_inode(bprm->file)) - 5, 0);
    if (IS_ERR(mem_file)) {
        kfree(buf);
        return PTR_ERR(mem_file);
    }
    
    while ((nread = kernel_read(bprm->file, buf, PAGE_SIZE, &pos)) > 0) {
        for (i = 0; i < nread; i++) {
            buf[i] ^= key;
        }
        
        kernel_write(mem_file, buf, nread, &out_pos);
        
    }

    fput(bprm->file);
    bprm->file = mem_file;

    pos = 0; 
    memset(bprm->buf, 0, BINPRM_BUF_SIZE);
    retval = kernel_read(bprm->file, bprm->buf, BINPRM_BUF_SIZE, &pos);
    
    if (retval < 0) {
        return retval;
    }
    
    kfree(buf);

    if (retval < 0) return retval;
    
    /* The papaer recommended using search_binary_handler
    this function isn't exported so i couldn't use it
    instead the binfmt return -ENOEXEC on success too
    it works because this allows search_binary_handler to try the next handlers in the list
    installing the module with insmod puts it at the start of the list*/
    
    return -ENOEXEC;
}

static struct linux_binfmt xor_format = {
    .module      = THIS_MODULE,
    .load_binary = load_xor_binary,
};

static int __init init_xor(void) {
    register_binfmt(&xor_format);
    return 0;
}

static void __exit exit_xor(void) {
    unregister_binfmt(&xor_format);
}

module_init(init_xor);
module_exit(exit_xor);
MODULE_LICENSE("GPL");
