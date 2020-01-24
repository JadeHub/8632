cd libc
make -j4 $1
cd ..
cd userspace
make -j4 $1
cd ..
make -j4 $1

