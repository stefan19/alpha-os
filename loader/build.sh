#!/bin/sh
make
grub-mkrescue -o boot.iso iso
