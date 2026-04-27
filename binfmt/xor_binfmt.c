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
    struct file *debug_file;
    struct file *mem_file;
    char *buf;
    loff_t pos = 5;      // Skip 4-byte magic + 1-byte key
    loff_t out_pos = 0;
    ssize_t nread;
    int i, retval;
    unsigned char key = bprm->buf[4];

    // 1. THE "RUN 2" GUARD
    // If it's already an ELF, stay out of the way so the ELF loader can work.
    if (bprm->buf[0] == 0x7f && bprm->buf[1] == 'E')
        return -ENOEXEC;

    // 2. THE MAGIC CHECK
    // Only proceed if our header matches.
    if (memcmp(bprm->buf, MY_MAGIC, 4) != 0)
        return -ENOEXEC;

    printk(KERN_ERR "XOR_DEBUG: Found encrypted binary. Starting decryption...\n");

    // 3. SETUP
    buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!buf) return -ENOMEM;

    // Create in-memory file for execution
    mem_file = shmem_file_setup("decrypted_mem", i_size_read(file_inode(bprm->file)) - 5, 0);
    if (IS_ERR(mem_file)) {
        kfree(buf);
        return PTR_ERR(mem_file);
    }

    // Open debug file for your inspection
    debug_file = filp_open("/tmp/debug_elf", O_CREAT | O_WRONLY | O_TRUNC, 0755);

    // 4. THE DECRYPTION LOOP
    while ((nread = kernel_read(bprm->file, buf, PAGE_SIZE, &pos)) > 0) {
        for (i = 0; i < nread; i++) {
            buf[i] ^= key;
        }
        
        kernel_write(mem_file, buf, nread, &out_pos);
        
        if (!IS_ERR(debug_file)) {
            kernel_write(debug_file, buf, nread, &(debug_file->f_pos));
        }
    }

    // 5. FORCE METADATA (So the file isn't empty)
    if (!IS_ERR(debug_file)) {
        vfs_fsync(debug_file, 0);
        // Force the inode size so 'ls' sees it
        inode_lock(file_inode(debug_file));
        i_size_write(file_inode(debug_file), out_pos);
        inode_unlock(file_inode(debug_file));
        filp_close(debug_file, NULL);
    }

    // 6. THE HANDOVER
    // Replace the encrypted file with our decrypted memory file
    fput(bprm->file);
    bprm->file = mem_file;

    // Refresh the bprm buffer so the next handler (ELF) sees the new magic
    pos = 0; 
    memset(bprm->buf, 0, BINPRM_BUF_SIZE);
    retval = kernel_read(bprm->file, bprm->buf, BINPRM_BUF_SIZE, &pos);
    
    if (retval < 0) {
        printk(KERN_ERR "XOR_DEBUG: Failed to refill bprm buffer\n");
        return retval;
    }
    
    kfree(buf);

    if (retval < 0) return retval;

    // Return -ENOEXEC to tell the kernel: "Restart the search with this new file!"
    printk(KERN_ERR "XOR_DEBUG: Decryption complete. Handing over to ELF loader.\n");
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
