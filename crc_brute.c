/**
	Made by Edoardo Mantovani 2024
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
}__attribute__((packed));       // note that is packed !!

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
}__attribute__((packed));       // note that is packed !!

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

int main(int argc, char *argv[]){
        unsigned char   *file_name  = argv[1];
        unsigned char   *mapped_fw  = NULL;
        struct stat     *file_stats = NULL;
        unsigned int    file_size   = 0;
        int             file_fd     = 0;
        int             res         = 0;

        if( file_name == NULL || strlen(file_name) <= 1 ){
                printf("please provide a valid file!\n");
                //exit(ERROR(ERROR_NO_VALID_FILE));
        }

        file_fd         = open(file_name, O_RDONLY);

        if( file_fd < 0 ){
                printf("file do not exists!\n");
                //exit(ERROR(ERROR_FILE_NOT_EXIST));
        }

        file_stats      = (struct stat *)malloc(sizeof(struct stat));
        fstat(file_fd, file_stats);
        file_size       = file_stats->st_size;

        if( file_size == 0 ){
                printf("file size is 0!\n");
                //exit(ERROR(ERROR_FILE_SIZE));
        }else{
                printf("file size is: %d\n", file_size);
        }

        free(file_stats);

        mapped_fw       = (unsigned char *)mmap(
                                        (void *)0x00,
                                        file_size,
                                        PROT_READ,
                                        MAP_FILE | MAP_SHARED,
                                        file_fd,
                                        0x00
                  );

        if( mapped_fw   == NULL ){
                printf("file mmap is NULL!\n");
                //exit(ERROR(ERROR_FILE_MMAP));

        }

        /** here we start with the function 'mt76_connac_mcu_send_ram_firmware' code **/
        const struct mt76_connac2_fw_trailer *hdr = (const struct mt76_connac2_fw_trailer *)(mapped_fw + file_size - sizeof(*hdr));
        printf("chip_id: 0x%x\neco_code: 0x%x\nn_region: %d\nfw_ver: %.10s\nbuild_time: %.15s, crc: %x\n",
                hdr->chip_id,
                hdr->eco_code,
                hdr->n_region,
                hdr->fw_ver,
                hdr->build_date,
		hdr->crc
        );

	unsigned char *start_of_segment_ptr			= NULL;
        unsigned int MTK_FW_ALLOCATE_SIZE                       = 0x00;
	unsigned int counter					= 0x00;
	__le32 result_crc 					= 0x00;

        for(unsigned int i = 0; i < hdr->n_region; i++){
                const struct mt76_connac2_fw_region *region     = NULL;
                uint32_t len                                    = 0;
                uint32_t addr                                   = 0;
                uint32_t mode                                   = 0;
                region                                          = (const void *)((const u8 *)hdr - (hdr->n_region - i) * sizeof(*region));

                len                                             = le32_to_cpu(region->len);
                addr                                            = le32_to_cpu(region->addr);

                start_of_segment_ptr                            = (unsigned char *)( mapped_fw + MTK_FW_ALLOCATE_SIZE );

                /** DO FEATURE REPORTING **/
                if (region->feature_set & FW_FEATURE_NON_DL ){
                        printf("[region %d] won't be loaded into memory!\n", i);
                }else{
                        MTK_FW_ALLOCATE_SIZE                    += region->len;
                }

		__le32 crc_to_check				 = *((__le32 *)(start_of_segment_ptr + len - sizeof(__le32)));

		while( result_crc != crc_to_check ){
			result_crc = wlanCRC32(start_of_segment_ptr, counter);
                	counter++;
	                printf("result_crc: %x == crc_to_check: %x for counter: %d\n", result_crc, crc_to_check, counter);
			if( counter > len ){
				exit(0);
			}
        	}
	}

	return 0;
}
