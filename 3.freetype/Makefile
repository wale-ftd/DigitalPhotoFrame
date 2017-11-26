#target := show_font_arm-linux
#obj := show_font.c
target := show_lines_arm-linux
obj := show_lines.c

#example1: example1.c
#	gcc -o example1 example1.c -I/usr/local/include/freetype2/ -lfreetype -lm

#example1_arm-linux: example1.c
#	/work/arm_linux_gcc/gcc-linaro-4.9.4-2017.01-x86_64_armv8l-linux-gnueabihf/bin/armv8l-linux-gnueabihf-gcc -o $@ example1.c -I/work/arm_linux_gcc/gcc-linaro-4.9.4-2017.01-x86_64_armv8l-linux-gnueabihf/armv8l-linux-gnueabihf/libc/usr/include/freetype2 -lfreetype -lm

$(target): $(obj)
	/work/arm_linux_gcc/gcc-linaro-4.9.4-2017.01-x86_64_armv8l-linux-gnueabihf/bin/armv8l-linux-gnueabihf-gcc -finput-charset=UTF-8 -fexec-charset=GBK -o $@ $< -I/work/arm_linux_gcc/gcc-linaro-4.9.4-2017.01-x86_64_armv8l-linux-gnueabihf/armv8l-linux-gnueabihf/libc/usr/include/freetype2 -lfreetype -lm

cp:
	sudo cp $(target) /work/rootfs/x6818_rootfs_mini/test/ -f

clean:
	#rm example1
	#rm example1_arm-linux
	rm $(target)




