/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <secure_boot/secure.h>

#define VERIFY_DBG_LV 1

#define VERIFY_DBG_MSG(level, fmt){}
#define VERIFY_DBG_HEX(level, val){}
#define VERIFY_DBG_SHOW_ARRAY(level, addr, len, fmt){}

#ifdef SPRAM_IMG
/*spram.img will run on IMEM/DMEM, DMEM is 4k for MT7510
*spram.img data section is about 1.2k,so the max stack size is 2.8k
*if code has been changed, pay attention to change this size
*/
#define KERNEL_STACK_SIZE 2560 
#else
#define KERNEL_STACK_SIZE 8192
#endif

extern int rsa_verify(struct secure_img*);

typedef unsigned long ulg;

unsigned char init_task_union[KERNEL_STACK_SIZE];

unsigned long kernelsp;

#define RETURN_VALUE_ADDR	(0xbfb00280)
#define	BOOTIMG_ADDR		(get_bootram_link_addr())

extern unsigned int get_bootram_link_addr(void);

void start_verify(void)
{
	int *return_val_addr = (int*)(RETURN_VALUE_ADDR);
	void (*return_val)();
	struct SECURE_HEADER *sHeader;
	enum SIGNATURE_VERIFY_STATUS rsa_verify_status;
	struct secure_img img;
	unsigned int sHdrLen;
	unsigned int imgLen;

	VERIFY_DBG_MSG(0, "verify start\n");

	/* setting return function */
	return_val = (void(*)()) (*(return_val_addr));

	/* read secure header */
	sHeader = (struct SECURE_HEADER *)BOOTIMG_ADDR;

	if(sHeader->v2.version == 2) {
		sHdrLen = sHeader->v2.header_len;
		imgLen = sHeader->v2.image_len;
	} else {
		sHdrLen = sizeof(struct SECURE_HEADER_V1);
		imgLen = sHeader->v1.image_len;
	}
	
	/* kernel image */
	img.sHeader = sHeader;
	img.start = ((unsigned char *)(img.sHeader)) + sHdrLen;
	/* reduce CRC32 length */
	img.len = imgLen - 4;

	VERIFY_DBG_SHOW_ARRAY(0, BOOTIMG_ADDR, 0x120, "sHeader & boot.img");

	VERIFY_DBG_MSG(0, "verify_img_start:0x");
	VERIFY_DBG_HEX(0, img.start);
	VERIFY_DBG_MSG(0, "\n");
	VERIFY_DBG_MSG(0, "verify_img_len:0x");
	VERIFY_DBG_HEX(0, img.len);
	VERIFY_DBG_MSG(0, "\n");

	/* Verify kernel image */
	rsa_verify_status = rsa_verify(&img);
	if(rsa_verify_status == SIGNATURE_VERIFY_INCORRECT) {
		VERIFY_DBG_MSG(1, "image verify fail\n");
		*return_val_addr = -1;
	} else {
		VERIFY_DBG_MSG(0, "image verify success\n");
		*return_val_addr = 0;
	}

	(*return_val)();
} /* end of start_verify() */

