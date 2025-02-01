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

#include "include.h"

#if defined(FEATURE_LOAD) || defined(FEATURE_STEAL) // cpu

typedef struct jiffies_spent_s {
    uint64 user;
    uint64 nice;
    uint64 system;
    uint64 idle;
    uint64 iowait;
    uint64 irq;
    uint64 softirq;
    uint64 steal;
#ifdef MEASURE_GUEST_TIME
    uint64 guest;
    uint64 guest_nice;
#endif
    uint64 work; /* = user+nice+system */
    uint64 total;
} jiffies_spent; // 72 or 88 bytes

jiffies_spent jiffies_start, diff;
char buf[256];

void get_current_jiffies(jiffies_spent *dest) {
    uint64 tmp;
    char *ptr = buf;
    dest->total = 0;
    int fd = open("/proc/stat", O_RDONLY);
    if (fd == -1 || read(fd, buf, sizeof(buf)) < 128)
        exit(9);
    close(fd);
    ptr += 3; // skip "cpu"
    while (isspace(*++ptr) && PTR_IS_IN_BUF(ptr, buf));
    if (!PTR_IS_IN_BUF(ptr, buf))
        exit(10);
#ifndef MEASURE_GUEST_TIME
    for (uint8 i = 0; i < 8; ++i) {
#else
    for (uint8 i = 0; i < 10; ++i) {
#endif
        tmp = atol(ptr);
        dest->total += tmp;
        memcpy((uint64 *)((uint64)dest + (uint64)(8 * i)), &tmp, 8);
        while (!isspace(*++ptr) && PTR_IS_IN_BUF(ptr, buf));
        if (!PTR_IS_IN_BUF(ptr, buf))
            exit(10);
        while (isspace(*++ptr) && PTR_IS_IN_BUF(ptr, buf));
        if (!PTR_IS_IN_BUF(ptr, buf))
            exit(10);
    }
#ifndef MEASURE_GUEST_TIME
    dest->work = dest->user + dest->nice + dest->system;
#else
    dest->work = dest->user + dest->nice + dest->system + dest->guest + dest->guest_nice;
#endif
}

void calculate_jiffies_diff(void) {
#ifdef FEATURE_LOAD
    diff.work -= jiffies_start.work;
#endif
#ifdef FEATURE_STEAL
    diff.steal -= jiffies_start.steal;
#endif
    diff.total -= jiffies_start.total;
}

#endif  // cpu

measured_load load;

void measure_load(uint16 sleep_seconds) {
    double integer;
#if defined(FEATURE_LOAD) || defined(FEATURE_STEAL)
    get_current_jiffies(&jiffies_start);
    load.time = (uint32)(((uint32)((uint64)time(NULL) - 1577836800 /* 1.1.2020 */)) + (uint32)(sleep_seconds / 2));
#endif
    sleep(sleep_seconds);
#if defined(FEATURE_LOAD) || defined(FEATURE_STEAL)
    get_current_jiffies(&diff);
    calculate_jiffies_diff();
#ifdef FEATURE_LOAD
    double cpu_load_percent = ((double)diff.work / (double)diff.total) * 100.0;
    load.cpu_load_percent = (uint8)cpu_load_percent;
    load.cpu_load_percent_after_dot = (uint16)((cpu_load_percent - (double)load.cpu_load_percent) * 100.0);
#endif
#ifdef FEATURE_STEAL
    double cpu_steal_percent = ((double)diff.steal / (double)diff.total) * 100.0;
    load.cpu_steal_percent = (uint8)cpu_steal_percent;
    load.cpu_steal_percent_after_dot = (uint16)((cpu_steal_percent - (double)load.cpu_steal_percent) * 100.0);
#endif
#ifdef FEATURE_IOWAIT
    double iowait_percent = ((double)diff.iowait / (double)diff.total) * 100.0;
    load.iowait_percent = (uint8)iowait_percent;
    load.iowait_percent_after_dot = (uint16)((iowait_percent - (double)load.iowait_percent) * 100.0);
#endif
#endif // cpu logging
#ifdef FEATURE_MEMORY
    char *ptr = buf + strlen("MemTotal:");
    int fd = open("/proc/meminfo", O_RDONLY);
    if (fd == -1 || read(fd, buf, sizeof(buf)) < 128)
        exit(9);
    close(fd);
    while (isspace(*++ptr) && PTR_IS_IN_BUF(ptr, buf));
    uint32 total_memory = atol(ptr); // will overflow if more than 4 TiB of memory is installed (as it is in KiB)
    while (*++ptr != 'F' && PTR_IS_IN_BUF(ptr, buf)); // Free
    while (*++ptr != 'A' && PTR_IS_IN_BUF(ptr, buf)); // Available
    ptr += strlen("vailable ");
    while (isspace(*++ptr) && PTR_IS_IN_BUF(ptr, buf));
    double memory_usage_percent = ((double)(total_memory - atol(ptr) /* total memory - available memory = used memory */) / total_memory) * 100.0;
    load.memory_usage_percent = (uint8)memory_usage_percent;
    load.memory_usage_percent_after_dot = (uint16)((memory_usage_percent - (double)load.memory_usage_percent) * 100.0);
#endif
}

bool always_true_sleep(int time) {
    sleep(time);
    return true;
}

// return values:
    // 1: open() failed
    // 2: writing failed
    // 3: did not write all at first, and the second attempt did not succeed
    // 9: reading /proc/stat or /proc/meminfo failed
    // 10: debug: memory out of buf
    // 99: can't happen

int main(int argc, char **argv) {
    char *save_to;
    uint16 sleep_seconds;
    if (argc != 3 || !memcmp(argv[1], "-h", sizeof("-h")) || !memcmp(argv[1], "--help", sizeof("--help")) || !(sleep_seconds = atoi(argv[2]))) {
        WRITE_STDERR("Usage: ");
        WRITE_STDERR(argv[0]);
        WRITE_STDERR(" FILE INTERVAL\nThis program saves the measured load (as configured) every INTERVAL seconds to FILE in a binary format.\n");
        exit(1);
    }
    int fd = open(save_to = argv[1], O_APPEND | O_CREAT | O_WRONLY, 0644), write_return;
    if (fd == -1)
        return 1;
    for (;;) {
        measure_load(sleep_seconds);
        if ((write_return = write(fd, &load, sizeof(load))) == -1)
            return 2;
        else if (write_return < sizeof(load) && always_true_sleep(5) && write(fd, &load + write_return, sizeof(load) - write_return) != (sizeof(load) - write_return))
            return 3;
    }
    return 99;
}
