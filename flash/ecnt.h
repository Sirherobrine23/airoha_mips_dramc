/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef ECONET_ECNT_H
#define ECONET_ECNT_H

#define ECONET_BOOT_MAGIC       0x45434e54U /* "ECNT" */
#define ECONET_BOOT_VERSION     1U
#define ECONET_BOOT_HEADER_SIZE 32U
#define ECONET_BOOT_VERSION_V2  2U
#define ECONET_BOOT_HEADER_V2   48U
#define ECONET_COMP_NONE        0U
#define ECONET_COMP_GZIP        1U
#define ECONET_COMP_LZMA        2U

/* V2 retains offsets 0..28 of V1; all integers are big endian.
 * +12: stored size, +24: stored CRC32, +28: CRC32 of the whole header
 * (with +28 zeroed), +32: compression, +36: unpacked size,
 * +40: unpacked CRC32, +44: reserved, must be zero.
 */
#endif
