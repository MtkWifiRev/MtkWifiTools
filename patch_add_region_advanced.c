#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <endian.h>
#include <stdint.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include <linux/types.h>
#include <linux/const.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
        #pragma message "using little endian!"
        #include <linux/byteorder/little_endian.h>
#else
        #pragma message "using big endian!"
        #include <linux/byteorder/big_endian.h>
#endif

#ifndef MT_WIFI_PATCH_MIN_SIZE
	#define MT_WIFI_PATCH_MIN_SIZE	256
#endif

#define MT_BUILD_DATE_LEN		16
#define MT_PLATFORM_LEN			4

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

int main(int argc, char *argv[]){
	const char 	*MT_WIFI_PATCH_NAME 	= argv[1];
	const char      *MT_WIFI_NEW_REG_NAME	= argv[2];
	int		 MT_WIFI_PATCH_FD   	= 0x00;
	int		 MT_WIFI_PATCH_SIZE	= 0x00;
	struct stat     *MT_WIFI_PATCH_STATS	= NULL;
	unsigned char   *MT_WIFI_PATCH_MMAP	= NULL;
	unsigned char   *MT_WIFI_PATCH_MMAP_TMP = NULL;
	MT_PATCH_HDR	*MT_WIFI_PATCH_HDR	= NULL;

	if( MT_WIFI_PATCH_NAME == NULL ){
		printf("missing patch name!\n");
		return -1;
	}

        if( MT_WIFI_NEW_REG_NAME == NULL ){
                printf("missing the new region name!\n");
                return -1;
        }

	MT_WIFI_PATCH_FD 			= open(MT_WIFI_PATCH_NAME, O_RDONLY);
	if( MT_WIFI_PATCH_FD < 0 ){
		printf("missing patch file!\n");
		return -2;
	}

	MT_WIFI_PATCH_STATS			= (struct stat *)malloc(sizeof(struct stat));
	fstat(MT_WIFI_PATCH_FD, MT_WIFI_PATCH_STATS);

	if( MT_WIFI_PATCH_STATS->st_size <= MT_WIFI_PATCH_MIN_SIZE ){
		printf("patch file is empty!\n");
		close(MT_WIFI_PATCH_FD);
		return -3;
	}else{
		MT_WIFI_PATCH_SIZE		= MT_WIFI_PATCH_STATS->st_size;
	}

	free(MT_WIFI_PATCH_STATS);

        MT_WIFI_PATCH_MMAP                  = (unsigned char *)mmap(
                                                                        NULL,
                                                                        MT_WIFI_PATCH_SIZE,
                                                                        PROT_READ,
                                                                        MAP_FILE  | MAP_SHARED,
                                                                        MT_WIFI_PATCH_FD,
                                                                        0x00
                                                );


        close(MT_WIFI_PATCH_FD);


	if( MT_WIFI_PATCH_MMAP == NULL ){
		printf("error while MMAPing the patch!\n");
		close(MT_WIFI_PATCH_FD);
		exit(-1);
	}

	MT_WIFI_PATCH_HDR				= (MT_PATCH_HDR *)MT_WIFI_PATCH_MMAP;

	printf("=================== ORIGINAL PATCH =========================\n");
	printf("[WIFI_PATCH_HDR] build_date: %.15s\n", 	MT_WIFI_PATCH_HDR->build_date);
        printf("[WIFI_PATCH_HDR] platform:   %.4s\n", 	MT_WIFI_PATCH_HDR->platform);

	printf("[WIFI_PATCH_HDR] hw_sw_ver:  %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->hw_sw_ver));
	printf("[WIFI_PATCH_HDR] patch_ver:  %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->patch_ver));
	printf("[WIFI_PATCH_HDR] checksum:   %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->checksum));
	printf("[WIFI_PATCH_HDR] rsv:	     %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->rsv));
	printf("[WIFI_PATCH_HDR] patch_ver:  %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.patch_ver));
	printf("[WIFI_PATCH_HDR] subsys:     %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.subsys));
	printf("[WIFI_PATCH_HDR] feature:    %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.feature));
	printf("[WIFI_PATCH_HDR] n_region:   %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region));
	printf("[WIFI_PATCH_HDR] crc:        %x\n", 	__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.crc));

	for(unsigned char j = 0; j < __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region); j++){
		MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;
		uint32_t len				= 0;
		uint32_t addr				= 0;
		uint32_t mode				= 0;
		uint32_t sec_info			= 0;
		uint8_t *dl				= NULL;

		MT_WIFI_PATCH_CURR_SECT 		= (MT_PATCH_SEC *)(MT_WIFI_PATCH_MMAP + sizeof(*MT_WIFI_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT));

		if ( ( __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->type) & PATCH_SEC_TYPE_MASK) != PATCH_SEC_TYPE_INFO) {
			/** do nothing **/
		}

		addr 					= 0x0		     + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr);
		len 					= 0x0		     + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len);
		sec_info 				= 0x0		     + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.sec_key_idx);
                dl                                      = MT_WIFI_PATCH_MMAP + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs);

		printf("[section number %d] offset: 0x%x, size: %d, address: 0x%x, len: %d\n",
			j,
			__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs),
			__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->size),
			__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr),
			__be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len)
		);

		mode 					= mt76_connac2_get_data_mode(j, sec_info);


	}

	// start with the patch modification

	unsigned char  address_to_patch_string[12]	= { 0 };
	void  	       *address_to_patch_addr 		= NULL;
	unsigned char  *new_region_patch_data		= NULL;
	unsigned int    new_region_patch_size		= 0x00;
	unsigned char   final_number_of_patch_regions   = __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region) + 1;
	int	 	final_patch_fd			= 0x00;
	unsigned char  *patch_final_code		= NULL;
	unsigned int    final_patch_ptr_counter		= 0x00;
	int             final_region_file_fd		= 0x00;
	struct stat    *final_region_file_stat		= NULL;

	final_region_file_fd				= open(MT_WIFI_NEW_REG_NAME, O_RDONLY);

	if( final_region_file_fd < 0 ){
		printf("failed to open %s!\n", MT_WIFI_NEW_REG_NAME);
		exit(-2);
	}

	final_region_file_stat				= (struct stat *)malloc(sizeof(struct stat));
	memset(final_region_file_stat,		0x00,	sizeof(struct stat));
	fstat(final_region_file_fd, 			final_region_file_stat);
	new_region_patch_size				= final_region_file_stat->st_size;
	free(final_region_file_stat);

	printf("> insert the address where the second segment of the patch will be loaded: ");
	memset(address_to_patch_string, ' ', 12);
	scanf("%s", address_to_patch_string);
	sscanf(address_to_patch_string, "%p", &address_to_patch_addr);

	printf("segment %d will be loaded at address: 0x%x with size: %d\n", __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region) + 1, address_to_patch_addr, new_region_patch_size);

	new_region_patch_data				= (unsigned char *)mmap(
								NULL,
								new_region_patch_size,
								PROT_READ,
								MAP_FILE | MAP_SHARED,
								final_region_file_fd,
								0x00
							);

	// create the new region

	patch_final_code				= (unsigned char *)mmap(
								NULL,
								MT_WIFI_PATCH_SIZE + sizeof(MT_PATCH_SEC) + new_region_patch_size,
								PROT_READ | PROT_WRITE,
								MAP_ANON  | MAP_PRIVATE,
								-1,
								0x00
							);


	memset(patch_final_code,			0x00,			MT_WIFI_PATCH_SIZE + sizeof(MT_PATCH_SEC) + new_region_patch_size);
	// copy the patch header + regions
	memcpy(
			patch_final_code + final_patch_ptr_counter,
			MT_WIFI_PATCH_MMAP,
			sizeof(MT_PATCH_HDR) + __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region) * sizeof(MT_PATCH_SEC)
	);

	final_patch_ptr_counter			       += sizeof(MT_PATCH_HDR) + __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region) * sizeof(MT_PATCH_SEC);

	// craft the new region starting from the first one

	memcpy(
		patch_final_code 	+ final_patch_ptr_counter,
		MT_WIFI_PATCH_MMAP	+ sizeof(MT_PATCH_HDR),
		sizeof(MT_PATCH_SEC)
	);

	// for every 'old' region, fix the offset by adding "+ sizeof(MT_PATCH_SEC)"

        for(unsigned char j = 0; j < __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region); j++){
                MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;
                MT_WIFI_PATCH_CURR_SECT                 = (MT_PATCH_SEC *)(patch_final_code + sizeof(*MT_WIFI_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT));
		MT_WIFI_PATCH_CURR_SECT->offs	       += __be32_to_cpu(sizeof(MT_PATCH_SEC));
	}

	// obtain a pointer to the last region's header of the new patch

	MT_PATCH_SEC *new_patch_region_header		= (MT_PATCH_SEC *)( patch_final_code + final_patch_ptr_counter );

	if( new_patch_region_header == NULL ){
		printf("[!] new_patch_region_header seems to be NULL!\n");
        	free(new_region_patch_data);
        	munmap(patch_final_code,                        MT_WIFI_PATCH_SIZE + sizeof(MT_PATCH_SEC) + new_region_patch_size);
		exit(-1);
	}

	new_patch_region_header->size			=  __be32_to_cpu(new_region_patch_size);
        new_patch_region_header->info.addr		=  __be32_to_cpu(address_to_patch_addr);
        new_patch_region_header->info.len		=  __be32_to_cpu(new_region_patch_size);
        new_patch_region_header->info.sec_key_idx	=  __be32_to_cpu(0x00);

	final_patch_ptr_counter			       += sizeof(MT_PATCH_SEC);

	// increase the number of the regions
	MT_PATCH_HDR *new_patch_header			= (MT_PATCH_HDR *)( patch_final_code + 0x00 );
	new_patch_header->desc.n_region			= MT_WIFI_PATCH_HDR->desc.n_region + __be32_to_cpu(1);

	// at that point, start to copy the "old" regions in the right order
        for(unsigned char j = 0; j < __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region); j++){
		uint8_t      *dl			= NULL;
		uint32_t      len			= 0x00;
                MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;
                MT_WIFI_PATCH_CURR_SECT                 = (MT_PATCH_SEC *)(MT_WIFI_PATCH_MMAP + sizeof(*MT_WIFI_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT));
                dl                                      = MT_WIFI_PATCH_MMAP + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs);
		len                                     = 0x0                + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len);

                memcpy(
			patch_final_code        + final_patch_ptr_counter,
			dl,
			len
		);
		final_patch_ptr_counter		      += len;
        }

	// now just add our region's data and then update the 'offs' field
	memcpy(
		patch_final_code + final_patch_ptr_counter,
		new_region_patch_data,
		new_region_patch_size
	);

	new_patch_region_header->offs			= __be32_to_cpu(final_patch_ptr_counter);

	// create a new file
	final_patch_fd					= open("final_patch.bin", O_RDWR | O_CREAT, 0777);
	write(final_patch_fd, patch_final_code,		MT_WIFI_PATCH_SIZE + sizeof(MT_PATCH_SEC) + new_region_patch_size);
	close(final_patch_fd);
	munmap(patch_final_code,			MT_WIFI_PATCH_SIZE + sizeof(MT_PATCH_SEC) + new_region_patch_size);
	munmap(new_region_patch_data,			new_region_patch_size);

	close(final_region_file_fd);
	return 0;
}
