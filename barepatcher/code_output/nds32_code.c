// patch at 0xe0271db8
__attribute__((aligned(4))) int _start(int param_1){
        void (*ptr)(char *str) = (void *)0x92b0a0;
	void (*ptr1)(int perm) = (void *)0x92f272;
	ptr1(4);
	ptr((char *)0x2021bd0);
	ptr((char *)0x2021bd0);
	ptr((char *)0x2021bd0);
	ptr1(4);
	ptr((char *)0x2021bd0);
	return 0;
}

