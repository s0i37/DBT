## build qemu-tcg

git clone https://github.com/atos-tools/qemu && cd qemu

./configure --disable-capstone --enable-tcg-plugin --target-list=x86_64-softmmu

make CFLAGS='-g -w'

## build module

cp test.c path/to/qemu/tcg/plugins/

make M=tcg LDFLAGS='-lcapstone -shared'

path/to/qemu/x86_64-softmmu/qemu-system-x86_64 -hda disk.qcow2 --tcg-plugin test

