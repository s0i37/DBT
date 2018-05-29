#include "panda/plugin.h"
#include "cpu.h"
#include "trace_int_fns.h"

#include <stdio.h>
#include <stdlib.h>

#define VERSION "0.10"
#define MAX_OPCODE_SIZE 15

bool translate_callback(CPUState *env, target_ulong pc);
int exec_callback(CPUState *env, target_ulong pc);

int virt_mem_before_read(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size);
int virt_mem_before_write(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf);
int virt_mem_after_read(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf);
int virt_mem_after_write(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf);

int phys_mem_before_read(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size);
int phys_mem_before_write(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf);
int phys_mem_after_read(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf);
int phys_mem_after_write(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf);

bool init_plugin(void *);
void uninit_plugin(void *);

int ins_count;
FILE * out_file;
uint64_t asid = 0;
uint64_t from = 0;
uint64_t to = 0;

/*
uint64_t asids[100];
void init_asids(void);
unsigned int check_asid(uint64_t asid);
void add_asid(uint64_t asid);

void init_asids(void)
{
	unsigned int i;
	for( i = 0; i < 100; i++ )
		asids[i] = -1;
}
unsigned int check_asid(uint64_t asid)
{
	unsigned int i;
	for( i = 0; i < 100; i++ )
	{
		if( asids[i] == asid )
			return -1;
		else if( asids[i] == -1 )
			return i;
	}
	return -1;
}
void add_asid(uint64_t asid)
{
	unsigned int i;
	if( ( i = check_asid(asid) ) == -1 )
		return;
	asids[i] = asid;
	printf("%lx\n", asid);
}
*/


bool translate_callback(CPUState *env, target_ulong pc)
{
	return true;
}

/*
int virt_mem_before_read(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size)
{
#ifdef TARGET_I386
	//CPUState * cpu = first_cpu;
	CPUArchState * cpu = (CPUArchState *)env->env_ptr;
	if( (uint64_t)( (CPUX86State *)cpu )->cr[3] != asid )
		return 0;
	fprintf( out_file, "read [0x%08x,0x%08x]:0x%08x(%u)->?\n", 
		(unsigned int)pc,
		(unsigned int)( (CPUX86State *)cpu )->eip, 
		(unsigned int)addr, 
		(unsigned int)size );
#endif
	return 0;
}
int virt_mem_before_write(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf)
{
#ifdef TARGET_I386
	//CPUState * cpu = first_cpu;
	CPUArchState * cpu = (CPUArchState *)env->env_ptr;
	if( (uint64_t)( (CPUX86State *)cpu )->cr[3] != asid )
		return 0;
	fprintf( out_file, "write [0x%08x,0x%08x]:0x%08x(%u)<-%08X\n", 
		(unsigned int)pc,
		(unsigned int)( (CPUX86State *)cpu )->eip, 
		(unsigned int)addr, 
		(unsigned int)size, 
		*((unsigned int *)buf) );
#endif
	return 0;
}
*/

int virt_mem_after_read(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf)
{
#ifdef TARGET_I386
	unsigned int i;
	//CPUState * cpu = first_cpu;
	CPUArchState * cpu = (CPUArchState *)env->env_ptr;
	if( asid == 0 || (uint64_t)( (CPUX86State *)cpu )->cr[3] != asid )
		return 0;
	fprintf( out_file, "r [0x%08x]: 0x%08x(%u) -> ", (unsigned int)pc, (unsigned int)addr, (unsigned int)size );
	for(i = 0; i < size; i++)
		fprintf( out_file, "%02X", * (unsigned char*) ( (unsigned long)buf + i ) );
	fprintf( out_file, "\n" );
#endif
	return 0;
}

int virt_mem_after_write(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf)
{
#ifdef TARGET_I386
	unsigned int i;
	//CPUState * cpu = first_cpu;
	CPUArchState * cpu = (CPUArchState *)env->env_ptr;
	if( asid == 0 || (uint64_t)( (CPUX86State *)cpu )->cr[3] != asid )
		return 0;
	fprintf( out_file, "w [0x%08x]: 0x%08x(%u) <- ", (unsigned int)pc, (unsigned int)addr, (unsigned int)size );
	for(i = 0; i < size; i++)
		fprintf( out_file, "%02X", * (unsigned char*) ( (unsigned long)buf + i ) );
	fprintf( out_file, "\n" );
#endif
	return 0;
}

/*
int phys_mem_before_read(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size)
{
#ifdef TARGET_I386
	//CPUState * cpu = first_cpu;
	CPUArchState * cpu = (CPUArchState *)env->env_ptr;
	if( (uint64_t)( (CPUX86State *)cpu )->cr[3] != asid )
		return 0;
	fprintf( out_file, "read phys before [0x%08x]0x%08x (%u)->?\n", 
		(unsigned int)( (CPUX86State *)cpu )->eip, 
		(unsigned int)addr, 
		(unsigned int)size );
#endif
	return 0;
}
int phys_mem_before_write(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf)
{
#ifdef TARGET_I386
	//CPUState * cpu = first_cpu;
	CPUArchState * cpu = (CPUArchState *)env->env_ptr;
	if( (uint64_t)( (CPUX86State *)cpu )->cr[3] != asid )
		return 0;
	fprintf( out_file, "write phys before [0x%08x]0x%08x (%u)->%08X\n", 
		(unsigned int)( (CPUX86State *)cpu )->eip, 
		(unsigned int)addr, 
		(unsigned int)size, 
		*((unsigned int *)buf) );
#endif
	return 0;
}
int phys_mem_after_read(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf)
{
#ifdef TARGET_I386
	//CPUState * cpu = first_cpu;
	CPUArchState * cpu = (CPUArchState *)env->env_ptr;
	if( (uint64_t)( (CPUX86State *)cpu )->cr[3] != asid )
		return 0;
	fprintf( out_file, "read phys after [0x%08x]0x%08x (%u)->%08X\n", 
	   	(unsigned int)( (CPUX86State *)cpu )->eip, 
	   	(unsigned int)addr, 
	   	(unsigned int)size, 
	   	*((unsigned int *)buf) );
#endif
    return 0;
}
int phys_mem_after_write(CPUState *env, target_ulong pc, target_ulong addr, target_ulong size, void *buf)
{
#ifdef TARGET_I386
	//CPUState * cpu = first_cpu;
	CPUArchState * cpu = (CPUArchState *)env->env_ptr;
	if( (uint64_t)( (CPUX86State *)cpu )->cr[3] != asid )
		return 0;
	fprintf( out_file, "write phys after [0x%08x]0x%08x (%u)->%08X\n", 
	  	(unsigned int)( (CPUX86State *)cpu )->eip, 
	   	(unsigned int)addr, 
	   	(unsigned int)size, 
	   	*((unsigned int *)buf) );
#endif
    return 0;
}
*/

int exec_callback(CPUState *env, target_ulong pc)
{
#ifdef TARGET_I386
	unsigned int i;
	uint8_t buf[MAX_OPCODE_SIZE];
	//CPUState * cpu = first_cpu;
	CPUArchState * cpu = (CPUArchState *)env->env_ptr;
	//add_asid( (uint64_t)( (CPUX86State *)cpu )->cr[3] );
	if( asid == 0 || (uint64_t)( (CPUX86State *)cpu )->cr[3] != asid )
		return 0;
	if( (from == 0 && to == 0) || ((unsigned int)pc >= from && (unsigned int)pc <= to) )
	{
		fprintf( out_file, "0x%08x:%x {", (unsigned int)pc, ( (struct X86CPU *)cpu )->thread_id );
		panda_virtual_memory_read(env, pc, buf, MAX_OPCODE_SIZE+1);
		for(i = 0; i < MAX_OPCODE_SIZE; i++)
			fprintf( out_file, "%02X", ( (unsigned char *)buf )[i] );
		fprintf( out_file, "} 0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x\n",
			(unsigned int)( (CPUX86State *)cpu )->regs[R_EAX],
			(unsigned int)( (CPUX86State *)cpu )->regs[R_ECX],
			(unsigned int)( (CPUX86State *)cpu )->regs[R_EDX], 
			(unsigned int)( (CPUX86State *)cpu )->regs[R_EBX], 
			(unsigned int)( (CPUX86State *)cpu )->regs[R_ESP], 
			(unsigned int)( (CPUX86State *)cpu )->regs[R_EBP], 
			(unsigned int)( (CPUX86State *)cpu )->regs[R_ESI], 
			(unsigned int)( (CPUX86State *)cpu )->regs[R_EDI]
		);
		ins_count++;
		if( ins_count % 1000000 == 0 )
			printf( "ins_count: %d M\n", ins_count/1000000 );
	}
		/*(unsigned int)( (CPUX86State *)cpu )->cr[0], 
		(unsigned int)( (CPUX86State *)cpu )->cr[1], 
		(unsigned int)( (CPUX86State *)cpu )->cr[2], 
		(unsigned int)( (CPUX86State *)cpu )->cr[3], */
		//(unsigned int) panda_current_asid( ENV_GET_CPU( (struct CPUState *)cpu ) ), 
		//(unsigned int)( (CPUX86State *)cpu )->eip,
#endif
	return 0;
}

panda_arg_list *args;
bool init_plugin(void *self)
{
	panda_cb pcb;
	const char * out_file_name;
	ins_count = 0;
	//init_asids();
	args = panda_get_args("trace");
	printf("[*] trace " VERSION "\n");
	panda_enable_memcb();
	panda_enable_precise_pc();
	/*
	if(args)
		for(i = 0; i < args->nargs; i++)
		{
			if( strncmp(args->list[i].key, "file", 4) == 0 )
				out_file_name = args->list[i].value;
			else if( strncmp(args->list[i].key, "asid", 4) == 0 )
			{
				asid = (uint64_t) atol( args->list[i].key );
				printf("[+] select CR3=%lx\n", asid);
			}
		}
	*/
	out_file_name = panda_parse_string_opt(args, "outfile", "trace.txt", "output filename, where will be store trace information in humanable form");
	asid = panda_parse_uint64_opt(args, "asid", 0, "only this CR3 (default all address spaces)");
	from = panda_parse_uint64_opt(args, "from", 0, "start address for tracing");
	to = panda_parse_uint64_opt(args, "to", 0, "end address for tracing");
	if(asid)
		printf("[+] selected CR3=0x%lx\n", asid);
	if(from || to)
		printf("[+] from: 0x%08lx, to: 0x%08lx\n", from, to);

	out_file = fopen(out_file_name, "w");

	pcb.insn_translate = translate_callback;
	panda_register_callback(self, PANDA_CB_INSN_TRANSLATE, pcb);
	pcb.insn_exec = exec_callback;
	panda_register_callback(self, PANDA_CB_INSN_EXEC, pcb);

	//pcb.virt_mem_before_read = virt_mem_before_read;
	//panda_register_callback(self, PANDA_CB_VIRT_MEM_BEFORE_READ, pcb);
	//pcb.virt_mem_before_write = virt_mem_before_write;
	//panda_register_callback(self, PANDA_CB_VIRT_MEM_BEFORE_WRITE, pcb);

	/*
	pcb.virt_mem_after_read = virt_mem_after_read;
	panda_register_callback(self, PANDA_CB_VIRT_MEM_AFTER_READ, pcb);
	pcb.virt_mem_after_write = virt_mem_after_write;
	panda_register_callback(self, PANDA_CB_VIRT_MEM_AFTER_WRITE, pcb);
	*/

	//pcb.phys_mem_before_read = phys_mem_before_read;
	//panda_register_callback(self, PANDA_CB_PHYS_MEM_BEFORE_READ, pcb);
	//pcb.phys_mem_before_write = phys_mem_before_write;
	//panda_register_callback(self, PANDA_CB_PHYS_MEM_BEFORE_WRITE, pcb);

	//pcb.phys_mem_after_read = phys_mem_after_read;
    //panda_register_callback(self, PANDA_CB_PHYS_MEM_AFTER_READ, pcb);
    //pcb.phys_mem_after_write = phys_mem_after_write;
    //panda_register_callback(self, PANDA_CB_PHYS_MEM_AFTER_WRITE, pcb);
	return 1;
}

void uninit_plugin(void *self)
{
	printf("[done]\n");
	panda_free_args(args);
	fflush(out_file);
	fclose(out_file);
}
