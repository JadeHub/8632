cd test
nasm  -f bin test.asm 
cd ..
dd conv=notrunc if=test/test of=hdd.img

