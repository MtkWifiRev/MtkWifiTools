// patch at 0xe0271db8
__attribute__((aligned(4))) int _start(int param_1){
        void (*ptr)(char *str) = (void *)0x9218a6;
	int  (*ptr1)(int param_1, int c) = (void *)0x921000;
	ptr1(0x25, 1);
        ptr(0x2021f38);
        ptr1(0x25, 1);
	return 0;
}

