/*
 * TCG plugin for QEMU: count the number of executed instructions per
 *                      CPU.
 *
 * Copyright (C) 2011 STMicroelectronics
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>

#include "tcg-plugin.h"

#include <capstone/capstone.h>

csh cs_handle;
cs_insn *insn;
size_t count = 0;
uint64_t ins_count = 0;

static void pre_tb_helper_code(const TCGPluginInterface *tpi,
                               TPIHelperInfo info, uint64_t address,
                               uint64_t data1, uint64_t data2,
                               const TranslationBlock* tb)
{
    CPUArchState *cpu = tpi_current_cpu_arch(tpi);
    
    if((unsigned int) ( (CPUX86State *)cpu )->cr[3] != 0x00a2f000)  /* just kernel */
      return;
    
    ins_count++;
    if(ins_count%10000 != 0)
      return;

    /* also we can change current CPU state - leads to BSOD of course */
    //( (CPUX86State *)cpu )->regs[R_EAX] = 0x13371337;

    if(count)
      fprintf(tpi->output, "0x%08x: %s %s\n", insn->address, insn->mnemonic, insn->op_str);
    fprintf(tpi->output, "EIP: 0x%08lx\n", address);
    fprintf(tpi->output, "EAX: 0x%08x ", (unsigned int) ( (CPUX86State *)cpu )->regs[R_EAX] );
    fprintf(tpi->output, "ECX: 0x%08x ", (unsigned int) ( (CPUX86State *)cpu )->regs[R_ECX] );
    fprintf(tpi->output, "EDX: 0x%08x ", (unsigned int) ( (CPUX86State *)cpu )->regs[R_EDX] );
    fprintf(tpi->output, "EBX: 0x%08x\n", (unsigned int) ( (CPUX86State *)cpu )->regs[R_EBX] );
    fprintf(tpi->output, "ESP: 0x%08x ", (unsigned int) ( (CPUX86State *)cpu )->regs[R_ESP] );
    fprintf(tpi->output, "EBP: 0x%08x ", (unsigned int) ( (CPUX86State *)cpu )->regs[R_EBP] );
    fprintf(tpi->output, "ESI: 0x%08x ", (unsigned int) ( (CPUX86State *)cpu )->regs[R_ESI] );
    fprintf(tpi->output, "EDI: 0x%08x\n", (unsigned int) ( (CPUX86State *)cpu )->regs[R_EDI] );
    fprintf(tpi->output, "CR0: 0x%08x ", (unsigned int) ( (CPUX86State *)cpu )->cr[0] );
    fprintf(tpi->output, "CR2: 0x%08x ", (unsigned int) ( (CPUX86State *)cpu )->cr[2] );
    fprintf(tpi->output, "CR3: 0x%08x ", (unsigned int) ( (CPUX86State *)cpu )->cr[3] );
    fprintf(tpi->output, "CR4: 0x%08x\n", (unsigned int) ( (CPUX86State *)cpu )->cr[4] );
    fprintf(tpi->output, "================================================================\n");
    //sleep(1);
}

/*
static void pre_tb_helper_data(const TCGPluginInterface *tpi,
                                          TPIHelperInfo info, uint64_t address,
                                          uint64_t *data1, uint64_t *data2,
                                          const TranslationBlock* tb)
{

}
*/

static void after_gen_opc(const TCGPluginInterface *tpi, const TPIOpCode *tpi_opcode)
{
    cs_insn *insns;

    /* SEGFAULTs happen sometimes */
    //count = cs_disasm(cs_handle, (void *)(intptr_t)tpi_guest_ptr(tpi, tpi_opcode->pc), 16, tpi_opcode->pc, 1, &insns);
    if(count)
      insn = &insns[0];
}

static void cpus_stopped(const TCGPluginInterface *tpi)
{
    /* because (monitor) loadvm state   leads cpu stop */
    //cs_free(insn, 1);
    //cs_close(&cs_handle);
}

void tpi_init(TCGPluginInterface *tpi)
{
    TPI_INIT_VERSION_GENERIC(tpi);

    if (cs_open(CS_ARCH_X86, CS_MODE_64, &cs_handle) != CS_ERR_OK)
      return;

    tpi->pre_tb_helper_code = pre_tb_helper_code;
    //tpi->pre_tb_helper_data = pre_tb_helper_data;
    tpi->after_gen_opc = after_gen_opc;
    tpi->cpus_stopped = cpus_stopped;
}
