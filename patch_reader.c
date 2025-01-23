/**
	PATCH READER

**/

#include <iostream>
#include <libsleigh.hh>

class AssemblyPrinter : public ghidra::AssemblyEmit {
	public:
		void dump(const ghidra::Address &addr, const std::string &mnemonic, const std::string &body) override {
			addr.printRaw(std::cout);
			std::cout << ": " << mnemonic << ' ' << body << std::endl;
  		}
};

static void PrintAssembly(ghidra::Sleigh &engine, uint64_t addr, size_t len) {
	AssemblyPrinter asm_emit;
	ghidra::Address cur_addr(engine.getDefaultCodeSpace(), addr),
      	last_addr(engine.getDefaultCodeSpace(), addr + len);
  	while (cur_addr < last_addr) {
    		int32_t instr_len = engine.printAssembly(asm_emit, cur_addr);
    		cur_addr = cur_addr + instr_len;
  	}
}


static int MT_DISASM_PATCH(void *MT_NDS32_CODE_PTR, unsigned int MT_NDS32_CODE_SIZE){
	ghidra::AttributeId::initialize();
	ghidra::ElementId::initialize();
	ghidra::ContextInternal ctx;
	//ghidra::Sleigh engine(NULL, &ctx);
	ghidra::DocumentStorage storage;

	//engine.initialize(storage);
	//engine.allowContextSet(false);

	//const auto sla_file_path = args->root_sla_dir ? sleigh::FindSpecFile(args->sla_file_name, {*args->root_sla_dir}) : sleigh::FindSpecFile(args->sla_file_name);

	return 0;
}

extern "C" {

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
	int		 MT_WIFI_PATCH_FD   	= 0x00;
	int		 MT_WIFI_PATCH_SIZE	= 0x00;
	struct stat     *MT_WIFI_PATCH_STATS	= NULL;
	unsigned char   *MT_WIFI_PATCH_MMAP	= NULL;

	MT_PATCH_HDR	*MT_WIFI_PATCH_HDR	= NULL;

	if( MT_WIFI_PATCH_NAME == NULL ){
		printf("missing patch name!\n");
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

	MT_WIFI_PATCH_MMAP			= (unsigned char *)mmap(
									NULL,
									MT_WIFI_PATCH_SIZE,
									PROT_READ,
									MAP_FILE | MAP_SHARED,
									MT_WIFI_PATCH_FD,
									0x00
						);


	if( MT_WIFI_PATCH_MMAP == NULL ){
		printf("error while MMAPing the patch!\n");
		close(MT_WIFI_PATCH_FD);
	}

	MT_WIFI_PATCH_HDR			= (MT_PATCH_HDR *)MT_WIFI_PATCH_MMAP;

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

	munmap(MT_WIFI_PATCH_MMAP, MT_WIFI_PATCH_SIZE);
	close(MT_WIFI_PATCH_FD);
	return 0;
}


}
