/*
   Made by Edoardo Mantovani, 2023
   This is the complete program for deobfuscating/splitting every Mediatek's firmware for mobile market, supports:
 * GEN2
 * GEN3
 * GEN4
 * GEN4M
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <sys/mman.h>
#include <sys/stat.h>

#define ACTIVE_DEBUG    1

#define KRED  "\x1B[31m"
#define KNRM  "\x1B[0m"
#define KGRN  "\x1B[32m"
#define KMAG  "\x1B[35m"
#define KBLU  "\x1B[34m"

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#ifdef ACTIVE_DEBUG
#define DBGLOG(...)     printf(__VA_ARGS__);
#else
#define DBGLOG(...)
#endif

#ifndef BIT
#define BIT(nr)                 ((unsigned long)1 << (nr))
#define BITS(m, n)              (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif

#define IN
#define OUT

#define BOOLEAN bool
static BOOLEAN read_mode    = false;
static BOOLEAN extract_mode = false;
static BOOLEAN repack_mode  = false;

struct extract_mode_S{
    unsigned char current_name[16];
    void          *tmp_data_pointer;
};

struct extract_mode_S extract_mode_struct = { 0 };

#define BOOLEAN bool
typedef void  VOID;
typedef void * PVOID;
typedef unsigned char UCHAR, *PUCHAR, **PPUCHAR;
typedef unsigned char UINT_8, *PUINT_8, **PPUINT_8, *P_UINT_8;
typedef unsigned short UINT_16, *PUINT_16, **PPUINT_16;
typedef unsigned int UINT_32, *PUINT_32, **PPUINT_32;
typedef signed   int INT_32;

static int gen_fw_automatic_detection = FALSE;

typedef enum _ENUM_IMG_DL_IDX_T{
    IMG_DL_IDX_N9_FW,
    IMG_DL_IDX_CR4_FW,
    IMG_DL_IDX_PATCH
} ENUM_IMG_DL_IDX_T, *P_ENUM_IMG_DL_IDX_T;

///////////////////////////////// GEN4 structure declaration START /////////////////////////////////
//slightly different in my chip, see: "struct TAILER_COMMON_FORMAT_T"
typedef struct _tailer_format_tag {
    UINT_32 addr;
    UINT_8 chip_info;
    UINT_8 feature_set;
    UINT_8 eco_code;
    UINT_8 ram_version[10];
    UINT_8 ram_built_date[15];
    UINT_32 len;

} tailer_format_t;

typedef struct _fw_image_tailer_tag {
    tailer_format_t ilm_info;
    tailer_format_t dlm_info;
} fw_image_tailer_t;

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
#define WIFI_FW_DECOMPRESSION_FAILED        0xFF
typedef struct _INIT_CMD_WIFI_DECOMPRESSION_START {
    UINT_32 u4Override;
    UINT_32 u4Address;
    UINT_32 u4Region1length;
    UINT_32 u4Region2length;
    UINT_32	u4Region1Address;
    UINT_32	u4Region2Address;
    UINT_32	u4BlockSize;
    UINT_32 u4Region1CRC;
    UINT_32 u4Region2CRC;
    UINT_32	u4DecompressTmpAddress;
} INIT_CMD_WIFI_DECOMPRESSION_START, *P_INIT_CMD_WIFI_DECOMPRESSION_START;

typedef struct _tailer_format_tag_2 {
    UINT_32 crc;
    UINT_32 addr;
    UINT_32 block_size;
    UINT_32 real_size;
    UINT_8  chip_info;
    UINT_8  feature_set;
    UINT_8  eco_code;
    UINT_8  ram_version[10];
    UINT_8  ram_built_date[15];
    UINT_32 len;
} tailer_format_t_2;
typedef struct _fw_image_tailer_tag_2 {
    tailer_format_t_2 ilm_info;
    tailer_format_t_2 dlm_info;
} fw_image_tailer_t_2;
typedef struct _fw_image_tailer_check {
    UINT_8	chip_info;
    UINT_8	feature_set;
    UINT_8	eco_code;
    UINT_8	ram_version[10];
    UINT_8	ram_built_date[15];
    UINT_32 len;
} fw_image_tailer_check;

#endif
///////////////////////////////// GEN4 structure declaration END   ///////////////////////////////////

///////////////////////////////// GEN4M structure declaration START /////////////////////////////////
/*
 * FW feature set
 * bit(0)  : encrypt or not.
 * bit(1,2): encrypt key index.
 * bit(3)  : compressed image or not. (added in CONNAC)
 * bit(4)  : encrypt mode, 1 for scramble, 0 for AES.
 * bit(5)  : replace RAM code starting address with image
 *           destination address or not. (added in CONNAC)
 * bit(7)  : download to EMI or not. (added in CONNAC)
 */
#define FW_FEATURE_SET_ENCRY	BIT(0)
#define FW_FEATURE_SET_KEY_MASK	BITS(1, 2)
#define GET_FW_FEATURE_SET_KEY(p) (((p) & FW_FEATURE_SET_KEY_MASK) >> 1)
#define FW_FEATURE_COMPRESS_IMG	BIT(3)
#define FW_FEATURE_ENCRY_MODE	BIT(4)
#define FW_FEATURE_OVERRIDE_RAM_ADDR	BIT(5) /* is it the same of 'DOWNLOAD_CONFIG_VALID_RAM_ENTRY' ? */
#define FW_FEATURE_NOT_DOWNLOAD	BIT(6)
#define FW_FEATURE_DL_TO_EMI	BIT(7)

struct TAILER_FORMAT_T {
    uint32_t addr;
    uint8_t chip_info;
    uint8_t feature_set;
    uint8_t eco_code;
    uint8_t ram_version[10];
    uint8_t ram_built_date[15];
    uint32_t len;
};

struct TAILER_COMMON_FORMAT_T {
    uint8_t ucChipInfo;
    uint8_t ucEcoCode;
    uint8_t ucRegionNum;
    uint8_t ucFormatVer;
    uint8_t ucFormatFlag;
    uint8_t aucReserved[2];
    uint8_t aucRamVersion[10];
    uint8_t aucRamBuiltDate[15];
    uint32_t u4CRC;
};

struct TAILER_REGION_FORMAT_T {
    uint32_t u4CRC;
    uint32_t u4RealSize;
    uint32_t u4BlockSize;
    uint8_t aucReserved1[4];
    uint32_t u4Addr;
    uint32_t u4Len;
    uint8_t ucFeatureSet;
    uint8_t aucReserved2[15];
};

struct HEADER_RELEASE_INFO {
    uint16_t u2Len;
    uint8_t ucPaddingLen;
    uint8_t ucTag;
};
///////////////////////////////// GEN4M structure declaration END //////////////////////////////////


///////////////////////////////// GEB2 functions declaration START /////////////////////////////////
/*uint32_t wlanAdapterStart(IN P_REG_INFO_T prRegInfo,
  IN PVOID pvFwImageMapFile,
  IN UINT_32 u4FwImageFileLength,
  uint32_t nothing,
  ){
#if CFG_ENABLE_FW_DOWNLOAD
UINT_32 u4FwLoadAddr;
#if CFG_ENABLE_FW_DIVIDED_DOWNLOAD
P_FIRMWARE_DIVIDED_DOWNLOAD_T prFwHead;
BOOLEAN fgValidHead;
const UINT_32 u4CRCOffset = offsetof(FIRMWARE_DIVIDED_DOWNLOAD_T, u4NumOfEntries);
#endif
#endif


}*/
///////////////////////////////// GEN2 functions declaration END   /////////////////////////////////

///////////////////////////////// GEN4 functions declaration START /////////////////////////////////

#define DOWNLOAD_CONFIG_ENCRYPTION_MODE         BIT(0)
#define DOWNLOAD_CONFIG_KEY_INDEX_MASK          BITS(1, 2)
#define DOWNLOAD_CONFIG_RESET_OPTION            BIT(3)
#define DOWNLOAD_CONFIG_WORKING_PDA_OPTION      BIT(4)
#define DOWNLOAD_CONFIG_ACK_OPTION              BIT(31)

static void wlanImageSectionGetFwInfo(
        IN PVOID pvFwImageMapFile,
        IN UINT_32 u4FwImageFileLength,
        IN UINT_8 ucTotSecNum,
        IN UINT_8 ucCurSecNum,
        IN ENUM_IMG_DL_IDX_T eDlIdx,
        OUT PUINT_32 pu4StartOffset,
        OUT PUINT_32 pu4Addr,
        OUT PUINT_32 pu4Len,
        OUT PUINT_32 pu4DataMode){
    UINT_32 u4DataMode = 0;
    fw_image_tailer_t *prFwHead;
    tailer_format_t *prTailer;
    prFwHead = (fw_image_tailer_t *) (pvFwImageMapFile + u4FwImageFileLength - sizeof(fw_image_tailer_t));
    if (ucTotSecNum == 1){
        prTailer = &prFwHead->dlm_info;
    }else{
        prTailer = &prFwHead->ilm_info;
    }

    prTailer = &prTailer[ucCurSecNum];
    *pu4StartOffset = 0;
    *pu4Addr = prTailer->addr;
#ifndef LEN_4_BYTE_CRC
#define LEN_4_BYTE_CRC 4
#endif
    *pu4Len = (prTailer->len + LEN_4_BYTE_CRC);
    if (prTailer->feature_set & DOWNLOAD_CONFIG_ENCRYPTION_MODE) {
        u4DataMode |= DOWNLOAD_CONFIG_RESET_OPTION;
        u4DataMode |= (prTailer->feature_set & DOWNLOAD_CONFIG_KEY_INDEX_MASK);
        u4DataMode |= DOWNLOAD_CONFIG_ENCRYPTION_MODE;
    }

    if (eDlIdx == IMG_DL_IDX_CR4_FW)
        u4DataMode |= DOWNLOAD_CONFIG_WORKING_PDA_OPTION;

#if CFG_ENABLE_FW_DOWNLOAD_ACK
    u4DataMode |= DOWNLOAD_CONFIG_ACK_OPTION;	/* ACK needed */
#endif
    *pu4DataMode = u4DataMode;
    if (ucCurSecNum) {
        DBGLOG("[%s] %s INFO: chip[%u:E%u] feature[0x%02X], date[%s] version[%c%c%c%c%c%c%c%c%c%c]\n",
                __FUNCTION__,
                (eDlIdx == IMG_DL_IDX_N9_FW) ? "N9" : "CR4",
                prTailer->chip_info,
                prTailer->eco_code + 1,
                prTailer->feature_set,
                prTailer->ram_built_date,
                prTailer->ram_version[0], prTailer->ram_version[1],
                prTailer->ram_version[2], prTailer->ram_version[3],
                prTailer->ram_version[4], prTailer->ram_version[5],
                prTailer->ram_version[6], prTailer->ram_version[7],
                prTailer->ram_version[8], prTailer->ram_version[9]);
    }
}

static void wlanImageSectionGetInfo(
        IN PVOID pvFwImageMapFile,
        IN UINT_32 u4FwImageFileLength,
        IN UINT_8 ucTotSecNum,
        IN UINT_8 ucCurSecNum,
        IN ENUM_IMG_DL_IDX_T eDlIdx,
        OUT PUINT_32 pu4StartOffset,
        OUT PUINT_32 pu4Addr,
        OUT PUINT_32 pu4Len,
        OUT PUINT_32 pu4DataMode){
    wlanImageSectionGetFwInfo(pvFwImageMapFile, u4FwImageFileLength, ucTotSecNum, ucCurSecNum, eDlIdx, pu4StartOffset, pu4Addr, pu4Len, pu4DataMode);
}

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
#define COMPRESSION_OPTION_OFFSET   4
#define COMPRESSION_OPTION_MASK     BIT(4)

VOID wlanImageSectionGetCompressFwInfo(
        IN PVOID pvFwImageMapFile,
        IN UINT_32 u4FwImageFileLength,
        IN UINT_8 ucTotSecNum,
        IN UINT_8 ucCurSecNum,
        IN ENUM_IMG_DL_IDX_T eDlIdx,
        OUT PUINT_32 pu4StartOffset,
        OUT PUINT_32 pu4Addr,
        OUT PUINT_32 pu4Len,
        OUT PUINT_32 pu4DataMode,
        OUT PUINT_32 pu4BlockSize,
        OUT PUINT_32 pu4CRC,
        OUT PUINT_32 pu4UncompressedLength
        ){
    UINT_32 u4DataMode = 0;
    fw_image_tailer_t_2 *prFwHead;
    tailer_format_t_2 *prTailer;

    prFwHead = (fw_image_tailer_t_2 *) (pvFwImageMapFile + u4FwImageFileLength - sizeof(fw_image_tailer_t_2));
    if (ucTotSecNum == 1){
        prTailer = &prFwHead->dlm_info;
    }else{
        prTailer = &prFwHead->ilm_info;
    }
    prTailer = &prTailer[ucCurSecNum];

    *pu4StartOffset = 0;
    *pu4Addr = prTailer->addr;
    *pu4Len = (prTailer->len);
    *pu4BlockSize = (prTailer->block_size);
    *pu4CRC = (prTailer->crc);
    *pu4UncompressedLength = (prTailer->real_size);
    if (prTailer->feature_set & DOWNLOAD_CONFIG_ENCRYPTION_MODE) {
        u4DataMode |= DOWNLOAD_CONFIG_RESET_OPTION;
        u4DataMode |= (prTailer->feature_set & DOWNLOAD_CONFIG_KEY_INDEX_MASK);
        u4DataMode |= DOWNLOAD_CONFIG_ENCRYPTION_MODE;
    }
    if (eDlIdx == IMG_DL_IDX_CR4_FW)
        u4DataMode |= DOWNLOAD_CONFIG_WORKING_PDA_OPTION;

#if CFG_ENABLE_FW_DOWNLOAD_ACK
    u4DataMode |= DOWNLOAD_CONFIG_ACK_OPTION;	/* ACK needed */
#endif

    *pu4DataMode = u4DataMode;

    /* Dump image information */
    if (ucCurSecNum == 0) {
        DBGLOG("%s INFO: chip_info[%u:E%u] feature[0x%02X]\n",
                (eDlIdx == IMG_DL_IDX_N9_FW)?"N9":"CR4", prTailer->chip_info,
                prTailer->eco_code, prTailer->feature_set);
        DBGLOG("date[%s] version[%c%c%c%c%c%c%c%c%c%c]\n", prTailer->ram_built_date,
                prTailer->ram_version[0], prTailer->ram_version[1],
                prTailer->ram_version[2], prTailer->ram_version[3],
                prTailer->ram_version[4], prTailer->ram_version[5],
                prTailer->ram_version[6], prTailer->ram_version[7],
                prTailer->ram_version[8], prTailer->ram_version[9]);
        DBGLOG("u4DataMode: %x\n", u4DataMode);
    }
}

BOOLEAN wlanImageSectionCheckFwCompressInfo(
        IN PVOID pvFwImageMapFile,
        IN UINT_32 u4FwImageFileLength,
        IN ENUM_IMG_DL_IDX_T eDlIdx
        ){
    UINT_8 ucCompression;
    fw_image_tailer_check *prCheckInfo;

    if (eDlIdx == IMG_DL_IDX_PATCH)
        return FALSE;

    prCheckInfo = (fw_image_tailer_check *)(pvFwImageMapFile + u4FwImageFileLength - sizeof(fw_image_tailer_check));
    DBGLOG("feature_set %d\n", prCheckInfo->feature_set);
    ucCompression = (UINT_8)((prCheckInfo->feature_set & COMPRESSION_OPTION_MASK) >> COMPRESSION_OPTION_OFFSET);
    DBGLOG("Compressed Check INFORMATION %d\n", ucCompression);
    if (ucCompression == 1) {
        DBGLOG("Compressed FW\n");
        return TRUE;
    }
    return FALSE;
}

static int wlanImageSectionDownloadStage(
        IN PVOID pvFwImageMapFile,
        IN UINT_32 u4FwImageFileLength,
        IN UINT_8 ucSectionNumber,
        IN ENUM_IMG_DL_IDX_T eDlIdx,
        OUT PUINT_8 pucIsCompressed,
        OUT P_INIT_CMD_WIFI_DECOMPRESSION_START prFwImageInFo
        ){
    DBGLOG("%s HAS BEEN CALLED (decompression mode)!!!\n", __FUNCTION__);
    DBGLOG("PASSED ARGOUMENTS:\n-map file address: 0x%x\n-map file size: %ld\n-number of sections: %d\n",
            pvFwImageMapFile,
            u4FwImageFileLength,
            ucSectionNumber
          );
    UINT_32 u4ImgSecSize;
    UINT_32 j, i;
    INT_32  i4TotalLen;
    UINT_32 u4FileOffset = 0;
    UINT_32 u4StartOffset = 0;
    UINT_32 u4DataMode = 0;
    UINT_32 u4Addr, u4Len, u4BlockSize, u4CRC, u4UnCompressedLength;
    PUINT_8 pucSecBuf, pucStartPtr;
    UINT_32 u4offset = 0, u4ChunkSize;
    INT_32  u4Status = 0;
    for(i = 0; i < ucSectionNumber; ++i) {
        if(wlanImageSectionCheckFwCompressInfo(pvFwImageMapFile, u4FwImageFileLength, eDlIdx) == 1){
            wlanImageSectionGetCompressFwInfo(
                    pvFwImageMapFile,
                    u4FwImageFileLength,
                    ucSectionNumber,
                    i,
                    eDlIdx,
                    &u4StartOffset,
                    &u4Addr,
                    &u4Len,
                    &u4DataMode,
                    &u4BlockSize,
                    &u4CRC,
                    &u4UnCompressedLength);
            u4offset = 0;
            if (i == 0) {
                prFwImageInFo->u4BlockSize = u4BlockSize;
                prFwImageInFo->u4Region1Address = u4Addr;
                prFwImageInFo->u4Region1CRC = u4CRC;
                prFwImageInFo->u4Region1length = u4UnCompressedLength;
            } else {
                prFwImageInFo->u4Region2Address = u4Addr;
                prFwImageInFo->u4Region2CRC = u4CRC;
                prFwImageInFo->u4Region2length = u4UnCompressedLength;
            }
            i4TotalLen = u4Len;
            DBGLOG("[%s] DL Offset[%u] addr[0x%x] len[%u] datamode[0x%08x]\n", __FUNCTION__, u4FileOffset, u4Addr, u4Len, u4DataMode);
            DBGLOG("[%s] DL BLOCK[%u]  COMlen[%u] CRC[%u]\n", __FUNCTION__, u4BlockSize, u4UnCompressedLength, u4CRC);
            pucStartPtr = (PUINT_8)pvFwImageMapFile + u4StartOffset;
            while (i4TotalLen) {
                //u4ChunkSize =  *((unsigned int *)(pucStartPtr+u4FileOffset));
                //u4FileOffset += 4;
                //DBGLOG( "Downloaded Length %d! Addr %x\n", i4TotalLen, u4Addr + u4offset);
                //DBGLOG("u4ChunkSize Length %d!\n", u4ChunkSize);
                // 'wlanImageSectionConfig' is used to issue a FW_DWNLOAD command to the adapter, not interesting for us.
                /*if (wlanImageSectionConfig((u4Addr + u4offset), u4ChunkSize, u4DataMode, eDlIdx) != 0) {
                  DBGLOG("Firmware download configuration failed!\n");
                  u4Status = -1;
                  break;
                  }*/
                // still to complete from here
            }

        }
    }
}
#else
static int wlanImageSectionDownloadStage(
        IN PVOID pvFwImageMapFile,
        IN UINT_32 u4FwImageFileLength,
        IN UINT_8 ucSectionNumber,
        IN ENUM_IMG_DL_IDX_T eDlIdx
        ){
    DBGLOG("%s HAS BEEN CALLED!!!\n", __FUNCTION__);
    DBGLOG("PASSED ARGOUMENTS:\n-map file address: 0x%x\n-map file size: %ld\n-number of sections: %d\n",
            pvFwImageMapFile,
            u4FwImageFileLength,
            ucSectionNumber
          );
    UINT_32 u4FileOffset = 0;
    UINT_32 u4StartOffset = 0;
    UINT_32 u4DataMode = 0;
    UINT_32 u4Addr, u4Len;
    PUINT_8 pucSecBuf, pucStartPtr;
    for (int i = 0; i < ucSectionNumber; ++i) {
        wlanImageSectionGetInfo(
                pvFwImageMapFile,
                u4FwImageFileLength,
                ucSectionNumber,
                i,
                eDlIdx,
                &u4StartOffset,
                &u4Addr,
                &u4Len,
                &u4DataMode);
        pucStartPtr = (PUINT_8)(pvFwImageMapFile + u4StartOffset);
        DBGLOG("[%s] DL Offset[%u] addr[0x%08x] len[%u] datamode[0x%08x]\n", __FUNCTION__, u4FileOffset, u4Addr, u4Len, u4DataMode)
            u4FileOffset += u4Len;
    }
}
#endif

///////////////////////////////// GEN4 functions declaration END  ///////////////////////////////////

///////////////////////////////// GEN4M functions declaration START /////////////////////////////////


uint32_t wlanGetHarvardTailerInfo(
        IN void *prFwBuffer,
        IN uint32_t u4FwSize,
        IN uint32_t ucTotSecNum,
        IN ENUM_IMG_DL_IDX_T eDlIdx
        ){
    DBGLOG(KRED " %s " KNRM  "HAS BEEN CALLED!!!\n", __FUNCTION__);
    DBGLOG("PASSED ARGOUMENTS:\n-map file address: 0x%x\n-map file size: %ld\n-number of sections: %d\n",
            prFwBuffer,
            u4FwSize,
            ucTotSecNum
          );

    struct TAILER_FORMAT_T *prTailers;
    uint8_t *pucStartPtr;
    uint32_t u4SecIdx;
    uint8_t aucBuf[32];
    pucStartPtr = prFwBuffer + u4FwSize - sizeof(struct TAILER_FORMAT_T) * ucTotSecNum;
    for (u4SecIdx = 0; u4SecIdx < ucTotSecNum; u4SecIdx++) {
        /* Dump image information */
        DBGLOG("%s Section[%d]: chip_info[%u:E%u] feature[0x%02X]\n",
                (eDlIdx == IMG_DL_IDX_N9_FW) ? "N9" : "CR4", u4SecIdx,
                prTailers[u4SecIdx].chip_info,
                prTailers[u4SecIdx].eco_code + 1,
                prTailers[u4SecIdx].feature_set);

        memset(aucBuf, 0x00, 32);
        strncpy(aucBuf, prTailers[u4SecIdx].ram_version, sizeof(prTailers[u4SecIdx].ram_version));
        DBGLOG("date[%s] version[%s]\n", prTailers[u4SecIdx].ram_built_date, aucBuf);
    }
}

static void fwDlGetReleaseManifest(struct HEADER_RELEASE_INFO *prRelInfo, uint8_t *pucStartPtr){
    unsigned char *tmp_buffer = (unsigned char *)malloc(prRelInfo->u2Len);
    memset(tmp_buffer, 0x00, prRelInfo->u2Len);
    memcpy(tmp_buffer, pucStartPtr, prRelInfo->u2Len);
    DBGLOG("Release manifest: %s\n", tmp_buffer);
    free(tmp_buffer);
}

static void fwDlGetReleaseInfoSection(uint8_t *pucStartPtr){
    struct HEADER_RELEASE_INFO *prFirstInfo;
    struct HEADER_RELEASE_INFO *prRelInfo;
#ifndef RELEASE_INFO_SEPARATOR_LEN
#define RELEASE_INFO_SEPARATOR_LEN 16
#endif
    uint8_t *pucCurPtr = pucStartPtr + RELEASE_INFO_SEPARATOR_LEN;
    uint16_t u2Len = 0, u2Offset = 0;
    uint8_t ucManifestExist = 0;

    prFirstInfo = (struct HEADER_RELEASE_INFO *)pucCurPtr;
    DBGLOG("Release info tag[%u] len[%u]\n", prFirstInfo->ucTag, prFirstInfo->u2Len);

    pucCurPtr += sizeof(struct HEADER_RELEASE_INFO);
    while (u2Offset < prFirstInfo->u2Len) {
        prRelInfo = (struct HEADER_RELEASE_INFO *)pucCurPtr;
        DBGLOG("Release info tag[%u] len[%u] padding[%u]\n",
                prRelInfo->ucTag, prRelInfo->u2Len,
                prRelInfo->ucPaddingLen);

        pucCurPtr += sizeof(struct HEADER_RELEASE_INFO);
        switch (prRelInfo->ucTag) {
            case 0x01:
                fwDlGetReleaseManifest(prRelInfo, pucCurPtr);
                ucManifestExist = 1;
                break;
            case 0x02:
                if (!ucManifestExist)
                    fwDlGetReleaseManifest(prRelInfo, pucCurPtr);
                break;
            default:
                DBGLOG("Not support release info tag[%u]\n", prRelInfo->ucTag);
        }

        u2Len = prRelInfo->u2Len + prRelInfo->ucPaddingLen;
        pucCurPtr += u2Len;
        u2Offset += u2Len + sizeof(struct HEADER_RELEASE_INFO);
    }
}

static void CUSTOM_scan_for_detected_feature(uint8_t feature){
    if( feature & FW_FEATURE_SET_ENCRY ){
        DBGLOG("firmware is encrypted\n");
        if( ( feature & FW_FEATURE_ENCRY_MODE ) == 1 ){
            DBGLOG("firmware uses 'scramble' encryption\n");
        }else{
            DBGLOG("firmware uses 'AES' encryption\n");
        }
    }else{
        DBGLOG("firmware isn't encrypted\n");
    }
    if( feature & FW_FEATURE_COMPRESS_IMG ){
        DBGLOG("firmware is in a compressed format\n");
    }else{
        DBGLOG("firmware isn't in a compressed format\n");
    }
    if( feature & FW_FEATURE_OVERRIDE_RAM_ADDR ){
        DBGLOG("this section is the RAM entry point\n");
    }
    if( feature & FW_FEATURE_NOT_DOWNLOAD ){
        DBGLOG("firmware is not downloadable\n");
    }

    if( feature & FW_FEATURE_DL_TO_EMI ){
        DBGLOG("this firmware section will be loaded through EMI\n");
    }
}

uint32_t wlanGetDataMode(IN ENUM_IMG_DL_IDX_T eDlIdx, IN uint8_t ucFeatureSet){
    uint32_t u4DataMode = 0;

    if (ucFeatureSet & FW_FEATURE_SET_ENCRY) {
        u4DataMode |= DOWNLOAD_CONFIG_RESET_OPTION;
        u4DataMode |= (ucFeatureSet &
                FW_FEATURE_SET_KEY_MASK);
        u4DataMode |= DOWNLOAD_CONFIG_ENCRYPTION_MODE;
        if (ucFeatureSet & FW_FEATURE_ENCRY_MODE){
#ifndef DOWNLOAD_CONFIG_ENCRY_MODE_SEL
#define DOWNLOAD_CONFIG_ENCRY_MODE_SEL	BIT(6)
#endif
            u4DataMode |= DOWNLOAD_CONFIG_ENCRY_MODE_SEL;
        }
    }

    if (eDlIdx == IMG_DL_IDX_CR4_FW)
        u4DataMode |= DOWNLOAD_CONFIG_WORKING_PDA_OPTION;

#if CFG_ENABLE_FW_DOWNLOAD_ACK
    u4DataMode |= DOWNLOAD_CONFIG_ACK_OPTION;       /* ACK needed */
#endif
    return u4DataMode;
}


struct PATCH_FORMAT_T {
    uint8_t aucBuildDate[16];
    uint8_t aucPlatform[4];
    uint32_t u4SwHwVersion;
    uint32_t u4PatchVersion;
    uint16_t u2CRC;		/* CRC calculated for image only */
    uint8_t ucPatchImage[0];
};

struct ROM_EMI_HEADER {
    uint8_t ucDateTime[16];
    uint8_t ucPLat[4];
    uint16_t u2HwVer;
    uint16_t u2SwVer;
    uint32_t u4PatchAddr;
    uint32_t u4PatchType;
    uint32_t u4CRC[4];
};


void wlanImageSectionGetPatchInfo(
        IN void *pvFwImageMapFile,
        IN uint32_t u4FwImageFileLength,
        OUT uint32_t *pu4StartOffset,
        OUT uint32_t *pu4Addr,
        OUT uint32_t *pu4Len,
        OUT uint32_t *pu4DataMode
        ){
    struct PATCH_FORMAT_T *prPatchFormat;
    struct ROM_EMI_HEADER *prRomEmiHdr;
    uint8_t aucBuffer[32];

    prPatchFormat = (struct PATCH_FORMAT_T *) pvFwImageMapFile;
    prRomEmiHdr   = (struct ROM_EMI_HEADER *)pvFwImageMapFile;

    *pu4StartOffset = offsetof(struct PATCH_FORMAT_T, ucPatchImage);
    *pu4Addr = prRomEmiHdr->u4PatchAddr; //prChipInfo->patch_addr;
    *pu4Len = u4FwImageFileLength - offsetof(struct PATCH_FORMAT_T, ucPatchImage);
    *pu4DataMode = wlanGetDataMode(IMG_DL_IDX_PATCH, 0);

    /* Dump image information */
    memset(aucBuffer, 0x00, 32);
    strncpy(aucBuffer, prPatchFormat->aucPlatform, 4);
    DBGLOG("PATCH INFO: platform[%s] HW/SW ver[0x%04X] ver[0x%04X]\n", aucBuffer, prPatchFormat->u4SwHwVersion, prPatchFormat->u4PatchVersion);

    strncpy(aucBuffer, prPatchFormat->aucBuildDate, 16);
    DBGLOG("date[%s]\n", aucBuffer);
}

int soc3_0_wlanImageSectionDownloadStage(
        IN void *pvFwImageMapFile,
        IN uint32_t u4FwImageFileLength,
        IN unsigned char ucSectionNumber,
        IN ENUM_IMG_DL_IDX_T eDlIdx
        ){
    uint32_t u4SecIdx, u4Offset = 0;
    uint32_t u4Addr, u4Len, u4DataMode = 0;

    struct PATCH_FORMAT_T *prPatchHeader;
    struct ROM_EMI_HEADER *prRomEmiHeader;
    struct FWDL_OPS_T *prFwDlOps;

    wlanImageSectionGetPatchInfo(pvFwImageMapFile, u4FwImageFileLength, &u4Offset, &u4Addr, &u4Len, &u4DataMode);
    DBGLOG("FormatV1 DL Offset[%u] addr[0x%08x] len[%u] datamode[0x%08x]\n", u4Offset, u4Addr, u4Len, u4DataMode);
}


/* note that 'uint8_t postnum' is just for fixing a boring bug caused by other functions argoument */
int  wlanGetConnacTailerInfo(
        IN void *prFwBuffer,
        IN uint32_t u4FwSize,
        uint8_t postnum,
        IN ENUM_IMG_DL_IDX_T eDlIdx
        ){
    DBGLOG(KRED "%s " KNRM "HAS BEEN CALLED!!!\n", __FUNCTION__);
    DBGLOG("PASSED ARGOUMENTS:\n-map file address: 0x%x\n-map file size: %ld\n",
            prFwBuffer,
            u4FwSize
          );

    struct TAILER_COMMON_FORMAT_T *prComTailer;
    struct TAILER_REGION_FORMAT_T *prRegTailer;
    uint8_t *pucImgPtr     = NULL;
    uint8_t *pucTailertPtr = NULL;
    uint8_t *pucStartPtr   = NULL;
    uint32_t u4SecIdx      = 0;
    uint8_t  aucBuf[32]    = { 0 }; 

    /** size counter is used only for containing the section length sum in case of "extract" ops **/
    uint64_t size_counter  = 0;


    pucImgPtr = prFwBuffer;
    pucStartPtr = prFwBuffer + u4FwSize - sizeof(struct TAILER_COMMON_FORMAT_T);
    prComTailer = (struct TAILER_COMMON_FORMAT_T *) pucStartPtr;

    memset(aucBuf, 0x00, 32);
    strncpy(aucBuf, prComTailer->aucRamVersion, sizeof(prComTailer->aucRamVersion));

    /* Dump image information */
    DBGLOG(	"%s: chip_info[%u:E%u] region_num[%d] date[%s] version[%s]\n",
            (eDlIdx == IMG_DL_IDX_N9_FW) ? "N9" : "CR4",
            prComTailer->ucChipInfo,
            prComTailer->ucEcoCode + 1,
            prComTailer->ucRegionNum,
            prComTailer->aucRamBuiltDate,
            aucBuf);
#ifndef MAX_FWDL_SECTION_NUM
#define MAX_FWDL_SECTION_NUM 10
#endif
    if (prComTailer->ucRegionNum > MAX_FWDL_SECTION_NUM) {
        DBGLOG("Regions number[%d] > max section number[%d]\n",
                prComTailer->ucRegionNum, MAX_FWDL_SECTION_NUM);
        return -1;
    }

    pucStartPtr -= (prComTailer->ucRegionNum * sizeof(struct TAILER_REGION_FORMAT_T));
    pucTailertPtr = pucStartPtr;
    for (u4SecIdx = 0; u4SecIdx < prComTailer->ucRegionNum; u4SecIdx++) {
        prRegTailer = (struct TAILER_REGION_FORMAT_T *) pucStartPtr;
        /* Dump image information */
        DBGLOG("Region[" KBLU "%d" KNRM "]: addr[" KGRN "0x%08X" KNRM "] feature[0x%02X] size[" KMAG "%u" KNRM"]\n",
                u4SecIdx, prRegTailer->u4Addr,
                prRegTailer->ucFeatureSet, prRegTailer->u4Len);
        CUSTOM_scan_for_detected_feature(prRegTailer->ucFeatureSet);
        DBGLOG("uncompress_crc[0x%08X] uncompress_size[0x%08X] block_size[0x%08X]\n",
                prRegTailer->u4CRC, prRegTailer->u4RealSize,
                prRegTailer->u4BlockSize);
        if( extract_mode == TRUE ){
#ifndef MAX_STRLEN_OUTPUT_FILE
#define MAX_STRLEN_OUTPUT_FILE 64
#endif
            snprintf(extract_mode_struct.current_name, MAX_STRLEN_OUTPUT_FILE, "out_%d_0x%x", u4SecIdx, prRegTailer->u4Addr);
            /** create a new file with the name of "extract_mode_struct.current_name" **/
            int fd = open(extract_mode_struct.current_name, O_CREAT | O_WRONLY);
            extract_mode_struct.tmp_data_pointer = malloc(prRegTailer->u4Len);
            memcpy(extract_mode_struct.tmp_data_pointer, prFwBuffer + size_counter, prRegTailer->u4Len);
            /** write the data to the file **/
            write(fd, extract_mode_struct.tmp_data_pointer, prRegTailer->u4Len);
            memset(extract_mode_struct.tmp_data_pointer, 0x00, prRegTailer->u4Len);
            free(extract_mode_struct.tmp_data_pointer);
            close(fd);
            size_counter += prRegTailer->u4Len;
        }
        /*
		if( repack_mode == TRUE ){
			long final_file_size = 0;
			for( int i = 0; i < u4SecIdx; i++ ){
				snprintf(extract_mode_struct.current_name, MAX_STRLEN_OUTPUT_FILE, out_%d_0x%x, u4SecIdx, prRegTailer->u4Addr);
				int fd = open(extract_mode_struct.current_name, O_RDWR);
				if( ! fd ){
					break;
				}
				struct stat *fs = (struct stat *)malloc(sizeof(fs));
				fstat(fd, fs);
				if( fs->st_size != 0 ){
					final_file_size += fs->st_size;
                }
			}
			unsigned char *final_file_content = mmap(
								NULL,
								final_file_size,
								PROT_READ   | PROT_WRITE,
								MAP_PRIVATE | MAP_ANON,
								-1,
								0x00
								);
		}
        */
        pucImgPtr += prRegTailer->u4Len;
        pucStartPtr += sizeof(struct TAILER_REGION_FORMAT_T);
    }

    if (prComTailer->ucFormatFlag && pucImgPtr < pucTailertPtr){
        fwDlGetReleaseInfoSection(pucImgPtr);
    }
    return 0;
}

///////////////////////////////// GEN4M functions declaration END  //////////////////////////////////

enum mtk_fw_index_list{
    MTK_WIFI_FW_GEN2 = 0,
    MTK_WIFI_FW_GEN3,
    MTK_WIFI_FW_GEN4,
    MTK_WIFI_FW_GEN4M,
    MTK_WIFI_FW_SOC3,
    MTK_WIFI_FW_TOTAL_NUMBER,
};

struct mtk_fw_cb_table{
    int	             (*disassemble_firmware)(void *firmware_buffer, uint32_t firmware_size, uint8_t ucSectionNumber, ENUM_IMG_DL_IDX_T eDlIdx);
    int                firmware_bitops_flags;
}__attribute__((packed)) mtk_fw_global_table[MTK_WIFI_FW_TOTAL_NUMBER] = {
    [MTK_WIFI_FW_GEN2] = {

    },
    [MTK_WIFI_FW_GEN3] = {

    },
    [MTK_WIFI_FW_GEN4] = {
        .disassemble_firmware  = wlanImageSectionDownloadStage,
        .firmware_bitops_flags = 0,
    },
    [MTK_WIFI_FW_GEN4M] = {
        .disassemble_firmware = wlanGetConnacTailerInfo,
        .firmware_bitops_flags = 0,
    },
    [MTK_WIFI_FW_SOC3] = {
        .disassemble_firmware = soc3_0_wlanImageSectionDownloadStage,
        .firmware_bitops_flags = 0,
    },
};

static inline unsigned char *convert_ops_to_string(unsigned char *ops){
    if( ! ( strcmp(ops, "read") ) ){
        read_mode = true;
        return "displaying file sections!";
    }
    if( ! (strcmp(ops, "extract") ) ){
        extract_mode = true;
        return "display AND extract file sections!";
    }
    if( ! (strcmp(ops, "repack") ) ){
        repack_mode = true;
        return "display AND repack the file sections, fix the CRC field too!";
    }
    return NULL;
}

enum mtk_fw_index_list switch_fw_type_from_cmdline(unsigned char *string){
    if( gen_fw_automatic_detection ){
        DBGLOG("	gen_fw_automatic_detection	is enabled\n");
        return MTK_WIFI_FW_TOTAL_NUMBER + 1;
    }
    if( *(string + 0) == 'g' || *(string + 0) == 'G' &&
            *(string + 1) == 'e' || *(string + 1) == 'E' &&
            *(string + 2) == 'n' || *(string + 2) == 'N' ){
        if( *(string + 3) == '2' ){
            return MTK_WIFI_FW_GEN2;
        }
        if( *(string + 3) == '3' ){
            return MTK_WIFI_FW_GEN3;
        }
        if( *(string + 3) == '4' ){
            if( *(string + 4) == 'M' || *(string + 4) == 'm' ){
                if( *(string + 5) == '3' ){
                    return MTK_WIFI_FW_SOC3;
                }else{
                    return MTK_WIFI_FW_GEN4M;
                }
            }else{
                return MTK_WIFI_FW_GEN4;
            }
        }
        return MTK_WIFI_FW_TOTAL_NUMBER;
    }else{
        return MTK_WIFI_FW_TOTAL_NUMBER;
    }
}

int main(int argc, char *argv[]){
    unsigned char *fw_name  = argv[1];
    unsigned char *fw_model = argv[2];
    unsigned char *ops      = NULL;

    /** added ops, possible operations:
      - read:    display the file sections
      - extract: extract the sections
      - repack:  read if in the current folder there are the extracted sections and if there are, recalculate the CRC and then create a new, modified, Mediatek Wifi firmware
     **/

    if( fw_name == NULL ){
        DBGLOG("FAILED TO OBTAIN THE FW NAME FROM CMDLINE!\n");
        return 0;
    }
    if( fw_model == NULL ){
        gen_fw_automatic_detection = TRUE;
    }
    /** initialize ops **/
    if( argv[3] == NULL ){
        ops = (unsigned char *)malloc(sizeof("read"));
        strncpy(ops, "read", sizeof("read"));
    }else{
        ops = (unsigned char *)malloc(strlen(argv[3]));
        strncpy(ops, argv[3], strlen(argv[3]));
    }

    int fd = 0;
    fd = open(fw_name, O_RDONLY);
    if( fd < 0 ){
        DBGLOG("FAILED TO OPEN THE FILE!\n");
        return -3;
    }
    struct stat *fd_stat = (struct stat *)malloc(sizeof(struct stat));
    fstat(fd, fd_stat);
    unsigned long fw_size = fd_stat->st_size;
    unsigned char *fw_mapped_in_mem = (unsigned char *)mmap(
            NULL,
            fw_size,
            PROT_READ,
            MAP_FILE | MAP_SHARED,
            fd,
            0x00
            );
    enum mtk_fw_index_list declared_fw_type = switch_fw_type_from_cmdline(fw_model);
    DBGLOG("firmware is mapped at 0x%x with size: %ld\nthe firmware version is %s with corresponding code: %d\n",
            fw_mapped_in_mem,
            fw_size,
            fw_model,
            declared_fw_type
          );

    DBGLOG("actual operation on firmware; %s\n", convert_ops_to_string(ops));

    if( declared_fw_type >= MTK_WIFI_FW_TOTAL_NUMBER ){
        DBGLOG("invalid firmware type detected!\n");
        return -2;
    }
    if( mtk_fw_global_table[declared_fw_type].disassemble_firmware != NULL ){
        mtk_fw_global_table[declared_fw_type].disassemble_firmware(fw_mapped_in_mem, fw_size, 2, IMG_DL_IDX_N9_FW);
    }
    free(fd_stat);
    munmap(fw_mapped_in_mem, fw_size);
    close(fd);
    return 0;
}
