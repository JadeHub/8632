cd ktest
make -j
sudo cp bin/ktest ../fat_image/
sync
cd ..

cd shell
make -j
cp shell ../sysroot/initrd/bin/
cd ..

cd ramfs_pack/
./pack.sh

cd ..
./make_iso.sh


