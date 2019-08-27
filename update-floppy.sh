#!/bin/sh

mount boot/os.img /mnt
cp i386-kernel /mnt/i386-kernel
cp boot/menu.lst /mnt/menu.lst
umount /mnt
