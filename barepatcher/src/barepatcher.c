/**
        Made by Edoardo Mantovani, 2025
        Complete in memory jit compiler for mt76 wifi hw (base version)
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdarg.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>

#include <elf.h>
#include <disasm.h>

#include <sys/mman.h>
#include <sys/stat.h>

#define IEEE80211_DEBUGFS               "/sys/kernel/debug/ieee80211/"
#define IO_PATH                         "/sys/kernel/debug/ieee80211/%s/mt76/%s"
#define MT76_DEBUGFS_PATH               "/sys/kernel/debug/ieee80211/phy%d/mt76/"
#define MT76_DEBUGFS_PATH_IO            "/sys/kernel/debug/ieee80211/phy%d/mt76/%s"

#define REGIDX                          "regidx"
#define REGVAL                          "regval"

#define MT76_MAX_LEN                    sizeof(MT76_DEBUGFS_PATH)
#define REGIDX_MAX_LEN                  sizeof(IO_PATH) + sizeof(REGIDX)
#define REGVAL_MAX_LEN                  sizeof(IO_PATH) + sizeof(REGVAL)

#define REGVAL_ALIGNMENT                sizeof(uint32_t)
#define REGVAL_BUFSIZE                  sizeof("0x80000000")
#define IO_COMMAND_LEN                  REGVAL_BUFSIZE

#define MAX_REGVAL_ADDR                 REGVAL_BUFSIZE

/** minimal disassemble context **/
typedef struct{
        char                            *insn_buffer;
        bool                             reenter;
}MT76_STREAM_STATE;

#define REGVAL_WRITE_FAIL                       2
__attribute__((hot)) static signed int REGVAL_SET_VALUE(unsigned char *REGVAL_NAME,
                                                        unsigned int   REGVAL_RET_ADDR) {
        signed int  REGVAL_WRITE_RES            = 0x00;
        signed int  REGVAL_FD                   = 0x00;

        signed char REGVAL_WRITE_BUF[32];

        snprintf(REGVAL_WRITE_BUF, sizeof(REGVAL_WRITE_BUF), "0x%x", REGVAL_RET_ADDR);

        REGVAL_FD                               = open(REGVAL_NAME, O_RDWR);
        if( REGVAL_FD < 0 ){
                printf("[!] REGVAL: I/O Error!\n");
                return -REGVAL_WRITE_FAIL;
        }
        REGVAL_WRITE_RES                        = write(REGVAL_FD, REGVAL_WRITE_BUF, strlen(REGVAL_WRITE_BUF));

        close(REGVAL_FD);

        return REGVAL_WRITE_RES;
}

#define REGIDX_WRITE_FAIL                       2
__attribute__((hot)) static signed int REGIDX_SET_VALUE(unsigned char *REGIDX_NAME,
                                                        unsigned int   REGIDX_RET_ADDR) {
        signed int  REGIDX_WRITE_RES            = 0x00;
        signed int  REGIDX_FD                   = 0x00;

        signed char REGIDX_WRITE_BUF[32];

        snprintf(REGIDX_WRITE_BUF, sizeof(REGIDX_WRITE_BUF), "0x%x", REGIDX_RET_ADDR);

        REGIDX_FD                               = open(REGIDX_NAME, O_RDWR);
        if( REGIDX_FD < 0 ){
                printf("[!] REGIDX: I/O Error! %s\n", REGIDX_NAME);
                return -REGIDX_WRITE_FAIL;
        }
        REGIDX_WRITE_RES                        = write(REGIDX_FD, REGIDX_WRITE_BUF, strlen(REGIDX_WRITE_BUF));

        close(REGIDX_FD);

        return REGIDX_WRITE_RES;
}

#define REGVAL_READ_FAIL                        3
__attribute__((hot)) static signed int REGVAL_GET_VALUE(unsigned char *REGVAL_NAME,
                                                        unsigned int  *IO_COMMAND_OUTPUT) {
        signed int REGVAL_READ_RES              = 0x00;
        signed int REGVAL_READ_VALUE            = 0x00;
        signed int REGVAL_FD                    = 0x00;
        char       REGVAL_BUFF[32];

        REGVAL_FD                               = open(REGVAL_NAME, O_RDONLY);

        REGVAL_READ_RES                         = read(REGVAL_FD, REGVAL_BUFF, sizeof(REGVAL_BUFF) - 1);

        if (REGVAL_READ_RES < 0) {
                printf("[!] REGVAL FAILED READ\n");
                close(REGVAL_FD);
        }

        REGVAL_BUFF[REGVAL_READ_RES]            = '\0';

        if( sscanf(REGVAL_BUFF, "0x%x", IO_COMMAND_OUTPUT) != 0x1 ){
                close(REGVAL_FD);
                return -REGVAL_READ_FAIL;
        }

        close(REGVAL_FD);

        return 0;
}

static signed int  MT76_FPRINTF(void *CURRENT_DATA_STREAM, const char *fmt, ...){
        MT76_STREAM_STATE *StreamState          = (MT76_STREAM_STATE *)CURRENT_DATA_STREAM;
        va_list            arg;
        va_start(arg, fmt);

        if( ! StreamState->reenter ){
                vasprintf(&StreamState->insn_buffer, fmt, arg);
                StreamState->reenter            = true;
        }else{
                char *tmp_buff_1                = NULL;
                char *tmp_buff_2                = NULL;
                vasprintf(&tmp_buff_1, fmt, arg);
                asprintf(&tmp_buff_2, "%s%s", StreamState->insn_buffer, tmp_buff_1);
                free(StreamState->insn_buffer);
                free(tmp_buff_1);
                StreamState->insn_buffer        = tmp_buff_2;
        }

        va_end(arg);
        return 0;
}


static signed int  MT76_DISASSEMBLE_with_bytes_CODE(unsigned char  *MT76_REGVAL_BYTES, unsigned int MT76_REGVAL_LEN, unsigned long REGVAL_ADDR, unsigned char is_ram){
        if( MT76_REGVAL_BYTES == NULL ){
                return false;
        }
        unsigned char *disassembled             = NULL;
        size_t         pc                       = 0x00;

        MT76_STREAM_STATE ss                    = {};
        disassemble_info disasm_info            = {};

        init_disassemble_info(&disasm_info, &ss,  MT76_FPRINTF);
        disasm_info.arch                        = bfd_arch_nds32;
        disasm_info.mach                        = bfd_mach_riscv32;
        disasm_info.read_memory_func            = buffer_read_memory;
        disasm_info.buffer                      = MT76_REGVAL_BYTES;
        disasm_info.buffer_vma                  = 0;
        disasm_info.buffer_length               = MT76_REGVAL_LEN;

        disassemble_init_for_target(&disasm_info);

        disassembler_ftype disasm               = {};
        disasm                                  = disassembler(disasm_info.arch, false, disasm_info.mach, NULL);
        if( disasm == NULL ){
                //write(fd, "disasm is NULL\n", sizeof("disasm is NULL\n"));
                exit(-1);
        }

	printf("------------------------\n");
	printf("dumped memory from %s\n", (is_ram == 1) ? "RAM" : "compiled file");
	printf("------------------------\n");

        while (pc < MT76_REGVAL_LEN) {
                size_t  insn_size               = disasm(pc, &disasm_info);
                unsigned int printf_count       = 0;
                pc                             += insn_size;
		printf("[0x%x] %s\n", 		REGVAL_ADDR, ss.insn_buffer);
		REGVAL_ADDR 			+= pc;

                /* Reset the stream state after each instruction decode. */
                free(ss.insn_buffer);
                ss.reenter = false;
        }
        return 0;
}

static signed int compile_buffer(unsigned long ADDR, unsigned char REGIDX_PATH[], unsigned char REGVAL_PATH[]){
        signed int res_fd               = 0x00;
	uint32_t   regval_res		= 0x00;
        res_fd                          = open("code_output/nds32_code.c", O_RDONLY);

        FILE *fp                        = NULL;
        unsigned char gcc_output[4096]  = {};

        memset(gcc_output,      0x00,   4096);

        if( res_fd < 0 ){
                return -1;
        }

        close(res_fd);
        res_fd                          = open("./compiler/nds32le-linux-glibc-v3-upstream/bin/nds32le-linux-gcc", O_RDONLY) ;

        if( res_fd < 0 ){
                return -2;
        }
        close(res_fd);

        res_fd                          = open("code_output/linker_script/pp_nds32.ld", O_RDONLY) ;

        if( res_fd < 0 ){
                return -2;
        }
        close(res_fd);

        system("./compiler/nds32le-linux-glibc-v3-upstream/bin/nds32le-linux-gcc\
        -fno-PIC -nostdlib -mlittle-endian                                      \
	-Wno-error								\
        -Os -T code_output/linker_script/pp_nds32.ld  ./code_output/nds32_code.c\
        -o ./binary_output/nds32_code.o");

        system("./compiler/nds32le-linux-glibc-v3-upstream/bin/nds32le-linux-objcopy -O\
        binary binary_output/nds32_code.o binary_output/nds32_code.obj");

	system("rm -rf ./binary_output/nds32_code.o");

	res_fd				= open("binary_output/nds32_code.obj", O_RDONLY);
	struct stat *fs_stat		= NULL;
	fs_stat				= (struct stat *)malloc(sizeof(struct stat));
	memset(fs_stat,		0x00,	sizeof(struct stat));
	fstat(res_fd,	fs_stat);
	unsigned char *mm_b		= (unsigned char *)mmap(
					NULL,
					fs_stat->st_size,
					MAP_SHARED | MAP_FILE,
					PROT_READ,
					res_fd,
					0x00
					);

	unsigned char *mm		= (unsigned char *)malloc(fs_stat->st_size);
	memset(mm,   0x00, 		fs_stat->st_size);
	memcpy(mm,   mm_b, 		fs_stat->st_size);
        munmap(mm_b, fs_stat->st_size);

	MT76_DISASSEMBLE_with_bytes_CODE(mm, fs_stat->st_size, ADDR, 0);

	printf("\n\nstarting the patching..\n");

        // now start the patching
        for(unsigned int x = 0; x < fs_stat->st_size; x += 0x4){
                REGIDX_SET_VALUE(REGIDX_PATH, ADDR + x);
                usleep(2500);
                REGVAL_SET_VALUE(REGVAL_PATH, *(uint32_t *)(mm + x));
                printf("set the value 0x%x and writing 0x%08x\n", ADDR + x, *(uint32_t *)(mm + x));
        }
	printf("\n");

	memset(mm, 0x00, fs_stat->st_size);
        for(unsigned int x = 0; x < fs_stat->st_size; x += 0x4){
                REGIDX_SET_VALUE(REGIDX_PATH, ADDR + x);
                usleep(2500);
                REGVAL_GET_VALUE(REGVAL_PATH, &regval_res);
		memcpy(mm + x, &regval_res, 0x4);
        }
        MT76_DISASSEMBLE_with_bytes_CODE(mm, fs_stat->st_size, ADDR, 1);

	free(mm);
        return 0;
}

static signed int disasm_ram(long ADDR, long mem_range, unsigned char REGIDX_PATH[], unsigned char REGVAL_PATH[]){
	unsigned char *current_memory_range 	= (unsigned char *)malloc(mem_range);
	unsigned long  regval_read		= 0x00;

	memset(current_memory_range, 0x00,  mem_range);
        for(unsigned int x = 0; x < mem_range; x += 0x4){
                REGIDX_SET_VALUE(REGIDX_PATH, ADDR + x);
                usleep(2500);
                REGVAL_GET_VALUE(REGVAL_PATH, &regval_read);
		memcpy(current_memory_range + x, &regval_read, 0x4);
        }

	MT76_DISASSEMBLE_with_bytes_CODE(current_memory_range, mem_range, ADDR, 1);
	free(current_memory_range);
	compile_buffer(ADDR, REGIDX_PATH, REGVAL_PATH);
	return 0;
}

static signed int detect_mt76_devices(unsigned char REGIDX_PATH[], unsigned char REGVAL_PATH[]){
        memset(REGIDX_PATH, 0x00, REGIDX_MAX_LEN);
        memset(REGVAL_PATH, 0x00, REGVAL_MAX_LEN);

        signed   int  OPEN_FD   = 0x00;

        DIR *dir                = NULL;
        struct dirent *dirnt    = NULL;

        dir                     = opendir(IEEE80211_DEBUGFS);

        if (dir) {
                while ((dirnt = readdir(dir)) != NULL) {
                        if( strstr(dirnt->d_name, "phy") != NULL ){
                                snprintf(REGIDX_PATH, REGIDX_MAX_LEN, IO_PATH, dirnt->d_name, REGIDX);
                                snprintf(REGVAL_PATH, REGVAL_MAX_LEN, IO_PATH, dirnt->d_name, REGVAL);
                                OPEN_FD = open(REGIDX_PATH, O_RDWR);
                                if( OPEN_FD < 0 ){
                                        continue;
                                }else{
                                        close(OPEN_FD);
                                        closedir(dir);
                                        return 0;
                                }
                        }
                }
                closedir(dir);
        }

        return -1;
}

int main(){
       if (geteuid() != 0) {
                printf("root is necessary for working with the mt76 driver!\n");
                exit(1);
        }
	unsigned char REGIDX_PATH[REGIDX_MAX_LEN];
	unsigned char REGVAL_PATH[REGVAL_MAX_LEN];
	unsigned int  ADDR;

        if( detect_mt76_devices(REGIDX_PATH, REGVAL_PATH) == - 1 ){
                printf("adapter not found\n");
                return 0;
        }

	#ifdef DEBUG
	printf("regidx path: %s regval path: %s\n", REGIDX_PATH, REGVAL_PATH);
	#endif

	printf("insert patching address:\n");
	while( 1 ){
		printf("> ");
		scanf("%p", &ADDR);
		if( ADDR % 4 != 0x00 ){
			printf("[!] the address given must be 4 byte aligned!\n");
		}else{
			break;
		}
	}

	disasm_ram(ADDR, 96, REGIDX_PATH, REGVAL_PATH);
	return 0;
}
