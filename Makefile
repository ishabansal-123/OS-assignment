all: 	
	qemu-system-x86_64 -kernel linux-5.11.5/arch/x86_64/boot/bzImage -boot c -m 400M -hda ./buildroot-2021.02.2/output/images/rootfs.ext4 -append "root=/dev/sda rw console=ttyS0,115200 acpi=off nokaslr" -serial stdio -display none 


fs:
	cd ./buildroot-2021.02.2 && ${MAKE}


