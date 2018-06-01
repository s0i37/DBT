echo trace >> panda/plugins/config.panda

make M=panda/plugins/trace

./i386-softmmu/qemu-system-i386 -replay record_name -m 2047M -os windows-32-7 -panda trace:asid=$((0x19498000)),to=$((0x80000000))