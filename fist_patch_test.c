#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

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


unsigned char section_text[] = {
  0xfc, 0x61, 0x47, 0xd0, 0x00, 0x00, 0x59, 0xde, 0x80, 0x00, 0xdd, 0xbd,
  0x44, 0x00, 0x78, 0x25, 0x84, 0xc0, 0x12, 0x0f, 0x80, 0x02, 0x46, 0xb0,
  0x08, 0x00, 0x50, 0x9f, 0x80, 0x04, 0x46, 0x70, 0x08, 0x22, 0x81, 0xa6,
  0x85, 0x82, 0x44, 0xa0, 0x0a, 0x93, 0x40, 0x23, 0x2c, 0x00, 0x80, 0xad,
  0x80, 0x8c, 0x84, 0x62, 0x80, 0x2a, 0x80, 0x09, 0x8c, 0xc1, 0xdd, 0x27,
  0x97, 0xb0, 0xd5, 0x00
};

int main(int argc, char *argv[]){
        const char      *MT_WIFI_PATCH_NAME     = argv[1];
        int              MT_WIFI_PATCH_FD       = 0x00;
        int              MT_WIFI_PATCH_SIZE     = 0x00;
        struct stat     *MT_WIFI_PATCH_STATS    = NULL;
        unsigned char   *MT_WIFI_PATCH_MMAP     = NULL;


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

	unsigned char *real_file_output		= (unsigned char *)mmap(
									NULL,
									MT_WIFI_PATCH_SIZE,
									PROT_READ  | PROT_WRITE,
									MAP_SHARED | MAP_ANON,
									-1,
									0x00
						);

        memcpy(real_file_output, MT_WIFI_PATCH_MMAP, MT_WIFI_PATCH_SIZE);
        munmap(MT_WIFI_PATCH_MMAP, MT_WIFI_PATCH_SIZE);
	struct mt76_connac2_fw_trailer *hdr = (struct mt76_connac2_fw_trailer *)(
                                                                real_file_output   +
                                                                MT_WIFI_PATCH_SIZE -
                                                                sizeof(*hdr)
                                                        );

	printf("crc is %lx\n", hdr->crc);
	hdr->crc = NULL;
	printf("now crc is %lx\n", hdr->crc);
	unsigned int function_start_offset = 14618 + 2;
	memcpy(real_file_output + function_start_offset, section_text, sizeof(section_text));
	int fd = open("patched_binary.bin", O_CREAT | O_RDWR, 0777);
	write(fd, real_file_output, MT_WIFI_PATCH_SIZE);
	close(fd);
	munmap(real_file_output, MT_WIFI_PATCH_SIZE);
	return 0;
}
