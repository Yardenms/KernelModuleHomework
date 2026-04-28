# Xor ELF binfmt

Xor ELF binfmt is a linux kernel module that allows executing ELF files that were encrypted with xor encryption. We will call such files xelf files.

##  XELF format
The first four bytes of XELF files are 0x12 0x34 0x56 0x78, these are the MAGIC bytes. The module knows whether it should handle a file by whether it starts with these 4 MAGIC bytes or not.
The fifth byte is the xor key.
The rest of the bytes in the file are the entirety of the original ELF file (including its original header) after each byte was xor'd with the xor key from before.

## How does the module work

The module initially checks if the file starts with the 4 MAGIC bytes. If it doesn't the module returns -ENOEXEC which allows search_binary_handler to keep searching for the appropriate handler.
The other case is that the file does start with the MAGIC bytes. The module will then
1) Read the xor key (fifth byte)
2) Create a new file in memory
3) Write into the new file all the bytes from the original file from position 5 and onward after applying xor with the key to each byte
4) Update the file field of bprm to point to the new file
5) Update the buf field of bprm to contain the first 128 bytes of the new decrypted file
6) Return -NOEXEC so search_binary_handler continues until it reaches the handler for regular ELF files

### notice
The paper recommended using search_binary_handler
this function isn't exported so i couldn't use it
instead the binfmt return -ENOEXEC on success too
it works because this allows search_binary_handler to try the next handlers in the list
and installing the module with insmod puts it at the start of the list

## Files
The project contains

1. ```./install_scripts.sh```, a script which installs all the required dependencies

2. The ```binfmt``` folder which includes the code and makefile of the kernel module

3. The ```utils``` folder which includes the code and make file for a simple "Hello World" program and an encryption script. Which receives a path to a regular ELF file and a key and encrypts the ELF file with the key using xor encryption and saves the file in XELF format.


## Installation

1. Clone the project, for example you can clone the project using https with the following command ```https://github.com/Yardenms/KernelModuleHomework.git```
2. Enter the project folder using ```cd KernelModuleHomework```
3. Run the following command ```./install_scripts.sh``` to make sure all the dependencies required to compile and run the module are installed
4. To compile the module enter the ```binfmt``` directory and run ```make```
5. To install the module run ```sudo insmod out\xor_binfmt.ko```
6. You can now run XELF files by simply running them

### The module was only tested on Ubuntu 22.04

## Extras
### Usage example with encryptor
1. Enter the `utils` folder
2. Run ```make```, this will compile the Encryptor and a simple "Hello World" program and create two new ELF files `encryptor` and `hello_world`
3. Encrypt the `hello_world` ELF file using the following command ```./encryptor hello_world 10```.
This will encrypt the hello_world program with xor encryption and use `10` as a key, it will then save the file in XELF format to `encrypted_hello_world`
4. After installing the module you can now run the encrypted hello_world program using ```./encrypted_hello_world```


## Theoretical Question
I think such modules could allow running malicious files on a machine while making it harder to detect once you manage to install the module on a target machine. For example regular signature detection that some anti-viruses use might not work on such encrypted ELF files, signature detection looks for predetermined patterns of bytes while the patterns in encrypted files will be completely different, causing them to be missed. It can also reduce detection even when having the file looked at manually, while regular non-obfusticated executeables can be decompiled and have their code and purpose easily analyzed, simply using a decompiler won't work on such encrypted files. The person analyzing will first have to figure how to decrypt the file or read it from memory after it was decrypted. This allows sneaking malicious code or a hidden functionality into the software while making it hard for the user to either find out about it at all or find out how it works.
