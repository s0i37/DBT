#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>

#include "tcg-plugin.h"

#define VERSION "0.10"

unsigned int asid = 0;
uint64_t bb_count = 0;

/* =============================================================== Translation-time =============================================================== */

static void before_gen_tb(const TCGPluginInterface *tpi)
{
    tpi->tb;
}

static void after_gen_tb(const TCGPluginInterface *tpi)
{
    tpi_current_cpu_index(tpi);
    tpi_tb_icount(tpi->tb);
}

static void after_gen_opc(const TCGPluginInterface *tpi, const TPIOpCode *tpi_opcode)
{
    const char *symbol = NULL;
    const char *filename = NULL;
    uint64_t symbol_address = 0;
    uint64_t symbol_size = 0;
    if (tpi_opcode->operator != INDEX_op_insn_start)
      return;

    if (!lookup_symbol4(
            tpi_opcode->pc, &symbol, &filename, &symbol_address, &symbol_size)
        || symbol[0] == '\0')
      return;
}


/* ================================================================ Execution-time ================================================================ */

static void pre_tb_helper_data(const TCGPluginInterface *tpi,
                               TPIHelperInfo info, uint64_t address,
                               uint64_t *data1, uint64_t *data2,
                               const TranslationBlock* tb)
{
    const char *symbol = NULL;
    const char *filename = NULL;
    lookup_symbol2(address, &symbol, &filename);
    *data1 = (uintptr_t)symbol;       // используется для передачи произвольных данных
    *data2 = (uintptr_t)filename;     // между вызовами pre_tb_helper_data/pre_tb_helper_code
}

static void pre_tb_helper_code(const TCGPluginInterface *tpi,
                               TPIHelperInfo info, uint64_t address,
                               uint64_t data1, uint64_t data2,
                               const TranslationBlock* tb)
{
    void *guest_mem;
    unsigned int i;
    CPUArchState *cpu = tpi_current_cpu_arch(tpi);
    /*
      tpi_thread_pid(tpi);        - qemu-user: id процесса
      tpi_thread_tid(tpi);        - qemu-user: поток
      tpi_current_cpu_index(tpi); - id виртуального процессора
      tpi_tb_size(tb);            - размер TCG BB
      tpi_tb_icount(tb);          - количество TCG инструкций
    */

    /*
      TranslationBlock *tpi - информация о конфигурации плагина
      TPIHelperInfo info - информация о Guest базовом блоке, который выполняется
      {
          uint16_t cpu_index;
          uint16_t size;
          uint32_t icount;
      }
      uint64_t address  - EIP
      uint64_t data1    - произвольные данные из pre_tb_helper_data()
      uint64_t data2    - произвольные данные из pre_tb_helper_data()
      TranslationBlock *tb  - транслированный TCG BB, работа с этой структурой через tpi_tb_*() функции
      {
          target_ulong pc;      - адрес 
          uint16_t size;        - размер
          uint16_t icount;      - количество TCG инструкций
          ...
      }
    */
    
    //symbol = (const char *)(uintptr_t)data1;
    //filename = (const char *)(uintptr_t)data2;
    
    if((unsigned int) ( (CPUX86State *)cpu )->cr[3] != asid)
      return;

    guest_mem = tpi_guest_ptr(tpi, address);   // доступ к Guest memory

    fprintf(tpi->output, "%lli:0x%08x:0x%x (i:%d,b:%d){", 
      bb_count,
      (unsigned int) ( (CPUX86State *)cpu )->eip,
      ( (struct X86CPU *)cpu )->thread_id,
      info.icount, info.size);
    /*for(i = 0; i < info.size; i++) ACCESS VIOLATION sometimes
      fprintf(tpi->output, "%02X", ((unsigned char *)guest_mem)[i] );*/
    fprintf(tpi->output, "} 0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x\n",
      (unsigned int) ( (CPUX86State *)cpu )->regs[R_EAX],
      (unsigned int) ( (CPUX86State *)cpu )->regs[R_ECX],
      (unsigned int) ( (CPUX86State *)cpu )->regs[R_EDX],
      (unsigned int) ( (CPUX86State *)cpu )->regs[R_EBX],
      (unsigned int) ( (CPUX86State *)cpu )->regs[R_ESP],
      (unsigned int) ( (CPUX86State *)cpu )->regs[R_EBP],
      (unsigned int) ( (CPUX86State *)cpu )->regs[R_ESI],
      (unsigned int) ( (CPUX86State *)cpu )->regs[R_EDI]);

    bb_count++;
}

/* ================================================================ cpu stop ================================================================ */

static void cpus_stopped(const TCGPluginInterface *tpi)
{
  fprintf(tpi->output, "[done]\n");
}

void tpi_init(TCGPluginInterface *tpi)
{
    /* QEMU транслирует Guest code в TCG code порциями */
    TPI_INIT_VERSION_GENERIC(tpi);

    /* Translation-time - вызывается один раз для каждого нового Guest BB*/
//    tpi->before_gen_tb = before_gen_tb;         // перед трансляцией Guest BB -> TCG BB
//    tpi->after_gen_tb = after_gen_tb;           // после трансляции Guest BB -> TCG BB
//    tpi->after_gen_opc = after_gen_opc;         // после трансляции каждой TCG instr       

    /* Execution-time - вызывается на каждом выполнении TCG BB */
//    tpi->pre_tb_helper_data = pre_tb_helper_data; // перед выполнением транслированного TCG BB
    tpi->pre_tb_helper_code = pre_tb_helper_code; // выполнение транслированного TCG BB

    tpi->cpus_stopped = cpus_stopped;             // qemu-system: выключение, перезагрузка; qemu-user: завершение

    /* TPI_OUTPUT=out.log TPI_LOW_PC=0xFROM TPI_HIGH_PC=0xTO TPI_VERBOSE=1 qemu-system -hda os.qcow */
    if(getenv("QEMU_CR3"))
      asid = atoi(getenv("QEMU_CR3"));
}
