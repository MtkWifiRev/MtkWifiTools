/**
  Made by Daniel Wegemer, 2024
**/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <sys/ioctl.h>
#include <linux/wireless.h>

#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

typedef enum _ENUM_RF_AT_FUNCID_T {

    RF_AT_FUNCID_VERSION = 0,
    RF_AT_FUNCID_COMMAND,
    RF_AT_FUNCID_POWER,
    RF_AT_FUNCID_RATE,
    RF_AT_FUNCID_PREAMBLE,
    RF_AT_FUNCID_ANTENNA,
    RF_AT_FUNCID_PKTLEN,
    RF_AT_FUNCID_PKTCNT,
    RF_AT_FUNCID_PKTINTERVAL,
    RF_AT_FUNCID_TEMP_COMPEN,
    RF_AT_FUNCID_TXOPLIMIT,  //10
    RF_AT_FUNCID_ACKPOLICY,
    RF_AT_FUNCID_PKTCONTENT,
    RF_AT_FUNCID_RETRYLIMIT,
    RF_AT_FUNCID_QUEUE,
    RF_AT_FUNCID_BANDWIDTH,
    RF_AT_FUNCID_GI,
    RF_AT_FUNCID_STBC,
    RF_AT_FUNCID_CHNL_FREQ,
    RF_AT_FUNCID_RIFS,
    RF_AT_FUNCID_TRSW_TYPE,   //20
    RF_AT_FUNCID_RF_SX_SHUTDOWN,
    RF_AT_FUNCID_PLL_SHUTDOWN,
    RF_AT_FUNCID_SLOW_CLK_MODE,
    RF_AT_FUNCID_ADC_CLK_MODE,
    RF_AT_FUNCID_MEASURE_MODE,
    RF_AT_FUNCID_VOLT_COMPEN,
    RF_AT_FUNCID_DPD_TX_GAIN,
    RF_AT_FUNCID_DPD_MODE,
    RF_AT_FUNCID_TSSI_MODE,
    RF_AT_FUNCID_TX_GAIN_CODE,  //30
    RF_AT_FUNCID_TX_PWR_MODE,

    /* Query command */
    RF_AT_FUNCID_TXED_COUNT = 32,
    RF_AT_FUNCID_TXOK_COUNT,
    RF_AT_FUNCID_RXOK_COUNT,
    RF_AT_FUNCID_RXERROR_COUNT,
    RF_AT_FUNCID_RESULT_INFO,
    RF_AT_FUNCID_TRX_IQ_RESULT,
    RF_AT_FUNCID_TSSI_RESULT,
    RF_AT_FUNCID_DPD_RESULT,
    RF_AT_FUNCID_RXV_DUMP,
    RF_AT_FUNCID_RX_PHY_STATIS,
    RF_AT_FUNCID_MEASURE_RESULT,
    RF_AT_FUNCID_TEMP_SENSOR,
    RF_AT_FUNCID_VOLT_SENSOR,
    RF_AT_FUNCID_READ_EFUSE,
    RF_AT_FUNCID_RX_RSSI,

    /* Set command */
    RF_AT_FUNCID_SET_RX_DEF_ANT = 63,
    RF_AT_FUNCID_SET_DPD_RESULT = 64,
    RF_AT_FUNCID_SET_CW_MODE,
    RF_AT_FUNCID_SET_JAPAN_CH14_FILTER,
    RF_AT_FUNCID_WRITE_EFUSE,
    RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
    RF_AT_FUNCID_SET_MAC_SRC_ADDRESS,
    RF_AT_FUNCID_SET_RXOK_MATCH_RULE,
    RF_AT_FUNCID_SET_CHANNEL_BANDWIDTH = 71,
    RF_AT_FUNCID_SET_DATA_BANDWIDTH = 72,
    RF_AT_FUNCID_SET_PRI_SETTING = 73,
    RF_AT_FUNCID_SET_TX_ENCODE_MODE = 74,
    RF_AT_FUNCID_SET_J_MODE_SETTING = 75,

    /*ICAP command*/
    RF_AT_FUNCID_SET_ICAP_CONTENT = 80,
    RF_AT_FUNCID_SET_ICAP_MODE,
    RF_AT_FUNCID_SET_ICAP_STARTCAP,
    RF_AT_FUNCID_SET_ICAP_SIZE = 83,
    RF_AT_FUNCID_SET_ICAP_TRIGGER_OFFSET,
    RF_AT_FUNCID_QUERY_ICAP_DUMP_FILE = 85,
    RF_AT_FUNCID_QUERY_ICAP_STARTING_ADDR = 86,
    RF_AT_FUNCID_QUERY_ICAP_END_ADDR = 87,
    RF_AT_FUNCID_QUERY_ICAP_WAPPER_ADDR = 88,

    /*2G 5G Band*/
    RF_AT_FUNCID_SET_BAND = 90,

    /*Reset Counter*/
    RF_AT_FUNCID_RESETTXRXCOUNTER = 91,

    /*FAGC RSSI Path*/
    RF_AT_FUNCID_FAGC_RSSI_PATH = 92,

    /*Set RX Filter Packet Length*/
    RF_AT_FUNCID_RX_FILTER_PKT_LEN = 93,

    /*Tone*/
    RF_AT_FUNCID_SET_TONE_RF_GAIN = 96,
    RF_AT_FUNCID_SET_TONE_DIGITAL_GAIN = 97,
    RF_AT_FUNCID_SET_TONE_TYPE = 98,
    RF_AT_FUNCID_SET_TONE_DC_OFFSET = 99,
    RF_AT_FUNCID_SET_TONE_BW = 100,

    /*MT6632 Add*/
    RF_AT_FUNCID_SET_MAC_HEADER = 101,
    RF_AT_FUNCID_SET_SEQ_CTRL = 102,
    RF_AT_FUNCID_SET_PAYLOAD = 103,
    RF_AT_FUNCID_SET_DBDC_BAND_IDX = 104,
    RF_AT_FUNCID_SET_BYPASS_CAL_STEP = 105,

    /*Set RX Path*/
    RF_AT_FUNCID_SET_RX_PATH = 106,

    /*Set Frequency Offset*/
    RF_AT_FUNCID_SET_FRWQ_OFFSET = 107,

    /*Get Frequency Offset*/
    RF_AT_FUNCID_GET_FREQ_OFFSET = 108,

    /*Set RXV Debug Index*/
    RF_AT_FUNCID_SET_RXV_INDEX = 109,

    /*Set Test Mode DBDC Enable*/
    RF_AT_FUNCID_SET_DBDC_ENABLE = 110,

    /*Get Test Mode DBDC Enable*/
    RF_AT_FUNCID_GET_DBDC_ENABLE = 111,

    /*Set ICAP Ring Capture*/
    RF_AT_FUNCID_SET_ICAP_RING = 112,

    /*Set TX Path*/
    RF_AT_FUNCID_SET_TX_PATH = 113,

    /*Set Nss*/
    RF_AT_FUNCID_SET_NSS = 114,

    /*Set TX Antenna Mask*/
    RF_AT_FUNCID_SET_ANTMASK = 115,


    /*TMR set command*/
    RF_AT_FUNCID_SET_TMR_ROLE=116,
    RF_AT_FUNCID_SET_TMR_MODULE=117,
    RF_AT_FUNCID_SET_TMR_DBM=118,
    RF_AT_FUNCID_SET_TMR_ITER=119,

    /* Set ADC For IRR Feature */
    RF_AT_FUNCID_SET_ADC = 120,

    /* Set RX Gain For IRR Feature */
    RF_AT_FUNCID_SET_RX_GAIN = 121,

    /* Set TTG For IRR Feature */
    RF_AT_FUNCID_SET_TTG = 122,

    /* Set TTG ON/OFF For IRR Feature */
    RF_AT_FUNCID_TTG_ON_OFF = 123,

    /* Set TSSI for QA Tool Setting */
    RF_AT_FUNCID_SET_TSSI = 124,

    /* Set Recal Cal Step*/
    RF_AT_FUNCID_SET_RECAL_CAL_STEP = 125,

    /* Set iBF/eBF enable */
    RF_AT_FUNCID_SET_IBF_ENABLE = 126,
    RF_AT_FUNCID_SET_EBF_ENABLE = 127,

    /* Set MPS Setting */
    RF_AT_FUNCID_SET_MPS_SIZE = 128,
    RF_AT_FUNCID_SET_MPS_SEQ_DATA = 129,
    RF_AT_FUNCID_SET_MPS_PAYLOAD_LEN = 130,
    RF_AT_FUNCID_SET_MPS_PKT_CNT = 131,
    RF_AT_FUNCID_SET_MPS_PWR_GAIN = 132,
    RF_AT_FUNCID_SET_MPS_NSS = 133,
    RF_AT_FUNCID_SET_MPS_PACKAGE_BW = 134,
    RF_AT_FUNCID_SET_MPS_SYSTEM_BW = 135,
    RF_AT_FUNCID_GET_CH_TX_PWR_OFFSET = 136,
    RF_AT_FUNCID_SET_CH_TX_PWR_OFFSET = 137,
    RF_AT_FUNCID_GET_EEPROM_ADDR = 138,
    RF_AT_FUNCID_GET_FREQ_OFFSET_EEPROM_ADDR = 139,
    RF_AT_FUNCID_GET_AUTO_ISOLATION_VALUE = 140,
    RF_AT_FUNCID_SET_SINGLE_SKU = 141,
    RF_AT_FUNCID_SET_ABSPOWER = 142,
    /*query TX OK/ERR count at the same time*/
    RF_AT_FUNCID_GET_TX_DATA = 143,
    /*query RX OK/ERR count at the same time*/
    RF_AT_FUNCID_GET_RX_DATA = 144,
    /*get wifi test tool version that matched with FW*/
    RF_AT_FUNCID_GET_VERSION = 145,
    /*Force close calibration dump*/
    RF_AT_FUNCID_SET_CAL_DUMP_CONTROL = 146,
    RF_AT_FUNCID_SET_CHANNEL = 147,
    RF_AT_FUNCID_SET_RATEOFFSET = 155,  /* registered by ohter command, version controlled by FEATURE_SYNC_RATE_OFFSEST_EXT_SUPPORT*/
    RF_AT_FUNCID_SET_MU_AID = 157,
    RF_AT_FUNCID_SET_TX_HE_TB_TTRCR0 = 158,
    RF_AT_FUNCID_SET_TX_HE_TB_TTRCR1 = 159,
    RF_AT_FUNCID_SET_TX_HE_TB_TTRCR2 = 160,
    RF_AT_FUNCID_SET_TX_HE_TB_TTRCR3 = 161,
    RF_AT_FUNCID_SET_TX_HE_TB_TTRCR4 = 162,
    RF_AT_FUNCID_SET_TX_HE_TB_TTRCR5 = 163,
    RF_AT_FUNCID_SET_TX_HE_TB_TTRCR6 = 164,
    RF_AT_FUNCID_SET_SECURITY_MODE = 165,
    RF_AT_FUNCID_SET_HWTX_MODE = 167,
    RF_AT_FUNCID_SET_RATEOFFSET_EX = 170
} ENUM_RF_AT_FUNCID_T;


typedef enum _ENUM_RF_AT_COMMAND_T {
    RF_AT_COMMAND_STOPTEST = 0,
    RF_AT_COMMAND_STARTTX,
    RF_AT_COMMAND_STARTRX,
    RF_AT_COMMAND_RESET,
    RF_AT_COMMAND_OUTPUT_POWER,     /* Payload */
    RF_AT_COMMAND_LO_LEAKAGE,       /* Local freq is renamed to Local leakage */
    RF_AT_COMMAND_CARRIER_SUPPR,    /* OFDM (LTF/STF), CCK (PI,PI/2) */
    RF_AT_COMMAND_TRX_IQ_CAL,
    RF_AT_COMMAND_TSSI_CAL,
    RF_AT_COMMAND_DPD_CAL,
    RF_AT_COMMAND_CW,
    RF_AT_COMMAND_SINGLE_TONE = 15,
    RF_AT_COMMAND_AUTOISO = 17,
    RF_AT_COMMAND_NUM
} ENUM_RF_AT_COMMAND_T;

typedef enum _ENUM_CH_BAND_T {
    CH_BAND_2G_5G    = 0,
    CH_BAND_6G_7G    = 2,
    CH_BAND_NUM      = 3
} ENUM_CH_BAND_T, *P_ENUM_CH_BAND_T;

typedef enum _ENUM_RF_AT_RXOK_MATCH_RULE_T {
    RF_AT_RXOK_DISABLED = 0,
    RF_AT_RXOK_MATCH_RA_ONLY,
    RF_AT_RXOK_MATCH_TA_ONLY,
    RF_AT_RXOK_MATCH_RA_TA,
    RF_AT_RXOK_NUM
} ENUM_RF_AT_RXOK_MATCH_RULE_T, *P_ENUM_RF_AT_RXOK_MATCH_RULE_T;

struct PARAM_WIFI_LOG_LEVEL_UI {
	uint32_t u4Version;
	uint32_t u4Module;
	uint32_t u4Enable;
};

struct PARAM_WIFI_LOG_LEVEL {
	uint32_t u4Version;
	uint32_t u4Module;
	uint32_t u4Level;
};

struct PARAM_CUSTOM_MEM_DUMP_STRUCT {
	uint32_t u4Address;
	uint32_t u4Length;
	uint32_t u4RemainLength;
	uint32_t u4IcapContent;
	uint8_t ucFragNum;
};

struct PARAM_GET_WIFI_TYPE {
	struct net_device *prNetDev;
	uint8_t arWifiTypeName[8];
};

enum ENUM_WIFI_LOG_LEVEL_VERSION_T {
	ENUM_WIFI_LOG_LEVEL_VERSION_V1 = 1,
	ENUM_WIFI_LOG_LEVEL_VERSION_NUM
};

enum ENUM_WIFI_LOG_LEVEL_T {
	ENUM_WIFI_LOG_LEVEL_OFF = 0,
	ENUM_WIFI_LOG_LEVEL_DEFAULT,
	ENUM_WIFI_LOG_LEVEL_EXTREME,
	ENUM_WIFI_LOG_LEVEL_NUM
};

enum ENUM_WIFI_LOG_MODULE_T {
	ENUM_WIFI_LOG_MODULE_DRIVER = 0,
	ENUM_WIFI_LOG_MODULE_FW,
	ENUM_WIFI_LOG_MODULE_NUM,
};

enum ENUM_WIFI_LOG_LEVEL_SUPPORT_T {
	ENUM_WIFI_LOG_LEVEL_SUPPORT_DISABLE = 0,
	ENUM_WIFI_LOG_LEVEL_SUPPORT_ENABLE,
	ENUM_WIFI_LOG_LEVEL_SUPPORT_NUM
};

typedef enum _ENUM_IQ_T {
    IQ_0 = 0,
    IQ_1,
    IQ_NUM
} ENUM_IQ_T, *P_ENUM_IQ_T;

typedef union _IQ_FORMAT_T {
    struct {
        int16_t Value:14;
        int16_t Reserved:2;
    } Value14Bit;
    int16_t     Value16Bit;
} IQ_FORMAT_T, *P_IQ_FORMAT_T;


#ifndef IFNAMSIZ
	#define IFNAMSIZ		16
#endif

#ifndef true
	#define true 			1
#endif

#ifndef false
	#define false			0
#endif

// SIOCIWFIRSTPRIV 0x89F0 to 0x89FF
#define IOCTL_SET_INT                   (SIOCIWFIRSTPRIV + 0)
#define IOCTL_GET_INT                   (SIOCIWFIRSTPRIV + 1)
#define IOCTL_QA_TOOL_DAEMON            (SIOCIWFIRSTPRIV + 16)
#define IOCTL_SET_STRUCT                (SIOCIWFIRSTPRIV + 8)
#define IOCTL_GET_STRUCT                (SIOCIWFIRSTPRIV + 9)
#define PRIV_CMD_ACCESS_MCR     	19
#define PRIV_CMD_DUMP_MEM		27
#define CMD_OID_BUF_LENGTH		4096

#define PRIV_CMD_TEST_MAGIC_KEY 	2011
#define PRIV_CMD_OID                    15
#define CMD_SET_FW_LOG          	57
#define HQA_CMD_MAGIC_NO 		0x18142880
#define ICAP_TIMEOUT            	10
#define WIFI_TEST_CH_BW_20MHZ 		0
#define OID_CUSTOM_TEST_MODE            0xFFA0C901
#define OID_CUSTOM_TEST_ICAP_MODE       0xFFA0C913
#define OID_CUSTOM_ABORT_TEST_MODE      0xFFA0C906
#define OID_CUSTOM_MTK_WIFI_TEST_       0xFFA0C911
#define OID_IPC_WIFI_LOG_UI             0xFFA0CC01
#define OID_IPC_WIFI_LOG_LEVEL          0xFFA0CC02
#define OID_CUSTOM_MEM_DUMP             0xFFA0C807

#define MAX_ANTENNA_NUM 		1
//#define ICAP_SIZE       (15363)
#define ICAP_SIZE       		(122904)

//IQ_FORMAT_T rRawData[MAX_ANTENNA_NUM][IQ_NUM][ICAP_SIZE];
//uint32_t rRawData[IQ_NUM][ICAP_SIZE];

IQ_FORMAT_T 				rRawData[IQ_NUM][ICAP_SIZE];
int my_socket;
char ifname[] = "wlan0";
static uint8_t aucOidBuf[CMD_OID_BUF_LENGTH] = { 0 };

typedef struct _NDIS_TRANSPORT_STRUCT {
    uint32_t    ndisOidCmd;
    uint32_t    inNdisOidlength;
    uint32_t    outNdisOidLength;
    uint8_t     ndisOidContent[16];
} NDIS_TRANSPORT_STRUCT, *P_NDIS_TRANSPORT_STRUCT;

typedef struct _PARAM_MTK_WIFI_TEST_STRUC_T {
    uint32_t u4FuncIndex;
    uint32_t u4FuncData;
} PARAM_MTK_WIFI_TEST_STRUC_T, *P_PARAM_MTK_WIFI_TEST_STRUC_T;

typedef struct _PARAM_CUSTOM_MEM_DUMP_STRUCT_T {
	uint32_t u4Address;
	uint32_t u4Length;
	uint32_t u4RemainLength;
	uint8_t ucFragNum;
} PARAM_CUSTOM_MEM_DUMP_STRUCT_T, *P_PARAM_CUSTOM_MEM_DUMP_STRUCT_T;

struct _mtk_util_iwreq_memdump {
    union
    {
        char ifrn_name[IFNAMSIZ];
    } d;
    union iwreq_data u;
    uint8_t buffer;
//    uint32_t u4Address;
//    uint32_t u4Lenght;
}__attribute__((packed));

struct HQA_CMD_FRAME {
	uint32_t MagicNo;
	uint16_t Type;
	uint16_t Id;
	uint16_t Length;
	uint16_t Sequence;
	uint8_t Data[2048];
}__attribute__((packed));

typedef struct _ICAP_CTRL_T {
    int i4Channel;
    uint32_t u4Cbw;
    uint32_t u4BwMhz;
    uint32_t u4RxPath;
} ICAP_CTRL_T, *P_ICAP_CTRL_T;

struct _mtk_util_iwreq_data {
    char                name[IFNAMSIZ];
    struct iw_point	essid;		/* Extended network name */
    struct iw_param	nwid;		/* network id (or domain - the cell) */
    struct iw_freq	freq;		/* frequency or channel : * 0-1000 = channel * > 1000 = frequency in Hz */

    struct iw_param	sens;		/* signal level threshold */
    struct iw_param	bitrate;	/* default bit rate */
    struct iw_param	txpower;	/* default transmit power */
    struct iw_param	rts;		/* RTS threshold */
    struct iw_param	frag;		/* Fragmentation threshold */
    __u32		mode;		/* Operation mode */
    struct iw_param	retry;		/* Retry limits & lifetime */

    struct iw_point	encoding;	/* Encoding stuff : tokens */
    struct iw_param	power;		/* PM duration/timeout */
    struct iw_quality qual;		/* Quality part of statistics */

    struct sockaddr	ap_addr;	/* Access point address */
    struct sockaddr	addr;		/* Destination address (hw/mac) */

    struct iw_param	param;		/* Other small parameters */
    struct iw_point	data;		/* Other large parameters */
};

void
hexdump(const char *desc, const void *addr, int len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != 0)
        printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

void
WIFI_TEST_init() {
    if ((my_socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
            printf("error opening socket\n");
    }
}

void
WIFI_TEST_deinit() {
    close(my_socket);
}

int
WIFI_TEST_SetIcapStartStop(P_ICAP_CTRL_T prIcapCtrl, uint32_t start_stop) {
    uint32_t u4Control;
    uint32_t u4Trigger;
    uint32_t u4RingCapEn;
    uint32_t u4TriggerEvent;
    uint32_t u4CaptureNode;
    uint32_t u4CaptureLen;
    uint32_t u4CapStopCycle;
    uint32_t u4BW;
    uint32_t u4MacTriggerEvent;
    uint32_t aucSourceAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint32_t band;
    struct HQA_CMD_FRAME cmd_frame;
    struct iwreq mtk_iwr;
    int ret;

    memset(&mtk_iwr, 0, sizeof(struct iwreq));
    memset(&cmd_frame, 0, sizeof(struct HQA_CMD_FRAME));

    cmd_frame.MagicNo = htonl(HQA_CMD_MAGIC_NO);
    cmd_frame.Type = htons(0x0008);
    cmd_frame.Id = htons(0x1580); //HQA_CapWiFiSpectrum
    cmd_frame.Length = htons(4 * 8); /* NOTE: Needs to be ajusted for MacTriggerEvent */
    cmd_frame.Sequence = htons(0);

    u4Control = htonl(1); //ICAP start
    memcpy(cmd_frame.Data + 4 * 0, &u4Control, 4);
    u4Trigger = htonl(start_stop); //ICAP start if 1; shutdown if 0
    memcpy(cmd_frame.Data + 4 * 1, &u4Trigger, 4);
    u4RingCapEn = htonl(0);
    memcpy(cmd_frame.Data + 4 * 2, &u4RingCapEn, 4);
    u4TriggerEvent = htonl(1); // 0 by default, 1 means "free_run"!
    memcpy(cmd_frame.Data + 4 * 3, &u4TriggerEvent, 4);
    u4CaptureNode = htonl(8); //?
    memcpy(cmd_frame.Data + 4 * 4, &u4CaptureNode, 4);
    u4CaptureLen = htonl(800); //400 by default
    memcpy(cmd_frame.Data + 4 * 5, &u4CaptureLen, 4);
    u4CapStopCycle = htonl(800); //400 by default
    memcpy(cmd_frame.Data + 4 * 6, &u4CapStopCycle, 4);
    u4BW = htonl(prIcapCtrl->u4Cbw);
    memcpy(cmd_frame.Data + 4 * 7, &u4BW, 4);

    /* NOTE: the follwoing attributes might not be needed for newer chips/drivers:
     * - MacTriggerEvent
     * - aucSourceAddress
     * */

    /*
    u4MacTriggerEvent = htonl(0);
    memcpy(cmd_frame.Data + 4 * 8, &u4MacTriggerEvent, 4);
    memcpy(cmd_frame.Data + 4 * 9, &aucSourceAddress, sizeof(aucSourceAddress));
    */

    /* configure struct iwreq */
    mtk_iwr.u.data.pointer = &cmd_frame;
    mtk_iwr.u.data.length = sizeof(struct HQA_CMD_FRAME);
    mtk_iwr.u.data.flags = 0;

    snprintf(mtk_iwr.ifr_name, sizeof(mtk_iwr.ifr_name), "%s", ifname);
    ret = ioctl(my_socket, IOCTL_QA_TOOL_DAEMON, &mtk_iwr);
    printf("ioctl ret: %d\n", ret);

    return ret;
}

uint16_t
WIFI_TEST_GetIcapStatus(uint16_t *pu2Status) {
    uint32_t u4Control;
    struct iwreq mtk_iwr;
    struct HQA_CMD_FRAME cmd_frame;
    int s;
    int ret;

    memset(&cmd_frame, 0, sizeof(struct HQA_CMD_FRAME));
    memset(&mtk_iwr, 0, sizeof(struct iwreq));

    cmd_frame.MagicNo = htonl(HQA_CMD_MAGIC_NO);
    cmd_frame.Type = htons(0x0008);
    cmd_frame.Id = htons(0x1580); //HQA_CapWiFiSpectrum
    cmd_frame.Length = htons(4 * 1);
    cmd_frame.Sequence = htons(0);

    u4Control = htonl(2); //get status
    memcpy(cmd_frame.Data + 4 * 0, &u4Control, 4);

    /* configure struct iwreq */
    mtk_iwr.u.data.pointer = &cmd_frame;
    mtk_iwr.u.data.length = sizeof(struct HQA_CMD_FRAME);
    mtk_iwr.u.data.flags = 0;

    snprintf(mtk_iwr.ifr_name, sizeof(mtk_iwr.ifr_name), "%s", ifname);
    ret = ioctl(my_socket, IOCTL_QA_TOOL_DAEMON, &mtk_iwr);
    printf("ioctl ret: %d\n", ret);
    if(ret != 0) {
        return false;
    }

    memcpy(pu2Status, cmd_frame.Data, 2);
    *pu2Status = htons(*pu2Status);
    printf("WIFI_TEST_GetIcapStatus: %d\n", *pu2Status);

    return true;
}

int
WIFI_TEST_set(uint32_t functionIndex, uint32_t data) {
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;
    P_PARAM_MTK_WIFI_TEST_STRUC_T prTestStruct;
    int s;
    int ret;

    prTestStruct = (P_PARAM_MTK_WIFI_TEST_STRUC_T)rNdisStruct.ndisOidContent;

    // zeroize
    memset(&wrq, 0, sizeof(struct iwreq));

    // configure TEST_STRUCT
    prTestStruct->u4FuncIndex = functionIndex;
    prTestStruct->u4FuncData = data;

    // configure NDIS_TRANSPORT_STRUC
    rNdisStruct.ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST_;
    rNdisStruct.inNdisOidlength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);
    rNdisStruct.outNdisOidLength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);


    // configure struct iwreq
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", ifname);
    ret = ioctl(my_socket, IOCTL_SET_STRUCT, &wrq);
    printf("ioctl ret: %d\n", ret);

    return ret;
}

int
WIFI_TEST_RxStart() {
    return WIFI_TEST_set(RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_STARTRX);
}

int
WIFI_TEST_RxStop() {
    return WIFI_TEST_set(RF_AT_FUNCID_COMMAND, RF_AT_COMMAND_STOPTEST);
}

int
WIFI_TEST_Channel_Ex(int channel,uint32_t ch_band) {
    uint32_t u4Freq; //strange

    /* Legacy path */
    if (ch_band == CH_BAND_2G_5G){

        if(channel < 0) {
            return false; /* invalid channel number */
        }
        /* 2.4GHz band */
        else if(channel <= 13) {
            u4Freq = 2412000 + (channel - 1) * 5000;
        }
        else if(channel == 14) {
            u4Freq = 2484000;
        }
        /* 5GHz band */
        else if(channel >= 36) {
            u4Freq = 5180000 + (channel - 36) * 5000;
        }
        else {
            return false; /* invalid channel number */
        }

        return WIFI_TEST_set(RF_AT_FUNCID_CHNL_FREQ, u4Freq);
    } else {
        //not supported (yet)
        return false;
    }

    return WIFI_TEST_set(RF_AT_FUNCID_CHNL_FREQ, u4Freq);
}

int
WIFI_TEST_GetIcapDataFragment(int32_t u4WF, uint32_t u4IQ, uint32_t *pu4Len, uint32_t *pu4Buf) {
    int retval;
    struct iwreq wrq;
    uint32_t u4Control;
    uint32_t u4DataLen;
    uint32_t u4WFNum;
    uint32_t u4IQNum;
    int s;

    struct HQA_CMD_FRAME HqaCmdFrame;

    printf("Enter WIFI_TEST_GetIcapDataFragment()\n");


    memset(&HqaCmdFrame, 0, sizeof(struct HQA_CMD_FRAME));

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    HqaCmdFrame.MagicNo = htonl(HQA_CMD_MAGIC_NO);
    HqaCmdFrame.Type = htons(0x0008);
    HqaCmdFrame.Id = htons(0x1580);     // HQA_CapWiFiSpectrum
    HqaCmdFrame.Length = htons(4 * 3);
    HqaCmdFrame.Sequence = htons(0);

    u4Control = htonl(3); //getICapIQData
    memcpy(HqaCmdFrame.Data + 4 * 0, &u4Control, 4);
    u4WFNum = htonl(u4WF);
    memcpy(HqaCmdFrame.Data + 4 * 1, &u4WFNum, 4);
    u4IQNum = htonl(u4IQ);
    memcpy(HqaCmdFrame.Data + 4 * 2, &u4IQNum, 4);

    /* configure struct iwreq */
    wrq.u.data.pointer = &HqaCmdFrame;
    wrq.u.data.length = sizeof(struct HQA_CMD_FRAME);
    wrq.u.data.flags = 0;

    snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", ifname);
    retval = ioctl(my_socket, IOCTL_QA_TOOL_DAEMON, &wrq);
    printf("ioctl ret: %d\n", retval);
    if (retval != 0) return false;

    //hexdump(NULL, HqaCmdFrame.Data, sizeof(u4DataLen));
    memcpy(&u4DataLen, HqaCmdFrame.Data + 2 + 4 * 3, sizeof(u4DataLen));
    u4DataLen = htonl(u4DataLen);

    //hexdump(NULL, HqaCmdFrame.Data, u4DataLen * sizeof(uint32_t));
    memcpy(pu4Buf, HqaCmdFrame.Data + 2 + 4 * 4, u4DataLen * sizeof(uint32_t));

    for (unsigned int i = 0; i < u4DataLen; i++) {
        pu4Buf[i] = ntohl(pu4Buf[i]);
    }

    *pu4Len = u4DataLen;

    return true;
}

int
WIFI_TEST_GetIcapData() {
    uint32_t *prIcapData;
    uint32_t u4IQ;
    uint32_t IQNumberCount;
    uint32_t IQNumberTotalCount;
    uint32_t u4Loop = 0; //antenna loop
    int retval;

    prIcapData = malloc(ICAP_SIZE * sizeof(uint32_t));
    if(prIcapData == NULL) {
        return false;
    }

    memset(rRawData, 0, sizeof(rRawData));

    printf("Enter WIFI_TEST_GetIcapData()\n");

    for(u4IQ = 0; u4IQ < IQ_NUM; u4IQ++) {  // I, Q
        memset(prIcapData, 0, ICAP_SIZE);
        IQNumberTotalCount = 0;
        printf("\nIQ, now: %d, out of: %d\n\n", u4IQ, IQ_NUM-1);

        do {
            retval = WIFI_TEST_GetIcapDataFragment(u4Loop, u4IQ, &IQNumberCount, prIcapData + IQNumberTotalCount);
            if (retval == 0) {
                printf("ERROR: freeing prIcapData!\n");
                free(prIcapData);
                return false;
            }

            IQNumberTotalCount += IQNumberCount;
            printf("I/Q: %d, Got %d IQ Data, Total Count: %d\n", u4IQ, IQNumberCount, IQNumberTotalCount);
        } while (IQNumberCount > 0);

        for(uint32_t i = 0; i < IQNumberTotalCount; i++) {
            //printf("rRawData(Max) :: u4IQ: %d, ICAP SIZE: %d\n", IQ_NUM, ICAP_SIZE);
            //printf("rRawData(Cur) :: i: %d, total count: %d\n", i, IQNumberTotalCount);
            //printf("sizeof(prIcapData): %lu, prIcapData[%d]: 0x%x\n", ICAP_SIZE * sizeof(uint32_t), i, prIcapData[i]);
            //rRawData[MAX_ANTENNA_NUM][u4IQ][i].Value16Bit = prIcapData[i];
            rRawData[u4IQ][i].Value16Bit = prIcapData[i];
        }
    }

    for (unsigned int i = 0; i < IQNumberTotalCount; i++) {
        printf("\tIQ Data[%3d]: %+04d\t%+04d\t\n", i,
            rRawData[IQ_0][i].Value14Bit.Value,
            rRawData[IQ_1][i].Value14Bit.Value);
    }

    free(prIcapData);

    printf("GetIcapData, returning success!\n");
    return true;
}


int
WIFI_TEST_SetJMode(uint32_t jModeSetting) {
    int ret;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;
    P_PARAM_MTK_WIFI_TEST_STRUC_T prTestStruct;
    int s;

    prTestStruct = (P_PARAM_MTK_WIFI_TEST_STRUC_T)rNdisStruct.ndisOidContent;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    /* configure TEST_STRUCT */
    prTestStruct->u4FuncIndex = RF_AT_FUNCID_SET_J_MODE_SETTING;
    prTestStruct->u4FuncData = jModeSetting;

    /* configure NDIS_TRANSPORT_STRUC */
    rNdisStruct.ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST_;
    rNdisStruct.inNdisOidlength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);
    rNdisStruct.outNdisOidLength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);

    /* configure struct iwreq */
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", ifname);
    ret = ioctl(my_socket, IOCTL_SET_STRUCT, &wrq);
    printf("ioctl ret: %d\n", ret);

    return ret;
}

int icap_forever(P_ICAP_CTRL_T prIcapCtrl) {
        uint16_t u2Status;
        uint16_t u2WaitTime = 0;
	int ret = 0;
        ret = WIFI_TEST_RxStart();
        ret = WIFI_TEST_SetIcapStartStop(prIcapCtrl, 1);

        while(1) {
            ret = WIFI_TEST_GetIcapStatus(&u2Status);
	    if (ret == 0) return 1;

	    if (!u2Status) {  // ICAP done
		WIFI_TEST_RxStop();
		break;
	    }

	    if (u2WaitTime++ < ICAP_TIMEOUT) {
                printf("working...%d\n", u2WaitTime);
		sleep(1);  // 1 sec
	    } else {
		printf("timeout\n");
		return 1;
	    }
        }

        ret = WIFI_TEST_GetIcapData();
        ret = WIFI_TEST_SetIcapStartStop(prIcapCtrl, 0);
        return ret;
}

int main(int argc, char *argv[]){

    if(argc < 2) {
        printf("please provide correct arguments\n");
        exit(1);
    }

    //
    // Start Test Mode
    //

    else if(strcmp(argv[1], "-i1") == 0) {
        int ret 					= 0;
        struct iwreq wrq				= { 0 };
        NDIS_TRANSPORT_STRUCT rNdisStruct		= { 0 };

        WIFI_TEST_init();

        /* zeroize */
        memset(&wrq, 0, sizeof(struct iwreq));
        rNdisStruct.ndisOidCmd 				= OID_CUSTOM_TEST_MODE;

        rNdisStruct.inNdisOidlength 			= 0;
        rNdisStruct.outNdisOidLength 			= 0;

        /* configure struct iwreq */
        wrq.u.data.pointer 				= &rNdisStruct;
        wrq.u.data.length 				= sizeof(NDIS_TRANSPORT_STRUCT);
        wrq.u.data.flags 				= PRIV_CMD_OID;

        snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", ifname);

        ret 						= ioctl(my_socket, IOCTL_SET_STRUCT, &wrq);

	printf("ioctl ret: %d\n", ret);

        WIFI_TEST_deinit();
    }

    //
    // Start ICAP Test Mode
    //

    else if(strcmp(argv[1], "-i11") == 0) {
        int ret 					= 0;
        struct iwreq wrq				= { 0 };
        NDIS_TRANSPORT_STRUCT rNdisStruct		= { 0 };

        WIFI_TEST_init();

        /* zeroize */
        memset(&wrq, 0, sizeof(struct iwreq));
        rNdisStruct.ndisOidCmd 				= OID_CUSTOM_TEST_ICAP_MODE;

        rNdisStruct.inNdisOidlength 			= 0;
        rNdisStruct.outNdisOidLength			= 0;

        /* configure struct iwreq */
        wrq.u.data.pointer				= &rNdisStruct;
        wrq.u.data.length 				= sizeof(NDIS_TRANSPORT_STRUCT);
        wrq.u.data.flags 				= PRIV_CMD_OID;

        snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", ifname);

        ret 						= ioctl(my_socket, IOCTL_SET_STRUCT, &wrq);

	printf("ioctl START TEST MODE ret: %d\n", ret);

        WIFI_TEST_deinit();
    }

    //
    // Start CSI sending via QA Mode
    //
    else if(strcmp(argv[1], "-i2") == 0) {

        WIFI_TEST_init();

        //  HQA_CMD_TABLE []
        //      struct HQA_ICAP_CMDS[],
        //          QA_CapWiFiSpectrum,	    /* 0x1580 */
        //      sizeof(HQA_ICAP_CMDS) / sizeof(HQA_CMD_HANDLER),
        //      0x1580

        //WIFI_TEST_SetJMode(0);

	int ret 					= 0;
        ICAP_CTRL_T prIcapCtrl				= { 0 };
        //uint16_t u2Status;
        int channel 					= 1;
        int rx_path 					= 0x1;
        uint32_t u4DefaultRxPath 			= 0x00010000; // WF0

        prIcapCtrl.i4Channel 				= channel;
        prIcapCtrl.u4Cbw 				= WIFI_TEST_CH_BW_20MHZ;
        prIcapCtrl.u4BwMhz 				= 20;
        prIcapCtrl.u4RxPath 				= rx_path;

        WIFI_TEST_Channel_Ex(channel, CH_BAND_2G_5G);

        // WIFI_TEST_SetCbw(..);
        WIFI_TEST_set(RF_AT_FUNCID_SET_CHANNEL_BANDWIDTH , WIFI_TEST_CH_BW_20MHZ);

        // WIFI_TEST_SetRX()
        WIFI_TEST_set(RF_AT_FUNCID_SET_RXOK_MATCH_RULE, RF_AT_RXOK_DISABLED);

        // WIFI_TEST_SetRxPath(..);
        WIFI_TEST_set(RF_AT_FUNCID_SET_RX_PATH, u4DefaultRxPath);

        ret 						= WIFI_TEST_RxStart();
        if(ret) {
            printf("Error during WIFI_TEST_RxStart()\n");
            return 1;
        }

        while(true) {
            ret 					= icap_forever(&prIcapCtrl);
            //sleep(1);
        }

        /*
        ret = WIFI_TEST_SetIcapStartStop(&prIcapCtrl, 1);

        while(1) {
            ret = WIFI_TEST_GetIcapStatus(&u2Status);
	    if (ret == 0) return 1;

	    if (!u2Status) {  // ICAP done
		WIFI_TEST_RxStop();
		break;
	    }

	    if (u2WaitTime++ < ICAP_TIMEOUT) {
                printf("working...%d\n", u2WaitTime);
		sleep(1);  // 1 sec
	    } else {
		printf("timeout\n");
		return 1;
	    }
        }

        ret = WIFI_TEST_GetIcapData();
        */

        printf("\tGetIcapData result: %d (1: Success, 0: Fail)\n", ret);

	printf("errno: %d\n", errno);
	printf("errno str: %s\n", strerror(errno));

        WIFI_TEST_deinit();
    }

    //
    // Stop ICAP and Test Mode
    //

    else if(strcmp(argv[1], "-i3") == 0) {
        int ret 					= 0;
        struct iwreq wrq				= { 0 };
        NDIS_TRANSPORT_STRUCT rNdisStruct		= { 0 };
        ICAP_CTRL_T prIcapCtrl				= { 0 };

        prIcapCtrl.i4Channel 				= 1;
        prIcapCtrl.u4Cbw 				= WIFI_TEST_CH_BW_20MHZ;
        prIcapCtrl.u4BwMhz 				= 20;
        prIcapCtrl.u4RxPath 				= 0x1;

        WIFI_TEST_init();

        //Stop ICAP
        ret 						= WIFI_TEST_SetIcapStartStop(&prIcapCtrl, 0);

        /* zeroize */
        memset(&wrq, 0, sizeof(struct iwreq));
        rNdisStruct.ndisOidCmd 				= OID_CUSTOM_ABORT_TEST_MODE;

        rNdisStruct.inNdisOidlength 			= 0;
        rNdisStruct.outNdisOidLength 			= 0;

        /* configure struct iwreq */
        wrq.u.data.pointer 				= &rNdisStruct;
        wrq.u.data.length 				= sizeof(NDIS_TRANSPORT_STRUCT);
        wrq.u.data.flags 				= PRIV_CMD_OID;

        snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", ifname);

        ret 						= ioctl(my_socket, IOCTL_SET_STRUCT, &wrq);

	printf("ioctl STOP TEST MODE ret: %d\n", ret);
        WIFI_TEST_deinit();
    }

    //
    // Query Log Level Support
    //

    else if(strcmp(argv[1], "-ql") == 0) {
        int ret 					= 0;
        struct iwreq wrq				= { 0 };
        struct PARAM_WIFI_LOG_LEVEL *logLevel		= NULL;
        NDIS_TRANSPORT_STRUCT rNdisStruct		= { 0 };

        WIFI_TEST_init();

        logLevel 					= (struct PARAM_WIFI_LOG_LEVEL *)rNdisStruct.ndisOidContent;

        /* zeroize */
        memset(&wrq, 0, sizeof(struct iwreq));

        logLevel->u4Version 				= 1;
        logLevel->u4Module 				= 1; //0: Driver, 1: FW
        logLevel->u4Level 				= 0;

        rNdisStruct.ndisOidCmd 				= OID_IPC_WIFI_LOG_UI;
        rNdisStruct.inNdisOidlength 			= sizeof(struct PARAM_WIFI_LOG_LEVEL_UI);
        rNdisStruct.outNdisOidLength 			= sizeof(struct PARAM_WIFI_LOG_LEVEL_UI);

        /* configure struct iwreq */
        wrq.u.data.pointer 				= &rNdisStruct;
        wrq.u.data.length 				= sizeof(NDIS_TRANSPORT_STRUCT);
        wrq.u.data.flags 				= PRIV_CMD_OID;

        snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", ifname);

        ret 						= ioctl(my_socket, IOCTL_GET_STRUCT, &wrq);

	printf("ioctl QUERY LOG LEVEL ret: %d\n", ret);
        WIFI_TEST_deinit();

    }
    //
    // Get Log Level
    //

    else if(strcmp(argv[1], "-l") == 0) {
        int ret 					= 0;
        struct iwreq wrq				= { 0 };
        NDIS_TRANSPORT_STRUCT rNdisStruct		= { 0 };
        struct PARAM_WIFI_LOG_LEVEL *logLevel		= NULL;

        WIFI_TEST_init();

        logLevel 					= (struct PARAM_WIFI_LOG_LEVEL *)rNdisStruct.ndisOidContent;

        /* zeroize */
        memset(&wrq, 0, sizeof(struct iwreq));

        logLevel->u4Version 				= 1;
        logLevel->u4Module 				= 1; //0: Driver, 1: FW
        logLevel->u4Level 				= 0;

        rNdisStruct.ndisOidCmd 				= OID_IPC_WIFI_LOG_LEVEL;
        rNdisStruct.inNdisOidlength 			= sizeof(struct PARAM_WIFI_LOG_LEVEL);
        rNdisStruct.outNdisOidLength			= sizeof(struct PARAM_WIFI_LOG_LEVEL);

        /* configure struct iwreq */
        wrq.u.data.pointer 				= &rNdisStruct;
        wrq.u.data.length 				= sizeof(NDIS_TRANSPORT_STRUCT);
        wrq.u.data.flags 				= PRIV_CMD_OID;

        snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", ifname);

        ret 						= ioctl(my_socket, IOCTL_GET_STRUCT, &wrq);

	printf("ioctl GET LOG LEVEL ret: %d\n", ret);
        WIFI_TEST_deinit();

    }

    //
    // Set Log Level
    //

    else if(strcmp(argv[1], "-sl") == 0) {
	int ret 					= 0;
        struct iwreq wrq				= { 0 };
        struct PARAM_WIFI_LOG_LEVEL *logLevel		= NULL;
        NDIS_TRANSPORT_STRUCT rNdisStruct		= { 0 };

        WIFI_TEST_init();

        logLevel 					= (struct PARAM_WIFI_LOG_LEVEL *)rNdisStruct.ndisOidContent;

        /* zeroize */
        memset(&wrq, 0, sizeof(struct iwreq));

        logLevel->u4Version 				= 1;
        logLevel->u4Module 				= 0; //0: Driver, 1: FW
        logLevel->u4Level 				= 2; //1: More, 2: Extreme

        rNdisStruct.ndisOidCmd 				= OID_IPC_WIFI_LOG_LEVEL;
        rNdisStruct.inNdisOidlength 			= sizeof(struct PARAM_WIFI_LOG_LEVEL);
        rNdisStruct.outNdisOidLength 			= sizeof(struct PARAM_WIFI_LOG_LEVEL);

        /* configure struct iwreq */
        wrq.u.data.pointer 				= &rNdisStruct;
        wrq.u.data.length 				= sizeof(NDIS_TRANSPORT_STRUCT);
        wrq.u.data.flags 				= PRIV_CMD_OID;

        snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", ifname);

        ret 						= ioctl(my_socket, IOCTL_SET_STRUCT, &wrq);

	printf("ioctl SET LOG LEVEL ret: %d\n", ret);
        WIFI_TEST_deinit();

    }

    //
    // Dump Memory
    //

    else if(strcmp(argv[1], "-d") == 0) {
        int ret 					= 0;
        struct iwreq wrq				= { 0 };
        struct PARAM_CUSTOM_MEM_DUMP_STRUCT *memDump	= NULL;
        NDIS_TRANSPORT_STRUCT rNdisStruct		= { 0 };

        WIFI_TEST_init();

        memDump 					= (struct PARAM_CUSTOM_MEM_DUMP_STRUCT *)rNdisStruct.ndisOidContent;

        /* zeroize */
        memset(&wrq, 0, sizeof(struct iwreq));

        memDump->u4Address 				= 0xf021faec;
        memDump->u4Length 				= 0x10;

        rNdisStruct.ndisOidCmd 				= OID_CUSTOM_MEM_DUMP;
        rNdisStruct.inNdisOidlength 			= sizeof(struct PARAM_CUSTOM_MEM_DUMP_STRUCT);
        rNdisStruct.outNdisOidLength 			= sizeof(struct PARAM_CUSTOM_MEM_DUMP_STRUCT);

        /* configure struct iwreq */
        wrq.u.data.pointer 				= &rNdisStruct;
        wrq.u.data.length 				= sizeof(NDIS_TRANSPORT_STRUCT);
        wrq.u.data.flags 				= PRIV_CMD_OID;

        snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", ifname);

        ret 						= ioctl(my_socket, IOCTL_GET_STRUCT, &wrq);

	printf("ioctl DUMP MEM ret: %d\n", ret);
        WIFI_TEST_deinit();

    }

    //
    // Print Log
    //

    else if(strcmp(argv[1], "-p") == 0) {

        int fd						= 0;
        int ret						= 0;
        struct pollfd pfd				= { 0 };
        char fw_console[4096]				= { 0 };

        fd 						= open("/dev/fw_log_wifi", O_RDWR | O_NONBLOCK);

	if( fd < 0 ){
            perror("fd open error");
            exit(1);
	}

        pfd.fd						= fd;
        pfd.events 					= ( POLLIN | POLLRDNORM );

        while(1) {
            ret 					= poll(&pfd, (unsigned long)1, 100);   //wait for 5secs

            if( ret < 0 ) {
                perror("poll error");
                exit(1);
            }

            if( ( pfd.revents & POLLIN )  == POLLIN ) {
                read(pfd.fd, &fw_console, sizeof(fw_console));
                //printf("FW Console: '%s'\n", fw_console);
                hexdump(NULL, &fw_console, sizeof(fw_console));
                printf("\n\n");
            }
        }
    } else {
    	perror("unkown paramter!\n");
    }

    return 0;
}
