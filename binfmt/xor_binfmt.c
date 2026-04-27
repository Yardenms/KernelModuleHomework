#include <linux/module.h>
#include <linux/binfmts.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/shmem_fs.h>
#include <linux/slab.h>



static int load_xor_binary(struct linux_binprm *bprm) {
    const unsigned char MY_MAGIC[4] = {(unsigned char) 0x12, (unsigned char) 0x34, (unsigned char) 0x56, (unsigned char) 0x78};

    struct file *mem_file;
    loff_t pos = 5; // Start reading AFTER the 4-byte magic and 1-byte key
    ssize_t nread;
    unsigned char *buf;
    unsigned char key;
    int i, retval;

    // 1. Check for our custom magic "XORE"
    if (memcmp(bprm->buf, MY_MAGIC, 4) != 0) {
        return -ENOEXEC;
    }

    // 2. Extract the key from the 5th byte (index 4)
    key = bprm->buf[4];
    pr_info("xor_binfmt: Found XORE magic! Using key: 0x%02x\n", key);

    // 3. Setup in-memory file for the decrypted ELF
    // Size is original file minus our 5-byte header
    mem_file = shmem_file_setup("decrypted_elf", i_size_read(file_inode(bprm->file)) - 5, 0);
    if (IS_ERR(mem_file)) return PTR_ERR(mem_file);

    buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!buf) { fput(mem_file); return -ENOMEM; }

    // 4. Decrypt loop
    loff_t out_pos = 0;
    while ((nread = kernel_read(bprm->file, buf, PAGE_SIZE, &pos)) > 0) {
        for (i = 0; i < nread; i++) {
            buf[i] ^= key;
        }
        kernel_write(mem_file, buf, nread, &out_pos);
    }
    kfree(buf);

    // 5. Swap the file in the process parameters
    allow_write_access(mem_file);
    fput(bprm->file);
    bprm->file = mem_file;

    // Reset bprm buffer so the next handler (ELF) sees the decrypted start
    pos = 0;
    retval = kernel_read(bprm->file, bprm->buf, BINPRM_BUF_SIZE, &pos);

    // 6. Pass to standard ELF handler
    //return search_binary_handler(bprm);
    return -ENOEXEC;
}

static struct linux_binfmt xor_format = {
    .module      = THIS_MODULE,
    .load_binary = load_xor_binary,
};

static int __init init_xor(void) { register_binfmt(&xor_format); return 0; }
static void __exit exit_xor(void) { unregister_binfmt(&xor_format); }

module_init(init_xor);
module_exit(exit_xor);
MODULE_LICENSE("GPL");