#ifndef __MT19938AR_H
	#define __MT19938AR_H

	//void init_genrand(unsigned long s);
	//void init_by_array(unsigned long init_key[], int key_length);
	void genrand_init(void);
	unsigned long genrand_int32(void);
	long genrand_int31(void);
	double genrand_real1(void);
	double genrand_real2(void);
	double genrand_real3(void);
	double genrand_res53(void);

	int genrand_between(int from, int to);
#endif
