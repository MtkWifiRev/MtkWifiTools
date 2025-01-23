/**
        Made by Edoardo Mantovani, 8/9 December 2024
        given more than a 1 Mediatek wifi firmware, convert them into an ELF and sets a minimun amount of metadata necessary for fuzzing/analyzing it.
        This is useful for multiple fw like the WA,WM and WO blobs
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
};

/** linked list which will contains the input given by the user **/
typedef struct MTK_CANDIDATE_FW {
    	struct candidate_fw_metadata  current_metadata;
    	struct MTK_CANDIDATE_FW      *next;
} MTK_CANDIDATE_FW;

uint32_t wlanCRC32(uint8_t *buf, uint32_t len){
	uint32_t i, crc32 = 0xFFFFFFFF;
	const uint32_t crc32_ccitt_table[256] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
		0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
		0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
		0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
		0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
		0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
		0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
		0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
		0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
		0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
		0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
		0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
		0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
		0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
		0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
		0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
		0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
		0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
		0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
		0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
		0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
		0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
		0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
		0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
		0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
		0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
		0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
		0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
		0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
		0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
		0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
		0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
		0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
		0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
		0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
		0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
		0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
		0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
		0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
		0x2d02ef8d
	};

	for (i = 0; i < len; i++){
		crc32 = crc32_ccitt_table[(crc32 ^ buf[i]) & 0xff] ^ (crc32 >> 8);
	}

	return ~crc32;
}

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

/** our main function **/
int main(int argc, char* argv[]) {

	/** close if no args are given **/
    	if (argc < 2) {
    	    printf(RED "%s <fw1.bin> <fw2.bin> ...\n" CRESET, argv[0]);
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

//		printf("header at 0x%x\n", CANDIDATE_NEW_ELEM->current_metadata.candidate_file_memory_map  +
    //                                                            CANDIDATE_NEW_ELEM->current_metadata.candidate_file_size                -
  //                                                              sizeof(*hdr));
//
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
				printf(
					"region %d crc: %lx\n",
					k,
					wlanCRC32(
						CANDIDATE_FW_CURRENT->current_metadata.candidate_file_memory_map + final_elf_segment_total_counter,
						CURR_PHDR->p_memsz)
				);

				final_elf_segment_total_counter	+= len;
			}
		}
		/** display the final CRC **/
		printf(
			"final crc: %lx\n",
			wlanCRC32(
				CANDIDATE_FW_CURRENT->current_metadata.candidate_file_memory_map,
				final_elf_segment_total_counter
			)
		);

		/** reset it for every new ELF file to analyze **/
		final_elf_segment_total_counter		= 0;
        	CANDIDATE_FW_CURRENT 			= CANDIDATE_FW_CURRENT->next;

	}

        /** allocate the final ELF file **/
        int mtk_fw_final_elf            = 0;
	mtk_fw_final_elf                = open("MTK_WIFI_MERGED.elf", O_CREAT | O_RDWR, 0777);

        if( mtk_fw_final_elf < 0 ){
                printf("impossible to open the output final elf file! aborting!\n");
        }

	write(mtk_fw_final_elf, ELF_HDR, final_elf_segment_total_size);
	printf("final elf size: %d number of segment in total: %d\n", final_elf_segment_total_size, final_elf_segment_total_number);

	MTK_FW_FREE_MAIN_LIST(CANDIDATE_FW_HEAD);

    return 0;
}
