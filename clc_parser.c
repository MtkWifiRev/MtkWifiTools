/**
	given the mt7921 wifi mtk firmware, extract the clc data from the last region (which is not loadable)
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
#include <asm-generic/errno-base.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
        #include <linux/byteorder/little_endian.h>
#else
        #include <linux/byteorder/big_endian.h>
#endif

#ifndef u8
        #define u8              unsigned char
#endif

#ifndef u16
	#define u16		unsigned short
#endif

#ifndef cpu_to_le16
        #define cpu_to_le16     __cpu_to_le16
#endif

#ifndef cpu_to_le32
	#define cpu_to_le32	__cpu_to_le32
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


struct mt7921_clc_rule {
	u8 alpha2[2];
	u8 type[2];
	__le16 len;
	u8 data[];
}__attribute__((packed));	// note that is packed !!

struct mt7921_clc {
	__le32 len;
	u8 idx;
	u8 ver;
	u8 nr_country;
	u8 type;
	u8 rsv[8];
	u8 data[];
}__attribute__((packed));	// note that is packed !!

enum {
	FW_TYPE_DEFAULT = 0,
	FW_TYPE_CLC = 2,
	FW_TYPE_MAX_NUM = 255
};

enum environment_cap {
	ENVIRON_ANY,
	ENVIRON_INDOOR,
	ENVIRON_OUTDOOR,
};

typedef struct mt76_connac2_fw_region	MT7921_FWREGION;


int mt7921_mcu_set_clc(struct mt7921_clc *clc, unsigned char *alpha2, enum environment_cap env_cap){
	int ret	 	 			= 0x00;
	int valid_cnt	 			= 0x00;
	uint32_t buf_len 			= 0x00;
	uint8_t  *pos	 			= 0x00;

	if (!clc){
		return 0;
	}

	buf_len 				= le32_to_cpu(clc->len) - sizeof(*clc);
	pos 					= clc->data;

	struct {
		u8 ver;
		u8 pad0;
		__le16 len;
		u8 idx;
		u8 env;
		u8 acpi_conf;
		u8 cap;
		u8 alpha2[2];
		u8 type[2];
		u8 env_6g;
		u8 mtcl_conf;
		u8 rsvd[62];
	}__attribute__((packed)) req = {
		//.ver = 1,
		//.idx = idx,
		//.env = env_cap,
		//.env_6g = dev->phy.power_type,
		//.acpi_conf = mt792x_acpi_get_flags(&dev->phy),
		//.mtcl_conf = mt792x_acpi_get_mtcl_conf(&dev->phy, alpha2),
	};

	while (buf_len > 16) {
		struct mt7921_clc_rule *rule 	= (struct mt7921_clc_rule *)pos;
		u16 len 			= le16_to_cpu(rule->len);
		u16 offset	 		= len + sizeof(*rule);

		pos 			       += offset;
		buf_len                        -= offset;
		if (rule->alpha2[0] != alpha2[0] || rule->alpha2[1] != alpha2[1]){
			continue;
		}

		req.len = cpu_to_le16(sizeof(req) + len);


		printf("req.len 0x%x 0x%x -> len: 0x%x\n", req.len, le16_to_cpu(req.len), len);

		valid_cnt++;
	}

	if (!valid_cnt){
		return -ENOENT;
	}
}

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

	hdr 					= (const void *)(MT_WIFI_PATCH_MMAP + MT_WIFI_PATCH_SIZE - sizeof(*hdr));

	unsigned int   offset			= 0x00;
	signed   int   ret			= 0x00;
        signed int len                  	= 0x00;

	unsigned char *clc_base			= NULL;
	struct mt7921_clc *clc			= NULL;

	for (int i = 0; i < hdr->n_region; i++){
		MT7921_FWREGION *region		= NULL;
		region 				= (void *)((const u8 *)hdr - (hdr->n_region - i) * sizeof(*region));
		len 				= le32_to_cpu(region->len);

		/* check if we have valid buffer size */
		if (offset + len > MT_WIFI_PATCH_SIZE) {
			printf("[!]Invalid firmware region\n");
			ret = -EINVAL;
			goto out;
		}

		if ((region->feature_set & FW_FEATURE_NON_DL) && region->type == FW_TYPE_CLC) {
			clc_base 		= (u8 *)(MT_WIFI_PATCH_MMAP + offset);
			break;
		}
		offset += len;
	 }

	if (!clc_base){
		goto out;
	}

	for (offset = 0; offset < len; offset += le32_to_cpu(clc->len)) {
		clc 				= (struct mt7921_clc *)(clc_base + offset);

		ret = mt7921_mcu_set_clc(clc, "00", ENVIRON_INDOOR);
	}

	out:
	munmap(MT_WIFI_PATCH_MMAP, MT_WIFI_PATCH_SIZE);

	return ret;
}

