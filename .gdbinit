set arch i386:x86-64:intel
symbol-file build/kernel.bin
target remote localhost:1234
break kmain
break irq.c:73
commands
p/x *frame
cont
end