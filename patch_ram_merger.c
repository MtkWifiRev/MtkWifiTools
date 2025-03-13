// given the rom patch and the ram, this tool will toggle the first region of the ram and it will append it into the last region of the ROM patch

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <endian.h>
#include <stdint.h>
#include <stdbool.h>

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
        #define MT_WIFI_PATCH_MIN_SIZE  256
#endif


#ifndef u8
        #define u8              unsigned char
#endif

#ifndef u32
        #define u32             uint32_t
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

#ifndef MT_RAM_WIFI_BYTES_TO_SKIP_FIRST_REGION
	#define MT_RAM_WIFI_BYTES_TO_SKIP_FIRST_REGION	0x64
#endif

#define MT_BUILD_DATE_LEN               16
#define MT_PLATFORM_LEN                 4

typedef struct MediatekRamRegionNode{
    	size_t 				length;
    	int 				flag;
	void			       *addr;
    	unsigned char 		       *data;
	struct MediatekRamRegionNode   *next;
}MediatekRamRegionNode;

typedef struct {
	MediatekRamRegionNode 	       *head;
	MediatekRamRegionNode	       *tail;
	size_t 				count;
}MediatekRamLinkedList;


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

#define MT_PATCH_HDR				struct mt76_connac2_patch_hdr
#define MT_PATCH_SEC				struct mt76_connac2_patch_sec
#define MT_RAM_HDR				struct mt76_connac2_fw_trailer
#define MT_RAM_SEC				struct mt76_connac2_fw_region


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

MediatekRamLinkedList *MtkWifiRamInitList(void){
	MediatekRamLinkedList *list 		= (MediatekRamLinkedList *)malloc(sizeof(MediatekRamLinkedList));
	if (list == NULL) {
		return NULL;
	}

    	list->head 				= NULL;
    	list->tail 				= NULL;
    	list->count 				= 0;
    	return list;
}

MediatekRamRegionNode *createNode(size_t length, int flag, void *addr){
	MediatekRamRegionNode *node 		= (MediatekRamRegionNode *)malloc(sizeof(MediatekRamRegionNode));

    	if (node == NULL) {
        	return NULL;
    	}

    	node->data 				= malloc(length);

    	if ( node->data == NULL ) {
    	    free(node);
    	    return NULL;
    	}

    	node->length 				= length;
    	node->flag 				= flag;
	node->addr				= addr;
    	node->next 				= NULL;
    	return node;
}

int addNode(MediatekRamLinkedList *list, void *addr, size_t length, int flag, const void *item) {

    if (list == NULL || item == NULL) {
        return -1;
    }

    MediatekRamRegionNode *node 		= createNode(length, flag, addr);

    if (node == NULL) {
        return -1;
    }

    memcpy(node->data, item, length);

    if (list->head == NULL) {
        list->head 				= node;
        list->tail 				= node;
    } else {
        list->tail->next 			= node;
        list->tail 				= node;
    }

    list->count++;
    return 0;
}

MediatekRamRegionNode *getNode(MediatekRamLinkedList *list, unsigned char index) {
	if (list == NULL || index >= list->count) {
        	return NULL;
    	}

    	MediatekRamRegionNode *current 		= list->head;

	for (size_t i = 0; i < index; i++) {
        	current 			= current->next;
    	}

    	return current;
}

bool verifyNode_ByAddr(MediatekRamLinkedList *list, void *addr){
        if ( list == NULL ) {
                return NULL;
        }

        MediatekRamRegionNode *current          = list->head;

        while ( current != NULL ) {
		if( current->addr == addr ){
			return true;
		}
                current                         = current->next;
        }

        return false;
}

MediatekRamRegionNode *getNode_ByAddr(MediatekRamLinkedList *list, void *addr) {
        if ( list == NULL ) {
                return NULL;
        }

        MediatekRamRegionNode *current          = list->head;

        while( current != NULL ) {
		if( current->addr	== addr ){
			return current;
		}
                current                         = current->next;
        }

        return NULL;
}

void printList(MediatekRamLinkedList *list) {
	if (list == NULL) {
        	printf("List is NULL\n");
        	return;
    	}

    	printf("the current list has %d elements\n", list->count);

    	MediatekRamRegionNode *current 		= list->head;
    	size_t index 				= 0;

    	while (current != NULL) {
        	printf("Node %zu: addr: 0x%x, length=%zu, flag=%d, first_bytes: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
			index,
			current->addr,
			current->length,
			current->flag,
			current->data[0],
			current->data[1],
			current->data[2],
			current->data[3],
			current->data[4],
			current->data[5]
		);
        	index++;
        	current 			= current->next;
    	}
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

int main(int argc, char *argv[]){
	const char 	*MT_WIFI_PATCH_NAME 	= NULL;
	const char      *MT_WIFI_RAM_NAME	= NULL;
	int		 MT_WIFI_PATCH_FD   	= 0x00;
	int		 MT_WIFI_PATCH_SIZE	= 0x00;
	int 		 MT_WIFI_RAM_FD		= 0x00;
	int		 MT_WIFI_RAM_SIZE	= 0x00;
	struct stat     *MT_WIFI_PATCH_STATS	= NULL;
	unsigned char   *MT_WIFI_PATCH_MMAP	= NULL;
	unsigned char   *MT_WIFI_PATCH_MMAP_TMP = NULL;
	unsigned char   *MT_WIFI_RAM_MMAP	= NULL;
	unsigned char   *MT_WIFI_RAM_MMAP_TMP	= NULL;
	MT_PATCH_HDR	*MT_WIFI_PATCH_HDR	= NULL;
        MT_RAM_HDR	*MT_WIFI_RAM_HDR	= NULL;

	unsigned char    MT_WIFI_RAM_FIRST_SEC	= 0x01;

	unsigned int	 offset_counter		= 0x00;
	unsigned int     final_rom_add_data	= 0x00;

	MediatekRamLinkedList *MT_WIFI_RAM_LL	= NULL;

	if( argc != 3 ){
		printf("%s <rom_patch> <ram_fw>\n", argv[0]);
		return -1;
	}

	for( unsigned char x = 0; x < 3; x++ ){
		if( strstr(argv[x], "_patch_mcu_") != NULL ){
			MT_WIFI_PATCH_NAME	= argv[x];
		}
		if( strstr(argv[x], "WIFI_RAM_CODE") != NULL ){
			MT_WIFI_RAM_NAME	= argv[x];
		}
	}

	if( MT_WIFI_PATCH_NAME == NULL ){
		printf("missing patch name!\n");
		return -1;
	}

	if( MT_WIFI_RAM_NAME == NULL ){
		printf("missing ram name!\n");
		return -1;
	}

	MT_WIFI_PATCH_FD 			= open(MT_WIFI_PATCH_NAME, O_RDONLY);
	if( MT_WIFI_PATCH_FD < 0 ){
		printf("missing patch file in the fs!\n");
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

	MT_WIFI_RAM_FD				= open(MT_WIFI_RAM_NAME, O_RDONLY);

	if( MT_WIFI_RAM_FD < 0 ){
                printf("missing ram fw file in the fs!\n");
                return -2;
	}

	memset(MT_WIFI_PATCH_STATS,	0x00,	 sizeof(struct stat));
	fstat(MT_WIFI_RAM_FD, MT_WIFI_PATCH_STATS);
        if( MT_WIFI_PATCH_STATS->st_size <= MT_WIFI_PATCH_MIN_SIZE ){
                printf("ram fw file is empty!\n");
		close(MT_WIFI_PATCH_FD);
                close(MT_WIFI_RAM_FD);
                return -3;
        }else{
                MT_WIFI_RAM_SIZE              = MT_WIFI_PATCH_STATS->st_size;
        }

	free(MT_WIFI_PATCH_STATS);

	MT_WIFI_RAM_LL				= MtkWifiRamInitList();
	// do a recap of the ram/rom size:

        printf("SELECTED RAM FIRMWARE NAME: %s        size: %d\n", MT_WIFI_RAM_NAME, MT_WIFI_RAM_SIZE);
        printf("SELECTED ROM PATCH NAME: %s    size: %d\n", MT_WIFI_PATCH_NAME, MT_WIFI_PATCH_SIZE);

        MT_WIFI_PATCH_MMAP                  	= (unsigned char *)mmap(
                                                                        NULL,
                                                                        MT_WIFI_PATCH_SIZE,
                                                                        PROT_READ,
                                                                        MAP_FILE  | MAP_SHARED,
                                                                        MT_WIFI_PATCH_FD,
                                                                        0x00
                                                );


        close(MT_WIFI_PATCH_FD);

	MT_WIFI_RAM_MMAP			= (unsigned char *)mmap(
									NULL,
									MT_WIFI_RAM_SIZE,
									PROT_READ,
									MAP_FILE   | MAP_SHARED,
									MT_WIFI_RAM_FD,
									0x00
						);

	close(MT_WIFI_RAM_FD);

	if( MT_WIFI_PATCH_MMAP == NULL || MT_WIFI_RAM_MMAP == NULL ){
		printf("error while MMAPing the patch || ram fw!\n");
		close(MT_WIFI_PATCH_FD);
		close(MT_WIFI_RAM_FD);
		exit(-1);
	}

	// start with the RAM

	MT_WIFI_RAM_HDR 				= (const void *)(MT_WIFI_RAM_MMAP + MT_WIFI_RAM_SIZE - sizeof(*MT_WIFI_RAM_HDR));

        printf("chip_id: 0x%x\neco_code: 0x%x\nn_region: %d\nfw_ver: %.10s\nbuild_time: %.15s\n",
                MT_WIFI_RAM_HDR->chip_id,
                MT_WIFI_RAM_HDR->eco_code,
                MT_WIFI_RAM_HDR->n_region,
                MT_WIFI_RAM_HDR->fw_ver,
                MT_WIFI_RAM_HDR->build_date
        );

	// fix: add the rom
	/**
	for(unsigned char j = 0; j < __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region); j++){
                MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;
                uint32_t len                            = 0;
                uint32_t addr                           = 0;
                uint32_t mode                           = 0;
                uint32_t sec_info                       = 0;
                uint8_t *dl                             = NULL;

                MT_WIFI_PATCH_CURR_SECT                 = (MT_PATCH_SEC *)(MT_WIFI_PATCH_MMAP + sizeof(*MT_WIFI_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT));

                if ( ( __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->type) & PATCH_SEC_TYPE_MASK) != PATCH_SEC_TYPE_INFO) {
                }

                addr                                    = 0x0                + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr);
                len                                     = 0x0                + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len);
                sec_info                                = 0x0                + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.sec_key_idx);
                dl                                      = MT_WIFI_PATCH_MMAP + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs);

		addNode(MT_WIFI_RAM_LL, addr, len, 0x00, dl);
	}
	**/


        for (int i = 0; i < MT_WIFI_RAM_HDR->n_region; i++) {
                MT_RAM_SEC *region		= NULL;
                uint32_t len			= 0x00;
		uint32_t addr			= 0x00;
		uint32_t mode			= 0x00;

                region 				= (const void *)((const u8 *)MT_WIFI_RAM_HDR - (MT_WIFI_RAM_HDR->n_region - i) * sizeof(*region));

                len     			= le32_to_cpu(region->len);
                addr    			= le32_to_cpu(region->addr);

                printf("addr: 0x%x len: %d\n", addr, len);

		if( addr != 0x00 ){
			if( MT_WIFI_RAM_FIRST_SEC == 1 ){
				// start copying only the first X bytes after the start of the region
				// allocate the linked list
				#ifdef LL_DEBUG
				printf("ram_ll: 0x%x, len: %d, ptr_start: 0x%x\n",
					MT_WIFI_RAM_LL,
					len - MT_RAM_WIFI_BYTES_TO_SKIP_FIRST_REGION,
					MT_WIFI_RAM_MMAP[MT_RAM_WIFI_BYTES_TO_SKIP_FIRST_REGION]);
				#endif

				if( addNode(MT_WIFI_RAM_LL, addr + MT_RAM_WIFI_BYTES_TO_SKIP_FIRST_REGION, len - MT_RAM_WIFI_BYTES_TO_SKIP_FIRST_REGION, 0x00, MT_WIFI_RAM_MMAP + MT_RAM_WIFI_BYTES_TO_SKIP_FIRST_REGION ) < 0 ){
					printf("addNode failed!\n");
				}
				offset_counter 		+= len; // - MT_RAM_WIFI_BYTES_TO_SKIP_FIRST_REGION;
				// set this to 0x00 now.
				MT_WIFI_RAM_FIRST_SEC	= 0x00;
			}else{
				if( addNode(MT_WIFI_RAM_LL, addr, len, 0x00, MT_WIFI_RAM_MMAP + offset_counter) < 0 ){
                	                printf("addNode failed!\n");
                	        }

				offset_counter 		+= len;
        		}
		}
	 }

	printf("LIST WITH THE WIFI RAM REGIONS ONLY:\n");
	printList(MT_WIFI_RAM_LL);

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

	// calculate the size of the WIFI RAM in the patch
	final_rom_add_data				= ( offset_counter - MT_RAM_WIFI_BYTES_TO_SKIP_FIRST_REGION ) + ( MT_WIFI_RAM_LL->count * sizeof(MT_PATCH_SEC) );
	printf("final data that will be added to the rom patch: %d\n", final_rom_add_data);

	// create the final mmapped area where every modification will be applied
	unsigned char j					= 0x00;
	MT_WIFI_PATCH_MMAP_TMP				= (unsigned char *)mmap(
										NULL,
										MT_WIFI_PATCH_SIZE,
										PROT_READ | PROT_WRITE,
										MAP_ANON  | MAP_PRIVATE,
										-1,
										0x00
							);

	memset(MT_WIFI_PATCH_MMAP_TMP,		      0x00,	MT_WIFI_PATCH_SIZE);
	memcpy(MT_WIFI_PATCH_MMAP_TMP,	MT_WIFI_PATCH_MMAP,	MT_WIFI_PATCH_SIZE);

        MT_PATCH_SEC *MtkWifiRegionList                 = (MT_PATCH_SEC *)calloc(__be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region) + MT_WIFI_RAM_LL->count, sizeof(MT_PATCH_SEC));

	MT_PATCH_HDR	*MT_WIFI_PATCH_HDR_TMP		= (MT_PATCH_HDR *)MT_WIFI_PATCH_MMAP_TMP;

	for(j = 0; j < __be32_to_cpu(MT_WIFI_PATCH_HDR_TMP->desc.n_region); j++){
                MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;

                MT_WIFI_PATCH_CURR_SECT                 = (MT_PATCH_SEC *)(MT_WIFI_PATCH_MMAP_TMP + sizeof(*MT_WIFI_PATCH_HDR_TMP) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT));
                MT_PATCH_SEC tmp_section                = { 0x00 };
                tmp_section.size                        = __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->size);
                tmp_section.info.addr                   = __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr);
                tmp_section.info.len                    = __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len);
                memcpy(&MtkWifiRegionList[j],           &tmp_section,        sizeof(*MT_WIFI_PATCH_CURR_SECT));

	}

        MediatekRamRegionNode *current                  = MT_WIFI_RAM_LL->head;
        size_t index                                    = 0x00;

        while (current != NULL) {
                // now convert the  node int a rom patch region
                MT_PATCH_SEC tmp_section                = { 0x00 };
                tmp_section.size                        = current->length;
                tmp_section.info.addr                   = current->addr;
                tmp_section.info.len                    = current->length;
                memcpy(&MtkWifiRegionList[j],           &tmp_section,   sizeof(MT_PATCH_SEC));
                j++;
                current                                 = current->next;
        }

	MT_WIFI_PATCH_HDR_TMP->desc.n_region		= __be32_to_cpu(MT_WIFI_PATCH_HDR_TMP->desc.n_region) + MT_WIFI_RAM_LL->count;

        for(j = 0; j < MT_WIFI_PATCH_HDR_TMP->desc.n_region; j++){
                printf("MtkWifiRegionList[%d] addr: 0x%x, len: %d, flag: 0x%x\n",
                        j,
                        MtkWifiRegionList[j].info.addr,
                        MtkWifiRegionList[j].info.len,
                        MtkWifiRegionList[j].type
                );
        }

	// start with the patch modification
	munmap(MT_WIFI_PATCH_MMAP_TMP,		MT_WIFI_PATCH_SIZE);

	MT_WIFI_PATCH_MMAP_TMP				= (unsigned char *)mmap(
										NULL,
										MT_WIFI_PATCH_SIZE + final_rom_add_data,
										PROT_READ   | PROT_WRITE,
										MAP_PRIVATE | MAP_ANON,
										-1,
										0x00
							);


	memset(MT_WIFI_PATCH_MMAP_TMP,		0x00,				MT_WIFI_PATCH_SIZE + final_rom_add_data);
	memcpy(MT_WIFI_PATCH_MMAP_TMP,		MT_WIFI_PATCH_MMAP,		sizeof(MT_PATCH_HDR));

	MT_PATCH_HDR	*FINAL_PATCH_HDR		= (MT_PATCH_HDR *)MT_WIFI_PATCH_MMAP_TMP;

	// increase the number of the 'n_region'
	FINAL_PATCH_HDR->desc.n_region			= FINAL_PATCH_HDR->desc.n_region + __be32_to_cpu(MT_WIFI_RAM_LL->count);

        printf("=================== FINAL PATCH =========================\n");
        printf("[WIFI_PATCH_HDR] build_date: %.15s\n",  FINAL_PATCH_HDR->build_date);
        printf("[WIFI_PATCH_HDR] platform:   %.4s\n",   FINAL_PATCH_HDR->platform);

        printf("[WIFI_PATCH_HDR] hw_sw_ver:  %x\n",     __be32_to_cpu(FINAL_PATCH_HDR->hw_sw_ver));
        printf("[WIFI_PATCH_HDR] patch_ver:  %x\n",     __be32_to_cpu(FINAL_PATCH_HDR->patch_ver));
        printf("[WIFI_PATCH_HDR] checksum:   %x\n",     __be32_to_cpu(FINAL_PATCH_HDR->checksum));
        printf("[WIFI_PATCH_HDR] rsv:        %x\n",     __be32_to_cpu(FINAL_PATCH_HDR->rsv));
        printf("[WIFI_PATCH_HDR] patch_ver:  %x\n",     __be32_to_cpu(FINAL_PATCH_HDR->desc.patch_ver));
        printf("[WIFI_PATCH_HDR] subsys:     %x\n",     __be32_to_cpu(FINAL_PATCH_HDR->desc.subsys));
        printf("[WIFI_PATCH_HDR] feature:    %x\n",     __be32_to_cpu(FINAL_PATCH_HDR->desc.feature));
        printf("[WIFI_PATCH_HDR] n_region:   %x\n",     __be32_to_cpu(FINAL_PATCH_HDR->desc.n_region));
        printf("[WIFI_PATCH_HDR] crc:        %x\n",     __be32_to_cpu(FINAL_PATCH_HDR->desc.crc));

        for(unsigned char j = 0; j < __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region); j++){
                MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;
                uint32_t addr                           = 0;
		uint32_t saved_len			= 0;
		uint32_t len				= 0;
                uint32_t mode                           = 0;
                uint32_t sec_info                       = 0;
                uint8_t *dl                             = NULL;

                MT_WIFI_PATCH_CURR_SECT                 = (MT_PATCH_SEC *)(MT_WIFI_PATCH_MMAP + sizeof(*MT_WIFI_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT));

                if ( ( __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->type) & PATCH_SEC_TYPE_MASK) != PATCH_SEC_TYPE_INFO) {
                        /** do nothing **/
                }

                addr                                    = 0x0                + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr);
                len                                     = 0x0                + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len);
                sec_info                                = 0x0                + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.sec_key_idx);
                dl                                      = MT_WIFI_PATCH_MMAP + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs);
		saved_len				= len;

		// before copying, resize the rom patch region if this overlaps with the RAM code.
        	MediatekRamRegionNode *current          = MT_WIFI_RAM_LL->head;

        	while ( current != NULL ) {
			if( addr <= current->addr && ( addr + len ) >= current->addr ){
				len				= (int)current->addr - (int)addr - MT_RAM_WIFI_BYTES_TO_SKIP_FIRST_REGION;
				printf("new len in linked list is %d with addr 0x%x\n", len, addr);
				// do the same for the 'MtkWifiRegionList' array
				for(unsigned char k = 0; MtkWifiRegionList[k].info.addr != NULL; k++ ){
					if( MtkWifiRegionList[k].info.addr == addr ){
						MtkWifiRegionList[k].info.len		= len;
						MtkWifiRegionList[k].size		= len;
					}
				}
				break;
			}
                	current                         = current->next;
        	}

		current					= MT_WIFI_RAM_LL->head;


		addNode(MT_WIFI_RAM_LL, addr, len, 0x00, dl);
	}

        printf("LIST WITH THE WIFI RAM REGIONS + THE ROM PATCH:\n");
        printList(MT_WIFI_RAM_LL);

        unsigned int final_rom_patch_header_total       = 0x00;
        final_rom_patch_header_total                    = sizeof(MT_PATCH_HDR) + __be32_to_cpu(FINAL_PATCH_HDR->desc.n_region) * sizeof(MT_PATCH_SEC);
	printf("final_rom_patch_header_total = %d\n", final_rom_patch_header_total);
        void        *final_rom_patch_start_area_ptr     = MT_WIFI_PATCH_MMAP_TMP + final_rom_patch_header_total;


        for(unsigned char j = 0; j < __be32_to_cpu(FINAL_PATCH_HDR->desc.n_region); j++){
                MT_PATCH_SEC 	*MT_WIFI_PATCH_CURR_SECT   	= NULL;
		unsigned char 	*first_bytes_regions		= NULL;

                MT_WIFI_PATCH_CURR_SECT                 	= (MT_PATCH_SEC *)(MT_WIFI_PATCH_MMAP_TMP + sizeof(*FINAL_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT));

                //MT_WIFI_PATCH_CURR_SECT->info.len       	= __be32_to_cpu(saved_len);

		printf("[PRETEST] addr: 0x%x len: %d ",
			MtkWifiRegionList[j].info.addr,
			MtkWifiRegionList[j].size
		);

		if ( verifyNode_ByAddr(MT_WIFI_RAM_LL, MtkWifiRegionList[j].info.addr) == true ){
                	MediatekRamRegionNode *AddrNode          = getNode_ByAddr(MT_WIFI_RAM_LL, MtkWifiRegionList[j].info.addr);
			if( AddrNode != NULL && AddrNode->data != NULL){
				printf("first byte of the data: %x %x %x %x\n", AddrNode->data[0], AddrNode->data[1], AddrNode->data[2], AddrNode->data[4]);
			}else{
				printf("AddrNode or AddrNode->data is NULL!\n");
			}
		}else{
			printf("address 0x%x not found in the linked list\n", MtkWifiRegionList[j].info.addr);
		}

		memcpy(MT_WIFI_PATCH_CURR_SECT, 	&MtkWifiRegionList[j], sizeof(MT_PATCH_SEC));
	}

        for(unsigned char j = 0; j < __be32_to_cpu(FINAL_PATCH_HDR->desc.n_region); j++){
                MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;
		uint32_t addr				= 0x00;
                uint32_t len                            = 0x00;
        	uint32_t mode				= 0x00;
		unsigned char *dl			= NULL;

		MediatekRamRegionNode *AddrNode		= NULL;

                MT_WIFI_PATCH_CURR_SECT                 = (MT_PATCH_SEC *)(MT_WIFI_PATCH_MMAP_TMP + sizeof(*FINAL_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT));

                addr                                    = 0x0                + (MT_WIFI_PATCH_CURR_SECT->info.addr);

                MtkWifiRegionList[j].offs               = final_rom_patch_header_total;

                len                                     = 0x0 + MtkWifiRegionList[j].info.len;

		final_rom_patch_header_total	       += len;

		dl					= MT_WIFI_PATCH_MMAP_TMP + MtkWifiRegionList[j].offs;

		printf("addr: 0x%x len: %d\n", MT_WIFI_PATCH_CURR_SECT->info.addr, MT_WIFI_PATCH_CURR_SECT->info.len);

                if( verifyNode_ByAddr(MT_WIFI_RAM_LL, addr) == true ){
                        printf("[region %d] loading from the linked list!\n", j);
			AddrNode 			= getNode_ByAddr(MT_WIFI_RAM_LL, addr);
			if( AddrNode != NULL ){
				memcpy(dl, AddrNode->data, AddrNode->length);
			}else{
                        	printf("ADDRNODE IS NULL!\n");
			}
                }else{
			printf("addr 0x%x not found\n", addr);
		}

		printf("first bytes of region %d: %x %x %x %x\n", j, AddrNode->data[0], AddrNode->data[1], AddrNode->data[2], AddrNode->data[3]);

		//memcpy(MT_WIFI_PATCH_MMAP_TMP   + final_rom_patch_header_total, MediatekRegionN->data, len);
	}

        for(unsigned char j = 0; j < __be32_to_cpu(FINAL_PATCH_HDR->desc.n_region); j++){
                MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;
                uint32_t len                            = 0;
                uint32_t addr                           = 0;
                uint32_t mode                           = 0;
                uint32_t sec_info                       = 0;
                uint8_t *dl                             = NULL;

                MT_WIFI_PATCH_CURR_SECT                 = (MT_PATCH_SEC *)(MT_WIFI_PATCH_MMAP_TMP + sizeof(*FINAL_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT));

                if ( ( __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->type) & PATCH_SEC_TYPE_MASK) != PATCH_SEC_TYPE_INFO) {
                        /** do nothing **/
                }

                addr                                    = 0x0                	 + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr);
                len                                     = 0x0                	 + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len);
                sec_info                                = 0x0                	 + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.sec_key_idx);
                dl                                      = MT_WIFI_PATCH_MMAP_TMP + __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs);

		/**
		if( verifyNode_ByAddr(MT_WIFI_RAM_LL, addr) == true ){
			printf("[region %d] loading from the linked list!\n");
			dl				=
		}else{
			memcpy(
		}
		**/

		MT_WIFI_PATCH_CURR_SECT->offs		= cpu_to_be32(MT_WIFI_PATCH_CURR_SECT->offs);
		MT_WIFI_PATCH_CURR_SECT->size		= cpu_to_be32(MT_WIFI_PATCH_CURR_SECT->size);
		MT_WIFI_PATCH_CURR_SECT->info.addr	= cpu_to_be32(MT_WIFI_PATCH_CURR_SECT->info.addr);
		MT_WIFI_PATCH_CURR_SECT->info.len	= cpu_to_be32(MT_WIFI_PATCH_CURR_SECT->info.len);

                printf("[FINAL section number %d] offset: %d, size: %d, address: 0x%x, len: %d\n",
                        j,
                        be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs),
                        be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->size),
                        be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr),
                        be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len)
                );

                mode                                    = mt76_connac2_get_data_mode(j, sec_info);

        }

	for(unsigned char j = 0; j < __be32_to_cpu(FINAL_PATCH_HDR->desc.n_region); j++){
                MT_PATCH_SEC    *MT_WIFI_PATCH_CURR_SECT        = NULL;
                unsigned char   *first_bytes_regions            = NULL;

                MT_WIFI_PATCH_CURR_SECT                         = (MT_PATCH_SEC *)(MT_WIFI_PATCH_MMAP_TMP + sizeof(*FINAL_PATCH_HDR) + j * sizeof(*MT_WIFI_PATCH_CURR_SECT));
		MT_WIFI_PATCH_CURR_SECT->size			= cpu_to_be32(MtkWifiRegionList[j].size);
		MT_WIFI_PATCH_CURR_SECT->info.addr		= cpu_to_be32(MtkWifiRegionList[j].info.addr);
		MT_WIFI_PATCH_CURR_SECT->info.len		= cpu_to_be32(MtkWifiRegionList[j].info.len);
		MT_WIFI_PATCH_CURR_SECT->offs			= cpu_to_be32(MtkWifiRegionList[j].offs);

		printf("region %d offset: 0x%x\n", 		j, cpu_to_be32(MtkWifiRegionList[j].offs));
                //memcpy(MT_WIFI_PATCH_CURR_SECT,         	MtkWifiRegionList[j], sizeof(MT_PATCH_SEC));
	}

	// rewrite the rom_patch
	int final_rom_patch_fd				= open("final_rom_patch.bin", O_CREAT | O_RDWR, 0777);
	write(final_rom_patch_fd, MT_WIFI_PATCH_MMAP_TMP, MT_WIFI_PATCH_SIZE + final_rom_add_data);

	close(final_rom_patch_fd);
	// for finishing, strip the wifi ram code and toggle most of its content.

	return 0;
}

