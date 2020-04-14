cd userspace
make -j
cp bin/* ../sysroot/initrd/bin/
cd ..

cd shell
make -j
cp shell ../sysroot/initrd/bin/
cd ..

cd ramfs_pack/
./pack.sh

cd ..
./make_iso.sh


