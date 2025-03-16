/**
	Patch Incorporator -- a bypass solution for modifying Mediatek's WIFI RAM firmware by abusing the ROM patch mechanism
		Made by Edoardo Mantovani, 2025
**/

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

/** some code is copied from the mt76 kernel driver, define the types here **/


#ifndef u8
        #define u8              uint8_t
#endif

#ifndef u16
	#define u16		uint16_t
#endif

#ifndef u32
        #define u32             uint32_t
#endif

/** thse MACRO are really important for the ROM and RAM reading **/

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

#define MT_BUILD_DATE_LEN               16
#define MT_PLATFORM_LEN                 4
#define MT_PATCH_SEC_SPEC_LEN           13

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

#define FW_FEATURE_SET_ENCRYPT                  BIT(0)
#define FW_FEATURE_SET_KEY_IDX                  GENMASK(2, 1)
#define FW_FEATURE_ENCRY_MODE                   BIT(4)
#define FW_FEATURE_OVERRIDE_ADDR                BIT(5)
#define FW_FEATURE_NON_DL                       BIT(6)

#define DL_MODE_ENCRYPT                         BIT(0)
#define DL_MODE_KEY_IDX                         GENMASK(2, 1)
#define DL_MODE_RESET_SEC_IV                    BIT(3)
#define DL_MODE_WORKING_PDA_CR4                 BIT(4)
#define DL_MODE_VALID_RAM_ENTRY                 BIT(5)
#define DL_CONFIG_ENCRY_MODE_SEL                BIT(6)
#define DL_MODE_NEED_RSP                        BIT(31)

#define FW_START_OVERRIDE                       BIT(0)
#define FW_START_WORKING_PDA_CR4                BIT(2)
#define FW_START_WORKING_PDA_DSP                BIT(3)

#define PATCH_SEC_NOT_SUPPORT                   GENMASK(31, 0)
#define PATCH_SEC_TYPE_MASK                     GENMASK(15, 0)
#define PATCH_SEC_TYPE_INFO                     0x2

#define PATCH_SEC_ENC_TYPE_MASK                 GENMASK(31, 24)
#define PATCH_SEC_ENC_TYPE_PLAIN                0x00
#define PATCH_SEC_ENC_TYPE_AES                  0x01
#define PATCH_SEC_ENC_TYPE_SCRAMBLE             0x02
#define PATCH_SEC_ENC_SCRAMBLE_INFO_MASK        GENMASK(15, 0)
#define PATCH_SEC_ENC_AES_KEY_MASK              GENMASK(7, 0)

#ifndef GENMASK
        #define GENMASK(h, l)                   __GENMASK(h, l)
#endif

#define __GENMASK(h, l) (((~_UL(0)) - (_UL(1) << (l)) + 1) & (~_UL(0) >> (__BITS_PER_LONG - 1 - (h))))

#define UL(x)           (_UL(x))
#define ULL(x)          (_ULL(x))
#define BIT(nr)         ((unsigned long)(1) << (nr))
#define __bf_shf(x)     (ffsll(x) - 1)

#define FIELD_PREP(_mask, _val)                                         \
        ({                                                              \
                ((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask);   \
        })

#define FIELD_GET(_mask, _reg)                                          \
        ({                                                              \
                (typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask)); \
        })


#define MT_PATCH_HDR                            struct mt76_connac2_patch_hdr
#define MT_PATCH_SEC                            struct mt76_connac2_patch_sec
#define MT_RAM_HDR                              struct mt76_connac2_fw_trailer
#define MT_RAM_SEC                              struct mt76_connac2_fw_region

/** this is the minimun size you can upload for a RAM region **/
#ifndef MT_WIFI_PATCH_MIN_SIZE
        #define MT_WIFI_PATCH_MIN_SIZE  256
#endif

#ifndef MT_WIFI_RAM_CRC_SIZE
	#define MT_WIFI_RAM_CRC_SIZE	sizeof(uint32_t)
#endif

// hardcode here for now the precomputed CRC of the various first 256 bytes WIFI RAM CODE

enum {
        MT7921_PCIE                     = 0x00,
        MT7921_USB                      = 0x01,
        MT7915_PCIE                     = 0x02,
        MT7981_PCIE                     = 0x03,
        MT7986_PCIE                     = 0x04,
        MT7922_PCIE                     = 0x05,
        MT7902_PCIE                     = 0x06,
        MT7925_PCIE                     = 0x07,
        MT7925_USB                      = 0x08,
        MT7990_PCIE                     = 0x09,

        MEDIATEK_MAX_CARD_NUMBER
};

struct MediatekFirst256BytesPreComputedCRC{
        unsigned char                   MediatekWifiFirmwareBuildTime[15];		           /**     use both the build time and              	**/
        unsigned char                   MediatekWifiFirmwareSwHw[10];                              /**     the sw/hw code for identifying a card fw 	**/
        uint32_t                        MediatekWifiFirmwareCRC32;                                 /** 	precomputed crc32			 	**/
        unsigned char                   MediatekBypassIsTested;                                    /** 	boolean value, if is set to '0', means that this specific card
													hasn't been tested			 	**/
}PRECOMPUTED_CRC_TABLE[MEDIATEK_MAX_CARD_NUMBER] = {
	[MT7921_USB] = {
		.MediatekWifiFirmwareBuildTime	= "20240826151030",
		.MediatekWifiFirmwareSwHw	= "____010000",
		.MediatekBypassIsTested		= true,
	},


};

typedef struct MediatekRamRegionNode{
        uint32_t                        length;
	uint16_t			new_length;		/** used for the new size of the overlapped RAM code **/
        int                             flag;
        void                           	*addr;
        unsigned char                  	*data;
	unsigned char			is_rom_patch;
        struct MediatekRamRegionNode   	*next;
}MediatekRamRegionNode;

typedef struct {
        MediatekRamRegionNode          *head;
        MediatekRamRegionNode          *tail;
        unsigned char                   count;
}MediatekRamLinkedList;

MediatekRamLinkedList *MtkWifiRamInitList(void){
        MediatekRamLinkedList *list             = (MediatekRamLinkedList *)malloc(sizeof(MediatekRamLinkedList));
        if (list == NULL) {
                return NULL;
        }

        list->head                              = NULL;
        list->tail                              = NULL;
        list->count                             = 0;
        return list;
}

MediatekRamRegionNode *createNode(size_t length, int flag, void *addr, unsigned char is_rom_patch){

        MediatekRamRegionNode *node             = (MediatekRamRegionNode *)malloc(sizeof(MediatekRamRegionNode));

        if (node == NULL) {
                return NULL;
        }

        node->data                              = malloc(length);

        if ( node->data == NULL ) {
            free(node);
            return NULL;
        }

        node->length                            = length;
        node->flag                              = flag;
        node->addr                              = addr;
	node->is_rom_patch			= is_rom_patch;
        node->next                              = NULL;
        return node;
}

int addNode(MediatekRamLinkedList *list, void *addr, size_t length, int flag, const void *item, unsigned char is_rom_patch) {

    if (list == NULL || item == NULL) {
        return -1;
    }

    MediatekRamRegionNode *node                 = createNode(length, flag, addr, is_rom_patch);

    if (node == NULL) {
        return -1;
    }

    memcpy(node->data, item, length);

    if (list->head == NULL) {
        list->head                              = node;
        list->tail                              = node;
    } else {
        list->tail->next                        = node;
        list->tail                              = node;
    }

    list->count++;
    return 0;
}

MediatekRamRegionNode *getNode(MediatekRamLinkedList *list, unsigned char index) {
        if (list == NULL || index >= list->count) {
                return NULL;
        }

        MediatekRamRegionNode *current          = list->head;

        for (size_t i = 0; i < index; i++) {
                current                         = current->next;
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
                if( current->addr       == addr ){
                        return current;
                }
                current                         = current->next;
        }

        return NULL;
}

void sortLinkedListByAddr(MediatekRamLinkedList *list) {
    if (list == NULL || list->head == NULL || list->head->next == NULL) {
        // List is empty or has only one element, no need to sort
        return;
    }

    MediatekRamRegionNode *sorted = NULL;
    MediatekRamRegionNode *current = list->head;
    MediatekRamRegionNode *temp;

    // Remove all nodes from original list and insert into sorted list
    while (current != NULL) {
        // Save the next node
        temp = current->next;

        // Insert current node in sorted linked list
        if (sorted == NULL || sorted->addr > current->addr) {
            // Insert at the beginning
            current->next = sorted;
            sorted = current;
        } else if (sorted->addr == current->addr) {
            // Same address: prioritize nodes with is_rom_patch=1
            if (current->is_rom_patch == 1 && sorted->is_rom_patch == 1) {
                // Put current before sorted (at the beginning)
                current->next = sorted;
                sorted = current;
            } else {
                // Insert after the first node
                current->next = sorted->next;
                sorted->next = current;
            }
        } else {
            // Find the right position to insert
            MediatekRamRegionNode *search = sorted;
            while (search->next != NULL && 
                  (search->next->addr < current->addr || 
                  (search->next->addr == current->addr && 
                   current->is_rom_patch == 0 && search->next->is_rom_patch == 1))) {
                search = search->next;
            }
            // Insert after search
            current->next = search->next;
            search->next = current;
        }

        // Move to next node
        current = temp;
    }

    // Update the list with the sorted nodes
    list->head = sorted;
    
    // Update the tail pointer
    current = sorted;
    while (current != NULL && current->next != NULL) {
        current = current->next;
    }
    list->tail = current;
}
void printList(MediatekRamLinkedList *list) {
        if (list == NULL) {
                printf("List is NULL\n");
                return;
        }

        printf("the current list has %d elements\n", list->count);

        MediatekRamRegionNode *current          = list->head;
        unsigned int index                      = 0;

        while (current != NULL) {
                printf("Node %d: addr: %p, length=%d, flag=%d, first_bytes: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
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
                current                         = current->next;
        }
}

static uint32_t mt76_connac2_get_data_mode(unsigned char section_number, uint32_t info){
        uint32_t mode                           = DL_MODE_NEED_RSP;

        switch (FIELD_GET(PATCH_SEC_ENC_TYPE_MASK, info)) {
                case PATCH_SEC_ENC_TYPE_PLAIN:
                        printf("[%s][sec %d] patch is in plaintext!\n", __FUNCTION__, section_number);
                                /** do nothing, this is a plaintext patch **/
                        break;
                case PATCH_SEC_ENC_TYPE_AES:
                                printf("[%s][sec %d] patch is in AES encrypted mode!\n", __FUNCTION__, section_number);
                                mode                   |= DL_MODE_ENCRYPT;
                                mode                   |= FIELD_PREP(DL_MODE_KEY_IDX, (info & PATCH_SEC_ENC_AES_KEY_MASK)) & DL_MODE_KEY_IDX;
                                mode                   |= DL_MODE_RESET_SEC_IV;
                        break;
                case PATCH_SEC_ENC_TYPE_SCRAMBLE:
                                printf("[%s][sec %d] patch is in simple scrambling mode!\n", __FUNCTION__, section_number);
                                mode                   |= DL_MODE_ENCRYPT;
                                mode                   |= DL_CONFIG_ENCRY_MODE_SEL;
                                mode                   |= DL_MODE_RESET_SEC_IV;
                        break;
                default:
                        printf("[%s][sec %d] Encryption type not supported!\n", __FUNCTION__, section_number);
        }

        return mode;
}

unsigned char *mt_wifi_id_to_fw(unsigned char fw_id){
	switch(fw_id){
		case MT7921_PCIE:
			return "mediatek 7921 for PCIe bus";
		break;
		case MT7921_USB:
			return "mediatek 7921 for USB bus";
		break;
		case MT7915_PCIE:
			return "mediatek 7915 for PCIe bus";
		break;
		case MT7981_PCIE:
			return "mediatek 7981 for PCIe bus";
		break;
		case MT7986_PCIE:
			return "mediatek 7986 for PCIe bus";
		break;
		case MT7922_PCIE:
			return "mediatek 7922 for PCIe bus";
		break;
		case MT7902_PCIE:
			return "mediatek 7902 for PCIe bus";
		break;
		case MT7925_PCIE:
			return "mediatek 7925 for PCIe bus";
		break;
		case MT7925_USB:
			return "mediatek 7925 for USB bus";
		break;
		case MT7990_PCIE:
			return "mediatek 7990 for PCIe bus";
		break;
	}
}

signed char check_fw(MT_RAM_HDR *MT_WIFI_RAM_HDR){
	for( unsigned char j = 0; j < MEDIATEK_MAX_CARD_NUMBER; j++){
		if( PRECOMPUTED_CRC_TABLE[j].MediatekWifiFirmwareBuildTime != NULL ){
			if(
				strncmp(MT_WIFI_RAM_HDR->build_date, 	PRECOMPUTED_CRC_TABLE[j].MediatekWifiFirmwareBuildTime, 15) == 0 &&
				strncmp(MT_WIFI_RAM_HDR->fw_ver,	PRECOMPUTED_CRC_TABLE[j].MediatekWifiFirmwareSwHw,	10) == 0
			){
				printf("[CHECK_FW] the current firmware is: %s\n", mt_wifi_id_to_fw(j));
				if( PRECOMPUTED_CRC_TABLE[j].MediatekBypassIsTested == 0x00 ){
					printf("[WARNING] the current firmware bypass has not been tested!\n");
				}
				return j;
			}
		}
	}
	return -1;
}

int main(int argc, char *argv[]){
	const char      *MT_WIFI_PATCH_NAME     	= NULL;
        const char      *MT_WIFI_RAM_NAME       	= NULL;
        int              MT_WIFI_PATCH_FD       	= 0x00;
        int              MT_WIFI_PATCH_SIZE     	= 0x00;
        int              MT_WIFI_RAM_FD         	= 0x00;
        int              MT_WIFI_RAM_SIZE       	= 0x00;
	int		 MT_WIFI_PATCH_FINAL_SIZE	= 0x00;
        struct stat     *MT_WIFI_PATCH_STATS    	= NULL;
        unsigned char   *MT_WIFI_PATCH_MMAP     	= NULL;
        unsigned char   *MT_WIFI_PATCH_MMAP_FINAL 	= NULL;
        unsigned char   *MT_WIFI_RAM_MMAP       	= NULL;
        unsigned char   *MT_WIFI_RAM_MMAP_FINAL   	= NULL;
        MT_PATCH_HDR    *MT_WIFI_PATCH_HDR      	= NULL;
        MT_RAM_HDR      *MT_WIFI_RAM_HDR        	= NULL;

        unsigned char    MT_WIFI_RAM_FIRST_SEC  	= 0x01;
	unsigned char    overlap_done			= 0x00;

        unsigned int     offset_counter         	= 0x00;
        unsigned int     final_rom_add_data     	= 0x00;

	int		 final_ram_fw_fd		= 0x00;
	int		 final_rom_fw_fd		= 0x00;

        MediatekRamLinkedList *MT_WIFI_RAM_LL   	= NULL;

        if( argc != 3 ){
                printf("%s <rom_patch> <ram_fw>\n", argv[0]);
                return -1;
        }

	/** get the ROM and RAM firmware files based on their names, for now supports only 1 ROM and 1 RAM file **/
        for( unsigned char x = 0; x < 3; x++ ){
                if( strstr(argv[x], "_patch_mcu_") != NULL ){
                        MT_WIFI_PATCH_NAME      	= argv[x];
                }
                if( strstr(argv[x], "WIFI_RAM_CODE") != NULL ){
                        MT_WIFI_RAM_NAME        	= argv[x];
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

	/** open and get the data for both files **/
        MT_WIFI_PATCH_FD                        	= open(MT_WIFI_PATCH_NAME, O_RDONLY);
        if( MT_WIFI_PATCH_FD < 0 ){
                printf("missing patch file in the fs!\n");
                return -2;
        }

        MT_WIFI_PATCH_STATS                     	= (struct stat *)malloc(sizeof(struct stat));
        fstat(MT_WIFI_PATCH_FD, MT_WIFI_PATCH_STATS);

        if( MT_WIFI_PATCH_STATS->st_size <= MT_WIFI_PATCH_MIN_SIZE ){
                printf("patch file is empty!\n");
                close(MT_WIFI_PATCH_FD);
                return -3;
        }else{
                MT_WIFI_PATCH_SIZE              	= MT_WIFI_PATCH_STATS->st_size;
        }

        MT_WIFI_RAM_FD                          	= open(MT_WIFI_RAM_NAME, O_RDONLY);

        if( MT_WIFI_RAM_FD < 0 ){
                printf("missing ram fw file in the fs!\n");
                return -2;
        }

        memset(MT_WIFI_PATCH_STATS,     0x00,    	sizeof(struct stat));
        fstat(MT_WIFI_RAM_FD, MT_WIFI_PATCH_STATS);

        if( MT_WIFI_PATCH_STATS->st_size <= MT_WIFI_PATCH_MIN_SIZE ){
                printf("ram fw file is empty!\n");
                close(MT_WIFI_PATCH_FD);
                close(MT_WIFI_RAM_FD);
                return -3;
        }else{
                MT_WIFI_RAM_SIZE              		= MT_WIFI_PATCH_STATS->st_size;
        }

        free(MT_WIFI_PATCH_STATS);

        MT_WIFI_RAM_LL                          	= MtkWifiRamInitList();

	MT_WIFI_PATCH_MMAP                      	= (unsigned char *)mmap(
                                                                        NULL,
                                                                        MT_WIFI_PATCH_SIZE,
                                                                        PROT_READ,
                                                                        MAP_FILE  | MAP_SHARED,
                                                                        MT_WIFI_PATCH_FD,
                                                                        0x00
                                                	);


        close(MT_WIFI_PATCH_FD);

        MT_WIFI_RAM_MMAP                        	= (unsigned char *)mmap(
                                                                        NULL,
                                                                        MT_WIFI_RAM_SIZE,
                                                                        PROT_READ,
                                                                        MAP_FILE   | MAP_SHARED,
                                                                        MT_WIFI_RAM_FD,
                                                                        0x00
                                                	);

        close(MT_WIFI_RAM_FD);

        // do a recap of the ram/rom size:

        printf("SELECTED RAM FIRMWARE NAME: %s        size: %d mmap area: %p\n", MT_WIFI_RAM_NAME, MT_WIFI_RAM_SIZE, MT_WIFI_RAM_MMAP);
        printf("SELECTED ROM PATCH NAME: %s    size: %d mmap area: %p\n", MT_WIFI_PATCH_NAME, MT_WIFI_PATCH_SIZE, MT_WIFI_PATCH_MMAP);

        MT_WIFI_RAM_HDR                                 = (void *)(MT_WIFI_RAM_MMAP + MT_WIFI_RAM_SIZE - sizeof(*MT_WIFI_RAM_HDR));

	if( check_fw(MT_WIFI_RAM_HDR) < 0 ){
		printf("[!] failed to find a suitable HW for this firmware! aborting..\n");
		goto RELEASE_FIRST_STAGE;
	}

	printf("=================== ORIGINAL FW    =========================\n");
	printf("[WIFI_RAM_HDR] fw_ver:     %.10s\n",	MT_WIFI_RAM_HDR->fw_ver);
	printf("[WIFI_RAM_HDR] build_date: %.15s\n",	MT_WIFI_RAM_HDR->build_date);

	printf("[WIFI_RAM_HDR] chip_id: 0x%x\n", 	MT_WIFI_RAM_HDR->chip_id);
	printf("[WIFI_RAM_HDR] eco_code: 0x%x\n",	MT_WIFI_RAM_HDR->eco_code);
	printf("[WIFI_RAM_HDR] n_region: %d\n",		MT_WIFI_RAM_HDR->n_region);
	printf("[WIFI_RAM_HDR] format_ver: %x\n",	MT_WIFI_RAM_HDR->format_ver);
	printf("[WIFI_RAM_HDR] crc: 0x%x\n",		MT_WIFI_RAM_HDR->crc);

        MT_WIFI_PATCH_HDR                               = (MT_PATCH_HDR *)MT_WIFI_PATCH_MMAP;

        printf("=================== ORIGINAL PATCH =========================\n");
        printf("[WIFI_PATCH_HDR] build_date: %.15s\n",  MT_WIFI_PATCH_HDR->build_date);
        printf("[WIFI_PATCH_HDR] platform:   %.4s\n",   MT_WIFI_PATCH_HDR->platform);

        printf("[WIFI_PATCH_HDR] hw_sw_ver:  %x\n",     __be32_to_cpu(MT_WIFI_PATCH_HDR->hw_sw_ver));
        printf("[WIFI_PATCH_HDR] patch_ver:  %x\n",     __be32_to_cpu(MT_WIFI_PATCH_HDR->patch_ver));
        printf("[WIFI_PATCH_HDR] checksum:   %x\n",     __be32_to_cpu(MT_WIFI_PATCH_HDR->checksum));
        printf("[WIFI_PATCH_HDR] rsv:        %x\n",     __be32_to_cpu(MT_WIFI_PATCH_HDR->rsv));
        printf("[WIFI_PATCH_HDR] patch_ver:  %x\n",     __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.patch_ver));
        printf("[WIFI_PATCH_HDR] subsys:     %x\n",     __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.subsys));
        printf("[WIFI_PATCH_HDR] feature:    %x\n",     __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.feature));
        printf("[WIFI_PATCH_HDR] n_region:   %x\n",     __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region));
        printf("[WIFI_PATCH_HDR] crc:        %x\n",     __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.crc));

	/** start by reading the RAM firmware **/
        for (unsigned char i = 0; i < MT_WIFI_RAM_HDR->n_region; i++) {
                MT_RAM_SEC *region              	= NULL;
                uint32_t len                    	= 0x00;
                uint32_t addr                   	= 0x00;
                uint32_t mode                   	= 0x00;

                region                          	= (void *)((u8 *)MT_WIFI_RAM_HDR - (MT_WIFI_RAM_HDR->n_region - i) * sizeof(*region));

                len                             	= le32_to_cpu(region->len);
                addr                            	= le32_to_cpu(region->addr);

                if( addr != 0x00 ){
                	if( addNode(MT_WIFI_RAM_LL, (void *)addr, len, 0x00, MT_WIFI_RAM_MMAP + offset_counter, false) < 0 ){
                        	// need to add the proper error handling code
                        }
	                offset_counter                 += len;
			MT_WIFI_PATCH_FINAL_SIZE       += len;
                }
        }


	for(unsigned char j = 0; j < __be32_to_cpu(MT_WIFI_PATCH_HDR->desc.n_region); j++){
                MT_PATCH_SEC *MT_WIFI_PATCH_CURR_SECT   = NULL;
                uint32_t len                            = 0;
                uint32_t addr                           = 0;
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

                printf("[section number %d] offset: 0x%x, size: %d, address: 0x%x, len: %d\n",
                        j,
                        __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->offs),
                        __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->size),
                        __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.addr),
                        __be32_to_cpu(MT_WIFI_PATCH_CURR_SECT->info.len)
                );

                mode                                    = mt76_connac2_get_data_mode(j, sec_info);

                /** now recalibrate the size of the RAM region which is overlapped with the ROM patch **/
		{

			MediatekRamRegionNode *LHead    = MT_WIFI_RAM_LL->head;

        	        while ( LHead != NULL ) {
                	        if( addr <= (uint32_t)LHead->addr && ( addr + len ) >= (uint32_t)LHead->addr ){
					/** now we can modify the RAM region's size, add also the 4 bytes of the CRC size **/
					LHead->new_length	= (uint16_t)( addr + len - (uint32_t)LHead->addr ); // + MT_WIFI_RAM_CRC_SIZE;
					LHead->length	       -= LHead->new_length;
					/** modify the address too for avoiding to overwrite the other ROM memory region, this would cause a mcu's crash **/
					LHead->addr	       += LHead->new_length;
					#ifdef DEBUG
                                        printf("new len is %d, old len is: %d, address is: 0x%x\n", LHead->new_length, LHead->length, LHead->addr);
					#endif
					break;
				}
			}
		}

		/** add the node in the list **/
                addNode(MT_WIFI_RAM_LL, (void *)addr, len, 0x00, dl, true);
	}

	/** print the final layout of the memory **/
        printList(MT_WIFI_RAM_LL);

	/** final step: now that we have the complete list, we need to rewrite the ROM and RAM file **/

	MT_WIFI_PATCH_FINAL_SIZE		       += 	MT_WIFI_RAM_LL->count * sizeof(MT_RAM_SEC);
	MT_WIFI_PATCH_FINAL_SIZE		       += 	MT_WIFI_PATCH_SIZE;

	printf("[rom_fw] the final size for the ROM patch is: %d\n", MT_WIFI_PATCH_FINAL_SIZE);

	/** the ROM file is the most 'complex' **/

	MT_WIFI_PATCH_MMAP_FINAL			=	(unsigned char *)mmap(
									NULL,
									MT_WIFI_PATCH_FINAL_SIZE,
									PROT_READ | PROT_WRITE,
									MAP_ANON  | MAP_PRIVATE,
									-1,
									0x00
							);

	memset(MT_WIFI_PATCH_MMAP_FINAL,		0x00,			MT_WIFI_PATCH_FINAL_SIZE);
	/** copy the header **/
	memcpy(MT_WIFI_PATCH_MMAP_FINAL, 		MT_WIFI_PATCH_HDR,	sizeof(MT_PATCH_HDR));
	/** reuse 'offset_counter' **/
	offset_counter					= sizeof(MT_PATCH_HDR) + ( sizeof(MT_PATCH_SEC) * MT_WIFI_RAM_LL->count );

	/** start by overwriting the ROM's header 'n_region' field **/
        MT_WIFI_PATCH_HDR                               = (MT_PATCH_HDR *)MT_WIFI_PATCH_MMAP_FINAL;
	MT_WIFI_PATCH_HDR->desc.n_region		= cpu_to_be32(MT_WIFI_RAM_LL->count);

	/** before starting to write the region header table for the ROM patch, sort the list in base of the lowest-address-is-before **/
	sortLinkedListByAddr(MT_WIFI_RAM_LL);

	/** generate now the region headers for every element in the linked list **/

        {

		#define MT_WIFI_PATCH_FINAL		MT_WIFI_PATCH_MMAP_FINAL

		uint32_t rom_region_len			= sizeof(MT_PATCH_HDR);
		uint8_t  rom_region_counter		= 0x00;
        	MediatekRamRegionNode *LHead    	= MT_WIFI_RAM_LL->head;

                while ( LHead != NULL ) {
			MT_PATCH_SEC PATCH_REG  	= { 0 };
			/** this permit to pass the mt76 driver check **/
			PATCH_REG.type          	= cpu_to_be32(PATCH_SEC_TYPE_INFO);
			PATCH_REG.offs			= cpu_to_be32(offset_counter);
			PATCH_REG.size			= cpu_to_be32(LHead->length);
			PATCH_REG.info.len		= cpu_to_be32(LHead->length);
			PATCH_REG.info.addr		= cpu_to_be32(LHead->addr);

			printf("[rom region %d] header at = %d, offset = %d, size = %d, address = 0x%x\n",
				rom_region_counter,
				rom_region_len,
				offset_counter,
				LHead->length,
				LHead->addr
			);

			/** memcpy the region's header **/
			memcpy(MT_WIFI_PATCH_FINAL + 	rom_region_len, &PATCH_REG,  sizeof(PATCH_REG));

			/** now memcpy the region's data **/
			memcpy(MT_WIFI_PATCH_FINAL + 	offset_counter, LHead->data, LHead->length);

			#ifdef DEBUG
			printf("offset_counter BEFORE: %d, rom_region_len BEFORE: %d, offset_counter AFTER: %d, rom_region_len AFTER: %d\n",
				offset_counter,
				rom_region_len,
				offset_counter + be32_to_cpu(PATCH_REG.size),
				rom_region_len + sizeof(PATCH_REG)
			);
			#endif

			offset_counter	       	       += be32_to_cpu(PATCH_REG.size);
			rom_region_len	       	       += sizeof(PATCH_REG);
			rom_region_counter	       += 0x1;

			LHead				= LHead->next;
		}
	}

	/** finally, rewrite the ROM file **/
	final_rom_fw_fd                                 = open("final_rom_fw.bin", O_CREAT | O_RDWR,   0777);
        write(final_rom_fw_fd,                                MT_WIFI_PATCH_FINAL, MT_WIFI_PATCH_FINAL_SIZE);
        close(final_rom_fw_fd);

	/** the RAM is the easiest one: we can just allocate a new memory pool and change the region's headers **/

	MT_WIFI_RAM_MMAP_FINAL				=	(unsigned char *)mmap(
									NULL,
									MT_WIFI_RAM_SIZE,
									PROT_READ | PROT_WRITE,
									MAP_ANON  | MAP_PRIVATE,
									-1,
									0x00
							);


	/** create a clean area of writable memory, copy the RAM code and then make the changes, at the end, just write everything into a file **/
	memset(MT_WIFI_RAM_MMAP_FINAL,		0x00,	MT_WIFI_RAM_SIZE);
	memcpy(MT_WIFI_RAM_MMAP_FINAL, MT_WIFI_RAM_MMAP,MT_WIFI_RAM_SIZE);

	/** reuse the pointer again **/
        MT_WIFI_RAM_HDR                                 = NULL;
	MT_WIFI_RAM_HDR					= (void *)(MT_WIFI_RAM_MMAP_FINAL + MT_WIFI_RAM_SIZE - sizeof(*MT_WIFI_RAM_HDR));

        for (unsigned char i = 0; i < le32_to_cpu(MT_WIFI_RAM_HDR->n_region); i++) {
                MT_RAM_SEC *region              	= NULL;
		MediatekRamRegionNode *LHead		= NULL;
                uint32_t len                    	= 0x00;
                uint32_t addr                   	= 0x00;
                uint32_t mode                   	= 0x00;

                region                          	= (void *)((u8 *)MT_WIFI_RAM_HDR - (MT_WIFI_RAM_HDR->n_region - i) * sizeof(*region));

                len                             	= le32_to_cpu(region->len);
                addr                            	= le32_to_cpu(region->addr);

                LHead    				= MT_WIFI_RAM_LL->head;

                while ( LHead != NULL && !overlap_done ) {
                	if( addr + LHead->new_length   == (uint32_t)LHead->addr && region->feature_set & FW_FEATURE_OVERRIDE_ADDR ){
				region->len		= cpu_to_le32(LHead->new_length);
				printf("[ram_fw] final address and size of the OVERLAP region: 0x%x %d\n", region->addr, region->len);
				/** now overwrite the NEXT 4 bytes with the new crc value precomputed by us **/
				overlap_done		= 0x1;
                                break;
                        }
			LHead				= LHead->next;
                }

                if( !( region->feature_set & FW_FEATURE_NON_DL ) && !( region->feature_set & FW_FEATURE_OVERRIDE_ADDR ) ) {
                        printf("[ram_fw] adding the FW_FEATURE_NON_DL for addr 0x%x\n", region->addr);
                        region->feature_set    	       |= FW_FEATURE_NON_DL;
                }

	}


	/** now rewrite the final RAM file **/
        final_ram_fw_fd                          	= open("final_ram_fw.bin", O_CREAT | O_RDWR, 0777);
        write(final_ram_fw_fd,                       	MT_WIFI_RAM_MMAP_FINAL,    MT_WIFI_RAM_SIZE);
        close(final_ram_fw_fd);

	munmap(MT_WIFI_RAM_MMAP_FINAL,			MT_WIFI_RAM_SIZE);
	munmap(MT_WIFI_PATCH_MMAP_FINAL,		MT_WIFI_PATCH_FINAL_SIZE);

	RELEASE_FIRST_STAGE:
        munmap(MT_WIFI_RAM_MMAP,                        MT_WIFI_RAM_SIZE);
        munmap(MT_WIFI_PATCH_MMAP,                      MT_WIFI_PATCH_SIZE);

	return 0;
}

