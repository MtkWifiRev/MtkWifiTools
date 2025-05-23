/**
	Made by Edoardo Mantovani, 2025
	Simple tool which permits to control the character device under /dev/fw_wifi_log

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/ioctl.h>

#define WIFI_FW_LOG_IOC_MAGIC        	(0xfc)
#define WIFI_FW_LOG_IOCTL_ON_OFF     	_IOW(WIFI_FW_LOG_IOC_MAGIC, 0, int)
#define WIFI_FW_LOG_IOCTL_SET_LEVEL  	_IOW(WIFI_FW_LOG_IOC_MAGIC, 1, int)
#define WIFI_FW_LOG_IOCTL_GET_VERSION  	_IOR(WIFI_FW_LOG_IOC_MAGIC, 2, char*)	/** not useful for us **/

#define WIFI_FW_LOG_CMD_ON_OFF        	0x00
#define WIFI_FW_LOG_CMD_SET_LEVEL     	0x01

#define DIE(string)			printf(string); exit(0);


#ifdef USE_HEXDUMP
static void hexdump(const char *desc, const void *addr, int len){
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
#endif

int main(int argc, char *argv[]){
	if( geteuid() != 0 ){
		DIE("run the program with the root permissions!\n");
	}
	if( argc < 2 ){
		goto ERROR_;
	}

	unsigned char *current_cmd	= argv[1];
	unsigned char *level_ptr	= NULL;
	unsigned int   log_level	= 0x00;
	signed   int   fw_log_fd	= 0x00;
	signed   int   ioctl_ret	= 0x00;
	signed   int   poll_ret		= 0x00;
	unsigned int   ioctl_log_idx	= 0x00;

	struct pollfd 	pfd		= {};
        unsigned char fw_log[4096]	= {};

	memset(&pfd,	0x00,		sizeof(struct pollfd));
	memset(fw_log,	0x00,		sizeof(fw_log));

	fw_log_fd			= open("/dev/fw_log_wifi", O_RDONLY);

	if( fw_log_fd < 0 ){
		DIE("cannot open /dev/fw_log_wifi !\n");
	}

	if( strcmp(current_cmd, "on")   == 0 ){
		ioctl_ret		= ioctl(fw_log_fd, WIFI_FW_LOG_IOCTL_ON_OFF, 1);
	}else if(strcmp(current_cmd, "off") == 0 ){
		ioctl_ret		= ioctl(fw_log_fd, WIFI_FW_LOG_IOCTL_ON_OFF, 0);
	}else if(strstr(current_cmd, "set_level") != NULL ){
		strtok(current_cmd, "=");
		level_ptr		= strtok(NULL, "");
		if( level_ptr != NULL ){
			ioctl_log_idx	= atoi(level_ptr);
			ioctl_ret	= ioctl(fw_log_fd, WIFI_FW_LOG_IOCTL_SET_LEVEL, ioctl_log_idx);
			printf("set log idx to %d\n", ioctl_log_idx);
		}else{
			goto ERROR_;
		}
	}else if( strstr(current_cmd, "read") != NULL ){
		/** this option permits to read continously from the logshell automatically **/

		if( ioctl_log_idx == 0 ){
			ioctl_log_idx	= 3;
		}

		/** switch the logshell on 'ON' **/
		ioctl_ret               = ioctl(fw_log_fd, WIFI_FW_LOG_IOCTL_ON_OFF, 1);

		if( ioctl_ret < 0 ){
			close(fw_log_fd);
			DIE("[!] Error failed to set to ON the fw log\n");
		}

		/** set the logshell level **/
		ioctl_ret       = ioctl(fw_log_fd, WIFI_FW_LOG_IOCTL_SET_LEVEL, ioctl_log_idx);

		/** read the data **/

		if( ioctl_ret < 0 ){
			close(fw_log_fd);
			DIE("[!] Error failed to set the logging level\n");
		}

		pfd.fd 			= fw_log_fd;
		pfd.events		= ( POLLIN | POLLRDNORM );

		while(1) {
			poll_ret= poll(&pfd, (unsigned long)1, 100);

			if( poll_ret < 0 ) {
				close(fw_log_fd);
				DIE("[!] Error poll error!\n");
			}

			if( ( pfd.revents & POLLIN )  == POLLIN ) {
				read(fw_log_fd, &fw_log, sizeof(fw_log));
				#ifdef USE_HEXDUMP
				hexdump(NULL, &fw_log, sizeof(fw_log));
				#else
				printf("%s\n", fw_log);
				#endif
				printf("\n\n");
			}
		}


	}else{
		goto ERROR_;
	}

	printf("ioctl returned %d\n", ioctl_ret);

	close(fw_log_fd);
	return 0;
	ERROR_:
		close(fw_log_fd);
		DIE("./fw_log [<on> | <off> | <set_level=X>\n");
}
