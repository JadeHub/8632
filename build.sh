cd libc
make $1
cd ..
cd userspace
make $1
cd ..
make $1

