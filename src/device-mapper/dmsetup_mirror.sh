
sudo losetup /dev/loop5 loop5.ima 
sudo losetup /dev/loop6 loop6.ima 
sudo mkfs.ext4 /dev/loop5
sudo dumpe2fs /dev/loop5 | head
sudo mkfs.ext4 /dev/loop6
sudo dumpe2fs /dev/loop6 | head

sudo mount /dev/loop5 /mnt/ ; mount | grep loop5
sudo umount /mnt/
sudo mount /dev/loop6 /mnt/ ; mount | grep loop
sudo umount /mnt/

sudo dmsetup create test-mirror --table '0 48193 mirror core 1 1024 2 /dev/loop5 0 /dev/loop6 0' # 48193 should not be smaller


#dmsetup create test-mirror --table '0 1953125 mirror core 1 1024 2 /dev/loop3 /dev/loop4 0 1 handle_errors'

sudo mount /dev/mapper/test-mirror /mnt/
