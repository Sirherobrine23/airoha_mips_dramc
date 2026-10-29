/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 * 
 * Airoha Secure Headers
 */
#ifndef __SECURE_HEADER_H__
#define __SECURE_HEADER_H__

enum AES_MODE {
	AES_MODE_DISABLE = 0,
	AES_MODE_ECB,
	AES_MODE_MAX
};

struct AES_INFO {
	unsigned char iv[16];
	unsigned char hmac_sha256[32];
};

struct SECURE_HEADER_V2 {
	unsigned int magic;
	unsigned int version;
	unsigned int header_len;
	unsigned int image_len;
	unsigned char sig[512];
	unsigned char rsa_pub[512];
	enum AES_MODE aes_mode;
	unsigned char aes_img_key[256];
	struct AES_INFO aes_img_key_info;
	union {
		struct AES_INFO aes_efuse_info;
		struct AES_INFO aes_bootram_info;
		struct AES_INFO aes_kernel_info;
	};
	struct AES_INFO aes_spram_info;
	struct AES_INFO aes_lzma_info;
	struct AES_INFO aes_verify_info;
	unsigned char resv3[504];
	unsigned int crc;
};

struct SECURE_HEADER_V1 {
	unsigned int magic;
	unsigned int version;
	unsigned char sig[256];
	unsigned int image_len;
	unsigned int resv2;
	unsigned int crc;
};

struct SECURE_HEADER {
	union {
		struct SECURE_HEADER_V1 v1;
		struct SECURE_HEADER_V2 v2;
	};
};

enum SIGNATURE_VERIFY_STATUS {
	SIGNATURE_VERIFY_STATUS_CORRECT = 0,
	SIGNATURE_VERIFY_INCORRECT
};

struct secure_img {
	struct SECURE_HEADER *sHeader;
	unsigned char *start;
	unsigned int len;
	unsigned int crc;
};

#endif /* __SECURE_HEADER_H__ */
