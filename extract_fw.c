/**
	Made by Edoardo Mantovani, 5 December 2024
	given a Mediatek wifi firmware, convert it into an ELF and sets a minimun amount of metadata necessary for fuzzing/analyzing it.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <elf.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <linux/types.h>
#include <linux/const.h>
#include <linux/swab.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
        #include <linux/byteorder/little_endian.h>
#else
        #include <linux/byteorder/big_endian.h>
#endif

#ifndef u8
	#define u8		unsigned char
#endif

#ifndef cpu_to_le16
        #define cpu_to_le16     __cpu_to_le16
#endif

#ifndef le16_to_cpu
        #define le16_to_cpu     __le16_to_cpu
#endif

#ifndef le64_to_cpu
        #define le64_to_cpu     __le64_to_cpu
#endif

#ifndef le32_to_cpu
        #define le32_to_cpu     __le32_to_cpu
#endif

#ifndef cpu_to_le64
        #define cpu_to_le64     __cpu_to_le64
#endif

#ifndef cpu_to_le32
        #define cpu_to_le32     __cpu_to_le32
#endif

#ifndef be32_to_cpu
        #define be32_to_cpu     __be32_to_cpu
#endif

#ifndef be64_to_cpu
        #define be64_to_cpu     __be64_to_cpu
#endif

#ifndef cpu_to_be32
        #define cpu_to_be32     __cpu_to_be32
#endif

#ifndef cpu_to_be64
        #define cpu_to_be64     __cpu_to_be64
#endif

#define UL(x)				(_UL(x))
#define ULL(x)				(_ULL(x))

#define BIT(nr)				(UL(1) << (nr))
#define BIT_ULL(nr)			(ULL(1) << (nr))

#define FW_FEATURE_SET_ENCRYPT		BIT(0)
#define FW_FEATURE_SET_KEY_IDX		GENMASK(2, 1)
#define FW_FEATURE_ENCRY_MODE		BIT(4)
#define FW_FEATURE_OVERRIDE_ADDR	BIT(5)
#define FW_FEATURE_NON_DL		BIT(6)

struct mt76_connac2_fw_trailer {
	u8 chip_id;
	u8 eco_code;
	u8 n_region;
	u8 format_ver;
	u8 format_flag;
	u8 rsv[2];
	char fw_ver[10];
	char build_date[15];
	__le32 crc;
}__attribute__((packed));

struct mt76_connac2_fw_region {
	__le32 decomp_crc;
	__le32 decomp_len;
	__le32 decomp_blk_sz;
	u8 rsv[4];
	__le32 addr;
	__le32 len;
	u8 feature_set;
	u8 type;
	u8 rsv1[14];
}__attribute__((packed));

enum ERROR_MOTIVES{
	ERROR_NO_VALID_FILE	= 11,
	ERROR_FILE_NOT_EXIST	= 12,
	ERROR_FILE_SIZE		= 13,
	ERROR_FILE_MMAP		= 14,
	ERROR_OPEN_FINAL_FILE	= 15,

	ERROR_MAX
};

#ifndef ERROR
	#define ERROR(error_val)	-error_val
#endif

static void DEBUG_DUMP_ELF(unsigned char *ptr_to_elf){
	Elf32_Ehdr *dumped_hdr	= (Elf32_Ehdr *)ptr_to_elf;
	/**
	typedef struct elf32_hdr {
  		unsigned char	e_ident[EI_NIDENT];
  		Elf32_Half	e_type;
  		Elf32_Half	e_machine;
  		Elf32_Word	e_version;
  		Elf32_Addr	e_entry;
  		Elf32_Off	e_phoff;
  		Elf32_Off	e_shoff;
  		Elf32_Word	e_flags;
  		Elf32_Half	e_ehsize;
  		Elf32_Half	e_phentsize;
  		Elf32_Half	e_phnum;
  		Elf32_Half	e_shentsize;
  		Elf32_Half	e_shnum;
  		Elf32_Half	e_shstrndx;
	} Elf32_Ehdr;
	**/

	printf("ELF TYPE: 	%d\n", 	dumped_hdr->e_type);
	printf("ELF MACHINE: 	%d (NDS32)\n",	dumped_hdr->e_machine);
	printf("ELF SHDR ADDR: 	0x%x\n",	dumped_hdr->e_shoff);
	printf("ELF SHDR NUM:  	%d\n",	dumped_hdr->e_shnum);
	printf("ELF PHDR ADDR: 	0x%x\n",	dumped_hdr->e_phoff);
	printf("ELF PHDR NUM:  	%d\n",	dumped_hdr->e_phnum);

	for(unsigned int j = 0; j < dumped_hdr->e_phnum; j++){
		Elf32_Phdr *dumped_phdr = (Elf32_Phdr *)(ptr_to_elf + sizeof(Elf32_Ehdr) + sizeof(Elf32_Phdr) * j);
		printf("phdr %d p_vaddr: 0x%x p_memsz: %d p_offset 0x%x\n", j, dumped_phdr->p_vaddr, dumped_phdr->p_memsz, dumped_phdr->p_offset);
	}

}

int main(int argc, char *argv[]){
	unsigned char 	*file_name  = argv[1];
	unsigned char   *mapped_fw  = NULL;
	struct stat	*file_stats = NULL;
	unsigned int    file_size   = 0;
	int            	file_fd	    = 0;
	int	   	res	    = 0;

	if( file_name == NULL || strlen(file_name) <= 1 ){
		printf("please provide a valid file!\n");
		exit(ERROR(ERROR_NO_VALID_FILE));
	}

	file_fd 	= open(file_name, O_RDONLY);

	if( file_fd < 0 ){
		printf("file do not exists!\n");
		exit(ERROR(ERROR_FILE_NOT_EXIST));
	}

	file_stats 	= (struct stat *)malloc(sizeof(struct stat));
	fstat(file_fd, file_stats);
	file_size  	= file_stats->st_size;

	if( file_size == 0 ){
		printf("file size is 0!\n");
		exit(ERROR(ERROR_FILE_SIZE));
	}else{
		printf("file size is: %d\n", file_size);
	}

	free(file_stats);

	mapped_fw  	= (unsigned char *)mmap(
					(void *)0x00,
					file_size,
					PROT_READ,
					MAP_FILE | MAP_SHARED,
					file_fd,
					0x00
	          );

	if( mapped_fw 	== NULL ){
		printf("file mmap is NULL!\n");
		exit(ERROR(ERROR_FILE_MMAP));

	}

	/** here we start with the function 'mt76_connac_mcu_send_ram_firmware' code **/
	const struct mt76_connac2_fw_trailer *hdr = (const struct mt76_connac2_fw_trailer *)(mapped_fw + file_size - sizeof(*hdr));
	printf("chip_id: 0x%x\neco_code: 0x%x\nn_region: %d\nfw_ver: %.10s\nbuild_time: %.15s\n",
		hdr->chip_id,
		hdr->eco_code,
		hdr->n_region,
		hdr->fw_ver,
		hdr->build_date
	);

	/** first step: get the actual size for every loaded section in the fw, this will be summed to the header and the phdr of the ELF for the allocation
		of the final file in memory **/
	unsigned int MTK_FW_ALLOCATE_SIZE			= 0;

	for(unsigned int i = 0; i < hdr->n_region; i++){
		const struct mt76_connac2_fw_region *region 	= NULL;
		uint32_t len					= 0;
		uint32_t addr					= 0;
		uint32_t mode					= 0;
		region 						= (const void *)((const u8 *)hdr - (hdr->n_region - i) * sizeof(*region));

		len 						= le32_to_cpu(region->len);
		addr 						= le32_to_cpu(region->addr);

		/** DO FEATURE REPORTING **/
		if (region->feature_set & FW_FEATURE_NON_DL ){
			printf("[region %d] won't be loaded into memory!\n", i);
		}else{
			MTK_FW_ALLOCATE_SIZE		        += region->len;
		}
	}

	// now add to 'MTK_FW_ALLOCATE_SIZE' also the size of the elf header and the phdrs
	MTK_FW_ALLOCATE_SIZE		       += sizeof(Elf32_Ehdr) + ( sizeof(Elf32_Phdr) * ( hdr->n_region - 1 ) );

	/** create the ELF header and the phdrs **/
	unsigned int   ELF_ACTUAL_INCREMENT 	= 0;
	unsigned int   PHDR_ACTUAL_OFFSET	= 0;

	unsigned char *ELF_HDR			= (unsigned char *)mmap(
									NULL,
									MTK_FW_ALLOCATE_SIZE,
									PROT_READ | PROT_WRITE,
									MAP_ANON  | MAP_PRIVATE,
									-1,
									0x00
									);

	memset(ELF_HDR,	0x00, MTK_FW_ALLOCATE_SIZE);
	Elf32_Ehdr *REAL_HDR			= (Elf32_Ehdr *)ELF_HDR;
	REAL_HDR->e_ident[0]       		= 0x7f;
	REAL_HDR->e_ident[1]       		= 'E';
        REAL_HDR->e_ident[2]            	= 'L';
        REAL_HDR->e_ident[3]            	= 'F';
	REAL_HDR->e_ident[4]       		= 0x01;
	REAL_HDR->e_ident[5]       		= 0x01;
	REAL_HDR->e_ident[6]            	= 0x01;

	#ifndef EM_NDS32
		#define EM_NDS32	167
	#endif

	REAL_HDR->e_machine			= EM_NDS32;
	REAL_HDR->e_shnum			= 0;
	REAL_HDR->e_shoff			= 0x00;
	REAL_HDR->e_phoff			= 0x00 + sizeof(Elf32_Ehdr); /** base address (0x00) + the size of the header **/
	REAL_HDR->e_phnum			= (unsigned char )hdr->n_region - 1;
	REAL_HDR->e_shnum			= 0x00;
	REAL_HDR->e_phentsize      		= sizeof(Elf32_Phdr);
	REAL_HDR->e_shentsize			= sizeof(Elf32_Shdr);
	REAL_HDR->e_type           		= ET_EXEC;
	REAL_HDR->e_entry			= 0x00; /** this stills needs to be decided! **/
	REAL_HDR->e_version        		= 0x01;
	REAL_HDR->e_shstrndx			= 0x00;

	ELF_ACTUAL_INCREMENT	       	       += sizeof(Elf32_Ehdr);
	PHDR_ACTUAL_OFFSET	               += sizeof(Elf32_Ehdr) + ( sizeof(Elf32_Phdr) * ( hdr->n_region - 1 ) );

	unsigned int fs_counter = 0;

	for(unsigned int i = 0; i < hdr->n_region; i++){
		const struct mt76_connac2_fw_region *region = NULL;
		uint32_t len		= 0;
		uint32_t addr		= 0;
		uint32_t mode		= 0;
		region 			= (const void *)((const u8 *)hdr - (hdr->n_region - i) * sizeof(*region));

		if( !( region->feature_set & FW_FEATURE_NON_DL ) ){
			len 			= le32_to_cpu(region->len);
			addr 			= le32_to_cpu(region->addr);

			printf("[region: %d] decomp_crc: 0x%x, decomp_len: %d, decomp_blk_sz: %d, ",
				i,
				region->decomp_crc,
				region->decomp_len,
				region->decomp_blk_sz
			);
			printf("addr: 0x%x, len: %d, type: %x\n",
				addr,
				len,
				region->type
			);

			Elf32_Phdr *CURR_PHDR	= (Elf32_Phdr *)(ELF_HDR + ELF_ACTUAL_INCREMENT);
			CURR_PHDR->p_vaddr	= addr;
			CURR_PHDR->p_paddr	= addr;
			CURR_PHDR->p_filesz	= len;
			CURR_PHDR->p_memsz	= len;
			CURR_PHDR->p_offset	= PHDR_ACTUAL_OFFSET;
			CURR_PHDR->p_type	= PT_LOAD;
			CURR_PHDR->p_flags	= PF_R | PF_W;

			ELF_ACTUAL_INCREMENT   += sizeof(Elf32_Phdr);

			memcpy(ELF_HDR + PHDR_ACTUAL_OFFSET, mapped_fw + fs_counter, CURR_PHDR->p_memsz);

			PHDR_ACTUAL_OFFSET     += CURR_PHDR->p_filesz;

	                fs_counter += len;
		}
	}
	DEBUG_DUMP_ELF(ELF_HDR);

	/** allocate the final ELF file **/
	int mtk_fw_final_elf 		= 0;
	unsigned char *final_elf_name	= strdup(file_name);
	strncpy(final_elf_name + strlen(file_name) - sizeof(".elf"), ".elf", sizeof(".elf"));
	printf("final elf name %s\n", final_elf_name);
	mtk_fw_final_elf		= open(final_elf_name, O_CREAT | O_RDWR, 0777);

	if( mtk_fw_final_elf < 0 ){
		printf("impossible to open the output final elf file! aborting!\n");
		exit(ERROR(ERROR_OPEN_FINAL_FILE));
	}

	printf("final size of the file: %d bytes\n", MTK_FW_ALLOCATE_SIZE);

	write(mtk_fw_final_elf, ELF_HDR, MTK_FW_ALLOCATE_SIZE);

	munmap(ELF_HDR, MTK_FW_ALLOCATE_SIZE);
	munmap(mapped_fw, file_size);

	return 0;
}

