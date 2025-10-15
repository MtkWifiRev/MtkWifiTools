// to patch at 00071d0c

#include "inc/include.h"

__attribute__((nacked)) __attribute__((aligned(4))) void _start(void *ptr){
	//void (*addr)(void) = (void *)0x00805e56;
	//addr();

	uint32_t *src 		= (uint32_t *)0x068400fc; // last addr: 0x008433c8;
	uint32_t *dst 		= (uint32_t *)0xe020c338;
	uint16_t  sl 		= 250;
	for(unsigned char i = 0; i < sl; i++){
		//__asm__ volatile("" ::: "memory");
		dst[i] 	= src[i];
		//__asm__ volatile("" ::: "memory");
	}
}
// then dump memory at '0x7f82e'
