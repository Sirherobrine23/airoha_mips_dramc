/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef ECONET_SFC_H
#define ECONET_SFC_H

#include <stddef.h>
#include <stdint.h>

#define EIO 5
#define EINVAL 22
#define EOPNOTSUPP 95

int econet_sfc_init(void);
int econet_sfc_read(uint32_t offset, void *dst, size_t len);
int econet_sfc_erase(uint32_t offset, size_t len);
int econet_sfc_write(uint32_t offset, const void *src, size_t len);
int econet_sfc_verify(uint32_t offset, const void *src, size_t len);

#endif
