#pragma once

#define MG_ARCH MG_ARCH_FREERTOS

#define MG_ENABLE_LWIP 1

#define MG_ENABLE_FATFS 1
#define MG_ENABLE_PACKED_FS 1  // Enable "embedded", or packed, filesystem
#define MG_ENABLE_POSIX_FS 0   // Disable POSIX filesystem

#define MG_MAX_RECV_SIZE (256 * 1024)