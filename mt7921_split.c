// Made by Eodardo Mantovani, simple program which given the mt7921 fw, it splits into a separated file for every section found

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
        int              MT_WIFI_PATCH_FD       = 0x00;
        int              MT_WIFI_PATCH_SIZE     = 0x00;
        struct stat     *MT_WIFI_PATCH_STATS    = NULL;
        unsigned char   *MT_WIFI_PATCH_MMAP     = NULL;

	const struct mt76_connac2_fw_trailer *hdr = NULL;

        if( MT_WIFI_PATCH_NAME == NULL ){
                printf("missing patch name!\n");
                return -1;
        }

        MT_WIFI_PATCH_FD                        = open(MT_WIFI_PATCH_NAME, O_RDONLY);
        if( MT_WIFI_PATCH_FD < 0 ){
                printf("missing patch file!\n");
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

        free(MT_WIFI_PATCH_STATS);

        MT_WIFI_PATCH_MMAP                      = (unsigned char *)mmap(
                                                                        NULL,
                                                                        MT_WIFI_PATCH_SIZE,
                                                                        PROT_READ,
                                                                        MAP_FILE | MAP_SHARED,
                                                                        MT_WIFI_PATCH_FD,
                                                                        0x00
                                                );

	hdr = (const void *)(MT_WIFI_PATCH_MMAP + MT_WIFI_PATCH_SIZE - sizeof(*hdr));
	unsigned char file_name[24] = { 0 };
	unsigned int offset_counter = 0x00;
	void *first_region_start = NULL;

	for (int i = 0; i < hdr->n_region; i++) {
		memset(file_name, ' ', 24);
		const struct mt76_connac2_fw_region *region;
		u32 len, addr, mode;
		int err;

		region = (const void *)((const u8 *)hdr - (hdr->n_region - i) * sizeof(*region));
		if( first_region_start == NULL ){
			first_region_start = region;
		}

		len 	= le32_to_cpu(region->len);
		addr 	= le32_to_cpu(region->addr);

		printf("addr: 0x%x len: %d\n", addr, len);

		snprintf(file_name, 24, "out_%d_0x%x", i, addr);

		int open_file = open(file_name, O_CREAT | O_RDWR, 0777);
		write(open_file, MT_WIFI_PATCH_MMAP + offset_counter, len);
		close(open_file);
		offset_counter += len;
	 }

	// now do the final thing for the header too!
	snprintf(file_name, 24, "trailer.bin");
	int opened_header = open(file_name, O_CREAT | O_RDWR, 0777);
	write(opened_header, MT_WIFI_PATCH_MMAP + offset_counter, MT_WIFI_PATCH_SIZE - offset_counter);
	close(opened_header);
	return 0;
}
