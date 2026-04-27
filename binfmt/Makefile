obj-m += xor_binfmt.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

	mkdir -p ./out
	mv *.symvers *.order *.ko *.mod *.o *.mod.c ./out
	rm -f .xor_binfmt.* .cache.mk modules.order Module.symvers

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
