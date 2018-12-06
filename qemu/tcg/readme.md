cp test.c path/to/qemu/tcg/plugins/

make M=tcg

path/to/qemu/x86_64-softmmu/qemu-system-x86_64 --tcg-plugin test -hda disk.qcow2

