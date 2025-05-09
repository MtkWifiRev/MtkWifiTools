/**
	Made by Edoardo Mantovani, 2025
	Simple utility for auto-detecting a Mediatek wireless card attached to the device
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

/** start of the custom part **/
#define IEEE80211_DEBUGFS		"/sys/kernel/debug/ieee80211/"
#define IO_PATH                         "/sys/kernel/debug/ieee80211/%s/mt76/%s"

#define REGIDX                          "regidx"
#define REGVAL                          "regval"

#define REGIDX_MAX_LEN                  sizeof(IO_PATH) + sizeof(REGIDX)
#define REGVAL_MAX_LEN                  sizeof(IO_PATH) + sizeof(REGVAL)

int main(int argc, char *argv[]){
        unsigned char REGIDX_PATH[REGIDX_MAX_LEN];
        unsigned char REGVAL_PATH[REGVAL_MAX_LEN];

        memset(REGIDX_PATH, 0x00, REGIDX_MAX_LEN);
        memset(REGVAL_PATH, 0x00, REGVAL_MAX_LEN);

        signed   int  OPEN_FD   = 0x00;

	DIR *dir		= NULL;
	struct dirent *dirnt	= NULL;

	dir			= opendir(IEEE80211_DEBUGFS);

	if (dir) {
    		while ((dirnt = readdir(dir)) != NULL) {
			if( strstr(dirnt->d_name, "phy") != NULL ){
				snprintf(REGIDX_PATH, REGIDX_MAX_LEN, IO_PATH, dirnt->d_name, REGIDX);
				OPEN_FD	= open(REGIDX_PATH, O_RDWR);
				if( OPEN_FD < 0 ){
					continue;
				}else{
      					printf("%s is a mt76 driver!\n", REGIDX_PATH);
					close(OPEN_FD);
				}
			}
    		}
    		closedir(dir);
  	}

	return 0;
}

