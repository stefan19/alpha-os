#!/bin/sh

cp i386-kernel iso/
cp i386-loader iso/
grub-mkrescue -o boot.iso iso
