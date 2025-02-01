/*
Copyright 2024-2025 Lukas Tautz

This file is part of cpu_logger.

cpu_logger is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifdef __dietlibc__
#define _GNU_SOURCE /* for u_intN_t */
#define __bitwise /**/
#endif

#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>

typedef u_int8_t   uint8;
typedef u_int16_t  uint16;
typedef u_int32_t  uint32;
typedef u_int64_t  uint64;
typedef int8_t     int8;
typedef int16_t    int16;
typedef int32_t    int32;
typedef int64_t    int64;
typedef u_int8_t   bool;

#include "config.h"

#ifdef DEBUG
// it's usually not needed in production, and as the goal is to be as small as possible, it's disabled by default
#define PTR_IS_IN_BUF(ptr, buf) (ptr < (buf + sizeof(buf)))
#else
#define PTR_IS_IN_BUF(ptr, buf) true
#endif

#define true    1
#define false   0

#define STDOUT 1
#define STDERR 2

typedef struct __attribute__((__packed__)) measured_load_s { // it has to be packed so that it is predictable and not dependant on the used compiler
    uint32 time; // seconds since 1.1.2020 (UTC)
#ifdef FEATURE_LOAD
    uint8 cpu_load_percent : 7; // 
    uint8 cpu_load_percent_after_dot : 7;
#endif
#ifdef FEATURE_STEAL
    uint8 cpu_steal_percent : 7;
    uint8 cpu_steal_percent_after_dot : 7;
#endif
#ifdef FEATURE_IOWAIT
    uint8 iowait_percent : 7;
    uint8 iowait_percent_after_dot : 7;
#endif
#ifdef FEATURE_MEMORY
    uint8 memory_usage_percent : 7;
    uint8 memory_usage_percent_after_dot : 7;
#endif
} measured_load; // 4 + round_up(1,75 * features_count) bytes

uint8 itoa_fill(uint32 n, char *dest, uint8 fill_to);
uint8 itoa(uint32 n, char *s);

#define WRITE_STDERR(s) \
    write(STDERR, s, strlen(s))

// html+avg
#define _PRINT_PERCENT(name) \
    write(write_to, buf, itoa((uint8)name, buf)); \
    write(write_to, ".", strlen(".")); \
    write(write_to, buf, itoa_fill((uint32)(name * 100000) - (((uint8)name) * 100000), buf, 5)); \
    write(write_to, "%", strlen("%"));

#define _ADD_AVG_MAX(name) \
    tmp = (double)load.name##_percent + ((double)load.name##_percent_after_dot / 100.0); \
    name##_percent += tmp; \
    name##_max_percent = tmp > name##_max_percent ? tmp : name##_max_percent;

#define WRITE(s) \
    write(write_to, s, strlen(s))
