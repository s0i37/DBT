Speed: 20M/s

## build qemu-tcg

git clone https://github.com/atos-tools/qemu && cd qemu

./configure --disable-capstone --enable-tcg-plugin --target-list=x86_64-softmmu

make CFLAGS='-g -w'

## build module

cp trace.c path/to/qemu/tcg/plugins/trace2.c

make M=tcg CFLAGS='-w' LDFLAGS='-lcapstone -shared'

QEMU_CR3=$[0x0b028300] ./x86_64-softmmu/qemu-system-x86_64 -hda os.qcow2 --tcg-plugin trace2
