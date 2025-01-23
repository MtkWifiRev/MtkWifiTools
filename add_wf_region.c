// Made by Eodardo Mantovani, simple program which given the a wifi mediatek firmware, add a new region at the end for loading our patches

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

#ifndef u32
	#define u32		uint32_t
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


int main(int argc, char *argv[]){
	const char      *MT_WIFI_PATCH_NAME     = argv[1];
	const char      *MT_WIFI_NEW_REGION     = argv[2];
	const char      *MT_WIFI_NEW_REG_ADDR	= argv[3];
        int              MT_WIFI_PATCH_FD       = 0x00;
	int		 MT_WIFI_NEW_REGION_FD	= 0x00;
        int              MT_WIFI_PATCH_SIZE     = 0x00;
	int		 MT_WIFI_NEW_REGION_SIZE= 0x00;
        struct stat     *MT_WIFI_PATCH_STATS    = NULL;
        unsigned char   *MT_WIFI_PATCH_MMAP     = NULL;
	unsigned char   *MT_WIFI_NEW_REGION_MMAP= NULL;
	unsigned char   *MT_WIFI_FINAL_BLOB	= NULL;

	struct mt76_connac2_fw_trailer *hdr 	= NULL;

        if( MT_WIFI_PATCH_NAME == NULL ){
                printf("missing patch name!\n");
                return -1;
        }

	if( MT_WIFI_NEW_REGION == NULL ){
		printf("missing new region file name!\n");
		return -1;
	}

	if( MT_WIFI_NEW_REG_ADDR == NULL ){
		printf("missing the new region address!\n");
		return -1;
	}

        MT_WIFI_PATCH_FD                        = open(MT_WIFI_PATCH_NAME, O_RDONLY);
        if( MT_WIFI_PATCH_FD < 0 ){
                printf("missing patch file!\n");
                return -2;
        }

	MT_WIFI_NEW_REGION_FD			= open(MT_WIFI_NEW_REGION, O_RDONLY);
	if( MT_WIFI_NEW_REGION_FD < 0 ){
		printf("missing new region file!\n");
		return -2;
	}

        MT_WIFI_PATCH_STATS                     = (struct stat *)malloc(sizeof(struct stat));
        fstat(MT_WIFI_PATCH_FD, MT_WIFI_PATCH_STATS);

        if( MT_WIFI_PATCH_STATS->st_size <= 12 ){
                printf("patch file is empty!\n");
                close(MT_WIFI_PATCH_FD);
                return -3;
        }else{
                MT_WIFI_PATCH_SIZE              = MT_WIFI_PATCH_STATS->st_size;
        }

	/** reuse the MT_WIFI_PATCH_STATS **/
	memset(MT_WIFI_PATCH_STATS,	0x00,	sizeof(struct stat));
	fstat(MT_WIFI_NEW_REGION_FD, MT_WIFI_PATCH_STATS);

	if( MT_WIFI_PATCH_STATS->st_size <= 16 ){
		printf("new region file is empty!\n");
		close(MT_WIFI_NEW_REGION_FD);
		return -3;
	}else{
		MT_WIFI_NEW_REGION_SIZE		= MT_WIFI_PATCH_STATS->st_size;
	}

        free(MT_WIFI_PATCH_STATS);

	MT_WIFI_NEW_REGION_MMAP			= (unsigned char *)mmap(
									NULL,
									MT_WIFI_NEW_REGION_SIZE,
									PROT_READ,
									MAP_FILE | MAP_SHARED,
									MT_WIFI_NEW_REGION_FD,
									0x00
						);

        MT_WIFI_PATCH_MMAP                      = (unsigned char *)mmap(
                                                                        NULL,
                                                                        MT_WIFI_PATCH_SIZE,
                                                                        PROT_READ,
                                                                        MAP_FILE | MAP_SHARED,
                                                                        MT_WIFI_PATCH_FD,
                                                                        0x00
                                                );

	MT_WIFI_FINAL_BLOB			= (unsigned char *)mmap(
									NULL,
									MT_WIFI_PATCH_SIZE + MT_WIFI_NEW_REGION_SIZE + sizeof(struct mt76_connac2_fw_region),
									PROT_READ | PROT_WRITE,
									MAP_ANON  | MAP_PRIVATE,
									-1,
									0x00
						);

	memset(MT_WIFI_FINAL_BLOB,	0x00,	MT_WIFI_PATCH_SIZE + MT_WIFI_NEW_REGION_SIZE + sizeof(struct mt76_connac2_fw_region));
	hdr 					= (const void *)(MT_WIFI_PATCH_MMAP + MT_WIFI_PATCH_SIZE - sizeof(*hdr));
	unsigned int offset_counter 		= 0x00;
	void *first_region_start 		= NULL;
	void *trailer_space			= NULL;

	for (int i = 0; i < hdr->n_region; i++) {
		struct mt76_connac2_fw_region *region	= NULL;
		u32 len, addr, mode		= 0x00;
		int err				= 0x00;

		region 				= (const void *)((const u8 *)hdr - (hdr->n_region - i) * sizeof(*region));

		len			 	= le32_to_cpu(region->len);
		addr 				= le32_to_cpu(region->addr);

		printf("addr: 0x%x len: %d\n", addr, len);

		memmove(MT_WIFI_FINAL_BLOB + offset_counter, MT_WIFI_PATCH_MMAP + offset_counter, len);
		offset_counter 			+= len;
	}

	printf("trailer_space len: %d\n", 	(MT_WIFI_PATCH_SIZE - sizeof(*hdr) - (hdr->n_region) * sizeof(struct mt76_connac2_fw_region)) - offset_counter);
	unsigned int trailer_space_len 		= MT_WIFI_PATCH_SIZE - sizeof(*hdr) - (hdr->n_region) * sizeof(struct mt76_connac2_fw_region) - offset_counter;

	unsigned char *trailer_space_s 		= (unsigned char *)malloc(trailer_space_len);
	memmove(trailer_space_s, MT_WIFI_PATCH_MMAP + offset_counter, trailer_space_len);

	printf("adding the new region!\n");
	memmove(MT_WIFI_FINAL_BLOB + offset_counter, MT_WIFI_NEW_REGION_MMAP, MT_WIFI_NEW_REGION_SIZE);
	offset_counter 				+= MT_WIFI_NEW_REGION_SIZE;
        memmove(
                MT_WIFI_FINAL_BLOB + offset_counter,
                trailer_space_s,
		trailer_space_len
        );

	offset_counter 				+= trailer_space_len;
	printf("copying the region headers!\n");
	for(unsigned char j = 0; j < hdr->n_region; j++){
		struct mt76_connac2_fw_region *curr_copied_region   = NULL;
		curr_copied_region 		                    =  (const void *)((const u8 *)hdr - (hdr->n_region - j) * sizeof(*curr_copied_region));
		memmove(MT_WIFI_FINAL_BLOB + offset_counter, curr_copied_region, sizeof(struct mt76_connac2_fw_region));
		offset_counter			       += sizeof(struct mt76_connac2_fw_region);
		printf("copied header %d with addr: 0x%x and len: %d\n", j, curr_copied_region->addr, curr_copied_region->len);
	}

	printf("creating the last region!\n");
	// add finally the last region
	struct mt76_connac2_fw_region *new_region		     = (struct mt76_connac2_fw_region *)(MT_WIFI_FINAL_BLOB + offset_counter);
	new_region->addr					     = cpu_to_le32(strtoul(MT_WIFI_NEW_REG_ADDR, NULL, 16));
	new_region->len						     = cpu_to_le32(MT_WIFI_NEW_REGION_SIZE);
	new_region->feature_set					     = FW_FEATURE_OVERRIDE_ADDR;

	offset_counter 			                            += sizeof(struct mt76_connac2_fw_region);

	printf("new region metadata:\nlen: %d\naddr: 0x%x\n",
		new_region->len,
		new_region->addr
	);

	printf("copying the header!\n");

	printf("total_len - offset counter = %d vs %d\n",
		MT_WIFI_PATCH_SIZE + MT_WIFI_NEW_REGION_SIZE + sizeof(struct mt76_connac2_fw_region) - offset_counter,
		sizeof(*hdr)
		);

	memmove(
		MT_WIFI_FINAL_BLOB + offset_counter,
		hdr,
		sizeof(*hdr)
	);

	// now just modify the header for adding + 1 region
	struct mt76_connac2_fw_trailer* final_file_region = (struct mt76_connac2_fw_trailer *)(
		MT_WIFI_FINAL_BLOB 			+
		MT_WIFI_PATCH_SIZE 			+
		MT_WIFI_NEW_REGION_SIZE 		+
		sizeof(struct mt76_connac2_fw_region)   -
		sizeof(struct mt76_connac2_fw_trailer)
	);

	printf("incrementing the region number!\n");
	final_file_region->n_region++;
	printf("total region number: %d\n", 	final_file_region->n_region);

	int final_fd				= open("merged_firmware.bin", O_CREAT | O_RDWR, 0777);
	write(final_fd, 			MT_WIFI_FINAL_BLOB, MT_WIFI_PATCH_SIZE + MT_WIFI_NEW_REGION_SIZE + sizeof(struct mt76_connac2_fw_region));
	close(final_fd);

        munmap(MT_WIFI_FINAL_BLOB,              MT_WIFI_PATCH_SIZE + MT_WIFI_NEW_REGION_SIZE);
        munmap(MT_WIFI_NEW_REGION_MMAP,         MT_WIFI_NEW_REGION_SIZE);
        munmap(MT_WIFI_PATCH_MMAP,              MT_WIFI_PATCH_SIZE);
	return 0;
}
