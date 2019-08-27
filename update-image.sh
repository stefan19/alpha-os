#!/bin/sh
qemu-nbd -c /dev/nbd0 boot/OS.vdi
mount /dev/nbd0p1 /mnt
cp i386-kernel /mnt/i386-kernel
cp boot/menu.lst /mnt/menu.lst
cp init /mnt/init
umount /mnt
qemu-nbd -d /dev/nbd0
