#!/bin/sh

[ $1 = 'gdb' ] && extra_opts='-s -S'

qemu-system-arm -kernel one -M versatilepb -nographic $extra_opts
