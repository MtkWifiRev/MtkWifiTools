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


struct mtk_region_metadata{
	struct mt76_connac2_fw_region region_data;
	unsigned char 	              *region_data_start;
	unsigned int   		       region_data_length;
};

int main(int argc, char *argv[]){
	const char      *MT_WIFI_PATCH_NAME     = argv[1];
	const char      *MT_WIFI_NEW_REGION	= argv[2];
	const char      *MT_WIFI_NEW_REGION_ADDR= argv[3];
	const char	*MT_WIFI_NEW_REGION_IDX	= argv[4];
	const char      *MT_WIFI_NEW_REGION_CRC = argv[5];
        int              MT_WIFI_PATCH_FD       = 0x00;
	int		 MT_WIFI_NEW_REGION_FD  = 0x00;
        int              MT_WIFI_PATCH_SIZE     = 0x00;
	int		 MT_WIFI_NEW_REGION_SIZE= 0x00;
        struct stat     *MT_WIFI_PATCH_STATS    = NULL;
        unsigned char   *MT_WIFI_PATCH_MMAP     = NULL;
	unsigned char   *MT_WIFI_NEW_REGION_TMP = NULL;
	unsigned char   *MT_WIFI_NEW_REGION_DATA= NULL;

	const struct mt76_connac2_fw_trailer *hdr = NULL;

	if( argc < 6 ){
		printf("usage: %s <original firmware> <new region file> <new region address> <new region index> <crc>\n",	argv[0]);
		return 0;
	}

        if( MT_WIFI_PATCH_NAME == NULL ){
                printf("missing fw name!\n");
                return -1;
        }

        if( MT_WIFI_NEW_REGION == NULL ){
                printf("missing fw new region name!\n");
                return -1;
        }

	if( MT_WIFI_NEW_REGION_ADDR == NULL ){
		printf("missing new region address!\n");
		return -1;
	}

	if( MT_WIFI_NEW_REGION_IDX == NULL ){
		printf("missing new region index!\n");
		return -1;
	}

        if( MT_WIFI_NEW_REGION_CRC == NULL ){
                printf("missing the new CRC! (pass 0x00 if you don't want to have it applied)\n");
                return -1;
        }

        MT_WIFI_PATCH_FD                        = open(MT_WIFI_PATCH_NAME, O_RDONLY);
        if( MT_WIFI_PATCH_FD < 0 ){
                printf("missing fw file!\n");
                return -2;
        }

	MT_WIFI_NEW_REGION_FD			= open(MT_WIFI_NEW_REGION, O_RDONLY);
        if( MT_WIFI_NEW_REGION_FD < 0 ){
                printf("missing fw new region file!\n");
                return -2;
        }

        MT_WIFI_PATCH_STATS                     = (struct stat *)malloc(sizeof(struct stat));
        fstat(MT_WIFI_PATCH_FD, MT_WIFI_PATCH_STATS);

        if( MT_WIFI_PATCH_STATS->st_size <= 12 ){
                printf("fw file is empty!\n");
                close(MT_WIFI_PATCH_FD);
                return -3;
        }else{
                MT_WIFI_PATCH_SIZE              = MT_WIFI_PATCH_STATS->st_size;
        }

	memset(MT_WIFI_PATCH_STATS, 0x00, sizeof(struct stat));
	fstat(MT_WIFI_NEW_REGION_FD, MT_WIFI_PATCH_STATS);
        if( MT_WIFI_PATCH_STATS->st_size <= 12 ){
                printf("fw new region file is empty\n");
                close(MT_WIFI_NEW_REGION_FD);
                return -3;
        }else{
                MT_WIFI_NEW_REGION_SIZE         = MT_WIFI_PATCH_STATS->st_size;
        }

        free(MT_WIFI_PATCH_STATS);

        #define CRC_LEN                         4

        MT_WIFI_PATCH_MMAP                      = (unsigned char *)mmap(
                                                                        NULL,
                                                                        MT_WIFI_PATCH_SIZE,
                                                                        PROT_READ,
                                                                        MAP_FILE | MAP_SHARED,
                                                                        MT_WIFI_PATCH_FD,
                                                                        0x00
                                                );


	MT_WIFI_NEW_REGION_TMP			= (unsigned char *)mmap(
									NULL,
									MT_WIFI_NEW_REGION_SIZE,
									PROT_READ,
									MAP_FILE | MAP_SHARED,
									MT_WIFI_NEW_REGION_FD,
									0x00
						);

	MT_WIFI_NEW_REGION_DATA			= (unsigned char *)mmap(
									NULL,
									MT_WIFI_NEW_REGION_SIZE + CRC_LEN,
									PROT_READ | PROT_WRITE,
									MAP_ANON  | MAP_PRIVATE,
									-1,
									0x00
						);

	memset(MT_WIFI_NEW_REGION_DATA,				0x00,			MT_WIFI_NEW_REGION_SIZE + CRC_LEN);
	memmove(MT_WIFI_NEW_REGION_DATA,			MT_WIFI_NEW_REGION_TMP,	MT_WIFI_NEW_REGION_SIZE);
	munmap(MT_WIFI_NEW_REGION_TMP,				MT_WIFI_NEW_REGION_SIZE);

	// increment with the CRC32 length
	MT_WIFI_NEW_REGION_SIZE				       += CRC_LEN;
	hdr 							= (const void *)(MT_WIFI_PATCH_MMAP + MT_WIFI_PATCH_SIZE - sizeof(*hdr));
	struct mtk_region_metadata *metadata_list		= (struct mtk_region_metadata *)calloc(hdr->n_region + 1, sizeof(struct mtk_region_metadata));
	unsigned int  offset_counter				= 0x00;
	unsigned int  new_region_index				= atoi(MT_WIFI_NEW_REGION_IDX);
	unsigned long new_region_addr				= strtoul(MT_WIFI_NEW_REGION_ADDR, NULL, 16);
	unsigned char final_layout_counter			= 0x00;
	unsigned char once					= 0x00;
	unsigned char *trailer_string				= NULL;
	unsigned int   trailer_string_len			= 0x00;
        uint32_t       new_region_crc                           = strtoul(MT_WIFI_NEW_REGION_CRC, NULL, 16);

        // append the crc data since now to the end of the new region
        memmove(MT_WIFI_NEW_REGION_DATA + MT_WIFI_NEW_REGION_SIZE - CRC_LEN, &new_region_crc, sizeof(uint32_t));

	printf("new crc32 applied at 0x%x: %x\n", MT_WIFI_NEW_REGION_DATA + MT_WIFI_NEW_REGION_SIZE, new_region_crc);

	if( new_region_index > hdr->n_region + 1 ){
		printf("the new region index is bigger than the number of region!\n");
		return -4;
	}

	printf("address of the new region: 0x%x\n",             new_region_addr);
        printf("lenght of the new region: %d\n",                MT_WIFI_NEW_REGION_SIZE);

	// initialize the metadata_list structure
	for(unsigned char j = 0; j < hdr->n_region + 1; j++){
                memset(&metadata_list[j].region_data,   0x00, sizeof(struct mt76_connac2_fw_region));
                metadata_list[j].region_data_start      = 0x00;
                metadata_list[j].region_data_length     = 0x00;
	}

	for(unsigned char j = 0; j < hdr->n_region; j++){
		struct mt76_connac2_fw_region *region	= NULL;
		u32 len, addr, mode;

		region 					= (const void *)((const u8 *)hdr - (hdr->n_region - j) * sizeof(*region));

		len 					= le32_to_cpu(region->len);
		addr 					= le32_to_cpu(region->addr);

		if( j == new_region_index && once == 0x00 ){
			struct mt76_connac2_fw_region new_region = {
				.decomp_crc		= 0x00,
				.decomp_len		= 0x00,
				.decomp_blk_sz		= 0x00,
				.addr			= le32_to_cpu(new_region_addr),
				.len			= le32_to_cpu(MT_WIFI_NEW_REGION_SIZE),
				.feature_set		= le32_to_cpu(FW_FEATURE_OVERRIDE_ADDR)
			};

			memcpy(&metadata_list[final_layout_counter].region_data, &new_region, sizeof(struct mt76_connac2_fw_region));

			metadata_list[final_layout_counter].region_data_start	= MT_WIFI_NEW_REGION_DATA;
			metadata_list[final_layout_counter].region_data_length	= le32_to_cpu(MT_WIFI_NEW_REGION_SIZE);
			once							= 1;
			final_layout_counter++;
		}
		memcpy(&metadata_list[final_layout_counter].region_data,	region, sizeof(struct mt76_connac2_fw_region));

		metadata_list[final_layout_counter].region_data_start		= MT_WIFI_PATCH_MMAP + offset_counter;
		metadata_list[final_layout_counter].region_data_length		= len;
		offset_counter	     		  		     	       += len;

		final_layout_counter++;
	}

	// the 'trailer_string' is usually a 152 byte string used for delimitate the end of the region data and the start of the region's header
	trailer_string_len	= MT_WIFI_PATCH_SIZE - sizeof(*hdr) - hdr->n_region * sizeof(struct mt76_connac2_fw_region) - offset_counter;
	printf("trailer string len: %d\n", trailer_string_len);
	trailer_string		= (unsigned char *)malloc(trailer_string_len);
	memmove(trailer_string,	MT_WIFI_PATCH_MMAP + offset_counter, trailer_string_len);

	// now increase the 'offset_counter'
	offset_counter	       += trailer_string_len;

	printf("actual offset counter: %d\n", offset_counter);

	// just print the new region's layout

	printf("new region layout:\n");
        for(unsigned char j = 0; j < hdr->n_region + 1; j++){
		printf("[region %d] addr: 0x%x len: %d mapped in mcu: 0x%X\n",
			j,
			metadata_list[j].region_data_start,
			metadata_list[j].region_data_length,
			metadata_list[j].region_data.addr
		);
	}

	unsigned int   rewrited_firmware_counter				= 0x00;
	unsigned char *rewrited_firmware					= (unsigned char *)mmap(
												NULL,
												MT_WIFI_PATCH_SIZE + sizeof(struct mt76_connac2_fw_region) + MT_WIFI_NEW_REGION_SIZE,
												PROT_READ 	| PROT_WRITE,
												MAP_PRIVATE 	| MAP_ANON,
												-1,
												0x00
										);

	memset(rewrited_firmware,	0x00,	MT_WIFI_PATCH_SIZE + sizeof(struct mt76_connac2_fw_region) + MT_WIFI_NEW_REGION_SIZE);

	// now start copying the firmware data!

	for(unsigned char j = 0; j < hdr->n_region + 1; j++){
		memcpy(rewrited_firmware + rewrited_firmware_counter,	metadata_list[j].region_data_start, metadata_list[j].region_data_length);
		rewrited_firmware_counter				+= metadata_list[j].region_data_length;
	}

	// at this point copy the trailer string
	memcpy(rewrited_firmware + rewrited_firmware_counter, 		trailer_string,	trailer_string_len);

	rewrited_firmware_counter					+= trailer_string_len;

	// start copying the regions metadata

	for(unsigned char j = 0; j < hdr->n_region + 1; j++){
		memcpy(rewrited_firmware + rewrited_firmware_counter,	&metadata_list[j].region_data,		sizeof(struct mt76_connac2_fw_region));
		rewrited_firmware_counter				+= sizeof(struct mt76_connac2_fw_region);
	}

	// at the end, copy the header

	memmove(
			rewrited_firmware + rewrited_firmware_counter,
			MT_WIFI_PATCH_MMAP + MT_WIFI_PATCH_SIZE - sizeof(struct mt76_connac2_fw_trailer),
			sizeof(struct mt76_connac2_fw_trailer)
	);

	// acquire the pointer to the header data and increase the numbers

	struct mt76_connac2_fw_trailer *final_region_hdr		= (struct mt76_connac2_fw_trailer *)(rewrited_firmware + rewrited_firmware_counter);
	final_region_hdr->n_region++;
	printf("final region numbers: %d\n", final_region_hdr->n_region);
	rewrited_firmware_counter				       += sizeof(struct mt76_connac2_fw_trailer);

	// now rewrite everything into a new file
	int final_fd							= open("rewrited_firmware.bin", O_RDWR | O_CREAT, 0777);
	write(final_fd,		rewrited_firmware,			rewrited_firmware_counter);
	close(final_fd);

	munmap(MT_WIFI_NEW_REGION_DATA,	MT_WIFI_NEW_REGION_SIZE);
	munmap(MT_WIFI_PATCH_MMAP,	MT_WIFI_PATCH_SIZE);
	munmap(rewrited_firmware,	MT_WIFI_PATCH_SIZE + sizeof(struct mt76_connac2_fw_region) + MT_WIFI_NEW_REGION_SIZE);
	free(trailer_string);
	return 0;
}
