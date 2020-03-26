Speed: 200k/s

echo trace >> panda/plugins/config.panda

make M=panda/plugins/trace

./i386-softmmu/qemu-system-i386 -hda winxp.qcow2 -m 1G -monitor stdio

(qemu) begin_record test1

(qemu) end_record

(qemu) pmemsave 0 0x40000000 pmem.dmp

volatility -f pmem.dmp --profile=WinXPSP3x86 psscan

Dump execution trace for process with Address Space ID 0x19498000 (CR3, PDB):

./i386-softmmu/qemu-system-i386 -replay test1 -m 1G -os windows-32-7 -panda trace:asid=$((0x19498000)),to=$((0x80000000)),near_bytes=6
