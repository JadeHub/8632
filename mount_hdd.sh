sudo umount fimg
sudo losetup -d /dev/loop21

sudo losetup /dev/loop21 hdd.img -o 1048576
sudo mount /dev/loop21 fimg/

