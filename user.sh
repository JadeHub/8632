cd userspace
make
cp user_space ../sysroot/initrd/bin/
cd ..
cd ramfs_pack/
./pack.sh
cd ..
./make_iso.sh


