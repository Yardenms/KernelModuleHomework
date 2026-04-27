#include <stdio.h>
#include <stdlib.h>

//const unsigned char MY_MAGIC[4] = {(unsigned char) 0x7F, (unsigned char) 0x45, (unsigned char) 0x4C, (unsigned char) 0x46};
const unsigned char MY_MAGIC[4] = {(unsigned char) 0x12, (unsigned char) 0x34, (unsigned char) 0x56, (unsigned char) 0x78};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <file_path> <key_0_255>\n", argv[0]);
        return 1;
    }

    char *path = argv[1];
    unsigned char key = (unsigned char)atoi(argv[2]);

    FILE *src = fopen(path, "rb");
    if (!src) { perror("Failed to open source"); return 1; }

    char out_path[256];
    snprintf(out_path, sizeof(out_path), "encrypted_%s", path);
    FILE *dst = fopen(out_path, "wb");

    fwrite(MY_MAGIC, 1, 4, dst);
    fputc(key, dst);

    int c;
    while ((c = fgetc(src)) != EOF) {
        fputc(c ^ key, dst);
    }

    printf("Created: %s with key 0x%02x\n", out_path, key);

    fclose(src);
    fclose(dst);
    return 0;
}