

#example1: example1.c
#	gcc -o example1 example1.c -I/usr/local/include/freetype2/ -lfreetype -lm

example1_arm-linux: example1.c
	/work/arm_linux_gcc/gcc-linaro-4.9.4-2017.01-x86_64_armv8l-linux-gnueabihf/bin/armv8l-linux-gnueabihf-gcc -o $@ example1.c -I/work/arm_linux_gcc/gcc-linaro-4.9.4-2017.01-x86_64_armv8l-linux-gnueabihf/armv8l-linux-gnueabihf/libc/usr/include/freetype2 -lfreetype -lm

cp:


clean:
	rm example1




