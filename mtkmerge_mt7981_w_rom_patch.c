/**
        Made by Edoardo Mantovani, 24 January 2025
        given more than a 1 Mediatek wifi firmware, convert them into an ELF and sets a minimun amount of metadata necessary for fuzzing/analyzing it.
        This is useful for multiple fw like the WA,WM and WO blobs + ROM + ROM PATCHes
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
        #define u8              unsigned char
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

/** COLORS DEFINITION **/

#define RED 			"\e[0;31m"
#define GREEN 			"\e[0;32m"
#define YELLOW 			"\e[0;33m"
#define BOLD 			"\e[1;30m"
#define CRESET 			"\e[0m"

/** kernel like macros **/
#define UL(x)                           (_UL(x))
#define ULL(x)                          (_ULL(x))

#define BIT(nr)                         (UL(1) << (nr))
#define BIT_ULL(nr)                     (ULL(1) << (nr))

/** mt76's specific macros **/
#define FW_FEATURE_SET_ENCRYPT          BIT(0)
#define FW_FEATURE_SET_KEY_IDX          GENMASK(2, 1)
#define FW_FEATURE_ENCRY_MODE           BIT(4)
#define FW_FEATURE_OVERRIDE_ADDR        BIT(5)
#define FW_FEATURE_NON_DL               BIT(6)

#ifndef MT_WIFI_PATCH_MIN_SIZE
	#define MT_WIFI_PATCH_MIN_SIZE	256
#endif

#define MT_BUILD_DATE_LEN		16
#define MT_PLATFORM_LEN			4

#define MT_PATCH_HDR	struct mt76_connac2_patch_hdr
#define MT_PATCH_SEC	struct mt76_connac2_patch_sec



#define FW_FEATURE_SET_ENCRYPT			BIT(0)
#define FW_FEATURE_SET_KEY_IDX			GENMASK(2, 1)
#define FW_FEATURE_ENCRY_MODE			BIT(4)
#define FW_FEATURE_OVERRIDE_ADDR		BIT(5)
#define FW_FEATURE_NON_DL			BIT(6)

#define DL_MODE_ENCRYPT				BIT(0)
#define DL_MODE_KEY_IDX				GENMASK(2, 1)
#define DL_MODE_RESET_SEC_IV			BIT(3)
#define DL_MODE_WORKING_PDA_CR4			BIT(4)
#define DL_MODE_VALID_RAM_ENTRY         	BIT(5)
#define DL_CONFIG_ENCRY_MODE_SEL		BIT(6)
#define DL_MODE_NEED_RSP			BIT(31)

#define FW_START_OVERRIDE			BIT(0)
#define FW_START_WORKING_PDA_CR4		BIT(2)
#define FW_START_WORKING_PDA_DSP		BIT(3)

#define PATCH_SEC_NOT_SUPPORT			GENMASK(31, 0)
#define PATCH_SEC_TYPE_MASK			GENMASK(15, 0)
#define PATCH_SEC_TYPE_INFO			0x2

#define PATCH_SEC_ENC_TYPE_MASK			GENMASK(31, 24)
#define PATCH_SEC_ENC_TYPE_PLAIN		0x00
#define PATCH_SEC_ENC_TYPE_AES			0x01
#define PATCH_SEC_ENC_TYPE_SCRAMBLE		0x02
#define PATCH_SEC_ENC_SCRAMBLE_INFO_MASK	GENMASK(15, 0)
#define PATCH_SEC_ENC_AES_KEY_MASK		GENMASK(7, 0)

#ifndef GENMASK
        #define GENMASK(h, l)   		__GENMASK(h, l)
#endif

#define __GENMASK(h, l) (((~_UL(0)) - (_UL(1) << (l)) + 1) & (~_UL(0) >> (__BITS_PER_LONG - 1 - (h))))

#define UL(x)		(_UL(x))
#define ULL(x)		(_ULL(x))
#define BIT(nr)         ((unsigned long)(1) << (nr))
#define __bf_shf(x) 	(ffsll(x) - 1)

#define FIELD_PREP(_mask, _val)						\
	({								\
		((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask);	\
	})

#define FIELD_GET(_mask, _reg)						\
	({								\
		(typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask));	\
	})

/** tail header present in the Mediatek wifi fw **/
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
}__attribute__((packed));	// note that is packed !!

/** specific section representing a segment of the firmware, placed before the tail header **/
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
}__attribute__((packed));	// note that is packed !!

/** definitions for the patch header and the regions **/
struct mt76_connac2_patch_hdr {
	char build_date[MT_BUILD_DATE_LEN];
	char platform[MT_PLATFORM_LEN];
	__be32 hw_sw_ver;
	__be32 patch_ver;
	__be16 checksum;
	uint16_t rsv;
	struct {
		__be32 patch_ver;
		__be32 subsys;
		__be32 feature;
		__be32 n_region;
		__be32 crc;
		uint32_t rsv[11];
	} desc;
}__attribute__((packed));

#define MT_PATCH_SEC_SPEC_LEN		13

struct mt76_connac2_patch_sec {
	__be32 type;
	__be32 offs;
	__be32 size;
	union {
		__be32 spec[MT_PATCH_SEC_SPEC_LEN];
		struct {
			__be32 addr;
			__be32 len;
			__be32 sec_key_idx;
			__be32 align_len;
			uint32_t rsv[9];
		} info;
	};
}__attribute__((packed));

/** main definition for the element in our list, a firmware will be represented by:
	* UNSIGNED CHAR *candidate_file_name:		given by the user
	* UNSIGNED CHAR *candidate_file_memory_map:	represent the memory address of the file mapped in memory
	* SIGNED   INT   candidate_file_fd:		represent the opened fw handle
	* SIGNED   INT   candidate_file_size:		obtained by fstat, represent the size of the firmware in the FS
**/

struct candidate_fw_metadata {
	unsigned char *candidate_file_name;
	unsigned char *candidate_file_memory_map;
	signed   int   candidate_file_fd;
	signed   int   candidate_file_size;
	unsigned char  is_rom;
	unsigned char  is_patch;
};

/** linked list which will contains the input given by the user **/
typedef struct MTK_CANDIDATE_FW {
    	struct candidate_fw_metadata  current_metadata;
    	struct MTK_CANDIDATE_FW      *next;
} MTK_CANDIDATE_FW;

/** MTK_ALLOCATE_CANDIDATE: used for allocating a single element for our linked list, it will copy the file name and initialize other basic fields **/
static MTK_CANDIDATE_FW *MTK_ALLOCATE_CANDIDATE(unsigned char *candidate_filename) {
	MTK_CANDIDATE_FW *MTK_CANDIDATE_ELEM = (MTK_CANDIDATE_FW *)malloc(sizeof(MTK_CANDIDATE_FW));


	MTK_CANDIDATE_ELEM->current_metadata.candidate_file_name 	= strdup(candidate_filename);
	MTK_CANDIDATE_ELEM->current_metadata.candidate_file_memory_map	= NULL;
	MTK_CANDIDATE_ELEM->current_metadata.candidate_file_size	= 0;
	MTK_CANDIDATE_ELEM->current_metadata.candidate_file_fd 		= -1;
	MTK_CANDIDATE_ELEM->next 					= NULL;

    	return MTK_CANDIDATE_ELEM;
}

/** MTK_FW_FREE_MAIN_LIST: used only when the program dies for de-allocating the linked list **/
static void MTK_FW_FREE_MAIN_LIST(MTK_CANDIDATE_FW *MTK_HEAD) {
	MTK_CANDIDATE_FW* MTK_CURR = MTK_HEAD;

	while (MTK_CURR != NULL) {
	    	MTK_CANDIDATE_FW *NEXT_MTK_CANDIDATE_FW = MTK_CURR->next;
		/** unmap the memory chunk allocated for the blob **/
		munmap(MTK_CURR->current_metadata.candidate_file_memory_map, MTK_CURR->current_metadata.candidate_file_size);
		/** THEN close the file **/
	     	if (MTK_CURR->current_metadata.candidate_file_fd >= 0) {
            		close(MTK_CURR->current_metadata.candidate_file_fd);
        	}
		/** free the allocated memory for the file name **/
        	free(MTK_CURR->current_metadata.candidate_file_name);
		/** and for ending, free the element itself! **/
        	free(MTK_CURR);
        	MTK_CURR 	= NEXT_MTK_CANDIDATE_FW;
    	}
}

unsigned char *__phdr_flag_to_string(Elf32_Phdr program_header){

	if( ( program_header.p_flags & PF_X ) && ( program_header.p_flags & PF_W ) ){
		return "EXEC | READ | WRITE";
	}
	if( ( program_header.p_flags & PF_R ) && ( program_header.p_flags & PF_W ) ){
		return "READ | WRITE";
	}
	if( program_header.p_flags & PF_R ){
		return "READ";
	}
	return NULL;
}

static uint32_t mt76_connac2_get_data_mode(unsigned char section_number, uint32_t info){
	uint32_t mode 				= DL_MODE_NEED_RSP;

	switch (FIELD_GET(PATCH_SEC_ENC_TYPE_MASK, info)) {
		case PATCH_SEC_ENC_TYPE_PLAIN:
			printf("[%s][sec %d] patch is in plaintext!\n", __FUNCTION__, section_number);
				/** do nothing, this is a plaintext patch **/
			break;
		case PATCH_SEC_ENC_TYPE_AES:
				printf("[%s][sec %d] patch is in AES encrypted mode!\n", __FUNCTION__, section_number);
				mode 		       |= DL_MODE_ENCRYPT;
				mode 		       |= FIELD_PREP(DL_MODE_KEY_IDX, (info & PATCH_SEC_ENC_AES_KEY_MASK)) & DL_MODE_KEY_IDX;
				mode 	               |= DL_MODE_RESET_SEC_IV;
			break;
		case PATCH_SEC_ENC_TYPE_SCRAMBLE:
				printf("[%s][sec %d] patch is in simple scrambling mode!\n", __FUNCTION__, section_number);
				mode		       |= DL_MODE_ENCRYPT;
				mode 		       |= DL_CONFIG_ENCRY_MODE_SEL;
				mode 		       |= DL_MODE_RESET_SEC_IV;
			break;
		default:
			printf("[%s][sec %d] Encryption type not supported!\n", __FUNCTION__, section_number);
	}

	return mode;
}

/** our main function **/
int main(int argc, char* argv[]) {

	/** close if no args are given **/
    	if (argc < 2) {
    	    printf(RED "%s <fw1.bin> <fw2.bin> <wf_rom.axf> <rom_patch.bin> ...\n" CRESET, argv[0]);
    	    return -1;
    	}

	/**
		CANDIDATE_FW_HEAD: 			head of the linked list
		CANDIDATE_FW_CURRENT:			current element being iterated in the linked list
	**/

    	MTK_CANDIDATE_FW *CANDIDATE_FW_HEAD		= NULL;
	MTK_CANDIDATE_FW *CANDIDATE_FW_CURRENT		= NULL;

	/**
		final_elf_segment_total_number:		variable which holds the number of the segment counted in EVERY file given by the user
		final_elf_segment_total_size:		variable which olds the total size of the segments counted in EVERY file given by the user
		final_elf_segment_total_counter:	variable used for holding the "jump" offset for copying the segment from the FW to the output ELF for EACH input
	**/

	unsigned int final_elf_segment_total_number	= 0;
	unsigned int final_elf_segment_total_size	= 0;
	unsigned int final_elf_segment_total_counter	= 0;

	/**
		used for counting the offset in the ELF header/program header
	**/

	unsigned int ELF_ACTUAL_INCREMENT		= 0;
	unsigned int PHDR_ACTUAL_OFFSET			= 0;

	/** start the main loop **/
      	for (int i = 1; i < argc; i++) {
		unsigned char    *candidate_fwname 		= argv[i];
		MTK_CANDIDATE_FW *CANDIDATE_NEW_ELEM		= MTK_ALLOCATE_CANDIDATE(candidate_fwname);
 		CANDIDATE_NEW_ELEM->current_metadata.candidate_file_fd 	= open(candidate_fwname, O_RDONLY);
		if (CANDIDATE_NEW_ELEM->current_metadata.candidate_file_fd == -1) {
       			printf("[CRITICAL ERROR]: Could not open file %s\n", candidate_fwname);
       			return -1;
		}

		struct stat *file_fstat = (struct stat *)malloc(sizeof(struct stat));
		fstat(CANDIDATE_NEW_ELEM->current_metadata.candidate_file_fd, file_fstat);
		CANDIDATE_NEW_ELEM->current_metadata.candidate_file_size = file_fstat->st_size;
		free(file_fstat);

		printf(YELLOW "============================================\n" CRESET);
		printf(YELLOW "	%s	\n" CRESET, argv[i]);
		printf(YELLOW "============================================\n" CRESET);

		printf(GREEN "[%s]" CRESET " size: %d\n", argv[i], CANDIDATE_NEW_ELEM->current_metadata.candidate_file_size);

		CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map = (unsigned char *)mmap(
									NULL,
									CANDIDATE_NEW_ELEM->current_metadata.candidate_file_size,
									PROT_READ,
									MAP_FILE | MAP_SHARED,
									CANDIDATE_NEW_ELEM->current_metadata.candidate_file_fd,
									0x00
									);

		printf(GREEN "[%s]" CRESET " mapped at 0x%x\n", argv[i], CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map);

		// first change: decide if we are handling a ROM or the RAM fw

		if( strstr(argv[i], "wf_rom.axd") != NULL ){
			Elf32_Ehdr	*wf_rom_header		  		= (Elf32_Ehdr *)CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map;
			Elf32_Phdr	*wf_rom_phdr				= (Elf32_Phdr *)(
											CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map +
											wf_rom_header->e_phoff
										);
			unsigned int     wf_rom_phdr_num			= (unsigned int)wf_rom_header->e_phnum;
			printf(GREEN "[*]" CRESET " actual phdr in the rom: %d\n", wf_rom_phdr_num);

			for(unsigned char j = 0; j < wf_rom_phdr_num; j++){
				if( wf_rom_phdr[j].p_memsz == 0 ){
					continue;
				}
				final_elf_segment_total_size		+= wf_rom_phdr[j].p_memsz;
				final_elf_segment_total_number++;
			}
			// set the ROM flag
			CANDIDATE_NEW_ELEM->current_metadata.is_patch	= 0;
			CANDIDATE_NEW_ELEM->current_metadata.is_rom	= 1;
		}else if( strstr(argv[i], "rom_patch") != NULL){
			// process ROM patch too
			MT_PATCH_HDR	*MT_WIFI_PATCH_HDR	= NULL;
			MT_WIFI_PATCH_HDR			= (MT_PATCH_HDR *)CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map;

			printf(GREEN "[WIFI_PATCH_HDR]" CRESET " build_date: %.15s\n", 	MT_WIFI_PATCH_HDR->build_date);
        		printf(GREEN "[WIFI_PATCH_HDR]" CRESET " platform:   %.4s\n", 	MT_WIFI_PATCH_HDR->platform);

			printf(GREEN "[WIFI_PATCH_HDR] " CRESET " hw_sw_ver:  %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->hw_sw_ver));
			printf(GREEN "[WIFI_PATCH_HDR] " CRESET " patch_ver:  %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->patch_ver));
			printf(GREEN "[WIFI_PATCH_HDR] " CRESET " checksum:   %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->checksum));
			printf(GREEN "[WIFI_PATCH_HDR] " CRESET " rsv:	     %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->rsv));
			printf(GREEN "[WIFI_PATCH_HDR] " CRESET " patch_ver:  %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.patch_ver));
			printf(GREEN "[WIFI_PATCH_HDR] " CRESET " subsys:     %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.subsys));
			printf(GREEN "[WIFI_PATCH_HDR] " CRESET " feature:    %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.feature));
			printf(GREEN "[WIFI_PATCH_HDR] " CRESET " n_region:   %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region));
			printf(GREEN "[WIFI_PATCH_HDR] " CRESET " crc:        %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.crc));

			// now scan for every region and increase the length of the total segment/segment data

			for(unsigned char j = 0; j < __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region); j++){
				MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;
				uint32_t len				= 0;
				uint32_t addr				= 0;
				uint32_t mode				= 0;
				uint32_t sec_info			= 0;
				uint8_t *dl				= NULL;

				MT_WIFI_PATCH_CURR_SECT 		= (MT_PATCH_SEC *)(
										CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map +
										sizeof(*MT_WIFI_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT)
									);


				if ( ( __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->type) & PATCH_SEC_TYPE_MASK) != PATCH_SEC_TYPE_INFO) {
					/** do nothing **/
				}

				addr 					= 0x0		     							+
										__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr);
				len 					= 0x0		     							+
										__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len);
				sec_info 				= 0x0		     							+
										__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.sec_key_idx);
                		dl                                      = 	CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map  +
									 	__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs);

				printf(RED "[section number %d]" CRESET " offset: 0x%x, size: %d, address: 0x%x, len: %d\n",
					j,
					__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs),
					__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->size),
					__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr),
					__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len)
				);

				mode 					= mt76_connac2_get_data_mode(j, sec_info);
                                final_elf_segment_total_size            += __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->size);
                                final_elf_segment_total_number++;

			}

			// set the ROM patch
			CANDIDATE_NEW_ELEM->current_metadata.is_rom	= 0;
			CANDIDATE_NEW_ELEM->current_metadata.is_patch	= 1;
		}else{
			CANDIDATE_NEW_ELEM->current_metadata.is_patch	= 0;
			CANDIDATE_NEW_ELEM->current_metadata.is_rom	= 0;
			const struct mt76_connac2_fw_trailer *hdr = (const struct mt76_connac2_fw_trailer *)(
									CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map 	+
									CANDIDATE_NEW_ELEM->current_metadata.candidate_file_size 		-
									sizeof(*hdr)
									);

        	        printf("header at 0x%x\n", CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map  +
                                                                CANDIDATE_NEW_ELEM->current_metadata.candidate_file_size                -
                                                                sizeof(*hdr) - CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map);


			printf(GREEN "[%s] " CRESET "chip_id: 0x%x eco_code: 0x%x n_region: %d fw_ver: %.10s build_time: %.15s, crc: %lx\n",
				argv[i],
        	        	hdr->chip_id,
        	        	hdr->eco_code,
        	        	hdr->n_region,
	                	hdr->fw_ver,
	                	hdr->build_date,
				hdr->crc
	        	);

			final_elf_segment_total_number	+= hdr->n_region;

			for(unsigned int k = 0; k < hdr->n_region; k++){
	                	const struct mt76_connac2_fw_region *region = NULL;
	                	uint32_t len            = 0;
	                	uint32_t addr           = 0;
	                	uint32_t mode           = 0;
	                	region                  = (const void *)((const u8 *)hdr - (hdr->n_region - k) * sizeof(*region));
				if( !( region->feature_set & FW_FEATURE_NON_DL ) ){
	        	                len                     = le32_to_cpu(region->len);
		                        addr                    = le32_to_cpu(region->addr);

	                	        printf(GREEN "[%s]" CRESET RED "[region: %d]" CRESET " decomp_crc: 0x%x, decomp_len: %d, decomp_blk_sz: %d, ",
						argv[i],
	                        	        k,
	                                	region->decomp_crc,
	                                	region->decomp_len,
	                                	region->decomp_blk_sz
	                        	);
	                        	printf("addr: 0x%x, len: %d, type: %x\n",
	                        	        addr,
	                        	        len,
	                        	        region->type
	                        	);
					final_elf_segment_total_size	+= len;

				}
			}
		}
	    	if (CANDIDATE_FW_HEAD == NULL) {
            		CANDIDATE_FW_HEAD = CANDIDATE_NEW_ELEM;
        	} else {
            		CANDIDATE_FW_CURRENT->next = CANDIDATE_NEW_ELEM;
        	}
        	CANDIDATE_FW_CURRENT = CANDIDATE_NEW_ELEM;
	}

	final_elf_segment_total_size	       += sizeof(Elf32_Ehdr) + ( sizeof(Elf32_Phdr) * final_elf_segment_total_number );
	unsigned char *ELF_HDR                  = (unsigned char *)mmap(
                                                                        NULL,
                                                                        final_elf_segment_total_size,
                                                                        PROT_READ | PROT_WRITE,
                                                                        MAP_ANON  | MAP_PRIVATE,
                                                                        -1,
                                                                        0x00
                                                                        );

        memset(ELF_HDR, 0x00, final_elf_segment_total_size);
        Elf32_Ehdr *REAL_HDR                    = (Elf32_Ehdr *)ELF_HDR;
        REAL_HDR->e_ident[0]                    = 0x7f;
        REAL_HDR->e_ident[1]                    = 'E';
        REAL_HDR->e_ident[2]                    = 'L';
        REAL_HDR->e_ident[3]                    = 'F';
        REAL_HDR->e_ident[4]                    = 0x01;
        REAL_HDR->e_ident[5]                    = 0x01;
        REAL_HDR->e_ident[6]                    = 0x01;

        #ifndef EM_NDS32
                #define EM_NDS32        167
        #endif

        REAL_HDR->e_machine                     = EM_NDS32;
        REAL_HDR->e_shnum                       = 0;
        REAL_HDR->e_shoff                       = 0x00;
        REAL_HDR->e_phoff                       = 0x00 + sizeof(Elf32_Ehdr); /** base address (0x00) + the size of the header **/
        REAL_HDR->e_phnum                       = final_elf_segment_total_number;
        REAL_HDR->e_shnum                       = 0x00;
        REAL_HDR->e_phentsize                   = sizeof(Elf32_Phdr);
        REAL_HDR->e_shentsize                   = sizeof(Elf32_Shdr);
        REAL_HDR->e_type                        = ET_EXEC;
        REAL_HDR->e_entry                       = 0x00; /** this stills needs to be decided! **/
        REAL_HDR->e_version                     = 0x01;
        REAL_HDR->e_shstrndx                    = 0x00;

        ELF_ACTUAL_INCREMENT                   += sizeof(Elf32_Ehdr);
        PHDR_ACTUAL_OFFSET                     += sizeof(Elf32_Ehdr) + ( sizeof(Elf32_Phdr) * final_elf_segment_total_number );

	CANDIDATE_FW_CURRENT = CANDIDATE_FW_HEAD;
	while (CANDIDATE_FW_CURRENT != NULL) {
		/** take the mapped file and then repeat the loop as before **/
		printf(GREEN "[%s]" CRESET " creating the PHDR:\n" CRESET, CANDIDATE_FW_CURRENT->current_metadata.candidate_file_name);

		if( CANDIDATE_FW_CURRENT->current_metadata.is_rom == 1 ){
			// processing the ROM
			// ROM is just a plain elf file, we need to extract the program header table and every segment which will be nearly equal to a region
			Elf32_Ehdr	*wf_rom_header		  		= (Elf32_Ehdr *)CANDIDATE_FW_CURRENT->current_metadata.candidate_file_memory_map;
			Elf32_Phdr	*wf_rom_phdr				= (Elf32_Phdr *)(
											CANDIDATE_FW_CURRENT->current_metadata.candidate_file_memory_map +
											wf_rom_header->e_phoff
										);
			unsigned int     wf_rom_phdr_num			= (unsigned int)wf_rom_header->e_phnum;
			printf(GREEN "[*]" CRESET " actual phdr in the rom: %d\n", wf_rom_phdr_num);

			for(unsigned char j = 0; j < wf_rom_phdr_num; j++){
				if( wf_rom_phdr[j].p_memsz == 0 ){
					continue;
				}
	        		Elf32_Phdr *ROM_CURR_PHDR   	= (Elf32_Phdr *)(ELF_HDR + ELF_ACTUAL_INCREMENT);
	                	ROM_CURR_PHDR->p_vaddr      	= wf_rom_phdr[j].p_vaddr;
	                	ROM_CURR_PHDR->p_paddr      	= wf_rom_phdr[j].p_vaddr;
	                	ROM_CURR_PHDR->p_filesz     	= wf_rom_phdr[j].p_memsz;
	                	ROM_CURR_PHDR->p_memsz      	= wf_rom_phdr[j].p_memsz;
	                	ROM_CURR_PHDR->p_offset     	= PHDR_ACTUAL_OFFSET;
	                	ROM_CURR_PHDR->p_type       	= PT_LOAD;
	        	        ROM_CURR_PHDR->p_flags      	= wf_rom_phdr[j].p_flags;
				ELF_ACTUAL_INCREMENT   	       += sizeof(Elf32_Phdr);
				printf(GREEN "[%s]" CRESET " phdr numbr: %d phys offset: %d mapped at 0x%x memsz: %d next region offset: %d\n",
					CANDIDATE_FW_CURRENT->current_metadata.candidate_file_name,
					j,
					PHDR_ACTUAL_OFFSET,
					ROM_CURR_PHDR->p_vaddr,
					ROM_CURR_PHDR->p_memsz,
					PHDR_ACTUAL_OFFSET + ROM_CURR_PHDR->p_memsz
				);

				memmove(
					ELF_HDR + PHDR_ACTUAL_OFFSET,
					CANDIDATE_FW_CURRENT->current_metadata.candidate_file_memory_map + wf_rom_phdr[j].p_offset,
					ROM_CURR_PHDR->p_memsz
				);

	        	        PHDR_ACTUAL_OFFSET     	        += ROM_CURR_PHDR->p_memsz;
	        	}
		}else if( CANDIDATE_FW_CURRENT->current_metadata.is_patch == 1 ) {
                        MT_PATCH_HDR    *MT_WIFI_PATCH_HDR      = NULL;
                        MT_WIFI_PATCH_HDR                       = (MT_PATCH_HDR *)CANDIDATE_FW_CURRENT->current_metadata.candidate_file_memory_map;

                        // now scan for every region and increase the length of the total segment/segment data

                        for(unsigned char j = 0; j < __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region); j++){
                                MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;
                                uint32_t len                            = 0;
                                uint32_t addr                           = 0;
                                uint32_t mode                           = 0;
                                uint32_t sec_info                       = 0;
                                uint8_t *dl                             = NULL;

                                MT_WIFI_PATCH_CURR_SECT                 = (MT_PATCH_SEC *)(
                                                                                CANDIDATE_FW_CURRENT->current_metadata.candidate_file_memory_map +
                                                                                sizeof(*MT_WIFI_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT)
                                                                        );


				addr 					= 0x0								     	+
									__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr);
				len 					= 0x0		     							+
									__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len);
				sec_info 				= 0x0		     							+
									__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.sec_key_idx);
                		dl                                      = CANDIDATE_FW_CURRENT->current_metadata.candidate_file_memory_map 	+
									__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs);

                                Elf32_Phdr *PATCH_CURR_PHDR       = (Elf32_Phdr *)(ELF_HDR + ELF_ACTUAL_INCREMENT);
                                PATCH_CURR_PHDR->p_vaddr          = addr;
                                PATCH_CURR_PHDR->p_paddr          = addr;
                                PATCH_CURR_PHDR->p_filesz         = len;
                                PATCH_CURR_PHDR->p_memsz          = len;
                                PATCH_CURR_PHDR->p_offset         = PHDR_ACTUAL_OFFSET;
                                PATCH_CURR_PHDR->p_type           = PT_LOAD;
                                PATCH_CURR_PHDR->p_flags          = PF_X | PF_W | PF_R;
                                ELF_ACTUAL_INCREMENT             += sizeof(Elf32_Phdr);
                                printf(GREEN "[%s]" CRESET " phdr numbr: %d phys offset: %d mapped at 0x%x memsz: %d next region offset: %d\n",
                                        CANDIDATE_FW_CURRENT->current_metadata.candidate_file_name,
                                        j,
                                        PHDR_ACTUAL_OFFSET,
                                        PATCH_CURR_PHDR->p_vaddr,
                                        PATCH_CURR_PHDR->p_memsz,
                                        PHDR_ACTUAL_OFFSET + PATCH_CURR_PHDR->p_memsz
                                );

                                memmove(
                                        ELF_HDR + PHDR_ACTUAL_OFFSET,
                                        dl,
                                        PATCH_CURR_PHDR->p_memsz
                                );

                                PHDR_ACTUAL_OFFSET              += PATCH_CURR_PHDR->p_memsz;

			}
		}else{
			const struct mt76_connac2_fw_trailer *hdr = (const struct mt76_connac2_fw_trailer *)(
								CANDIDATE_FW_CURRENT->current_metadata.candidate_file_memory_map 	+
								CANDIDATE_FW_CURRENT->current_metadata.candidate_file_size 		-
								sizeof(*hdr)
								);

			if( hdr == NULL ){
				printf("[%s] the header is NULL!!!\n", CANDIDATE_FW_CURRENT->current_metadata.candidate_file_name);
				MTK_FW_FREE_MAIN_LIST(CANDIDATE_FW_HEAD);
				return -1;
			}

			for(unsigned int k = 0; k < hdr->n_region; k++){
				const struct mt76_connac2_fw_region *region 	= NULL;
	                	uint32_t len            			= 0;
	                	uint32_t addr           			= 0;
	                	uint32_t mode           			= 0;
	                	region                  			= (const void *)((const u8 *)hdr - (hdr->n_region - k) * sizeof(*region));

				if( !( region->feature_set & FW_FEATURE_NON_DL ) ){

		        	        len                     	= le32_to_cpu(region->len);
			                addr                    	= le32_to_cpu(region->addr);

	        			Elf32_Phdr *CURR_PHDR   	= (Elf32_Phdr *)(ELF_HDR + ELF_ACTUAL_INCREMENT);
	                		CURR_PHDR->p_vaddr      	= addr;
	                		CURR_PHDR->p_paddr      	= addr;
	                		CURR_PHDR->p_filesz     	= len;
	                		CURR_PHDR->p_memsz      	= len;
	                		CURR_PHDR->p_offset     	= PHDR_ACTUAL_OFFSET;
	                		CURR_PHDR->p_type       	= PT_LOAD;
	        	        	CURR_PHDR->p_flags      	= PF_R | PF_W;

					ELF_ACTUAL_INCREMENT   	       += sizeof(Elf32_Phdr);
					printf(GREEN "[%s]" CRESET " phdr numbr: %d phys offset: %d mapped at 0x%x memsz: %d next region offset: %d\n",
						CANDIDATE_FW_CURRENT->current_metadata.candidate_file_name,
						k,
						PHDR_ACTUAL_OFFSET,
						CURR_PHDR->p_vaddr,
						CURR_PHDR->p_memsz,
						PHDR_ACTUAL_OFFSET + CURR_PHDR->p_memsz
					);

					memmove(
						ELF_HDR + PHDR_ACTUAL_OFFSET,
						CANDIDATE_FW_CURRENT->current_metadata.candidate_file_memory_map + final_elf_segment_total_counter,
						CURR_PHDR->p_memsz
					);

	        	        	PHDR_ACTUAL_OFFSET     	        += CURR_PHDR->p_filesz;

					final_elf_segment_total_counter	+= len;
				}
			}
		}

		/** reset it for every new ELF file to analyze **/
		final_elf_segment_total_counter		= 0;
        	CANDIDATE_FW_CURRENT 			= CANDIDATE_FW_CURRENT->next;

	}

        /** allocate the final ELF file **/
        int mtk_fw_final_elf            = 0;
	mtk_fw_final_elf                = open("mt7981.elf", O_CREAT | O_RDWR, 0777);

        if( mtk_fw_final_elf < 0 ){
                printf("impossible to open the output final elf file! aborting!\n");
        }

	write(mtk_fw_final_elf, ELF_HDR, final_elf_segment_total_size);
	printf("final elf size: %d number of segment in total: %d\n", final_elf_segment_total_size, final_elf_segment_total_number);

	MTK_FW_FREE_MAIN_LIST(CANDIDATE_FW_HEAD);

    return 0;
}

